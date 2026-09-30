param(
    [string]$Configuration = "Debug",
    [ValidateSet("Sites", "Single")]
    [string]$Mode = "Sites",
    [string]$Manifest = "",
    [string]$OutputDirectory = "",
    [int]$TimeoutMilliseconds = 1000,
    [string]$Name = "",
    [string]$Url = "",
    [string]$ExpectedTitle = "",
    [ValidateSet("Off", "OnCrash", "Always")]
    [string]$PageHeap = "OnCrash"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$debugger = Join-Path $root "$Configuration\ncompass-debug.exe"
if (-not (Test-Path $debugger)) {
    throw "Native harness not found: $debugger"
}
if ($TimeoutMilliseconds -lt 1) {
    throw "TimeoutMilliseconds must be positive"
}

if ($Mode -eq "Single") {
    if (-not $Url -or -not $ExpectedTitle) {
        throw "Single mode requires Url and ExpectedTitle"
    }
    if (-not $OutputDirectory) {
        $OutputDirectory = Join-Path $env:TEMP "ncompass-single"
    }
    New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
    $screenshot = Join-Path $OutputDirectory "render.bmp"
    $timeoutSeconds = [Math]::Max(
        1, [Math]::Ceiling($TimeoutMilliseconds / 1000.0))
    & $debugger $Url $ExpectedTitle $timeoutSeconds $screenshot
    exit $LASTEXITCODE
}

if (-not $Manifest) {
    $Manifest = Join-Path $root "tests\sites.tsv"
}
if (-not $OutputDirectory) {
    $stamp = Get-Date -Format "yyyyMMdd-HHmmss"
    $OutputDirectory = Join-Path $env:TEMP "ncompass-sites-$stamp"
}
$pageHeapArgument = switch ($PageHeap) {
    "Off" { "off" }
    "Always" { "always" }
    default { "on-crash" }
}

& $debugger --sites $Manifest $OutputDirectory $TimeoutMilliseconds `
    $pageHeapArgument
exit $LASTEXITCODE
