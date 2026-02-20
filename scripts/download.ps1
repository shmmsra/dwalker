$ErrorActionPreference = "Stop"

$Repo = "shmmsra/dwalker"
$AppName = "dwalker.exe"
$LatestReleaseUrl = "https://github.com/$Repo/releases/latest/download/dwalker.exe"

Write-Host "Downloading DWalker from latest release..."
$OutPath = Join-Path -Path $PWD -ChildPath $AppName

try {
    Invoke-WebRequest -Uri $LatestReleaseUrl -OutFile $OutPath
    Write-Host "Successfully downloaded DWalker to $OutPath"
} catch {
    Write-Error "Failed to download: $_"
    exit 1
}
