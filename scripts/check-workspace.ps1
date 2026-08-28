param(
    [Parameter(Mandatory = $true)]
    [string]$ExpectedBranch
)

$scriptDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path
$repositoryRoot = Split-Path -Parent $scriptDirectory

Push-Location $repositoryRoot
try {
    $actualBranch = (git branch --show-current).Trim()
    $remoteOutput = git remote get-url origin
    $porcelainStatus = git status --porcelain

    Write-Host "Repository: $repositoryRoot"
    Write-Host "Origin: $remoteOutput"
    Write-Host "Current branch: $actualBranch"

    if ($remoteOutput -ne "https://github.com/wuwingyue2006-droid/B5CacheVisualizer.git") {
        throw "Origin does not point to the team repository. Stop and contact the leader."
    }
    if ($actualBranch -ne $ExpectedBranch) {
        throw "Expected branch '$ExpectedBranch', but current branch is '$actualBranch'. Stop before editing."
    }
    if ($actualBranch -eq "main" -or $actualBranch -eq "dev") {
        throw "Members must not develop directly on main or dev."
    }
    if ($porcelainStatus) {
        Write-Warning "The working tree contains changes. Review git status before switching or pulling."
        git status --short
    } else {
        Write-Host "Working tree is clean."
    }
} finally {
    Pop-Location
}
