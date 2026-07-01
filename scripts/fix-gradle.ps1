# 首次构建 Android APK 时，Gradle 可能因网络超时下载失败。
# 用法：在构建失败后再运行一次本脚本，然后重新 Build Android APK。
param(
    [string]$Version = "9.3.1"
)

$ErrorActionPreference = "Stop"
$distRoot = Join-Path $env:USERPROFILE ".gradle\wrapper\dists\gradle-$Version-bin"
$hashDir = Get-ChildItem $distRoot -Directory -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $hashDir) {
    New-Item -ItemType Directory -Path (Join-Path $distRoot "manual") | Out-Null
    $hashDir = Get-ChildItem $distRoot -Directory | Select-Object -First 1
}
$zip = Join-Path $hashDir.FullName "gradle-$Version-bin.zip"
Remove-Item "$hashDir\*.lck", "$hashDir\*.part" -Force -ErrorAction SilentlyContinue

if ((Test-Path $zip) -and (Get-Item $zip).Length -gt 1MB) {
    Write-Host "Gradle 已在缓存：$zip"
    exit 0
}

$urls = @(
    "https://mirrors.cloud.tencent.com/gradle/gradle-$Version-bin.zip",
    "https://services.gradle.org/distributions/gradle-$Version-bin.zip"
)
foreach ($url in $urls) {
    try {
        Write-Host "下载 $url ..."
        Invoke-WebRequest -Uri $url -OutFile $zip -UseBasicParsing -TimeoutSec 600
        if ((Get-Item $zip).Length -gt 1MB) {
            Write-Host "完成：$zip"
            exit 0
        }
    } catch {
        Write-Host "失败：$_"
        Remove-Item $zip -Force -ErrorAction SilentlyContinue
    }
}
throw "Gradle 下载失败，请检查网络或手动下载 gradle-$Version-bin.zip 到 $zip"
