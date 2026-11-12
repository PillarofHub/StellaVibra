# Third-party notices

Stella Vibra 0.2.0 is licensed under the [MIT License](../LICENSE). That license does not replace the terms of the components below.

## Qt 6.8.3

The application dynamically links Qt under LGPL-3.0. License texts are in `licenses/LGPL-3.0.txt` and the accompanying Qt license files. The prepared package includes the corresponding qtbase source archive in `sources/`. You may replace or modify the Qt libraries and debug/reverse-engineer for that purpose as provided by the license.

[Qt licensing](https://doc.qt.io/qt-6.8/licensing.html) · [Corresponding source](https://download.qt.io/archive/qt/6.8/6.8.3/submodules/qtbase-everywhere-src-6.8.3.tar.xz)

The source checkout's local dependency cache is not included in Git. Preserve the applicable licenses and corresponding-source provision when distributing binaries.

## Lua 5.4.8

MIT License. Copyright 1994–2025 Lua.org, PUC-Rio. The package contains `licenses/Lua-5.4.8.html` and the source archive in `sources/`.

[Official source](https://www.lua.org/ftp/lua-5.4.8.tar.gz)

## Aurora Vibra

MIT reference implementation. Copyright 2025 Rodrigo Troncoso; Copyright 2026 chegevarko1982. Full notice: `licenses/Aurora-MIT.txt`.

HID protocols, telemetry selections, X-Plane RREF catalog, War Thunder endpoints and effect conditions were informed by Aurora. Stella does not use Aurora's waveform mixer.

## Microsoft Visual C++ runtime

Microsoft license terms apply. The prepared package includes app-local runtime DLLs from the installed Visual Studio Build Tools redistributable directory.

## Microsoft SimConnect

All rights in the SimConnect SDK client DLLs remain with Microsoft Corporation and its licensors. These components are not covered by Stella Vibra's MIT license; Microsoft's applicable license terms govern their use. Windows packages contain unmodified clients extracted from MSFS 2024 SDK Core 1.7.3 and MSFS 2020 SDK Core 0.24.6.0. They are kept separately under `bin/simconnect/MSFS2024/` and `bin/simconnect/MSFS2020/`. Each directory includes provenance.json; SDK EULA copies accompany the package.

The maintainer has chosen to include these unmodified clients with Stella Vibra. Explicit confirmation of redistribution permission has not been obtained; this inclusion and ownership notice do not constitute a grant of rights from Microsoft. Stella Vibra is not endorsed or supported by Microsoft. If Microsoft raises an objection, the maintainer intends to withdraw affected downloads and review the packaging. Withdrawal does not revoke copies already downloaded or resolve any prior distribution obligations.

[MSFS 2024 SDK EULA](https://docs.flightsimulator.com/msfs2024/retail/introduction/sdk-eula/)

The source repository retains the detailed internal review in SIMCONNECT-LICENSING.md. Aurora's SimConnect binaries were not copied. The application does not search game installation directories or use SimConnect_internal.dll.
