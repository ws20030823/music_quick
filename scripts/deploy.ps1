# 準備發佈目錄 dist\MusicQuick（供 Inno Setup 或手動分發使用）
# 用法：在 PowerShell 中執行
#   .\scripts\deploy.ps1
# 可選參數：
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
    throw "找不到 Release 構建產物：$Exe`n請先在 Qt Creator 中以 Release 模式編譯。"
}
if (-not (Test-Path $Windeployqt)) {
    throw "找不到 windeployqt：$Windeployqt`n請用 -QtBin 指定正確的 Qt bin 目錄。"
}
if (-not (Test-Path $ModuleDir)) {
    throw "找不到 QML 模組目錄：$ModuleDir"
}

Write-Host "清理並建立 $DistDir ..."
if (Test-Path $DistDir) {
    Remove-Item -Recurse -Force $DistDir
}
New-Item -ItemType Directory -Path $DistDir | Out-Null

Write-Host "複製 MusicQuick.exe ..."
Copy-Item $Exe $DistDir

Write-Host "執行 windeployqt ..."
Push-Location $DistDir
& $Windeployqt MusicQuick.exe --release --qmldir $QmlDir --compiler-runtime
if ($LASTEXITCODE -ne 0) {
    Pop-Location
    throw "windeployqt 失敗，退出碼 $LASTEXITCODE"
}
Pop-Location

Write-Host "複製 MusicQuick QML 模組 ..."
Copy-Item -Recurse -Force $ModuleDir (Join-Path $DistDir "MusicQuick")

Write-Host ""
Write-Host "完成。發佈目錄：$DistDir"
Write-Host "下一步：用 Inno Setup 編譯 installer\MusicQuick.iss"
