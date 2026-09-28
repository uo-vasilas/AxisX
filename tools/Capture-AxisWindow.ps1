# Captures the Axis X window (even when covered), optionally after a click, command or text input.
# Usage: .\Capture-AxisWindow.ps1 -Out C:\tmp\page.png [-ClickX 60 -ClickY 200]
# Click coordinates are relative to the client area of the window.
param(
    [Parameter(Mandatory = $true)][string]$Out,
    [int]$ClickX = -1,
    [int]$ClickY = -1,
    [string]$ProcessName = 'AxisX',
    # only this process (when several Axis instances are running)
    [int]$ProcessId = 0,
    [int]$WaitMs = 700,
    # optional: set the text of a control (ID from resource.h), e.g. -SetTextId 2001 -SetText 'katana'
    [int]$SetTextId = -1,
    [string]$SetText = '',
    # capture another visible window of the process (popup dialog) instead of the main window
    [switch]$Popup,
    # click a button by control ID (posted BM_CLICK, does not block on modal dialogs)
    [int]$ClickId = -1,
    # copy from the screen instead of PrintWindow (for windows whose thread is busy)
    [switch]$ScreenCopy,
    # send a menu command to the main window (WM_COMMAND with this ID from resource.h)
    [int]$Command = -1
)

Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class AxisWin {
    [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
    [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
    [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
    [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref POINT p);
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
    [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
    [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint x, uint y, uint d, UIntPtr e);
    [DllImport("user32.dll")] public static extern IntPtr ChildWindowFromPointEx(IntPtr h, POINT p, uint flags);
    [DllImport("user32.dll")] public static extern bool MapWindowPoints(IntPtr from, IntPtr to, ref POINT p, uint n);
    [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint msg, IntPtr w, IntPtr l);
    // Klick als Fensternachricht an das tiefste Kindfenster unter dem Punkt - braucht weder
    // Vordergrund noch Maus (Windows verweigert SetForegroundWindow, waehrend jemand am PC arbeitet).
    [DllImport("user32.dll", CharSet = CharSet.Ansi)] public static extern IntPtr SendMessage(IntPtr h, uint msg, IntPtr w, string l);
    public delegate bool EnumProc(IntPtr h, IntPtr l);
    [DllImport("user32.dll")] public static extern bool EnumChildWindows(IntPtr h, EnumProc cb, IntPtr l);
    [DllImport("user32.dll")] public static extern int GetDlgCtrlID(IntPtr h);
    [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
    public static IntPtr FindCtrl(IntPtr top, int id) {
        IntPtr found = IntPtr.Zero;
        EnumChildWindows(top, (h, l) => { if (GetDlgCtrlID(h) == id && IsWindowVisible(h)) { found = h; return false; } return true; }, IntPtr.Zero);
        return found;
    }
    [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr l);
    [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
    public static IntPtr FindPopup(uint pid, IntPtr main) {
        IntPtr found = IntPtr.Zero;
        EnumWindows((h, l) => { uint p; GetWindowThreadProcessId(h, out p);
            if (p == pid && h != main && IsWindowVisible(h)) { RECT r; GetWindowRect(h, out r);
                if (r.Right - r.Left > 120 && r.Bottom - r.Top > 60) { found = h; return false; } }
            return true; }, IntPtr.Zero);
        return found;
    }
    public static void PostClick(IntPtr top, int x, int y) {
        IntPtr cur = top; POINT p; p.X = x; p.Y = y;
        while (true) {
            IntPtr child = ChildWindowFromPointEx(cur, p, 1 | 4); // CWP_SKIPINVISIBLE | CWP_SKIPTRANSPARENT
            if (child == IntPtr.Zero || child == cur) break;
            MapWindowPoints(cur, child, ref p, 1);
            cur = child;
        }
        IntPtr l = (IntPtr)((p.Y << 16) | (p.X & 0xFFFF));
        PostMessage(cur, 0x0201, (IntPtr)1, l); // WM_LBUTTONDOWN
        PostMessage(cur, 0x0202, IntPtr.Zero, l); // WM_LBUTTONUP
    }
}
'@

$p = Get-Process $ProcessName -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowHandle -ne 0 -and ($ProcessId -eq 0 -or $_.Id -eq $ProcessId) } | Select-Object -First 1
if (-not $p) { throw "Kein Fenster von $ProcessName gefunden" }
$h = $p.MainWindowHandle

if ($ClickX -ge 0 -and $ClickY -ge 0) {
    [AxisWin]::PostClick($h, $ClickX, $ClickY)

}
if ($Command -ge 0) {
    [AxisWin]::PostMessage($h, 0x0111, [IntPtr]$Command, [IntPtr]::Zero) | Out-Null
}
if ($ClickId -ge 0) {
    # with -Popup the button is searched in the open popup, otherwise in the main window
    $hc = $h
    if ($Popup) { $pc = [AxisWin]::FindPopup([uint32]$p.Id, $h); if ($pc -ne [IntPtr]::Zero) { $hc = $pc } }
    $c = [AxisWin]::FindCtrl($hc, $ClickId)
    if ($c -eq [IntPtr]::Zero) { throw "Control $ClickId nicht gefunden" }
    [AxisWin]::PostMessage($c, 0x00F5, [IntPtr]::Zero, [IntPtr]::Zero) | Out-Null
}
if ($SetTextId -ge 0) {
    Start-Sleep -Milliseconds 800
    $c = [AxisWin]::FindCtrl($h, $SetTextId)
    if ($c -eq [IntPtr]::Zero) { throw "Control $SetTextId nicht gefunden" }
    [AxisWin]::SendMessage($c, 0x000C, [IntPtr]::Zero, $SetText) | Out-Null   # WM_SETTEXT (triggers EN_CHANGE)
}
Start-Sleep -Milliseconds $WaitMs

if ($Popup) {
    $pp = [AxisWin]::FindPopup([uint32]$p.Id, $h)
    if ($pp -eq [IntPtr]::Zero) { throw 'Kein Popup-Fenster offen' }
    $h = $pp
}
$r = New-Object AxisWin+RECT
[AxisWin]::GetWindowRect($h, [ref]$r) | Out-Null
$w = $r.Right - $r.Left; $hgt = $r.Bottom - $r.Top
$bmp = New-Object System.Drawing.Bitmap $w, $hgt
$g = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $g.GetHdc()
if ($ScreenCopy) { $g.ReleaseHdc($hdc); $g.CopyFromScreen($r.Left, $r.Top, 0, 0, $bmp.Size); $hdc = $g.GetHdc() }
else { [AxisWin]::PrintWindow($h, $hdc, 2) | Out-Null }   # PW_RENDERFULLCONTENT
$g.ReleaseHdc($hdc); $g.Dispose()
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
"$Out ($w x $hgt)"
