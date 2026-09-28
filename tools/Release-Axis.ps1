# Builds an Axis X release: sets the version, builds AxisX.exe and the installer and,
# with -Publish, commits, tags and publishes a GitHub release with the installer.
# Axis X finds new releases through the update check (GitHub "latest release").
#
# Usage: .\Release-Axis.ps1 -Version 1.1 [-Makensis <path to makensis.exe>] [-Publish]
# Without -Publish it stops after the installer, so the build can be tested first.
# Release notes come from the "***Axis X <Version>***" section in release\changelog.txt.
param(
    [Parameter(Mandatory = $true)][ValidatePattern('^\d+\.\d+(\.\d+)?$')][string]$Version,
    [string]$Makensis,
    [switch]$Publish
)

$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$enc = [Text.Encoding]::GetEncoding(1252)
$tag = "v$Version"

# release notes from the changelog
$changelog = [IO.File]::ReadAllText((Join-Path $repo 'release\changelog.txt'), $enc)
$head = "***Axis X $Version***"
$start = $changelog.IndexOf($head)
if ($start -lt 0) { throw "release\changelog.txt has no section $head" }
$end = $changelog.IndexOf('***Axis X ', $start + $head.Length)
$section = if ($end -lt 0) { $changelog.Substring($start) } else { $changelog.Substring($start, $end - $start) }
$items = ($section -split "`r?`n") | Where-Object { $_ -match '^-' } | ForEach-Object { '- ' + $_.Substring(1).Trim() }
if (-not $items) { throw "section $head has no entries" }

if ($Publish) {
    # only the files this script writes itself may be modified (e.g. from a test run without -Publish)
    if (git -C $repo status --porcelain -- . ':!release/changelog.txt' ':!AxisX/hlp/AxisX.chm' ':!AxisX/AxisX.rc' ':!release/AxisX.exe') { throw 'Working tree is not clean - commit first' }
    if (git -C $repo tag --list $tag) { throw "Tag $tag already exists" }
}

# makensis: parameter, PATH or the default NSIS folder
if (-not $Makensis) {
    $cmd = Get-Command makensis.exe -ErrorAction SilentlyContinue
    $Makensis = if ($cmd) { $cmd.Source } else { "${env:ProgramFiles(x86)}\NSIS\makensis.exe" }
}
if (-not (Test-Path $Makensis)) { throw "makensis.exe not found - install NSIS 3 or pass -Makensis" }

# version resource
$parts = @($Version.Split('.') | ForEach-Object { [int]$_ })
while ($parts.Count -lt 4) { $parts += 0 }
$rcFile = Join-Path $repo 'AxisX\AxisX.rc'
$rc = [IO.File]::ReadAllText($rcFile, $enc)
$comma = $parts -join ','
$rc = $rc -replace '(?m)^(\s*FILEVERSION\s+)[\d,]+', "`${1}$comma"
$rc = $rc -replace '(?m)^(\s*PRODUCTVERSION\s+)[\d,]+', "`${1}$comma"
$rc = $rc -replace '(VALUE "FileVersion", ")[^"]*(")', "`${1}$Version`${2}"
$rc = $rc -replace '(VALUE "ProductVersion", ")[^"]*(")', "`${1}$Version`${2}"
[IO.File]::WriteAllText($rcFile, $rc, $enc)

# exe and installer
& (Join-Path $PSScriptRoot 'Build-Axis.ps1') -Install
git -C $repo checkout -- AxisX/hlp/AxisX.chm 2>$null
$built = (Get-Item (Join-Path $repo 'release\AxisX.exe')).VersionInfo.FileVersionRaw
if ("$($built.Major).$($built.Minor).$($built.Build)" -ne ($parts[0..2] -join '.')) { throw "AxisX.exe has version $built instead of $Version" }

Push-Location (Join-Path $repo 'Setup')
try { & $Makensis /V2 "/DAXIS_VERSION=$Version" AxisX.nsi; if ($LASTEXITCODE -ne 0) { throw 'makensis failed' } }
finally { Pop-Location }
$setup = Join-Path $repo "Setup\AxisX_Setup_$Version.exe"
if (-not (Test-Path $setup)) { throw "$setup was not created" }
"Installer: $setup"

if (-not $Publish) {
    "Test the installer, then run again with -Publish."
    return
}

git -C $repo add AxisX/AxisX.rc release/AxisX.exe release/changelog.txt
git -C $repo commit -q -m "Axis X $Version"
git -C $repo tag -a $tag -m "Axis X $Version"
git -C $repo push -q origin HEAD $tag

$notes = "**Install or update:** download ``AxisX_Setup_$Version.exe`` below and run it - or use ""Update now"" on the Axis X overview. Settings and server profiles are kept.`n`n## Changes`n" + ($items -join "`n")
$notesFile = Join-Path $env:TEMP "axisx_notes_$Version.md"
[IO.File]::WriteAllText($notesFile, $notes, (New-Object Text.UTF8Encoding $false))
gh release create $tag $setup -R uo-vasilas/AxisX --title "Axis X $Version" --notes-file $notesFile --latest
if ($LASTEXITCODE -ne 0) { throw 'gh release create failed' }
"Published: https://github.com/uo-vasilas/AxisX/releases/tag/$tag"
