# Algorithm and novelty

## Processing pipeline

1. Read LDR ADC value and decide whether it is dark.
2. In daylight, turn all LED channels off and clear active lighting targets.
3. At night, detect rising edges from each PIR output.
4. Store timestamps for each zone and count events in a rolling window.
5. Estimate zone activity from recent event counts, normalized to a configured maximum.
6. Estimate movement direction when sequential zone events occur within a plausible time interval.
7. Set each zone's target PWM from its local activity score and baseline brightness.
8. If direction is estimated, temporarily boost the next zone (predictive neighbour illumination).
9. Smooth PWM changes to avoid abrupt brightness jumps.
10. Emit CSV telemetry for plotting and energy comparison.

## Brightness model

For each zone:

`target_percent = base_percent + activity_fraction * (max_percent - base_percent)`

The activity fraction is `min(event_count / MAX_EVENTS_FOR_FULL_BRIGHTNESS, 1.0)`. A direction-based neighbour boost is then applied, capped at 100%.

This is a prototype control model, not a calibrated traffic-density estimator.

## Direction heuristic

The firmware records the last zone event. A transition such as Zone 1 followed by Zone 2 inside the direction window estimates `1_TO_2`; Zone 2 followed by Zone 3 estimates `2_TO_3`. The reverse sequence estimates the reverse direction. Because HC-SR501 outputs can remain HIGH for a configurable hold time, the system cannot reliably distinguish every closely spaced movement. Direction should be demonstrated using slow, controlled passes.

## Novelty statement

"The project explores a low-cost spatio-temporal streetlight controller that combines rolling-window PIR activity estimates, sequence-based movement-direction inference, zone-specific PWM dimming, and temporary predictive illumination of the next zone. Unlike a single binary motion-triggered light, the prototype distributes illumination according to local activity and the estimated movement path. Its effectiveness is evaluated against a conventional baseline using logged brightness and measured energy."

## Limits to disclose

- PIR trigger counts are a proxy for activity, not actual vehicle/person counts.
- Three zones offer coarse direction estimation only.
- PIR sensors can miss stationary targets and have retrigger/hold delays.
- A tabletop demo does not establish road-safety compliance or real-world savings.
- Energy savings must be measured under matched test patterns; do not fabricate values.
