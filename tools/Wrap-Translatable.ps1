# Wraps German UI texts in C++ sources with AXT("...").
# Only clear display calls - Sphere commands (e.g. .Format("%sset ...")) are left alone.
# Usage: .\Wrap-Translatable.ps1 -Files ItemTab.cpp,SpawnTab.cpp  (relative to AxisX\)
param([string[]]$Files, [switch]$WhatIf)

$enc = [Text.Encoding]::GetEncoding(1252)
$lit = '"(?:[^"\\]|\\.)*"(?:\s*"(?:[^"\\]|\\.)*")*'   # "..." also as "a" "b"
$rules = @(
	@{ p = "AxisSetStatus\(_T\(($lit)\)"; r = 'AxisSetStatus(AXT($1)' },
	@{ p = "AxisSetStatus\(($lit)"; r = 'AxisSetStatus(AXT($1)' },
	@{ p = "AfxMessageBox\(($lit)"; r = 'AfxMessageBox(AXT($1)' },
	@{ p = "InsertColumn\((\s*\d+\s*),\s*_T\(($lit)\)"; r = 'InsertColumn($1, AXT($2)' },
	@{ p = "InsertColumn\((\s*\d+\s*),\s*($lit)"; r = 'InsertColumn($1, AXT($2)' },
	@{ p = "AddString\(_T\(($lit)\)\)"; r = 'AddString(AXT($1))' },
	@{ p = "AddString\(($lit)\)"; r = 'AddString(AXT($1))' },
	@{ p = "AddTool\(([^;]+?),\s*($lit)\)"; r = 'AddTool($1, AXT($2))' },
	@{ p = "SetDlgItemText\(([A-Z_]+),\s*_T\(($lit)\)\)"; r = 'SetDlgItemText($1, AXT($2))' },
	@{ p = "\.SetWindowText\(($lit)\)"; r = '.SetWindowText(AXT($1))' },
	@{ p = "(m_\w+)\.SendMessage\(EM_SETCUEBANNER,\s*TRUE,\s*\(LPARAM\)\s*L($lit)\)"; r = 'AxisSetCue($1.GetSafeHwnd(), $2)' },
	@{ p = "\b(csInfo|csMessage|csError|csMsg|csTitle|csMessageBox|csDetails|csSubtitle)\.Format\(_T\(($lit)\)"; r = '$1.Format(AXT($2)' },
	@{ p = "\b(csInfo|csMessage|csError|csMsg|csTitle|csMessageBox|csDetails|csSubtitle)\.Format\(($lit)"; r = '$1.Format(AXT($2)' },
	@{ p = "\b(csInfo|csDetails|csGreeting|csConn|csCount|csText)\s*(\+?=)\s*($lit)"; r = '$1 $2 AXT($3)' },
	@{ p = "Main->m_log\.Add\((\d),\s*($lit)"; r = 'Main->m_log.Add($1, AXT($2)' },
	@{ p = "m_log\.Add\((\d),\s*($lit)"; r = 'm_log.Add($1, AXT($2)' }
)
$total = 0
foreach ($file in $Files) {
	$path = Join-Path (Join-Path $PSScriptRoot '..\AxisX') $file
	$text = [IO.File]::ReadAllText($path, $enc)
	$orig = $text
	$n = 0
	foreach ($rule in $rules) {
		$m = [regex]::Matches($text, $rule.p)
		# skip places that are already wrapped (AXT( right before)
		$text = [regex]::Replace($text, $rule.p, { param($mm)
			if ($mm.Value -match 'AXT\(') { return $mm.Value }
			$script:n++
			return $mm.Result($rule.r)
		})
	}
	# turn AXT("") (empty texts) back into ""
	$text = $text.Replace('AXT("")', '""')
	if ($text -ne $orig) {
		if (-not $WhatIf) { [IO.File]::WriteAllText($path, $text, $enc) }
		"{0}: {1} Stellen" -f $file, $script:n
		$total += $script:n
	}
	$script:n = 0
}
"gesamt: $total"
