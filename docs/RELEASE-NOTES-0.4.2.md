# Stella Vibra 0.4.2 Release Notes

[README](../README.md) · [User manual](MANUAL.md)

## DCS feedback

- Ground Roll now uses wheel rotation arguments 101/102/103 with per-gear suspension rods instead of world velocity. Moving carriers no longer inherently trigger the effect while parked.
- Separate Center Gear, Left Gear and Right Gear outputs default to Stick:1, Throttle:1 and Throttle:2.
- Added Airbrake deployment feedback on both Throttle slots: Power **2**, PWM off.
- Airframe Shake defaults to Power **40**, input amplitude **0–2**, PWM off.
- G Load defaults to Power **20**, starting at **6 G** and reaching full power at **9 G**, PWM off. This is a configurable cue, not a structural-failure or blackout detector. Stronger Shake may take priority on the same motor.
- Six DCS default effects are included, for 47 effect scripts across all supported games.

## Project history

File → Open Recent → Configure Recent Projects supports removing selected entries with Save/Cancel. It does not delete project files or close the current project.

## Updating

Choose Settings → Set DCS Telemetry → **Update Scripts**, then restart DCS to install relay v3. The loader line, interval configuration and protocol 1 remain unchanged.

Existing project scripts and settings are preserved. Create a new project or copy updated templates into your project. Review the new Ground Roll outputs and remove unresolved old Ground / Minimum speed overrides. Existing Power overrides must be adjusted separately to use the new defaults.

## Verification and limitations

The owner confirmed Hornet operation on a carrier with the wheel-based Ground Roll implementation. This does not establish compatibility with every aircraft: P-51D/Fw 190 D-9 wheel inputs still need live verification. Rotation arguments are animation phases, not RPM, and cyclic sampling can miss rotations. Missing wheel/rod data stops the corresponding output.

Airbrake uses the exported speedbrakes value as synthetic deployment feedback; it is not measured buffet or universal spoiler support. The revised Power 2 and Power 20 defaults require user evaluation. GUI/runtime tests were not run by the development agent. Compilation, Lua syntax and diff checks are recorded in the handoff.
