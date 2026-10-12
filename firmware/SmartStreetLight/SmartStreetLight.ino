/*
  Direction-Aware Adaptive Smart Street Lighting System
  Board: ESP32 DevKit (Arduino framework)

  Telemetry CSV:
  ms,dark,pir1,pir2,pir3,count1,count2,count3,dir,pwm1,pwm2,pwm3,voltage_V,current_mA,power_mW,mode

  Set USE_INA219 to false if no INA219 is connected.
*/
#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#define USE_INA219 true
#if USE_INA219
  #include <Adafruit_INA219.h>
  Adafruit_INA219 ina219;
  bool inaReady = false;
#endif

// Pin map
const uint8_t PIR_PINS[3] = {25, 26, 27};
const uint8_t LDR_PIN = 34;
const uint8_t LED_PINS[3] = {18, 19, 23}; // GPIO 23 avoids INA219 SDA pin 21
const uint8_t I2C_SDA_PIN = 21;
const uint8_t I2C_SCL_PIN = 22;

// Tuning parameters
const int LDR_DARK_THRESHOLD = 1800;   // Calibrate from live ADC readings
const bool LDR_DARK_WHEN_BELOW = true; // Flip if your divider behaves oppositely
const uint8_t BASE_BRIGHTNESS_PERCENT = 20;
const uint8_t MAX_EVENTS_FOR_FULL_BRIGHTNESS = 8;
const unsigned long WINDOW_MS = 30000UL;
const unsigned long DIRECTION_WINDOW_MS = 8000UL;
const unsigned long DIRECTION_HOLD_MS = 5000UL;
const unsigned long TELEMETRY_MS = 500UL;
const float SMOOTHING_ALPHA = 0.18f;

// Set true only for a clearly documented fixed-output comparison mode.
const bool BASELINE_MODE = false;

#if defined(ESP32)
const uint32_t PWM_FREQ = 5000;
const uint8_t PWM_RESOLUTION = 8;
#endif

struct Event {
  uint8_t zone;
  unsigned long when;
};
const uint8_t MAX_STORED_EVENTS = 90;
Event events[MAX_STORED_EVENTS];
uint8_t eventCount = 0;

bool lastPirState[3] = {false, false, false};
unsigned long lastZoneEventMs[3] = {0, 0, 0};
unsigned long lastDirectionMs = 0;
String directionLabel = "NONE";
int directionFrom = -1;
int directionTo = -1;
float currentPercent[3] = {0, 0, 0};
float targetPercent[3] = {0, 0, 0};
unsigned long lastTelemetry = 0;
unsigned long lastEventMs = 0;

void writePwm(uint8_t channel, uint8_t percent) {
  uint32_t maxDuty = (1U << PWM_RESOLUTION) - 1U;
  uint32_t duty = (uint32_t)roundf((percent / 100.0f) * maxDuty);
  ledcWrite(LED_PINS[channel], duty);
}

void setAllOff() {
  for (int i = 0; i < 3; i++) {
    currentPercent[i] = 0;
    targetPercent[i] = 0;
    writePwm(i, 0);
  }
}

bool isDark(int adc) {
  return LDR_DARK_WHEN_BELOW ? (adc < LDR_DARK_THRESHOLD) : (adc > LDR_DARK_THRESHOLD);
}

void addEvent(uint8_t zone, unsigned long now) {
  if (eventCount < MAX_STORED_EVENTS) {
    events[eventCount++] = {zone, now};
  } else {
    // Shift oldest entry out when full.
    for (int i = 1; i < MAX_STORED_EVENTS; i++) events[i - 1] = events[i];
    events[MAX_STORED_EVENTS - 1] = {zone, now};
  }
  lastZoneEventMs[zone] = now;
  lastEventMs = now;

  // Direction inference from the sequence of zones. Same-zone repeats do not define direction.
  static int previousZone = -1;
  static unsigned long previousZoneMs = 0;
  if (previousZone >= 0 && zone != previousZone &&
      (now - previousZoneMs) <= DIRECTION_WINDOW_MS) {
    directionFrom = previousZone;
    directionTo = zone;
    if (previousZone == 0 && zone == 1) directionLabel = "1_TO_2";
    else if (previousZone == 1 && zone == 2) directionLabel = "2_TO_3";
    else if (previousZone == 2 && zone == 1) directionLabel = "3_TO_2";
    else if (previousZone == 1 && zone == 0) directionLabel = "2_TO_1";
    else if (previousZone == 0 && zone == 2) directionLabel = "1_TO_3";
    else if (previousZone == 2 && zone == 0) directionLabel = "3_TO_1";
    lastDirectionMs = now;
  }
  previousZone = zone;
  previousZoneMs = now;
}

void pruneEvents(unsigned long now) {
  uint8_t kept = 0;
  for (uint8_t i = 0; i < eventCount; i++) {
    if ((now - events[i].when) <= WINDOW_MS) events[kept++] = events[i];
  }
  eventCount = kept;
}

void countByZone(int counts[3]) {
  counts[0] = counts[1] = counts[2] = 0;
  for (uint8_t i = 0; i < eventCount; i++) {
    if (events[i].zone < 3) counts[events[i].zone]++;
  }
}

