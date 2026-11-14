# Stella Vibra 0.4.0 Release Notes

Milestone date: November 15, 2026.

[README](../README.md) · [User manual](MANUAL.md) · [Third-party notices](THIRD-PARTY-NOTICES.md)

## DCS support

- DCS is available as a project target, using native Export API telemetry over local UDP without a separate server.
- Settings → Set DCS Telemetry discovers profiles and installs, updates, restores or removes the dedicated relay. Other content in Export.lua is preserved; Stella manages its loader line and dedicated directory.
- The default update interval is 50 ms. Restart DCS after installing or updating scripts. Relay version 2 includes cannon ammunition and weapon-station inventory; the transport protocol remains version 1.
- Five default Lua effects: Ground Roll, Touchdown, Airframe Shake, G Load and Weapon Release. Together with the other games, 46 default effect scripts are included.
- Touchdown reads center and main gear rods separately, with airborne rearming and no landing-window timeout. Initial samples and missing data do not create landing events.
- Airframe Shake defaults to Stick and both Throttle slots; G Load to both Throttle slots; Weapon Release Gun/Release to Stick. High AoA and Overspeed are not DCS defaults.
- Weapon Release distinguishes cannon-ammunition decreases from same-type station-count decreases. It cannot reliably distinguish launch, release, jettison and all rearming operations. Missing or changed stations can cause an event to be missed.
- The Telemetry view, Lua completion and existing aircraft overrides support DCS data. Missing values remain unavailable rather than measured zero.

## Editor, help and fixes

- Explorer lists folders before files and sorts names within each group. Directory changes are synchronized, and newly created project folders are populated after asynchronous loading.
- Effect folders and scripts support creation, rename, deletion and internal drag moves. If recycling fails, permanent deletion requires a separate confirmation.
- New File uses an effects-root folder tree and supports creating missing subdirectories. New Project accepts relative paths beside the portable launcher.
- Fixed MSFS gear telemetry decoding after catalog fields were added: contact and animation values now use names rather than shifted numeric offsets.
- Fixed README content/logo loss caused by mixed HTML in the Qt Markdown reader. README and manual share improved typography, image sizing and table spacing.
- Temporary Settings → Logging and its telemetry capture code are excluded from the distribution build. Normal Debug Output and crash diagnostics remain available.

## Upgrade

Keep the complete portable folder together. Back up projects and settings before replacing application files. Existing projects, Lua copies, aircraft overrides and preferences are not overwritten by a template update. Create a new project or copy selected updated templates into your existing project. Remove obsolete Landing window ms and Minimum rod rise overrides if shown as unresolved.

Existing DCS users should choose **Update Scripts** and restart DCS to use Weapon Release. Removed High AoA/Overspeed copies in user projects are not automatically deleted. Application version 0.4.0 does not change project/preferences schema version 1.

## Verification and limitations

The project owner confirmed DCS profile discovery, Ground Roll and P-51 main-gear Touchdown, Explorer sorting and the improved README/manual display. Earlier MSFS 2024 takeoff/landing vibration was also reported working. These confirmations do not certify every aircraft or subsequent effect change. Center-gear behavior after correction, Weapon Release and comprehensive simulator/hardware regression remain user acceptance items.

Development checks cover compilation/linking, Lua syntax, diff review and package consistency. The agent did not launch the application, DCS or HID tests. Short P-51/Hornet recordings showed fractional Shake values with substantially different observed magnitudes; no common physical scale is assumed. MSFS 2020, X-Plane 12 and War Thunder live verification remains pending.

The package includes matching debug symbols, offline documentation, notices, runtime dependencies and Qt/Lua source archives. Personal projects, preferences and diagnostic recordings are excluded. Microsoft SimConnect components remain subject to their accompanying terms; see the third-party notices. Preparing a ZIP does not create a Git tag, publish a GitHub release or upload anything.
