# Stella Vibra 0.4.0 User Manual

[README](../README.md) · [0.4.0 release notes](RELEASE-NOTES-0.4.0.md) · [Third-party notices](THIRD-PARTY-NOTICES.md)

## Installation and first launch

Use a complete portable folder in a location where you can write files. Run the root `StellaVibra.exe`; it launches the application in `bin/`. Do not move the executable by itself. Qt and Microsoft runtime DLLs are supplied in the Windows package alongside the version-specific SimConnect clients. See the third-party notices for Microsoft ownership and applicable terms.

The app restores its window layout and last project from `StellaVibra.pref`. If the project is missing, it starts without a project. There is no game connection until a project selects a target game. Supported HID devices are discovered in the background.

With a project open, execution is enabled by default and can start producing vibration when valid flight telemetry arrives. Use **Stop All** in Motor Monitor while setting up, then **Resume All** when ready. Stop All is a session control, not a persisted startup safety switch. Opening the app, a project, a device, or the Motor Test dialog does not itself create a vibration request.

## Projects and files

**File → New Project** asks for a project directory, a target game, and whether to generate default scripts. A missing directory is created. An existing empty directory is accepted; a non-empty directory requires confirmation before replacing the project file or matching template files. Other files remain. Browse can create a typed directory; cancelling New Project afterward does not remove that directory.

A project contains a `.sv` JSON file, an `effects/` directory of Lua files, and an `aircrafts/` directory for aircraft overrides. Copy the entire project directory to move or back it up. Templates copied into a project become independent files: upgrading the app does not overwrite your effects or saved parameters.

The current development build groups New Project, Open Project, Open Recent, and Close Project, followed by New File, Save, Save All, Close File, and Exit. New File shows `effects` as the selectable top-level folder, with its subfolders beneath it. Clicking a folder fills in its path while keeping the typed filename. Both `effects/ground/custom/new_effect.lua` and `ground/custom/new_effect.lua` create the same script; the effects prefix is optional. Names and paths are validated before missing folders and the script are created. Cancel creates nothing; a write failure after folder creation may leave empty folders.

| Action | Shortcut | Behavior |
| --- | --- | --- |
| New Project | Ctrl+N | Create a project |
| Open Project | Ctrl+O | Open a project `.sv` file |
| Save | Ctrl+S | Save the selected effect's script and settings, or the selected raw JSON document |
| Save All | Ctrl+Shift+S | Save all pending project edits |
| Close File | Ctrl+W | Close an editor tab |
| Undo | Ctrl+Z | Undo script editor text edits |
| Redo | Ctrl+Y / Ctrl+Shift+Z | Redo script editor text edits |
| User Manual | F1 | Open this document |

Undo/Redo applies to text editing; it is not a global parameter-change history. A modified marker clears when the content and settings match the saved state again. Closing a project or the application prompts about unsaved work.

### Creating and organizing effects (development build)

Right-click the `effects/` root for **New Folder...** and **New Script...**. Subfolders also offer **Rename...** and **Delete...**; Lua scripts offer Rename and Delete. These creation commands ask for a name before creating an item. New scripts contain a valid, commented API example with an unassigned output and no active vibration calls. The `.lua` extension is added when omitted.

Drag a Lua file or subfolder onto another folder in the same Explorer to move it. Only folders inside effects are valid destinations; external drops, moving the effects root, moving into a folder's own descendants, overwriting, and merging are not supported. Rename and drag moves preserve effect IDs, global/aircraft settings, open tabs, and unsaved script edits. Linked paths are not managed by these commands. Save or discard raw JSON edits first.

Delete asks for confirmation, moves the file or entire folder to the Recycle Bin, removes its effect settings, and discards its unsaved editor drafts. If recycling fails, a separate confirmation asks whether to permanently delete the item; No is the default. Permanent deletion cannot be undone and refuses folders containing linked paths. A filesystem failure may leave a partially deleted folder and is reported. Restoring a recycled Lua file does not restore its removed project settings. File operations are immediate; editor Undo applies only to text.

