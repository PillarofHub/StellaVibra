param([switch]$Deploy, [switch]$Detailed, [switch]$WithoutTelemetryLogging)
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$log = Join-Path $root 'build/build.log'
New-Item -ItemType Directory -Force (Split-Path $log) | Out-Null
function Invoke-StellaBuild {
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { throw 'MSVC Build Tools are required' }
$cmake = Join-Path $vs 'Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe'
$capture = if($WithoutTelemetryLogging){'OFF'}else{'ON'}
& $cmake -S $root -B "$root\build" -G 'Visual Studio 18 2026' -A x64 "-DCMAKE_PREFIX_PATH=$root/.deps" "-DSTELLA_TELEMETRY_LOGGING=$capture"
if ($LASTEXITCODE) { throw 'CMake configure failed' }
& $cmake --build "$root\build" --config Release -- /m:1 /nr:false /nologo /verbosity:minimal
if ($LASTEXITCODE) { throw 'Build failed' }
# Release is always a runnable app-local layout; -Deploy additionally creates dist.
$destinations = @("$root\build\Release")
if ($Deploy) { $destinations += "$root\dist\StellaVibra" }
foreach ($destination in $destinations) {
    & $cmake --install "$root\build" --config Release --prefix $destination
    if ($LASTEXITCODE) { throw 'Install failed' }
    # Remove only known obsolete shipped template files, never user project directories.
    $oldTemplates=@('ground_roll.lua','touchdown.lua','ground/ground_roll.lua','ground/touchdown.lua','ground/ground_thumps.lua','aerodynamics/bank.lua','aerodynamics/flaps.lua','aerodynamics/overspeed.lua','aerodynamics/spoilers.lua','aerodynamics/stall.lua','engines/startup.lua','engines/shutdown.lua','engines/thrust.lua')
    foreach($relative in $oldTemplates){$old=Join-Path "$destination/effects" $relative;if(Test-Path -LiteralPath $old -PathType Leaf){Remove-Item -LiteralPath $old}}
    $client2020="$root/.deps/msfs2020/client/SimConnect.dll"
    if(!(Test-Path $client2020) -or (Get-FileHash $client2020 -Algorithm SHA256).Hash -ne 'CFFB74B62B596A897B0FF101BAC3D66EA10236C540C75864658B451FFD1E6513'){throw 'Official MSFS 2020 SDK 0.24.6 client is missing or has an unexpected checksum'}
    New-Item -ItemType Directory -Force "$destination/bin/simconnect/MSFS2020" | Out-Null
    Copy-Item $client2020,"$root/.deps/msfs2020/client/provenance.json","$root/.deps/msfs2020/client/MSFS SDK EULA.pdf" "$destination/bin/simconnect/MSFS2020" -Force
    $qtDeployTool = "$root\.deps\bin\windeployqt.exe"
    # Windows PowerShell treats native stderr warnings as errors under Stop.
    $ErrorActionPreference = 'Continue'
    & $qtDeployTool --plugindir "$destination\bin\plugins" --release --no-translations --no-opengl-sw --no-system-d3d-compiler --no-compiler-runtime "$destination\bin\StellaVibra.exe"
    $qtExitCode = $LASTEXITCODE
    $ErrorActionPreference = 'Stop'
    if ($qtExitCode) { throw 'Qt deployment failed' }
    $redist = Get-ChildItem "$vs\VC\Redist\MSVC" -Directory | Where-Object { Test-Path (Join-Path $_.FullName 'x64') } | Sort-Object Name -Descending | Select-Object -First 1
    if (-not $redist) { throw 'MSVC app-local runtime not found' }
    Copy-Item "$($redist.FullName)\x64\Microsoft.VC*.CRT\*.dll" "$destination/bin" -Force
    Copy-Item "$root\licenses" $destination -Recurse -Force
    New-Item -ItemType Directory -Force "$destination\sources" | Out-Null
    Copy-Item "$root\.deps\qtbase-everywhere-src-6.8.3.tar.xz","$root\.deps\lua-5.4.8.tar.gz" "$destination\sources" -Force
    Set-Content -LiteralPath "$destination/bin/qt.conf" -Value "[Paths]`nPlugins=plugins" -Encoding ascii
    # Include original SDK clients under the maintainer distribution policy; retain provenance and EULAs.
    $localClient = "$root\.deps\msfs2024\client\SimConnect.dll"
    if (-not (Test-Path $localClient)) { throw 'Official SDK client missing: .deps/msfs2024/client/SimConnect.dll (see SIMCONNECT-LICENSING.md)' }
    if (Test-Path $localClient) {
        if ((Get-FileHash $localClient -Algorithm SHA256).Hash -ne 'B10DE7ADF4C62E5F66C89DD6D01B64091BBBCEC83411CFE191D6B85FBEE61D15') { throw 'Unexpected local SDK client checksum' }
        New-Item -ItemType Directory -Force "$destination/bin/simconnect/MSFS2024" | Out-Null
        Copy-Item $localClient "$destination/bin/simconnect/MSFS2024" -Force
        Copy-Item "$root\.deps\msfs2024\client\provenance.json" "$destination/bin/simconnect/MSFS2024/provenance.json" -Force
        Copy-Item "$root\.deps\msfs2024\client\MSFS SDK EULA.pdf" "$destination\licenses" -Force
    }
    # Remove only relocated generated files, after their replacements exist.
    # Unknown files and StellaVibra.pref are deliberately retained.
    $relocated=@{
        'THIRD-PARTY-NOTICES.md'='docs/THIRD-PARTY-NOTICES.md';
        'StellaVibra.pdb'='bin/StellaVibra.pdb';
        'SimConnect.dll'='bin/simconnect/MSFS2024/SimConnect.dll';'SimConnect-provenance.json'='bin/simconnect/MSFS2024/provenance.json'
    }
    foreach($dll in Get-ChildItem -LiteralPath "$destination/bin" -Filter '*.dll' -File){$relocated[$dll.Name]="bin/$($dll.Name)"}
    foreach($file in Get-ChildItem -LiteralPath "$destination/bin/plugins" -Recurse -File){
        $relative=$file.FullName.Substring(([IO.Path]::GetFullPath("$destination/bin/plugins").TrimEnd('\','/' )).Length + 1);$relocated[$relative]="bin/plugins/$relative"
    }
    foreach($name in @('SimConnect.dll','provenance.json','MSFS SDK EULA.pdf')){$relocated["simconnect/MSFS2020/$name"]="bin/simconnect/MSFS2020/$name"}
    foreach($relative in $relocated.Keys){
        $old=Join-Path $destination $relative;$replacement=Join-Path $destination $relocated[$relative]
        if((Test-Path -LiteralPath $replacement -PathType Leaf) -and (Test-Path -LiteralPath $old -PathType Leaf)){
            try { Remove-Item -LiteralPath $old -Force } catch { throw "Cannot remove relocated file ${old}: $($_.Exception.Message)" }
        }
    }
    # Retired internal documents from earlier local staging; never ship as user help.
    foreach($relative in @('docs/CONTRACTS.md','docs/SIMCONNECT-LICENSING.md','CONTRACTS.md','SIMCONNECT-LICENSING.md')){
        $old=Join-Path $destination $relative;if(Test-Path -LiteralPath $old -PathType Leaf){Remove-Item -LiteralPath $old -Force}
    }
    # Remove only empty legacy plugin directories (never recurse-delete content).
    foreach($relative in @('platforms','styles','imageformats','iconengines','networkinformation','tls','generic','simconnect/MSFS2020','simconnect')){
        $old=Join-Path $destination $relative
        if((Test-Path -LiteralPath $old -PathType Container) -and @(Get-ChildItem -LiteralPath $old -Force).Count -eq 0){Remove-Item -LiteralPath $old}
    }
    if($destination -eq "$root\build\Release" -and (Test-Path -LiteralPath "$root/build/tools/Release/LuaCompiler.exe")){
        foreach($name in @('LuaCompiler.exe','LuaCompiler.pdb','lua.lib')){if(Test-Path -LiteralPath "$destination/$name" -PathType Leaf){Remove-Item -LiteralPath "$destination/$name" -Force}}
    }
}
}
try {
    if ($Detailed) { Invoke-StellaBuild }
    else { Invoke-StellaBuild *> $log }
    Write-Host "Build/deploy succeeded. $root\build\Release\StellaVibra.exe"
    if (-not $Detailed) { Write-Host "Log: $log" }
} catch {
    $failure = $_.Exception.Message
    $location = $_.InvocationInfo.PositionMessage
    if (-not $Detailed -and (Test-Path $log)) {
        $errors = @(Select-String -Path $log -Pattern '(?i)error|fatal|failed|\uC624\uB958' | Select-Object -Last 12 | ForEach-Object { $_.Line })
        if ($errors.Count) { $errors | Write-Host }
        else { Get-Content $log -Tail 12 | Write-Host }
    }
    Write-Host "Build/deploy failed: $failure"
    Write-Host $location
    if (-not $Detailed) { Write-Host "Full log: $log" }
    exit 1
}
