# Tests the mini bar: minimizes Axis X via WM_SYSCOMMAND (no keystrokes), captures the bar
# and optionally clicks an icon (index from the left, 0-based).
# Usage: .\Test-AxisMiniBar.ps1 -Out C:\tmp\bar.png [-ClickIndex 3]
param(
    [Parameter(Mandatory = $true)][string]$Out,
    [int]$ClickIndex = -1,
    [switch]$NoMinimize
)
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
using System.Text;
public static class AxisBar {
    public delegate bool EnumProc(IntPtr h, IntPtr l);
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr l);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
    [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
    [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint f);
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
    public static IntPtr FindBar(uint pid, IntPtr main) {
        IntPtr found = IntPtr.Zero;
        EnumWindows((h, l) => {
            uint p; GetWindowThreadProcessId(h, out p);
            if (p != pid || h == main || !IsWindowVisible(h)) return true;
            var sb = new StringBuilder(64); GetClassName(h, sb, 64);
            if (sb.ToString() != "#32770") return true;
            RECT r; GetWindowRect(h, out r);
            if (r.Bottom - r.Top < 80) { found = h; return false; }   // die Leiste ist flach
            return true; }, IntPtr.Zero);
        return found;
    }
}
'@
# while the main window is hidden (minimized) MainWindowHandle is 0 - then only look for the bar
$p = Get-Process AxisX -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $p) { throw 'Axis X laeuft nicht' }
$main = $p.MainWindowHandle
if (-not $NoMinimize) {
    [AxisBar]::PostMessage($main, 0x0112, [IntPtr]0xF020, [IntPtr]::Zero) | Out-Null   # WM_SYSCOMMAND / SC_MINIMIZE
    Start-Sleep -Milliseconds 800
}
$bar = [AxisBar]::FindBar([uint32]$p.Id, $main)
if ($bar -eq [IntPtr]::Zero) { throw 'Mini-Leiste nicht sichtbar' }
$r = New-Object AxisBar+RECT
[AxisBar]::GetWindowRect($bar, [ref]$r) | Out-Null
$w = $r.Right - $r.Left; $hgt = $r.Bottom - $r.Top
$bmp = New-Object System.Drawing.Bitmap $w, $hgt
$g = [System.Drawing.Graphics]::FromImage($bmp); $hdc = $g.GetHdc()
[AxisBar]::PrintWindow($bar, $hdc, 2) | Out-Null
$g.ReleaseHdc($hdc); $g.Dispose(); $bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png); $bmp.Dispose()
"$Out ($w x $hgt) bei $($r.Left),$($r.Top)"
if ($ClickIndex -ge 0) {
    # cells: grip 14 + margin 4, then 34 px each; the restore button has 8 px extra spacing (see ToolBar.h)
    $x = 18 + $ClickIndex * 34 + 17; $y = 4 + 17
    $l = [IntPtr](($y -shl 16) -bor $x)
    [AxisBar]::PostMessage($bar, 0x0201, [IntPtr]1, $l) | Out-Null
    [AxisBar]::PostMessage($bar, 0x0202, [IntPtr]::Zero, $l) | Out-Null
    Start-Sleep -Milliseconds 800
    $p2 = Get-Process -Id $p.Id; $p2.Refresh()
    "Symbol $ClickIndex angeklickt; Hauptfenster sichtbar: $($p2.MainWindowHandle -ne 0); Leiste sichtbar: $([AxisBar]::IsWindowVisible($bar))"
}
