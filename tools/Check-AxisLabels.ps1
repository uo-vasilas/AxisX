# Checks all pages of Axis X for clipped labels.
# Optionally sets a window size (or maximizes), triggers the check (WM_APP+97 to the main window)
# and prints %TEMP%\axis_labels.txt.
# Usage: .\Check-AxisLabels.ps1 [-Width 1281 -Height 754] [-Maximize]
param(
    [int]$Width = 0,
    [int]$Height = 0,
    [switch]$Maximize
)
Add-Type @'
using System; using System.Runtime.InteropServices;
public static class AxisChk {
    [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
    [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h, IntPtr a, int x, int y, int cx, int cy, uint f);
    [DllImport("user32.dll")] public static extern IntPtr SendMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
}
'@
$p = Get-Process AxisX -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowHandle -ne 0 } | Select-Object -First 1
if (-not $p) { throw 'Axis X laeuft nicht (oder ist minimiert)' }
$h = $p.MainWindowHandle
if ($Maximize) {
    [AxisChk]::ShowWindow($h, 3) | Out-Null            # SW_MAXIMIZE
} elseif ($Width -gt 0 -and $Height -gt 0) {
    [AxisChk]::ShowWindow($h, 9) | Out-Null            # SW_RESTORE
    [AxisChk]::SetWindowPos($h, [IntPtr]::Zero, 0, 0, $Width, $Height, 0x0016) | Out-Null   # NOMOVE|NOZORDER|NOACTIVATE
}
Start-Sleep -Milliseconds 500
$n = [AxisChk]::SendMessage($h, 0x8000 + 97, [IntPtr]::Zero, [IntPtr]::Zero)
Get-Content (Join-Path $env:TEMP 'axis_labels.txt') -Encoding Default
