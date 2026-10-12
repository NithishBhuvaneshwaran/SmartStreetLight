# Bill of materials

## Essential
- 1 × ESP32 DevKit
- 3 × HC-SR501 PIR modules
- 1 × LDR
- 1–2 × 10 kΩ resistors for the LDR divider (one required for the shown circuit)
- 3 × white LED loads or 12 V LED strip sections
- 3 × logic-level N-channel MOSFETs suitable for 3.3 V gate drive and the load current
- 3 × 220 Ω gate resistors
- 3 × 10 kΩ gate pulldown resistors
- 1 × regulated 12 V DC supply sized for the LED loads
- 1–2 × breadboards for signal circuits only
- Male-male and male-female jumper wires
- 1 × USB data cable

## Recommended
- 1 × INA219 current/voltage sensor breakout (verify current rating)
- Fuse and holder for 12 V LED supply
- Terminal blocks / screw connectors for LED power wiring
- Cardboard/foam board and poles for the road model
- Optional OLED display

## Notes
- A bare LED requires a current-limiting resistor. Do not connect a bare LED directly to 12 V.
- For a 12 V LED strip, check its rated voltage/current.
- IRLZ44N and FQP30N06L are examples often used in low-voltage switching, but verify the datasheet and ensure acceptable on-resistance at a 3.3 V gate voltage. A MOSFET that only specifies low resistance at 10 V may not perform well at 3.3 V.
- Do not run LED-strip current through a solderless breadboard.