### External file changes

The Project Explorer follows Lua files inside `effects/` and its subdirectories. Each effect and subfolder has an enable checkbox; folder checkboxes toggle their effects together. The top-level effects directory is a container without a checkbox. Effects folders initially open expanded.

The Explorer lists folders before files, with case-insensitive ascending name order within each group, including after renaming, creating, or moving items.

For an unambiguous move with the same filename, settings follow the file. If both filename and location change, the new effect receives defaults. Missing effects and their settings are removed, not kept in a recovery archive. Avoid simultaneous external moves and unsaved JSON edits; save or discard raw JSON drafts before changing effect settings.

## Workspace

- **Project Explorer:** folders and files, enable toggles, and access to Lua/project/aircraft documents.
- **Effect Control:** file tabs, Parameters and Script Editor. Selecting a tab selects its parameters. Raw `.sv` files show a JSON editor and a generated-file notice, without parameter controls.
- **Motor Monitor:** connected supported motors, logical Slots, and last successfully sent HID strength. HID is an output value from 0 to 255, not joystick axis input or measured motor feedback.
- **Telemetry:** the selected game's available input fields and values. Disconnected, inactive/paused, stale, and unavailable states are distinguished.
- **Debug Output:** runtime messages and script errors, with filtering and clearing.

The View menu shows/hides each dock. **Reset Default Layout** restores hidden docks as well. Default widths are approximately 1:4:1 for Explorer:Effect Control:Monitor/Telemetry, with a 3:1 workspace:Debug Output height ratio and 1:1 Parameters:Script height ratio. Window resizing preserves adjusted proportions; very small panels may clip their contents.

## Parameters and enabled state

The Enabled/Disabled button above Parameters controls the same effect state as its Explorer checkbox. This is not a script-defined parameter.

Choose **Global** or **Specific Aircraft: aircraft name** at the top of Parameters. Aircraft choices become available from received aircraft identities; the simulator's reported identity determines which override applies.

Values resolve in this order: **script default → project Global → Specific Aircraft override**. A row reports Default, Global, or Specific. In Global mode, Reset to Default removes the override. In Specific mode, Reset to Global removes the aircraft override. Reset All also resets the selected effect's enabled state and output overrides in that scope. It does not alter other effects or the Lua source.

Parameter changes affect execution immediately but still need saving to persist. Values equal to their inherited values are omitted from saved overrides. Scripts are shared across aircraft and apply on Save. If a saved script or settings update is invalid, consult Debug Output; the last valid running version may remain active.

Numeric parameters have a slider and a number field. Unnecessary trailing zeroes are hidden. `haptic_power` combines strength (0–255), a PWM checkbox, ON time, and period. Timing fields appear when PWM is selected. ON cannot exceed period; effective PWM timing is rounded to the HID tick.

## DCS telemetry setup

Settings → **Set DCS Telemetry...** automatically discovers profiles in the Windows Saved Games location, including relocated Saved Games. Standard DCS / DCS.* folders and custom profiles containing Config/options.lua and Logs/dcs.log are candidates. One is selected automatically; if several are found, choose the intended profile from the detected list. Check the displayed target.

Choose **Update interval** in milliseconds: default **50 ms**, range **10–100 ms**. The dialog shows the detected path, interval, **Add / Update / Restore Scripts**, **Remove Scripts**, then **Done**. Restart DCS after changes; timing is not hot-reloaded. You may edit only the interval number in config.lua (10–100 ms); reopening this dialog displays that value without requesting restoration. The full config format is checked directly, without executing Lua or comparing its manifest hash. LF and CRLF are accepted; other format changes or invalid numbers require Restore Scripts.

