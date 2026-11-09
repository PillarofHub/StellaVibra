param([switch]$Deploy)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'MSVC Build Tools are required' }
$cmake = Join-Path $vs 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
& $cmake -S $root -B "$root\build" -G 'Visual Studio 18 2026' -A x64 "-DCMAKE_PREFIX_PATH=$root/.deps"
if ($LASTEXITCODE) { throw 'CMake configure failed' }
& $cmake --build "$root\build" --config Release -- /m:1 /nr:false
if ($LASTEXITCODE) { throw 'Build failed' }
# Release is always a runnable app-local layout; -Deploy additionally creates dist.
$destinations = @("$root\build\Release")
if ($Deploy) { $destinations += "$root\dist\StellaVibra" }
foreach ($destination in $destinations) {
    & $cmake --install "$root\build" --config Release --prefix $destination
    if ($LASTEXITCODE) { throw 'Install failed' }
    $qtDeployTool = "$root\.deps\bin\windeployqt.exe"
    & $qtDeployTool --release --no-translations --no-opengl-sw --no-system-d3d-compiler --no-compiler-runtime "$destination\StellaVibra.exe"
    if ($LASTEXITCODE) { throw 'Qt deployment failed' }
    $redist = Get-ChildItem "$vs\VC\Redist\MSVC" -Directory | Where-Object { Test-Path (Join-Path $_.FullName 'x64') } | Sort-Object Name -Descending | Select-Object -First 1
    if (-not $redist) { throw 'MSVC app-local runtime not found' }
    Copy-Item "$($redist.FullName)\x64\Microsoft.VC*.CRT\*.dll" $destination -Force
    Copy-Item "$root\licenses" $destination -Recurse -Force
    New-Item -ItemType Directory -Force "$destination\sources" | Out-Null
    Copy-Item "$root\.deps\qtbase-everywhere-src-6.8.3.tar.xz","$root\.deps\lua-5.4.8.tar.gz" "$destination\sources" -Force
    Copy-Item "$root\README.md" $destination -Force
    Copy-Item "$root\SIMCONNECT-LICENSING.md" $destination -Force
    # This SDK client is for this developer's local build, not the public package.
    $localClient = "$root\.deps\msfs2024\client\SimConnect.dll"
    if ($destination -eq "$root\build\Release" -and (Test-Path $localClient)) {
        if ((Get-FileHash $localClient -Algorithm SHA256).Hash -ne 'B10DE7ADF4C62E5F66C89DD6D01B64091BBBCEC83411CFE191D6B85FBEE61D15') { throw 'Unexpected local SDK client checksum' }
        Copy-Item $localClient $destination -Force
        Copy-Item "$root\.deps\msfs2024\client\provenance.json" "$destination\SimConnect-provenance.json" -Force
        Copy-Item "$root\.deps\msfs2024\client\MSFS SDK EULA.pdf" "$destination\licenses" -Force
    }
}
