# Test plan and evaluation

## Stage 1 — sensor commissioning
1. Upload firmware with LED supply disconnected.
2. Open Serial Monitor at 115200 baud.
3. Confirm LDR value changes when covered/uncovered.
4. Confirm each PIR input changes after its warm-up period.
5. Tune PIR sensitivity/time knobs and the LDR threshold.

## Stage 2 — PWM test
Test one low-current LED load first. Confirm that the three PWM channels can independently reach 20%, 50%, and 100%. Verify MOSFET temperature and wiring.

## Stage 3 — zone and direction test
Perform at least 10 controlled passes:
- Zone 1 → Zone 2 → Zone 3
- Zone 3 → Zone 2 → Zone 1
Record whether the estimated direction was correct. Report successes and failures rather than selecting only successful trials.

## Stage 4 — matched energy comparison
Compare two control modes under the same scripted sensor activity and the same test duration:
- Baseline: night-time lights remain at a defined full-brightness level (or use a clearly documented conventional policy).
- Proposed: adaptive zone-specific PWM and directional neighbour boost.

For each run, record duration, voltage, current, power, and energy. If INA219 integration is not implemented/calibrated, measure energy using an appropriate external DC power meter or report PWM duty cycle only; do not label PWM duty as measured energy.

Energy saved (%) = (baseline energy − proposed energy) / baseline energy × 100.

Repeat each scenario at least three times. Keep the test pattern and duration equal. Include sensor limitations and uncertainty in the report.

## Suggested results tables

### Direction estimation
| Trial | Actual path | Estimated path | Correct? |
|---|---|---|---|
| 1 | 1→2→3 | | |
| 2 | 3→2→1 | | |

### Energy
| Run | Mode | Duration (s) | Mean voltage (V) | Mean current (A) | Energy (Wh) |
|---|---|---:|---:|---:|---:|
| 1 | Baseline | | | | |
| 1 | Adaptive | | | | |

### Brightness response
| Timestamp | PIR1 events | PIR2 events | PIR3 events | PWM1 (%) | PWM2 (%) | PWM3 (%) |
|---:|---:|---:|---:|---:|---:|---:|
