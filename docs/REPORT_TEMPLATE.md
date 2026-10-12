# Direction-Aware Adaptive Smart Street Lighting System

## Abstract
[Summarize the problem, approach, prototype, test method, and measured results. Insert only measured results.]

## 1. Problem statement
Conventional fixed-output lighting consumes energy when full output is not needed. Simple motion-triggered systems often treat a zone as binary ON/OFF and may not account for local activity patterns or movement between zones.

## 2. Objective
Develop a low-cost three-zone prototype that estimates local activity from PIR event frequency, infers coarse movement direction from sensor sequence, and adjusts zone-specific PWM brightness with temporary neighbouring-zone illumination.

## 3. Existing system and limitation
[Describe fixed lighting and basic motion-triggered lighting. Cite sources you actually consulted.]

## 4. Proposed system
[Insert block diagram and explain LDR gating, rolling window, activity index, direction heuristic, PWM control, and dashboard.]

## 5. Hardware
[ESP32, PIR sensors, LDR divider, MOSFET drivers, LED loads, optional INA219.]

## 6. Software and algorithm
[Include flowchart, event-count formula, direction state machine, and smoothing.]

## 7. Experimental setup
[Road model dimensions, sensor spacing, LED load ratings, supply, test duration, traffic simulation method.]

## 8. Results
[Insert measured tables and graphs. Distinguish measured power from calculated estimates.]

## 9. Discussion and limitations
[Discuss PIR hold times, false triggers, limited direction resolution, test repeatability, and measurement uncertainty.]

## 10. Conclusion and future work
[Summarize what was demonstrated. Future extensions may include better sensors, more zones, ambient-light calibration, and field testing.]
