param(
    [string]$Configuration = "Debug"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$wrapper = Join-Path $root "$Configuration\ncompass-job.exe"
$fixtures = Join-Path $root "tests\parser"

if (-not (Test-Path $wrapper)) {
    throw "Job wrapper not found: $wrapper"
}

$listener = [System.Net.Sockets.TcpListener]::new(
    [System.Net.IPAddress]::Loopback, 0)
$listener.Start()
$port = ([System.Net.IPEndPoint]$listener.LocalEndpoint).Port
$listener.Stop()

$startInfo = [System.Diagnostics.ProcessStartInfo]::new()
$startInfo.FileName = $wrapper
$startInfo.UseShellExecute = $false
foreach ($argument in @(
    "--", "python", "-m", "http.server", "$port",
    "--bind", "127.0.0.1", "--directory", $fixtures)) {
    $startInfo.ArgumentList.Add($argument)
}

$process = [System.Diagnostics.Process]::Start($startInfo)
$serverProcessId = $null
try {
    for ($attempt = 0; $attempt -lt 50; $attempt++) {
        Start-Sleep -Milliseconds 100
        $connection = Get-NetTCPConnection -LocalPort $port `
            -State Listen -ErrorAction SilentlyContinue
        if ($connection) {
            $serverProcessId = $connection.OwningProcess
            break
        }
    }

    if (-not $serverProcessId) {
        throw "Wrapped HTTP server did not start"
    }

    $response = Invoke-WebRequest -UseBasicParsing `
        "http://127.0.0.1:$port/basic.html"
    if ($response.StatusCode -ne 200) {
        throw "Wrapped HTTP server returned $($response.StatusCode)"
    }

    Stop-Process -Id $process.Id
    $process.WaitForExit(5000) | Out-Null

    for ($attempt = 0; $attempt -lt 50; $attempt++) {
        Start-Sleep -Milliseconds 100
        if (-not (Get-NetTCPConnection -LocalPort $port `
            -State Listen -ErrorAction SilentlyContinue)) {
            break
        }
    }

    if (Get-NetTCPConnection -LocalPort $port `
        -State Listen -ErrorAction SilentlyContinue) {
        throw "HTTP server remained after the wrapper exited"
    }
    if (Get-Process -Id $serverProcessId -ErrorAction SilentlyContinue) {
        throw "HTTP server process remained after the wrapper exited"
    }

    Write-Host "PASS: closing the job wrapper removed its process tree"
}
finally {
    if (-not $process.HasExited) {
        Stop-Process -Id $process.Id
    }
    if ($serverProcessId -and
        (Get-Process -Id $serverProcessId -ErrorAction SilentlyContinue)) {
        Stop-Process -Id $serverProcessId
    }
}
