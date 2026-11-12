# Run build.ps1 -Deploy first. Never archive a user's entire runnable directory.
$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$source = Join-Path $root 'dist/StellaVibra'
$version = [regex]::Match((Get-Content "$root/CMakeLists.txt" -Raw), 'project\(StellaVibra VERSION ([0-9.]+)').Groups[1].Value
if (!$version) { throw 'Cannot read project version' }
foreach ($exe in @('StellaVibra.exe','bin/StellaVibra.exe')) {
    if ((Get-Item -LiteralPath "$source/$exe").VersionInfo.ProductVersion -ne $version) { throw 'Build/deploy this version first' }
}
$files = [Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
foreach ($p in @('StellaVibra.exe','LICENSE','README.md','assets/stella-vibra-logo.png','docs/MANUAL.md','docs/RELEASE-NOTES-0.2.0.md','docs/THIRD-PARTY-NOTICES.md','bin/StellaVibra.exe','bin/StellaVibra.pdb','bin/qt.conf','sources/qtbase-everywhere-src-6.8.3.tar.xz','sources/lua-5.4.8.tar.gz')) { [void]$files.Add($p) }
foreach ($file in Get-ChildItem -LiteralPath "$source/bin" -File -Filter '*.dll') { [void]$files.Add("bin/$($file.Name)") }
foreach ($file in Get-ChildItem -LiteralPath "$source/bin/plugins" -Recurse -File -Filter '*.dll') { [void]$files.Add([IO.Path]::GetRelativePath($source,$file.FullName)) }
foreach ($game in @('MSFS2020','MSFS2024')) {
    foreach ($name in @('SimConnect.dll','provenance.json')) { [void]$files.Add("bin/simconnect/$game/$name") }
}
[void]$files.Add('bin/simconnect/MSFS2020/MSFS SDK EULA.pdf')
[void]$files.Add('licenses/MSFS SDK EULA.pdf')
# Use the repository's template/license list, not arbitrary files added by users.
foreach ($dir in @('effects','licenses')) {
    foreach ($file in Get-ChildItem -LiteralPath "$root/$dir" -Recurse -File) { [void]$files.Add([IO.Path]::GetRelativePath($root,$file.FullName)) }
}
foreach ($relative in $files) {
    if (!(Test-Path -LiteralPath "$source/$relative" -PathType Leaf)) { throw "Missing deployed file: $relative" }
    if (Test-Path -LiteralPath "$root/$relative" -PathType Leaf) {
        if ((Get-FileHash -LiteralPath "$root/$relative").Hash -ne (Get-FileHash -LiteralPath "$source/$relative").Hash) { throw "Stale deployed file: $relative; run build.ps1 -Deploy" }
    }
}
$hashes=@{
    MSFS2020='CFFB74B62B596A897B0FF101BAC3D66EA10236C540C75864658B451FFD1E6513'
    MSFS2024='B10DE7ADF4C62E5F66C89DD6D01B64091BBBCEC83411CFE191D6B85FBEE61D15'
}
foreach ($game in $hashes.Keys) {
    if ((Get-FileHash -LiteralPath "$source/bin/simconnect/$game/SimConnect.dll").Hash -ne $hashes[$game]) { throw "Unexpected $game client" }
}
# Fresh staging on every invocation; no deletion or modification of the user's app.
$output = Join-Path $root ("dist/packages/" + [guid]::NewGuid().ToString('N'))
$stage = Join-Path $output 'StellaVibra'
foreach ($relative in $files) {
    $target=Join-Path $stage $relative
    New-Item -ItemType Directory -Force (Split-Path $target) | Out-Null
    Copy-Item -LiteralPath "$source/$relative" -Destination $target
}
Add-Type -AssemblyName System.IO.Compression.FileSystem
$zip = Join-Path $output "StellaVibra-$version-windows-x64.zip"
[IO.Compression.ZipFile]::CreateFromDirectory($stage,$zip,[IO.Compression.CompressionLevel]::Optimal,$true)
$checksum = (Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash.ToLowerInvariant()
Set-Content -LiteralPath "$zip.sha256" -Value "$checksum  $([IO.Path]::GetFileName($zip))" -Encoding ascii
Write-Host "Package: $zip"
Write-Host "SHA-256: $checksum"
Write-Host "Included $($files.Count) files; no preferences, diagnostics or user projects. No upload performed."
