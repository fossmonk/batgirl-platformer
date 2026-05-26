[CmdletBinding()]
param (
    [Parameter(Mandatory = $false, HelpMessage = "The root folder for installation. Defaults to .\tools\llvm-mingw")]
    [string]$DestinationFolder = "",

    [Parameter(Mandatory = $false, HelpMessage = "The specific runtime to target (ucrt or msvcrt). Default is ucrt.")]
    [ValidateSet("ucrt", "msvcrt")]
    [string]$Runtime = "ucrt"
)

# 1. Path Setup: Handle empty $PSScriptRoot environments safely
if ([string]::IsNullOrWhitespace($DestinationFolder)) {
    $basePath = if ([string]::IsNullOrWhitespace($PSScriptRoot)) { (Get-Location).Path } else { $PSScriptRoot }
    $DestinationFolder = Join-Path $basePath "tools\llvm-mingw"
}

$rootToolsDir = Split-Path -Path $DestinationFolder -Parent
$makeBinDir = Join-Path $toolsMakePath "bin"

Write-Host "Targeting Toolchain: $DestinationFolder" -ForegroundColor Cyan

# Ensure directories exist
if (-not (Test-Path $DestinationFolder)) { New-Item -ItemType Directory -Path $DestinationFolder -Force | Out-Null }
if (-not (Test-Path $toolsMakePath)) { New-Item -ItemType Directory -Path $toolsMakePath -Force | Out-Null }

# -------------------------------------------------------------------------
# Download and Extract LLVM-MinGW Toolchain
# -------------------------------------------------------------------------
Write-Host "`n=== Installing LLVM-MinGW Toolchain ===" -ForegroundColor Magenta

$repoUrl = "https://api.github.com/repos/mstorsjo/llvm-mingw/releases/latest"
$downloadUrl = $null
$fileName = "llvm-mingw-latest-$Runtime-x86_64.zip"

Write-Host "Attempting GitHub API query..." -ForegroundColor Cyan
try {
    $releaseInfo = Invoke-RestMethod -Uri $repoUrl -Method Get -Headers @{"User-Agent" = "PowerShell-Downloader"} -TimeoutSec 10
    $asset = $releaseInfo.assets | Where-Object { $_.name -like "*-$Runtime-x86_64.zip" } | Select-Object -First 1
    if ($asset) {
        $downloadUrl = $asset.browser_download_url
        $fileName = $asset.name
        Write-Host "API Resolution Successful: Found $fileName" -ForegroundColor Green
    }
} catch {
    Write-Host "GitHub API call failed or rate-limited. Falling back to explicit redirect parsing..." -ForegroundColor Yellow
}

if (-not $downloadUrl) {
    try {
        $webUrl = "https://github.com/mstorsjo/llvm-mingw/releases/latest"
        $request = [System.Net.WebRequest]::Create($webUrl)
        $request.AllowAutoRedirect = $false
        $response = $request.GetResponse()
        $redirectUrl = $response.Headers["Location"]
        $response.Close()

        if ($redirectUrl) {
            $version = Split-Path -Path $redirectUrl -Leaf
            $downloadUrl = "https://github.com/mstorsjo/llvm-mingw/releases/download/$version/llvm-mingw-$version-$Runtime-x86_64.zip"
            $fileName = "llvm-mingw-$version-$Runtime-x86_64.zip"
            Write-Host "Fallback Resolution Successful: Found $fileName" -ForegroundColor Green
        }
    } catch {
        Write-Error "Both GitHub API and Web parsing approaches failed. Verify your internet connection."
        return
    }
}

$tempZipPath = Join-Path $env:TEMP $fileName

Write-Host "Downloading toolchain from: $downloadUrl" -ForegroundColor Gray
try {
    Invoke-WebRequest -Uri $downloadUrl -OutFile $tempZipPath -UserAgent "PowerShell-Downloader" -TimeoutSec 300
} catch {
    Write-Error "Download payload execution failed. Details: $_"
    return
}

$tempExtractionPath = Join-Path $env:TEMP "llvm_mingw_extraction_temp"
if (Test-Path $tempExtractionPath) { Remove-Item $tempExtractionPath -Recurse -Force -ErrorAction SilentlyContinue }

Write-Host "Extracting archive..." -ForegroundColor Yellow
Expand-Archive -Path $tempZipPath -DestinationPath $tempExtractionPath -Force

$innerFolder = Get-ChildItem -Path $tempExtractionPath -Directory | Select-Object -First 1
if ($innerFolder) {
    Write-Host "Moving binaries to final destination..." -ForegroundColor Cyan
    Get-ChildItem -Path $innerFolder.FullName | ForEach-Object {
        Move-Item -Path $_.FullName -Destination $DestinationFolder -Force
    }
}

# -------------------------------------------------------------------------
# Cleanup
# -------------------------------------------------------------------------
Write-Host "`nCleaning up..." -ForegroundColor Gray
[GC]::Collect()
[GC]::WaitForPendingFinalizers()

if (Test-Path $tempZipPath) { Remove-Item $tempZipPath -Force -ErrorAction SilentlyContinue }
if (Test-Path $tempExtractionPath) {
    $retryCount = 0
    while ((Test-Path $tempExtractionPath) -and $retryCount -lt 5) {
        try { Remove-Item $tempExtractionPath -Recurse -Force -ErrorAction Stop }
        catch { $retryCount++; Start-Sleep -Seconds 1 }
    }
}

Write-Host "`nSetup Complete!" -ForegroundColor Green
