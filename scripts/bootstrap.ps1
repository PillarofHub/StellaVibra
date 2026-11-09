$ErrorActionPreference = 'Stop'
$root = Split-Path $PSScriptRoot -Parent
$deps = Join-Path $root '.deps'
New-Item -ItemType Directory -Force $deps | Out-Null
function Download($url, $name, $sha256) {
    $path = Join-Path $deps $name
    if (-not (Test-Path $path)) {
        & curl.exe -L --fail --retry 2 $url -o $path
        if ($LASTEXITCODE) { throw "Download failed: $url" }
    }
    if ($sha256 -and (Get-FileHash $path -Algorithm SHA256).Hash -ne $sha256) { throw "Checksum mismatch: $name" }
}
$qtUrl = 'https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt6_683/qt6_683/qt.qt6.683.win64_msvc2022_64/6.8.3-0-202503201308qtbase-Windows-Windows_11_23H2-MSVC2022-Windows-Windows_11_23H2-X86_64.7z'
Download $qtUrl 'qtbase.7z' '41688269FAC0565DB956C66D9EECAE777D16197E0C02CD81B88640C1F5D73D3F'
Download 'https://www.lua.org/ftp/lua-5.4.8.tar.gz' 'lua-5.4.8.tar.gz' '4F18DDAE154E793E46EEAB727C59EF1C0C0C2B744E7B94219710D76F530629AE'
Download 'https://download.qt.io/archive/qt/6.8/6.8.3/submodules/qtbase-everywhere-src-6.8.3.tar.xz' 'qtbase-everywhere-src-6.8.3.tar.xz' '56001B905601BB9023D399F3BA780D7FA940F3E4861E496A7C490331F49E0B80'
& tar.exe -xf "$deps\qtbase.7z" -C $deps
if ($LASTEXITCODE) { throw 'Qt extraction failed' }
& tar.exe -xf "$deps\lua-5.4.8.tar.gz" -C $deps
if ($LASTEXITCODE) { throw 'Lua extraction failed' }
Write-Output 'Dependencies ready. Run scripts/build.ps1 -Deploy. No application is launched.'