- **Add Scripts**: Export.lua is absent or contains no Stella Vibra line.
- **Update Scripts**: one exact loader line and verified managed files exist. This button remains enabled even when already up to date.
- **Restore Scripts**: Stella-related lines differ from the standard loader, are duplicated, or managed files are missing/modified. A confirmation with affected-line details is required. Add also asks before repairing leftover modified files.
- **Remove Scripts**: enabled when Stella-related lines or the dedicated folder exist, even if file hashes do not match. Confirmation covers deleting the entire **StellaVibra Telemetry** folder, including modified and additional files.

The installer keeps loader.lua, telemetry.lua, config.lua and manifest.json under **Scripts/StellaVibra Telemetry**. It does not back up or restore the whole Export.lua. It reads the current file and changes only physical lines containing StellaVibra or Stella Vibra (case-insensitive), preserving all other bytes. Restore consolidates those lines into one standard loader; confirmation warns that other code on the same physical line is also replaced. Remove deletes those lines and also deletes Export.lua if exactly zero bytes remain. Whitespace, line endings or a BOM still count as remaining bytes and are preserved. Existing backup folders from older builds are not touched.

Each file is saved atomically, but the installation is not one multi-file transaction. If files change during confirmation, the operation stops. Modified managed files can be repaired through Restore Scripts. UTF-16/32 Export.lua files and linked installation paths are not automatically modified. Do not run another exporter installer simultaneously.

Create or open a project with **Target Game: DCS** to receive telemetry. Select **Generate default Lua scripts** when creating a project to include the five effects below. Existing projects are not overwritten; you can copy selected files from the bundled effects/DCS directory into an existing project's effects directory. Other games keep their own connections. The DCS receiver binds only **127.0.0.1:43891 (UDP)** and closes when you switch games or close the project. A port conflict is reported in Debug Output. No separate server is required.

The relay uses DCS frame callbacks, sends at most once per configured interval, and cannot run faster than DCS frames. Packets carry protocol version 1, session, sequence, simulation and send times, pause/export permission state and selected native values. Duplicate, out-of-order, malformed and old packets are ignored. Input expires within 500 ms of sending or simulation-clock progress; replaying a snapshot inside Stella does not renew its lifetime. The first sample after a session/aircraft change or long gap establishes a baseline.

The Telemetry view and Lua completion expose **telemetry.dcs** with native API names, such as **LoGetInAir**, **LoGetShakeAmplitude**, **LoGetAccelerationUnits**, **LoGetMechInfo** and **LoGetEngineInfo.RPM**. **telemetry.transport** holds packet metadata. Missing values are nil, not measured zero. Ownship export restrictions are respected. This does not read force-feedback commands, and gear rod units/behavior remain aircraft-dependent and unverified. Weapon and cockpit-specific signals are not collected in this version.

Automatic profile discovery has been confirmed by the project owner. Installation/removal, exporter coexistence and live DCS reception still require user verification.

## Diagnostics

The 0.4.0 distribution excludes the temporary Settings → Logging menu and telemetry capture. Normal Debug Output and crash diagnostics remain available. Existing diagnostic files are not deleted by an update.

## Relative project paths

New Project accepts a folder name (for example `Hornet`) or a relative path (`.\Hornet`, `Projects\Hornet`). These resolve against the portable application folder beside the launcher, not the process working directory or the internal bin folder. The Browse dialog uses the same initial base. Full paths still work; empty paths and ambiguous drive-relative forms such as `C:Projects` are rejected. Existing nonempty destinations still require overwrite confirmation.

## DCS default effects

These are experimental starting settings, not aircraft-certified limits or force feedback. Missing inputs stop the dependent effect; they are never treated as measured zero. All effects use common Export API paths, without aircraft-specific cockpit mappings.

