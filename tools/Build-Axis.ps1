# Builds Axis X (Release|x64, VS 2026 toolset v145) and optionally installs the exe into Release\.
# A running Axis instance is only closed with -CloseRunning.
param([switch]$Install, [switch]$CloseRunning, [switch]$Start)

$ErrorActionPreference = 'Stop'
$repo = Resolve-Path (Join-Path $PSScriptRoot '..')
$out = Join-Path $env:TEMP 'axisx-build64\'
$msbuild = & "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find 'MSBuild\**\Bin\MSBuild.exe' | Select-Object -First 1

$log = & $msbuild (Join-Path $repo 'AxisX.sln') /p:Configuration=Release /p:Platform=x64 /p:PlatformToolset=v145 "/p:OutDir=$out" /m /nr:false /v:minimal /nologo 2>&1 | Out-String
$errors = ($log -split "`n") | Where-Object { $_ -match ' error ' }
if ($LASTEXITCODE -ne 0 -or $errors) { $errors | Select-Object -First 20; throw "Build fehlgeschlagen" }
"Build ok: $out" + "AxisX.exe"

if ($Install) {
    # only the instance started from this repo's Release folder - never an installed Axis
    $releaseExe = Join-Path $repo 'Release\AxisX.exe'
    $running = Get-Process AxisX -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $releaseExe }
    if ($running) {
        if (-not $CloseRunning) { throw "Axis X laeuft noch - schliessen oder -CloseRunning angeben" }
        $running | ForEach-Object { $_.CloseMainWindow() | Out-Null }
        # wait up to 5 s so Stop-Process does not hit a process that is already exiting
        for ($i = 0; $i -lt 25 -and (Get-Process AxisX -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $releaseExe }); $i++) { Start-Sleep -Milliseconds 200 }
        Get-Process AxisX -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $releaseExe } | ForEach-Object { try { $_ | Stop-Process -Force -ErrorAction Stop } catch {} }
        Start-Sleep 1
    }
    Copy-Item (Join-Path $out 'AxisX.exe') (Join-Path $repo 'Release\AxisX.exe') -Force
    "eingesetzt: Release\AxisX.exe"
}
if ($Start) {
    Start-Process (Join-Path $repo 'Release\AxisX.exe') -WorkingDirectory (Join-Path $repo 'Release')
    Start-Sleep 6
    "gestartet"
}
