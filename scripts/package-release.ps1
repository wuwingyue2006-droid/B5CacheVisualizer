param(
    [switch]$SkipBuild
)

$scriptDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path
$repositoryRoot = Split-Path -Parent $scriptDirectory
$releaseExecutable = Join-Path $repositoryRoot "bin\x64\Release\B5CacheVisualizer.exe"
$distRoot = Join-Path $repositoryRoot "dist"
$stagingRoot = Join-Path $distRoot "B5CacheVisualizer-submit"
$zipPath = Join-Path $distRoot "B5CacheVisualizer-submit.zip"

function Assert-ChildPath {
    param(
        [Parameter(Mandatory = $true)][string]$Parent,
        [Parameter(Mandatory = $true)][string]$Child
    )

    $resolvedParent = [System.IO.Path]::GetFullPath($Parent).TrimEnd('\') + '\'
    $resolvedChild = [System.IO.Path]::GetFullPath($Child)
    if (-not $resolvedChild.StartsWith($resolvedParent, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to modify a path outside the delivery directory: $resolvedChild"
    }
}

if (-not $SkipBuild) {
    & (Join-Path $scriptDirectory "test.ps1") -Configuration Release -Platform x64
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }
}

if (-not (Test-Path -LiteralPath $releaseExecutable)) {
    throw "Release executable was not generated: $releaseExecutable"
}

New-Item -ItemType Directory -Force -Path $distRoot | Out-Null
Assert-ChildPath -Parent $distRoot -Child $stagingRoot
Assert-ChildPath -Parent $distRoot -Child $zipPath

if (Test-Path -LiteralPath $stagingRoot) {
    Remove-Item -LiteralPath $stagingRoot -Recurse -Force
}
if (Test-Path -LiteralPath $zipPath) {
    Remove-Item -LiteralPath $zipPath -Force
}

New-Item -ItemType Directory -Path $stagingRoot | Out-Null
New-Item -ItemType Directory -Path (Join-Path $stagingRoot "Release") | Out-Null

$rootFiles = @(".gitignore", "AGENTS.md", "B5CacheVisualizer.sln", "README.md")
foreach ($file in $rootFiles) {
    Copy-Item -LiteralPath (Join-Path $repositoryRoot $file) -Destination $stagingRoot
}

$sourceDirectories = @("B5CacheVisualizer", "B5CacheCoreTests", "src", "examples", "docs", "scripts")
foreach ($directory in $sourceDirectories) {
    Copy-Item -LiteralPath (Join-Path $repositoryRoot $directory) -Destination $stagingRoot -Recurse
}

Copy-Item -LiteralPath $releaseExecutable -Destination (Join-Path $stagingRoot "Release\B5CacheVisualizer.exe")

$forbidden = Get-ChildItem -LiteralPath $stagingRoot -Recurse -Force | Where-Object {
    $_.Name -in @(".git", ".vs", "obj", "Debug") -or
    $_.Extension -in @(".pdb", ".ilk", ".idb", ".tlog", ".log", ".user", ".suo")
}
if ($forbidden) {
    $names = ($forbidden | Select-Object -ExpandProperty FullName) -join [Environment]::NewLine
    throw "Forbidden delivery artifacts were found:$([Environment]::NewLine)$names"
}

Compress-Archive -LiteralPath $stagingRoot -DestinationPath $zipPath -CompressionLevel Optimal

$zip = Get-Item -LiteralPath $zipPath
$limitBytes = 25MB
if ($zip.Length -ge $limitBytes) {
    throw "Delivery ZIP is $([math]::Round($zip.Length / 1MB, 2)) MB, which exceeds the 25 MB limit."
}

Write-Host "Release package created successfully."
Write-Host "Folder: $stagingRoot"
Write-Host "ZIP: $zipPath"
Write-Host "ZIP size: $([math]::Round($zip.Length / 1MB, 2)) MB"