| Script | Input and behavior | Default output / settings |
| --- | --- | --- |
| ground/ground_roll.lua | Not in air and horizontal world velocity above Minimum speed. Constant intensity, no simulated runway slabs. World velocity is not carrier-relative. | Stick:1, Throttle:1 and Throttle:2; strength 1, PWM 20/80 ms; minimum speed 0.5 in native velocity units |
| ground/touchdown.lua | First increase of each gear.main.left/right.rod or gear.nose.rod after airborne rearming and an air-to-ground transition. Rod direction, scale and aircraft support need flight verification. | Center Gear → Stick:1; Left/Right Gear → Throttle:1/2; strength 40 Constant; 200–2000 ms, Time multiplier 2 |
| aerodynamics/airframe_shake.lua | LoGetShakeAmplitude scaled between Minimum amplitude and Full power amplitude, clamped at Power strength. No artificial separation of engine shake and aerodynamic buffet. | Stick:1, Throttle:1 and Throttle:2; strength 30, Constant; amplitude 0–1 |
| aerodynamics/g_load.lua | Airborne LoGetAccelerationUnits.y scaled between Start G and Full power G. Direction selects Positive, Negative or Both; negative thresholds use magnitudes. | Throttle:1 and Throttle:2; strength 25, Constant; Positive, 2–6 G |
| weapons/weapon_release.lua | Cannon-ammunition and same-type station-count decreases; the cause of removal is not identified. | Gun and Release → Stick:1; strength 40, Constant; Gun 80 ms, Release 150 ms |


Shake maps the configured range to integer output: `round(Power.strength * clamp((shake - Minimum amplitude) / (Full power amplitude - Minimum amplitude), 0, 1))`. Power supports 0–255; strength zero turns the request off. With Power 30 and input range 0–1, shake 0.1 gives 3 and shake 0.25 gives 8. Full power amplitude remains 1 until the user tunes it; the observed tiny Hornet signal is not automatically amplified.

`weapons/weapon_release.lua` provides **Gun** and **Release**, both mapped to Stick:1 by default. Power defaults to 40 Constant; Gun ms 80 and Release ms 150 are independent event durations. Gun detects a decrease in LoGetPayloadInfo.Cannon.shells. Release detects a decrease in a station count with unchanged station ID, weapon.level1..4 and container value. Multiple decreases in one sample produce one pulse per output; later decreases extend that output's pulse without accumulating time. These are inventory-change cues, not proof of firing: release, launch, jettison and some rearming removals cannot be distinguished.

Use Settings → Set DCS Telemetry → **Update Scripts**, then restart DCS to install relay v2. The Export.lua loader line, interval configuration and protocol 1 stay unchanged. Missing payload data produces no weapon vibration. Station indices are preserved as string keys, for example `telemetry.dcs.LoGetPayloadInfo.Stations["1"].count`; absent stations are not converted to zero. Cannon and station availability are independent. First samples, invalid input and aircraft/session changes establish a new baseline. Increased counts, new stations or changed types reset comparisons and suppress pulses for 500 ms. Rearming removals can still be ambiguous; a station that disappears or changes type on its last release may be missed deliberately. No airborne restriction is imposed, so ground firing is supported when counts are provided.

The relay queries payload at the configured interval and sends only Cannon.shells and station count/container/type, up to 64 stations. If the payload would exceed 16 KiB it is omitted to preserve flight telemetry. Missing inputs remain unavailable. Payload fields appear in Telemetry; the earlier P-51/Hornet logs predate payload collection and cannot validate this effect. Old High AoA/Overspeed files are removed only from bundled templates; existing user projects keep their copies and can disable/delete them manually.

Rearm ms belongs to **Touchdown**, not Ground Roll. Each gear requires valid rod samples while airborne for Rearm ms (default 200) before accepting a landing. Once armed, each gear waits independently for its first valid rod increase while the aircraft is not in air. There is no Landing window timeout, so delayed center contact is not discarded. That gear consumes its armed state on the first rise and cannot trigger again until airborne rearming. An armed gear that never shows a landing rise can still respond to a later rise; rod is a proxy rather than an independent wheel-contact signal.

