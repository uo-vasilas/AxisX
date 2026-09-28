# Collects all German UI texts:
#  - dialog texts (CAPTION, controls) and the string table in AxisX.rc
#  - AXT("...") in the code
#  - entries of the tables that are translated at runtime via AxisTr()
# Output: one line per text (line breaks as \n), without duplicates.
# With -Table <file> only texts missing from that table are listed.
param([string]$Table = '')

$enc = [Text.Encoding]::GetEncoding(1252)
$root = Join-Path $PSScriptRoot '..\AxisX'
$lit = '"(?:[^"\\]|\\.)*"(?:\s*"(?:[^"\\]|\\.)*")*'

function Unescape-C([string]$s) {
	# join "a" "b", then resolve C escapes
	$parts = [regex]::Matches($s, '"((?:[^"\\]|\\.)*)"') | ForEach-Object { $_.Groups[1].Value }
	$t = ($parts -join '')
	$t = [regex]::Replace($t, '\\x([0-9A-Fa-f]{2})', { param($m) [string][char][Convert]::ToInt32($m.Groups[1].Value, 16) })
	$t = $t.Replace('\"', '"').Replace('\\', '\').Replace('\t', "`t")
	return $t   # \n stays as the two characters \n (as in the table)
}
function Unescape-Rc([string]$s) { return $s.Substring(1, $s.Length - 2).Replace('""', '"') }

$texts = New-Object System.Collections.Generic.List[string]
$seen = @{}
function Add-Text([string]$t) {
	if ($t -match '^\s*$' -or $t -notmatch '[A-Za-z]') { return }
	if (-not $seen.ContainsKey($t)) { $seen[$t] = 1; $texts.Add($t) }
}

# 1) .rc: dialog blocks and string table only
$rc = [IO.File]::ReadAllText((Join-Path $root 'AxisX.rc'), $enc)
foreach ($m in [regex]::Matches($rc, '(?ms)^\w+ DIALOGEX.*?^END')) {
	foreach ($line in ($m.Value -split "`r?`n")) {
		if ($line -match '^\s*(CAPTION|LTEXT|RTEXT|CTEXT|PUSHBUTTON|DEFPUSHBUTTON|GROUPBOX|CONTROL)\s+("(?:[^"]|"")*")') {
			$kind = $Matches[1]; $str = $Matches[2]
			if ($kind -eq 'CONTROL' -and $line -notmatch 'BS_AUTO|BS_PUSH|"Button"') { continue }
			Add-Text (Unescape-Rc $str)
		}
	}
}
foreach ($m in [regex]::Matches($rc, '(?ms)^STRINGTABLE.*?^END')) {
	foreach ($s in [regex]::Matches($m.Value, '(?m)^\s*IDS_\w+\s+("(?:[^"]|"")*")')) { Add-Text (Unescape-Rc $s.Groups[1].Value) }
}

# 2) AXT("...") in the code
foreach ($f in Get-ChildItem (Join-Path $root '*.cpp')) {
	$src = [IO.File]::ReadAllText($f.FullName, $enc)
	foreach ($m in [regex]::Matches($src, "AXT\(($lit)\)")) { Add-Text (Unescape-C $m.Groups[1].Value) }
}

# 3) runtime tables (translated via AxisTr): all literals in these initializers
$tables = @(
	@{ f = 'AxisXDlg.cpp'; p = 'g_axisFooterActions\[\]\s*=\s*\{(.*?)\};' },
	@{ f = 'AxisXDlg.cpp'; p = 'g_axisNavGroupNames\[\]\s*=\s*\{(.*?)\};' },
	@{ f = 'DashboardTab.cpp'; p = 'g_dashQuickActions\[\]\s*=\s*\{(.*?)\};' },
	@{ f = 'GeneralTab.cpp'; p = 'aTips\[\]\s*=\s*\{(.*?)\};' },
	@{ f = 'ItemTab.cpp'; p = 'aFlags\[\]\s*=\s*\{(.*?)\};' },
	@{ f = 'PlayerTweakTab.cpp'; p = 'aKinds\[\]\s*=\s*\{(.*?)\};' },
	@{ f = 'PlayerTweakTab.cpp'; p = 'aHints\[\]\s*=\s*\{(.*?)\};' },
	@{ f = 'Settings_General.cpp'; p = 'aTabs\[\]\s*=\s*\{(.*?)\};' },
	@{ f = 'CommandsTab.cpp'; p = 'aExamples\[\]\s*=\s*\{(.*?)\};' }
)
foreach ($tb in $tables) {
	$src = [IO.File]::ReadAllText((Join-Path $root $tb.f), $enc)
	$m = [regex]::Match($src, $tb.p, 'Singleline')
	if (-not $m.Success) { Write-Warning "Tabelle nicht gefunden: $($tb.f) $($tb.p)"; continue }
	foreach ($s in [regex]::Matches($m.Groups[1].Value, $lit)) {
		$t = Unescape-C $s.Value
		if ($t -match '^\.' -or $t -match '^[a-z]+( [\w<>\[\].-]+)*$' -and $t -notmatch '[\xC4\xD6\xDC\xE4\xF6\xFC\xDF]' -and $t -notmatch ' - ') { continue }   # skip plain commands (".where", "sector.snow")
		Add-Text $t
	}
}

if ($Table -ne '' -and (Test-Path $Table)) {
	$have = @{}
	foreach ($l in [IO.File]::ReadAllLines($Table, $enc)) { if ($l -match "^([^#][^\t]*)\t") { $have[$Matches[1]] = 1 } }
	$texts | Where-Object { -not $have.ContainsKey($_) }
} else { $texts }