void updateTargets(bool dark, const int counts[3], unsigned long now) {
  if (!dark) {
    for (int i = 0; i < 3; i++) targetPercent[i] = 0;
    return;
  }

  for (int i = 0; i < 3; i++) {
    if (BASELINE_MODE) {
      targetPercent[i] = 100.0f;
    } else {
      float activity = (float)counts[i] / (float)MAX_EVENTS_FOR_FULL_BRIGHTNESS;
      if (activity > 1.0f) activity = 1.0f;
      targetPercent[i] = BASE_BRIGHTNESS_PERCENT +
        activity * (100.0f - BASE_BRIGHTNESS_PERCENT);
      // Keep the recently active zone illuminated even if its event count is low.
      if ((now - lastZoneEventMs[i]) < 2500UL &&
          targetPercent[i] < 75.0f) targetPercent[i] = 75.0f;
    }
  }

  // Predictive neighbour illumination: brighten the next zone briefly after a
  // sequential-zone transition. This is a heuristic, not guaranteed prediction.
  if (!BASELINE_MODE && directionFrom >= 0 && directionTo >= 0 &&
      (now - lastDirectionMs) < DIRECTION_HOLD_MS) {
    int neighbour = directionTo;
    if (targetPercent[neighbour] < 65.0f) targetPercent[neighbour] = 65.0f;
  }
}

void smoothAndWrite(bool dark) {
  for (int i = 0; i < 3; i++) {
    float target = targetPercent[i];
    currentPercent[i] += SMOOTHING_ALPHA * (target - currentPercent[i]);
    if (!dark && currentPercent[i] < 1.0f) currentPercent[i] = 0.0f;
    int out = (int)roundf(currentPercent[i]);
    if (out < 0) out = 0;
    if (out > 100) out = 100;
    writePwm(i, (uint8_t)out);
  }
}

void printTelemetry(unsigned long now, bool dark, const bool pir[3],
                    const int counts[3], int ldrValue) {
  float voltage = NAN, currentmA = NAN, powermW = NAN;
#if USE_INA219
  if (inaReady) {
    voltage = ina219.getBusVoltage_V();
    currentmA = ina219.getCurrent_mA();
    powermW = ina219.getPower_mW();
  }
#endif
  Serial.printf("%lu,%d,%d,%d,%d,%d,%d,%d,%s,%d,%d,%d,",
    now, dark ? 1 : 0, pir[0] ? 1 : 0, pir[1] ? 1 : 0, pir[2] ? 1 : 0,
    counts[0], counts[1], counts[2], directionLabel.c_str(),
    (int)roundf(currentPercent[0]), (int)roundf(currentPercent[1]),
    (int)roundf(currentPercent[2]));
  if (isnan(voltage)) Serial.print("nan,nan,nan,");
  else Serial.printf("%.3f,%.3f,%.3f,", voltage, currentmA, powermW);
  Serial.println(BASELINE_MODE ? "BASELINE" : "ADAPTIVE");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  analogReadResolution(12);
  pinMode(LDR_PIN, INPUT);
  for (int i = 0; i < 3; i++) {
    pinMode(PIR_PINS[i], INPUT);
    pinMode(LED_PINS[i], OUTPUT);
    currentPercent[i] = 0;
    targetPercent[i] = 0;
    // ESP32 Arduino core 3.x LEDC API uses pin-based ledcAttach/ledcWrite.
    if (!ledcAttach(LED_PINS[i], PWM_FREQ, PWM_RESOLUTION)) {
      Serial.printf("WARNING: PWM attach failed for GPIO %u\n", LED_PINS[i]);
    }
    writePwm(i, 0);
  }

#if USE_INA219
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  inaReady = ina219.begin();
  if (!inaReady) Serial.println("# INA219 not detected; voltage/current/power will be nan");
  else Serial.println("# INA219 detected");
#endif

  Serial.println("# Smart Street Light ready; allow PIR sensors to warm up");
  Serial.println("ms,dark,pir1,pir2,pir3,count1,count2,count3,dir,pwm1,pwm2,pwm3,voltage_V,current_mA,power_mW,mode");
}

void loop() {
  unsigned long now = millis();
  int ldrValue = analogRead(LDR_PIN);
  bool dark = isDark(ldrValue);
  bool pir[3];

  for (int i = 0; i < 3; i++) {
    pir[i] = digitalRead(PIR_PINS[i]) == HIGH;
    // Rising edge detection avoids counting every loop while PIR remains HIGH.
    if (pir[i] && !lastPirState[i]) addEvent(i, now);
    lastPirState[i] = pir[i];
  }

  pruneEvents(now);
  int counts[3];
  countByZone(counts);

  if (!dark) {
    directionLabel = "NONE";
    directionFrom = directionTo = -1;
    setAllOff();
  } else {
    if (directionFrom >= 0 && (now - lastDirectionMs) >= DIRECTION_HOLD_MS) {
      directionLabel = "NONE";
      directionFrom = directionTo = -1;
    }
    updateTargets(dark, counts, now);
    smoothAndWrite(dark);
  }

  if (now - lastTelemetry >= TELEMETRY_MS) {
    lastTelemetry = now;
    printTelemetry(now, dark, pir, counts, ldrValue);
    // LDR calibration helper goes to a comment-prefixed line, ignored by CSV logger.
    Serial.printf("# LDR_ADC=%d threshold=%d\n", ldrValue, LDR_DARK_THRESHOLD);
  }
  delay(10);
}