The first rising segment's simulation-time duration, multiplied by Time multiplier, sets the bounded event duration. Later peaks do not restart that landing. Initial ground samples never trigger a touchdown; missing rods reset that gear and produce no fallback pulse. Invalid input resets all gear states. The nose path is named Center Gear and does not guarantee a nosewheel on every aircraft. A fixed 1e-6 numerical guard rejects negligible rod changes. Landing window ms and Minimum rod rise are no longer exposed parameters; obsolete overrides may appear as unresolved when updating an existing project and can be removed.

Continuous effects renew 100 ms requests while input remains valid. Touchdown retains a separate event deadline. Default transmission is 50 ms, input validity is bounded at 500 ms, and the default HID tick is 20 ms; repeated snapshots do not renew telemetry validity. A brief packet gap can hold the last continuous input until expiry. Effects targeting the same motor compete under the existing single-request priority rules; they are not mixed. Disable competing effects when evaluating individual responses. High AoA, Overspeed, separate stall buffet, ground thumps and engine RPM/start/stop effects are not supplied in this DCS set.

## Devices and routing

The implemented WINCTRL vendor ID is `4098` (hex). Only known product IDs and compatible HID output descriptors are opened; other joysticks are not generic fallback targets.

| Device | Product ID (hex) |
| --- | --- |
| URSA MINOR Airbus L / R | BC27 / BC28 |
| URSA MINOR Combat/Fighter L / R | BC29 / BC2A |
| URSA MINOR Space L / R | BC2B / BC2C |
| URSA MINOR Throttle / Throttle R | B920 / B930 |
| Combat Throttle Metal.EX | B970 |
| Orion Joystick Base II | BEA8 |
| Orion Throttle Base II | BD64 |

This is the implemented allowlist, not a claim that every listed model has been tested on hardware. The project owner has reported operation with their MSFS 2024 setup and URSA MINOR Combat Joystick R.

Routing has two stages: an effect's named **Output** selects logical slots; each physical motor's **Slots** determines which requests reach it. Categories are Yoke, Stick, Throttle, Pedal and Wheel, each with slots 1–4. Rudder in Settings uses the stored category name Pedal. Multiple motors may share a slot; a motor may receive requests from multiple slots.

**Settings → Set Channels...** chooses 0–4 channels for Yoke, Stick, Throttle and Rudder (default 4 each). Wheel retains four slots. These counts fit *script default outputs*: zero removes that category's default destinations, otherwise an out-of-range slot wraps using `(number - 1) % count + 1`. With two throttle channels, Throttle:3 becomes Throttle:1. Duplicates are removed. Explicit project/aircraft output overrides and physical mappings are preserved.

In Motor Monitor, double-click or right-click **Slots** to change a motor's mapping. Motors are sorted by mapped slot, then name. Mappings belong to the application folder's preferences, not to a project. Review them instead of relying on connection order.

Double-click or right-click **HID** to open Motor Test. Choose strength/timing/duration, then explicitly select Start. The test lasts at most 5 seconds and temporarily replaces game output for that motor. Stop, closing the dialog, or Stop All ends the test.

Requests are not summed. Each motor selects one request: Constant wins over PWM; within a mode, stronger wins; equal-strength PWM prefers a shorter period and then a longer ON time. An expired request stops contributing. Another active effect can still keep that motor running.

## Connecting a game

### MSFS 2024 and MSFS 2020

Open a project for the intended version, then run that simulator locally and enter a flight. Connections retry automatically. The prepared package has separate official SDK client DLLs under `bin/simconnect/MSFS2024/` and `bin/simconnect/MSFS2020/`; there is no DLL-path setting or game-directory search.

Run only the selected MSFS version. If both versions are running, connection is deferred to avoid selecting the wrong simulator. Stella uses standard SimVars, not aircraft-specific L-vars; add-ons may not update all expected values. The catalog contains 59 numeric/boolean fields plus aircraft TITLE, not every SimVar supported by MSFS.

### X-Plane 12

