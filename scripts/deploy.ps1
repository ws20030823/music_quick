# Prepare dist\MusicQuick for Inno Setup or manual distribution.
# Usage (from repo root):
#   .\scripts\deploy.ps1
# Optional:
#   -QtBin "C:\Qt\6.11.1\mingw_64\bin"
#   -BuildDir "build\Desktop_Qt_6_11_1_MinGW_64_bit-Release"

param(
    [string]$QtBin = "C:\Qt\6.11.1\mingw_64\bin",
    [string]$BuildDir = "build\Desktop_Qt_6_11_1_MinGW_64_bit-Release"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$ReleaseDir = Join-Path $Root $BuildDir
$DistDir = Join-Path $Root "dist\MusicQuick"
$Exe = Join-Path $ReleaseDir "MusicQuick.exe"
$Windeployqt = Join-Path $QtBin "windeployqt.exe"
$QmlDir = Join-Path $Root "qml"
$ModuleDir = Join-Path $ReleaseDir "MusicQuick"

if (-not (Test-Path $Exe)) {
    throw "Release build not found: $Exe`nBuild MusicQuick in Qt Creator with Release first."
}
if (-not (Test-Path $Windeployqt)) {
    throw "windeployqt not found: $Windeployqt`nPass the correct Qt bin path with -QtBin."
}
if (-not (Test-Path $ModuleDir)) {
    throw "QML module directory not found: $ModuleDir"
}

Write-Host "Cleaning and creating $DistDir ..."
if (Test-Path $DistDir) {
    Remove-Item -Recurse -Force $DistDir
}
New-Item -ItemType Directory -Path $DistDir | Out-Null

Write-Host "Copying MusicQuick.exe ..."
Copy-Item $Exe $DistDir

Write-Host "Running windeployqt ..."
Push-Location $DistDir
& $Windeployqt MusicQuick.exe --release --qmldir $QmlDir --compiler-runtime
if ($LASTEXITCODE -ne 0) {
    Pop-Location
    throw "windeployqt failed with exit code $LASTEXITCODE"
}
Pop-Location

Write-Host "Copying MusicQuick QML module ..."
Copy-Item -Recurse -Force $ModuleDir (Join-Path $DistDir "MusicQuick")

# Marker for portable mode: settings/data live under <exe>/data/
# Inno Setup excludes this file so installed builds use AppData/Registry.
Write-Host "Writing portable marker ..."
New-Item -ItemType File -Path (Join-Path $DistDir "portable") -Force | Out-Null

Write-Host ""
Write-Host "Done. Dist folder: $DistDir"
Write-Host "Portable zip: keep the 'portable' file next to MusicQuick.exe"
Write-Host "Installer: compile installer\MusicQuick.iss (excludes 'portable')"
