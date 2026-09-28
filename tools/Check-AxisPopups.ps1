# Checks the popup dialogs of Axis X for clipped labels.
# Opens each popup (menu command, or a page button via WM_APP+99), lets Axis check it
# (WM_APP+98 -> %TEMP%\axis_labels_popup.txt) and closes it with WM_CLOSE (= cancel, nothing
# is executed). No keystrokes are sent.
# Usage: .\Check-AxisPopups.ps1 [-Only Einstellungen]
param([string]$Only = '')

$res = Get-Content (Join-Path $PSScriptRoot '..\AxisX\resource.h')
function Id([string]$name) {
    $m = $res | Select-String -Pattern "^#define\s+$name\s+(\d+)" | Select-Object -First 1
    if (-not $m) { throw "ID $name nicht gefunden" }
    [int]$m.Matches[0].Groups[1].Value
}

Add-Type @'
using System; using System.Runtime.InteropServices; using System.Text; using System.Collections.Generic;
public static class AxisPop {
    public delegate bool P(IntPtr h, IntPtr l);
    [DllImport("user32.dll")] static extern bool EnumWindows(P cb, IntPtr l);
    [DllImport("user32.dll")] static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
    [DllImport("user32.dll")] static extern int GetClassName(IntPtr h, StringBuilder s, int n);
    [DllImport("user32.dll")] static extern bool GetWindowRect(IntPtr h, out R r);
    [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
    [DllImport("user32.dll")] public static extern IntPtr SendMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
    [StructLayout(LayoutKind.Sequential)] public struct R { public int L, T, Ri, B; }
    // sichtbare Dialogfenster des Prozesses ausser Hauptfenster und flacher Mini-Leiste
    public static List<IntPtr> Popups(uint pid, IntPtr main) {
        var list = new List<IntPtr>();
        EnumWindows((h, l) => {
            uint p; GetWindowThreadProcessId(h, out p);
            if (p != pid || h == main || !IsWindowVisible(h)) return true;
            var sb = new StringBuilder(64); GetClassName(h, sb, 64);
            if (sb.ToString() != "#32770") return true;
            R r; GetWindowRect(h, out r);
            if (r.B - r.T < 60) return true;
            list.Add(h); return true; }, IntPtr.Zero);
        return list;
    }
}
'@

$p = Get-Process AxisX -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1
if (-not $p) { throw 'Axis X laeuft nicht' }
$main = $p.MainWindowHandle
$WM_COMMAND = 0x0111; $WM_CLOSE = 0x0010; $WM_APP = 0x8000

# page index = AddPage order in AxisX.cpp
$cases = @(
    @{ Name = 'Einstellungen: Allgemein';   Cmd = 'ID_SETTINGS_GENERAL' },
    @{ Name = 'Einstellungen: Dateipfade';  Cmd = 'ID_SETTINGS_FILEPATHS' },
    @{ Name = 'Einstellungen: Items';       Cmd = 'ID_SETTINGS_ITEMTAB' },
    @{ Name = 'Einstellungen: Reisen';      Cmd = 'ID_SETTINGS_TRAVELTAB' },
    @{ Name = 'Einstellungen: Spawns';      Cmd = 'ID_SETTINGS_SPAWNTAB' },
    @{ Name = 'Einstellungen: MUL-Pfade';   Cmd = 'ID_SETTINGS_OVERRIDEPATHS' },
    @{ Name = 'Profile';                    Cmd = 'ID_PROFILES_OPTION' },
    @{ Name = 'Ueber Axis X';               Cmd = 'ID_HELP_ABOUTAXISX' },
    @{ Name = 'Alle Farben';                Page = 6; Btn = 'IDC_ITPALETTE' },
    @{ Name = 'Tuer-Assistent';             Page = 6; Btn = 'IDC_OPENDOORWIZ' },
    @{ Name = 'Licht-Assistent';            Page = 6; Btn = 'IDC_OPENLIGHTWIZ' },
    @{ Name = 'Weltkarte';                  Page = 2; Btn = 'IDC_WORLD' },
    @{ Name = 'Ziel speichern';             Page = 2; Btn = 'IDC_ADDLOC' },
    @{ Name = 'Remote-Konsole (Login)';     Page = 1; Btn = 'IDC_REMCONSOLE' }
)

function Close-Popups {
    for ($i = 0; $i -lt 4; $i++) {
        $open = [AxisPop]::Popups([uint32]$p.Id, $main)
        if ($open.Count -eq 0) { return }
        foreach ($w in $open) { [AxisPop]::PostMessage($w, $WM_CLOSE, [IntPtr]::Zero, [IntPtr]::Zero) | Out-Null }
        Start-Sleep -Milliseconds 600
    }
}

$total = 0
foreach ($c in $cases) {
    if ($Only -and $c.Name -notlike "*$Only*") { continue }
    Close-Popups
    if ($c.Cmd) {
        [AxisPop]::PostMessage($main, $WM_COMMAND, [IntPtr](Id $c.Cmd), [IntPtr]::Zero) | Out-Null
    } else {
        [AxisPop]::PostMessage($main, $WM_APP + 99, [IntPtr]$c.Page, [IntPtr](Id $c.Btn)) | Out-Null
    }
    Start-Sleep -Milliseconds 1500
    $open = [AxisPop]::Popups([uint32]$p.Id, $main)
    if ($open.Count -eq 0) { "== $($c.Name): kein Fenster aufgegangen"; continue }
    $n = [int][AxisPop]::SendMessage($main, $WM_APP + 98, [IntPtr]::Zero, [IntPtr]::Zero)
    $total += $n
    "== $($c.Name)"
    Get-Content (Join-Path $env:TEMP 'axis_labels_popup.txt') -Encoding Default | Where-Object { $_ -ne '' } | ForEach-Object { "   $_" }
}
Close-Popups
"Gesamt zu knapp: $total"