Use the local simulator's UDP RREF endpoint at `127.0.0.1:49000`. Stella subscribes automatically to its 86-field catalog. A non-default simulator UDP port is not configurable in 0.4.0. The inspector and Lua use original dataref paths and native numeric values. Fast fields are requested at 20 Hz and aircraft information at 1 Hz. Live simulator verification is pending.

### War Thunder

Stella polls local `/state` and `/indicators` at `127.0.0.1:8111`, up to 20 Hz, and retains their native JSON keys. Enter a flight with usable aircraft telemetry. Live game verification is pending.

There is no reliable simulation-clock/pause flag in this integration. Unchanged payloads are conservatively treated as stale; steady flight can therefore suppress output. Missing fields are not fabricated. Aircraft-specific limits may require manual parameters.

### Stale data and timing

Invalid, paused or stale telemetry stops driving effects. Reusing a snapshot does not renew its original receive time or validity. The status bar's connection state alone does not imply usable telemetry.

**Settings → Set Timings...** offers HID tick (5–100ms, default 20) and Maximum update period (5–500ms, default 100). Reset to Defaults resets both fields; OK applies and saves them. HID tick changes clear active requests/tests and reinitialize effect state. Maximum update period controls re-delivery when no new sample arrives; fresh telemetry is processed immediately. It does not change game subscriptions or the 100ms inspector refresh.

## Default effects

MSFS 2024/2020 and X-Plane include Ground Roll, Ground Thumps, Touchdown, Bank, Flaps, Overspeed, Spoilers, Stall, Startup, Shutdown, and Thrust (RPM in X-Plane). War Thunder includes Stall cue, manual Overspeed, Flaps, Gear, Firing, Startup, Shutdown, and Power.

- Ground Roll defaults to Power 1 and PWM ON, with a minimum speed of 0.1knots. Other templates default to PWM OFF.
- Common continuously renewed requests use 100ms. This is an example renewal duration, not a device requirement; slow inputs can leave gaps.
- Touchdown uses the first compression rise, with default minimum 200ms, maximum 2000ms and multiplier 2. It does not estimate the maximum peak of an entire landing.
- Ground Thumps uses three gear outputs and a virtual slab-distance model (default 4.6m). It is not a measured pavement-joint sensor. MSFS requires ON ANY RUNWAY plus the selected Concrete/Asphalt surface. X-Plane filters material but cannot distinguish matching taxiways/aprons. High-speed pulses are shortened and spaced; sub-tick pulses can be lost or indistinct.
- MSFS/X-Plane Overspeed uses game limits. War Thunder retains a manual limit because an equivalent input is unavailable.
- Engine strength scaling and event detection depend on the game's available fields. First samples establish state rather than inventing startup/shutdown edges.

Templates explicitly assign Center Gear to Stick:1, Left/Right Gear to Throttle:1/2, Airframe to Stick:1, and Engine 1–4 to Throttle:1–4, subject to channel counts. Tune values for your aircraft/device. Existing projects do not receive template updates automatically.

## Writing Lua effects

Each script returns an effect table. `init(context)` registers parameters and outputs. `update(context, telemetry, parameters)` receives separate API, game data, and resolved parameter tables. Keep instance state in `self`.

```lua
local effect = {}

function effect:init(context)
    context:parameter {
        name="Power", type="haptic_power",
        default={strength=1, pwm=false, on_ms=20, period_ms=80}
    }
    context:output("Airframe", {"Stick:1"})
end

function effect:update(context, telemetry, parameters)
    -- MSFS example. Other games use their own native telemetry fields.
    if context:input_valid() and telemetry.stall_warning then
        context:vibrate("Airframe", parameters.Power, 100)
    else
        context:vibrate("Airframe", off())
    end
end

return effect
```

Parameter types are `boolean`, `int`, `float`, `haptic_power`, and `enum`. All require a name, type and default. Int/float require min/max/step. Enum requires a dense array of unique string choices and a default from that list. Haptic power defaults ON/period to 20/80ms when omitted and optionally accepts strength min/max within 0–255.

