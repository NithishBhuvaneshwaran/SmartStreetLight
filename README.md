# Direction-Aware Adaptive Smart Street Lighting System

A one-week ESP32 prototype with three PIR sensing zones, an LDR day/night gate, three PWM-controlled 12 V LED loads, and optional INA219 power monitoring.

## What this prototype demonstrates

1. Day/night gating using an LDR.
2. Activity estimation from PIR events in a rolling time window.
3. Sequential-zone movement estimation (Zone 1 → 2 → 3 or reverse).
4. Zone-specific PWM brightness with temporary illumination of the next zone.
5. Serial telemetry for a Python dashboard.
6. Optional INA219 voltage/current/power readings.

**Important scientific limitation:** PIR trigger frequency is only a proxy for local activity, not a direct measurement of vehicle count or traffic density. Direction estimation is heuristic and works only when movement produces a detectable sequence across the sensors. HC-SR501 sensors have hold/retrigger delays and are not ideal for fast direction tracking.

## Folder contents

- `firmware/SmartStreetLight.ino` — ESP32 Arduino firmware.
- `dashboard/dashboard.py` — Python serial dashboard and CSV logger.
- `dashboard/requirements.txt` — Python dependencies.
- `docs/WIRING.md` — pin map and wiring instructions.
- `docs/ALGORITHM.md` — control algorithm and novelty statement.
- `docs/TEST_PLAN.md` — commissioning, experiments, and data collection.
- `docs/REPORT_TEMPLATE.md` — project report outline.
- `docs/COMPONENTS.md` — bill of materials and safety notes.

## Quick start

### 1. Assemble safely
Use a regulated 12 V DC supply for the LED loads. Do not connect any mains voltage. The ESP32 is powered by USB. **Connect the 12 V supply ground to ESP32 GND** so the MOSFET gate signals have a common reference.

### 2. Arduino IDE
Install the ESP32 board support package and select your ESP32 Dev Module. For INA219 support, install **Adafruit INA219** and **Adafruit BusIO** libraries. If you do not have an INA219, set `USE_INA219` to `false` in the firmware.

Open `firmware/SmartStreetLight.ino`, select the correct port, and upload.

### 3. First power-up
Before connecting the 12 V LED supply:
- Check every MOSFET pinout from its datasheet; packages are not all the same.
- Check for shorts and confirm ESP32 GPIOs never receive 12 V.
- Test sensors with USB power and Serial Monitor at 115200 baud.
- Use a low-current LED/load first.
- HC-SR501 modules are commonly powered from 5 V, but check your board. Their OUT signal is usually 3.3 V compatible; verify your specific module.

### 4. Python dashboard
Install Python 3.10+ and run:

```bash
cd dashboard
python -m pip install -r requirements.txt
python dashboard.py
```

Choose the serial port when prompted (or pass it as an argument, e.g. `python dashboard.py COM5` or `python dashboard.py /dev/ttyUSB0`). Close Arduino Serial Monitor first because only one program can normally own the port.

The dashboard creates `streetlight_log.csv` in the current working directory. It accepts CSV telemetry emitted by the firmware.

## Firmware configuration
At the top of `SmartStreetLight.ino`:
- `USE_INA219`: set false if the sensor is unavailable.
- `LDR_DARK_THRESHOLD`: tune using the Serial Monitor values for your LDR divider.
- `BASE_BRIGHTNESS_PERCENT`: minimum night brightness.
- `WINDOW_MS`: rolling activity window.
- `DIRECTION_HOLD_MS`: duration of directional neighbour illumination.

## Expected telemetry
The ESP32 prints a CSV header at startup and then rows like:

`ms,dark,pir1,pir2,pir3,count1,count2,count3,dir,pwm1,pwm2,pwm3,voltage_V,current_mA,power_mW,mode`

Direction values: `NONE`, `1_TO_2`, `2_TO_3`, `3_TO_2`, `2_TO_1`, `1_TO_3`, `3_TO_1`.

`mode` is `ADAPTIVE`. The conventional baseline can be tested by setting `BASELINE_MODE` to true in the firmware; see the test plan.

## Novelty claim to present honestly
The prototype combines rolling-window activity estimates with zone-specific PWM and sequence-based movement estimation to pre-brighten a neighbouring zone. The combination and its measured behaviour are the project's implementation contribution; do not claim that PIR/PWM smart lighting itself is new or that the prototype is a production-ready traffic detector.

## Troubleshooting
- **No sensor events:** wait 30–60 seconds after powering HC-SR501; adjust its sensitivity/time knobs and verify VCC/GND/OUT.
- **Lights never go off in daytime:** calibrate `LDR_DARK_THRESHOLD` using reported ADC values.
- **Direction is unreliable:** increase the separation between zones, tune PIRs, slow the test movement, and use controlled passes. PIR sensors can retrigger/hold their output for several seconds.
- **INA219 unavailable:** firmware should continue without it if `USE_INA219` is false; telemetry uses `nan` readings.
- **Dashboard says no data:** select the right port, close Serial Monitor, and check baud rate 115200.
