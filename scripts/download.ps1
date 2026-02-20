$ErrorActionPreference = "Stop"

$Repo = "YOUR-USERNAME/dwalker" # TODO: Update this to your actual GitHub username/repository
$AppName = "dwalker.exe"

Write-Host "Finding latest release..."
try {
    $LatestRelease = Invoke-RestMethod -Uri "https://api.github.com/repos/$Repo/releases/latest"
    # Filter for the .exe or a .zip artifact uploaded by the release workflow
    $LatestReleaseUrl = $LatestRelease.assets | Where-Object { $_.name -like "*dwalker*.exe" -or $_.name -like "*dwalker*.zip" } | Select-Object -ExpandProperty browser_download_url -First 1
} catch {
    Write-Error "Failed to fetch release info: $_"
    exit 1
}

if (-not $LatestReleaseUrl) {
    Write-Error "Error: Could not find a release for Windows."
    exit 1
}

Write-Host "Found release artifact: $LatestReleaseUrl"
Write-Host "Downloading into current directory..."

$OutPath = Join-Path -Path $PWD -ChildPath $AppName

try {
    if ($LatestReleaseUrl -match "\.zip$") {
        $TempZip = "$env:TEMP\dwalker_release.zip"
        Invoke-WebRequest -Uri $LatestReleaseUrl -OutFile $TempZip
        Write-Host "Extracting archive..."
        Expand-Archive -Path $TempZip -DestinationPath $PWD -Force
        Remove-Item -Path $TempZip
    } else {
        Invoke-WebRequest -Uri $LatestReleaseUrl -OutFile $OutPath
    }
    Write-Host "Successfully downloaded DWalker to $PWD"
} catch {
    Write-Error "Failed to download: $_"
    exit 1
}