Names with spaces use underscores in `parameters`: `Minimum speed knots` becomes `parameters.Minimum_speed_knots`. The result must be a valid non-reserved Lua identifier; colliding names are rejected. Original names remain display and storage keys.

Outputs are registered with `context:output(name, optional_default_slots)`. Omitted/empty defaults are unassigned. Names are simultaneously display labels, call keys, and storage keys. Each output can be controlled independently.

During update, use `context:now()` (monotonic milliseconds), `elapsed()`, `sample_is_new()`, `input_valid()`, `log(message)` and `vibrate(name, specification, duration_ms)`. The timing/input methods are not available in init. A specification can be a resolved haptic_power table, `constant(strength)`, `pwm{strength=...,on_ms=...,period_ms=...}`, or `off()`. Off needs no duration. Active durations must be positive and at most 60000ms.

A new request replaces that output's specification and deadline; it does not queue or add to previous requests. The deadline is this update's time plus duration, with changes applied from the next HID tick. To vary strength without rebuilding PWM, copy the power table and change only `strength`. Do not mutate a shared table repeatedly across multiple outputs and unintentionally compound scaling.

Autocomplete offers context methods after `context:`, telemetry keys after `telemetry.`, and registered parameters after `parameters.`. X-Plane uses `telemetry["dataref/path"]`; War Thunder uses `telemetry.state["IAS, km/h"]` and `telemetry.indicators`. It is a lightweight editor, not a complete Lua language server or debugger. Scripts run with restricted libraries, memory and execution budgets.

## Preferences, diagnostics and reset

`StellaVibra.pref` stores geometry/layout, recent/last projects, motor mappings, channel counts, and timings beside the root launcher. Closing the app and deleting that file resets app preferences. Deleting the application folder removes app-local state but does not delete projects stored elsewhere. No automatic migration from older AppData settings is performed.

Debug Output is also recorded under `diagnostics/`. Session logs rotate at 2MiB, keeping one previous file. An unhandled crash attempts to write a `.dmp`; not all failures produce a dump. Preserve the matching `bin/StellaVibra.exe` and `.pdb` when reporting crashes. Logs/dumps may contain local paths and session data. They are not uploaded automatically.

| Problem | What to check |
| --- | --- |
| A dock disappeared | Enable it in View, or Reset Default Layout |
| A motor does not appear | Exact device PID, USB connection, compatible HID descriptor, Debug Output; do not assume generic joystick support |
| Connected but no vibration | Stop/Resume state, effect enabled state, fresh telemetry, parameters, Output destinations and physical Slots |
| Changed script has no effect | Save it, then inspect script errors; the last valid version may still be running |
| Old defaults remain after upgrade | Existing Lua and overrides are preserved; copy desired templates and reset the relevant overrides |
| Cannot save preferences | Move the whole app folder to a writable location |
| A crash occurs | Preserve diagnostics and matching executable/PDB before replacing the build |
| Icons appear unchanged in Explorer | Windows may still be displaying a cached icon; verify the updated executable path |

## Help and release status

Help → README and Help → User Manual display the same local Markdown sources used on GitHub, in a nonmodal document window. The reader uses readable body text, heading spacing, monospaced code, and padded table cells. Images fit the available document width, with a maximum displayed width of 720 pixels. Back/Forward/Home navigate documents; relative links and images resolve from the current document, and web links open your browser. This is a Qt rich-text reader, not a full web browser or a pixel-identical GitHub page. About shows version and license information. Keep the full package, including docs and logo assets, together.

Version 0.4.0 adds DCS support. The project owner confirmed the improved README/manual display; effect and aircraft limitations remain as described in the [0.4.0 release notes](RELEASE-NOTES-0.4.0.md). Remote simulator connections, arbitrary device protocols and a full IDE/debugger are not included.
