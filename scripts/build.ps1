param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",

    [ValidateSet("x64", "Win32")]
    [string]$Platform = "x64"
)

$scriptDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path
$repositoryRoot = Split-Path -Parent $scriptDirectory
$solutionPath = Join-Path $repositoryRoot "B5CacheVisualizer.sln"
$vswherePath = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"

if (-not (Test-Path -LiteralPath $vswherePath)) {
    throw "Visual Studio Installer (vswhere.exe) was not found. Install Visual Studio 2022 with Desktop development with C++."
}

$visualStudioPath = & $vswherePath -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $visualStudioPath) {
    throw "Visual Studio 2022 C++ build tools were not found."
}

$msbuildPath = Join-Path $visualStudioPath "MSBuild\Current\Bin\MSBuild.exe"
if (-not (Test-Path -LiteralPath $msbuildPath)) {
    throw "MSBuild was not found at: $msbuildPath"
}

Write-Host "Building $solutionPath ($Configuration|$Platform)..."
& $msbuildPath $solutionPath /m /t:Build "/p:Configuration=$Configuration" "/p:Platform=$Platform"
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Write-Host "Build completed successfully."
