# Wiring guide

## ESP32 pin map

| Function | ESP32 pin |
|---|---|
| PIR Zone 1 OUT | GPIO 25 |
| PIR Zone 2 OUT | GPIO 26 |
| PIR Zone 3 OUT | GPIO 27 |
| LDR divider midpoint | GPIO 34 (ADC input only) |
| LED 1 MOSFET gate | GPIO 18 (PWM) |
| LED 2 MOSFET gate | GPIO 19 (PWM) |
| LED 3 MOSFET gate | GPIO 21 (PWM) |
| INA219 SDA | GPIO 21 by default I2C — **conflicts with LED 3** |
| INA219 SCL | GPIO 22 |

**Important pin conflict resolution:** To use INA219, use GPIO 23 for LED 3 instead of GPIO 21, or change I2C pins in firmware. The firmware config below uses LED pins 18, 19, and 23 and I2C SDA=21/SCL=22.

## LDR voltage divider

Wire as follows:

```text
3.3 V ── LDR ──┬── GPIO 34
               |
              10 kΩ
               |
              GND
```

With this arrangement, ADC value generally rises with light. Exact readings vary by LDR and resistor. Calibrate the threshold in firmware. If your divider is reversed, the direction of the reading change will also reverse.

## PIR sensors

For each HC-SR501:
- VCC → board-appropriate supply (commonly 5 V; check your module)
- GND → ESP32 GND
- OUT → GPIO 25 / 26 / 27 respectively

Do not feed a sensor output above the ESP32's 3.3 V GPIO limit. Check the actual module output before connecting.

## Each LED channel (12 V LED strip/load)

Use a logic-level N-channel MOSFET that is fully enhanced at a 3.3 V gate drive, such as a suitable logic-level part (verify its datasheet).

```text
12 V positive ───────── LED strip +
LED strip - ──────────── MOSFET DRAIN
MOSFET SOURCE ────────── 12 V negative / GND
ESP32 PWM GPIO ── 220 Ω ── MOSFET GATE
MOSFET GATE ───── 10 kΩ ── GND
```

Repeat for all three zones. Connect the ESP32 GND to the 12 V supply negative. The LED current must flow through the MOSFET and LED wiring, not through the ESP32 or breadboard rails. Use appropriately rated wiring and a suitable supply. A fuse on the 12 V LED supply is recommended.

**MOSFET pinouts differ.** Verify Gate/Drain/Source against the exact component datasheet; never rely only on the package shape.

## INA219 (optional)

Use an INA219 module rated for the bus voltage/current in your setup. Connect:
- VIN+ → 12 V supply positive
- VIN− → LED supply positive going to the LED load(s)
- VCC → 3.3 V (check breakout board)
- GND → common GND
- SDA → GPIO 21
- SCL → GPIO 22

If measuring all three lights together, place the sensor upstream of the three LED branches. Ensure the module and breakout are rated for the load current. Do not put the INA219 in the ground return path for this wiring arrangement.

## Power and safety checklist
- No 230 V mains wiring.
- USB powers the ESP32; 12 V powers the LEDs.
- Common ground between ESP32 and 12 V supply.
- Never connect 12 V to an ESP32 GPIO.
- Check MOSFET current and thermal ratings for the chosen LED strips.
- Avoid powering high-current LED strips through a breadboard.
