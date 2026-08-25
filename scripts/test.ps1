param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Debug",

    [ValidateSet("x64", "Win32")]
    [string]$Platform = "x64"
)

$scriptDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path
$repositoryRoot = Split-Path -Parent $scriptDirectory
$buildScript = Join-Path $scriptDirectory "build.ps1"
$testExecutable = Join-Path $repositoryRoot "bin\$Platform\$Configuration\B5CacheCoreTests.exe"

& $buildScript -Configuration $Configuration -Platform $Platform
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

if (-not (Test-Path -LiteralPath $testExecutable)) {
    throw "Test executable was not generated: $testExecutable"
}

Write-Host "Running core tests..."
& $testExecutable
exit $LASTEXITCODE
