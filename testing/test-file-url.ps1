param(
    [string]$Configuration = "Debug"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$debugger = Join-Path $root "$Configuration\ncompass-debug.exe"
$fixture = Join-Path $root "tests\parser\basic.html"
$testDirectory = Join-Path $env:TEMP "ncompass file url $PID"
$testFile = Join-Path $testDirectory "basic # fixture.html"
$screenshot = Join-Path $testDirectory "render.bmp"

try {
    New-Item -ItemType Directory -Path $testDirectory | Out-Null
    Copy-Item $fixture $testFile
    $url = [Uri]::new($testFile).AbsoluteUri

    & $debugger --wrap -- $debugger $url "Parser Basic Test" 8 $screenshot
    if ($LASTEXITCODE -ne 0) {
        throw "Standard file URL failed with exit code $LASTEXITCODE`: $url"
    }
    if (-not (Test-Path $screenshot)) {
        throw "Standard file URL did not produce a screenshot"
    }

    Write-Host "PASS: $url"
}
finally {
    Remove-Item $testFile -Force -ErrorAction SilentlyContinue
    Remove-Item $screenshot -Force -ErrorAction SilentlyContinue
    Remove-Item $testDirectory -Force -ErrorAction SilentlyContinue
}
