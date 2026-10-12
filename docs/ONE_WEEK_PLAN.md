# One-week build schedule

## Day 1 — Parts and sensor checks
- Verify board, sensor supply requirements, MOSFET pinouts, and LED current.
- Wire LDR and one PIR only.
- Print ADC and PIR values to Serial Monitor.

## Day 2 — One LED channel
- Test one MOSFET with a low-current load.
- Verify 0/20/50/80/100% PWM.
- Add the other two channels only after the first is reliable.

## Day 3 — Activity counting
- Confirm rising-edge counting and 30-second rolling counts.
- Tune LDR threshold and PIR knobs.
- Record known limitations of the PIR modules.

## Day 4 — Direction and neighbour boost
- Run slow passes through zones in both directions.
- Adjust `DIRECTION_WINDOW_MS` and sensor spacing.
- Record all trials, including incorrect direction estimates.

## Day 5 — Power measurement
- Add INA219 only after the lights are stable.
- Confirm the sensor's voltage/current range.
- If readings are unavailable, finish core control rather than delaying the project.

## Day 6 — Model and dashboard
- Build the road model and label zones.
- Run the Python logger and confirm CSV output.
- Create graphs from recorded data.

## Day 7 — Evaluation and presentation
- Run matched baseline/adaptive tests at equal duration.
- Calculate energy saving only from measured energy.
- Finish report, wiring diagram, limitations, and demo script.
