param(
    [string]$Configuration = "Debug",
    [string]$Url = "https://www.amazon.com/",
    [int]$TimeoutMilliseconds = 5000
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$debugger = Join-Path $root "$Configuration\ncompass-debug.exe"
$output = Join-Path $env:TEMP "ncompass-http-$PID.html"
$timeoutOutput = Join-Path $env:TEMP "ncompass-http-timeout-$PID"

try {
    if (-not (Test-Path $debugger)) {
        throw "Native harness not found: $debugger"
    }

    $started = [Diagnostics.Stopwatch]::StartNew()
    $metadata = & $debugger --fetch $Url $output $TimeoutMilliseconds
    $exitCode = $LASTEXITCODE
    $started.Stop()

    if ($started.ElapsedMilliseconds -gt ($TimeoutMilliseconds + 1000)) {
        throw "Hard timeout was exceeded: $($started.ElapsedMilliseconds) ms"
    }
    if ($exitCode -ne 0) {
        throw "Fetch failed with exit code $exitCode"
    }
    if (-not (Test-Path $output)) {
        throw "Fetch did not create an output file"
    }

    $bytes = [IO.File]::ReadAllBytes($output)
    if ($bytes.Length -lt 1000) {
        throw "Response was unexpectedly small: $($bytes.Length) bytes"
    }
    if ($bytes[0] -eq 0x1f -and $bytes[1] -eq 0x8b) {
        throw "Response still contains gzip-compressed data"
    }

    $prefix = [Text.Encoding]::ASCII.GetString(
        $bytes, 0, [Math]::Min($bytes.Length, 4096))
    if ($prefix -notmatch "(?i)<!doctype|<html") {
        throw "Decoded response does not look like HTML"
    }

    $metadata
    Write-Host "PASS: decoded HTTP response in $($started.ElapsedMilliseconds) ms"

    $dumpDirectory = Join-Path $root "$Configuration\dumps"
    $before = @(Get-ChildItem $dumpDirectory -Filter *.dmp `
        -ErrorAction SilentlyContinue)
    & $debugger --fetch "ncompass-test://hang" $timeoutOutput 250 | Out-Null
    if ($LASTEXITCODE -ne 124) {
        throw "Timeout self-test returned $LASTEXITCODE instead of 124"
    }
    if (Test-Path $timeoutOutput) {
        throw "Timeout self-test should not have produced an output file"
    }
    $after = @(Get-ChildItem $dumpDirectory -Filter *.dmp `
        -ErrorAction SilentlyContinue)
    $newDump = $after | Where-Object {
        $before.Name -notcontains $_.Name } | Select-Object -Last 1
    if (-not $newDump) {
        throw "Timeout self-test did not create a dump"
    }
    $stackPath = "$($newDump.FullName).stack.txt"
    if (-not (Test-Path $stackPath)) {
        throw "Timeout self-test did not create a stack trace"
    }
    if ((Get-Item $stackPath).Length -eq 0) {
        throw "Timeout stack trace is empty"
    }
    Write-Host "PASS: hard timeout captured dump and stack trace"
}
finally {
    Remove-Item $output -Force -ErrorAction SilentlyContinue
    Remove-Item $timeoutOutput -Force -ErrorAction SilentlyContinue
}

exit 0
