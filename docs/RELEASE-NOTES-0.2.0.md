# Stella Vibra 0.2.0 Release Notes

Milestone date: November 12, 2026.

[README](../README.md) · [User manual](MANUAL.md) · [Third-party notices](THIRD-PARTY-NOTICES.md)

## Release status

0.2.0 freezes the current feature set for acceptance testing. Version 0.2.0 is used in CMake, About, and both Windows executables. Project and preferences JSON schema version 1 remains unchanged.

The Windows package includes unmodified SimConnect clients by the maintainer's decision, with Microsoft ownership, SDK EULAs and provenance preserved. This decision is not confirmation of Microsoft redistribution permission. The application code's MIT license does not grant rights to Microsoft DLLs. See the third-party notices for details. Preparing this package does not publish it to GitHub.

## Included in this milestone

- Windows 10/11 x64 portable application with automatic game/device discovery.
- MSFS 2024/2020 SimConnect, X-Plane 12 RREF, and War Thunder HTTP integrations, selected per project.
- 41 bundled Lua effects using each game's own telemetry fields.
- Project creation/open/recent/close, per-effect Save and Save All, last-project restoration, and external effect-file tracking.
- Tabbed Lua/JSON editing, text Undo/Redo, lightweight completion, live parameters, and Global/Specific Aircraft overrides.
- Typed parameters including haptic_power and enum, compact numeric display, and minimum override storage.
- Channel-count settings, script-defined output defaults, physical motor mapping, and explicit motor tests.
- Dockable Explorer, Effect Control, Motor Monitor, Telemetry and Debug Output, with layout reset and proportional resizing.
- HID tick and maximum-update-period settings, app-local preferences, session logs, and best-effort crash dumps.
- New icon/logo and offline English README/User Manual accessible from Help.
- A root launcher, application/runtimes under bin, versioned SimConnect directories, and separate documentation/license/source directories.

Ground Roll defaults to strength 1 with PWM enabled and a 0.1-knot threshold. Other effects default to PWM disabled. General request renewal durations are 100ms. Touchdown defaults to 200–2000ms with multiplier 2. Ground Thumps uses per-gear virtual slab-boundary events and surface filtering. See the manual for limitations and configuration.

## Validation and known limitations

- The project owner reported working MSFS 2024 takeoff/landing vibration on the earlier working build. This does not certify every aircraft, effect, device, or subsequent UI change.
- Development validation covers C++/resource compilation, linking, Lua syntax compilation, static checks, and local deployment consistency. Syntax compilation does not execute Lua callbacks.
- Final package GUI, offline help navigation, and simulator/hardware regression checks remain manual acceptance items.
- MSFS 2020, X-Plane 12 and War Thunder live integrations remain unverified. Device protocols are implemented for an explicit allowlist; most models have not been tested on hardware.
- MSFS add-ons may omit or alter standard SimVars. X-Plane datarefs and War Thunder JSON fields vary by aircraft. Missing inputs must not be interpreted as measured zeroes.
- War Thunder's unchanged payloads expire conservatively and may suppress output during steady flight.
- Ground Thumps is a distance-based model, not measured wheel/joint geometry. X-Plane's material filter also matches taxiways/aprons. Sampling and HID timing limit short pulse resolution.
- Windows crash dumps are best effort. The previous MSFS-start crash was followed by a successful user session after DLL-lifetime mitigation, but the original cause was not proven from a dump.
- DCS is not included. No DCS Export.lua files are installed or changed.

## Updating from a development build

Close Stella Vibra before replacing files. Back up projects and StellaVibra.pref. Keep the entire new package together and launch its root StellaVibra.exe. The root pref location remains unchanged; bin contains the actual app and PDB.

Existing projects, scripts and overrides are not migrated to new template defaults. Create a new project to inspect current examples, or selectively copy templates and reset overrides. Older layout versions may reset; use View → Reset Default Layout when needed.

Temporary --ui-check startup/layout diagnostics have been removed. This version has no supported command-line self-test mode. Normal startup restores preferences and starts background discovery.

## Manual acceptance before publication

1. Launch the root executable from a complete clean package in a writable directory, including a path with spaces. Confirm version, icon, offline Help pages, local links and logo.
2. Create/open/save/close/reopen projects. Check recent/last-project handling, same-name file moves, removed files, tabs, Undo/Redo and invalid scripts.
3. Check Global/Specific resets, saved-value round trips, channel counts 0/1/2/4, default wrapping and explicit mappings.
4. Hide/show/reset docks; resize and maximize/restore; check preference persistence.
5. On each target game, verify start/stop/reconnect, pause, aircraft change and stale-input shutdown. Start with Stop All and low strengths, then explicitly resume.
6. Verify supported-device hotplug, mappings, per-motor tests, Stop All and conflicting-effect priority. Confirm the revised default effects in flight.
7. Confirm logs, troubleshooting information and writable-folder behavior. Preserve matching PDBs for reports.
8. Verify the bundled third-party notices, original SDK EULAs and provenance, and record the maintainer's SimConnect distribution decision. Exclude private pref files, diagnostics, user projects and local build caches from any published package.

No automatic GitHub publishing or external telemetry upload is part of 0.2.0.
