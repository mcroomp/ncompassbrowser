param(
    [string]$Configuration = "Debug"
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$parser = Join-Path $root "$Configuration\ncompass-parse.exe"
$fixtures = Join-Path $root "tests\parser"

if (-not (Test-Path $parser)) {
    throw "Parser helper not found: $parser"
}

$chunkSizes = 1, 2, 7, 31, 4096

function ConvertTo-NormalizedParserJson($parsed) {
    $tags = [System.Collections.Generic.List[object]]::new()
    foreach ($tag in $parsed.tags) {
        if ($tag.type -eq "text" -and $tags.Count -gt 0) {
            $previous = $tags[$tags.Count - 1]
            if ($previous.type -eq "text" -and
                $previous.fontFlags -eq $tag.fontFlags -and
                $previous.offset + $previous.length -eq $tag.offset) {
                $previous.length += $tag.length
                $previous.text += $tag.text
                continue
            }
        }

        $copy = [ordered]@{}
        foreach ($property in $tag.PSObject.Properties) {
            $copy[$property.Name] = $property.Value
        }
        $tags.Add([pscustomobject]$copy)
    }

    [ordered]@{
        loadState = $parsed.loadState
        title = $parsed.title
        plainText = $parsed.plainText
        backgroundColor = $parsed.backgroundColor
        textColor = $parsed.textColor
        hotlinkColor = $parsed.hotlinkColor
        oldHotlinkColor = $parsed.oldHotlinkColor
        backgroundPicture = $parsed.backgroundPicture
        tags = $tags
    } | ConvertTo-Json -Depth 8 -Compress
}

foreach ($fixture in Get-ChildItem $fixtures -Filter *.html) {
    $baseline = $null
    foreach ($chunkSize in $chunkSizes) {
        $output = & $parser $fixture.FullName $chunkSize `
            "http://parser.test/pages/input.html"
        if ($LASTEXITCODE -ne 0) {
            throw "$($fixture.Name) failed with chunk size $chunkSize"
        }

        $parsed = $output | ConvertFrom-Json
        if ($parsed.loadState -ne 3) {
            throw "$($fixture.Name) did not complete with chunk size $chunkSize"
        }

        $json = ConvertTo-NormalizedParserJson $parsed
        if ($null -eq $baseline) {
            $baseline = $json
        }
        elseif ($json -cne $baseline) {
            throw "$($fixture.Name) changed when chunk size was $chunkSize"
        }
    }

    Write-Host "PASS: $($fixture.Name)"
}

$scriptResult = & $parser `
    (Join-Path $fixtures "script-noscript.html") 1 `
    "http://parser.test/pages/input.html" | ConvertFrom-Json

if ($scriptResult.plainText -notmatch "Fallback content" -or
    $scriptResult.plainText -match "Script must not render") {
    throw "script/noscript behavior is incorrect"
}

Write-Host "PASS: script content ignored and noscript fallback parsed"
