# Generates the Axis X icons (neutral, dark/gold like the UI):
#   Setup\Icons\AxisX.ico     installer icon (16/24/32/48/64/128/256)
#   Setup\Icons\UnAxisX.ico   uninstaller icon (same sizes, red minus badge)
#   Setup\Icons\AxisX.bmp     installer header image (150x57)
#   AxisX\res\AxisX.ico       program icon (only with -App)
# Usage: .\Make-AxisIcons.ps1 [-App] [-Preview <png>]
param([switch]$App, [string]$Preview = '')

Add-Type -AssemblyName System.Drawing
$root = Resolve-Path (Join-Path $PSScriptRoot '..')

$gold1 = [System.Drawing.Color]::FromArgb(255, 0xF2, 0xCC, 0x7E)
$gold2 = [System.Drawing.Color]::FromArgb(255, 0xC4, 0x92, 0x44)
$goldLine = [System.Drawing.Color]::FromArgb(255, 0xE0, 0xB5, 0x63)
$bgTop = [System.Drawing.Color]::FromArgb(255, 0x2E, 0x27, 0x36)
$bgBottom = [System.Drawing.Color]::FromArgb(255, 0x14, 0x10, 0x18)

function New-RoundRect([float]$x, [float]$y, [float]$w, [float]$h, [float]$r) {
    $p = New-Object System.Drawing.Drawing2D.GraphicsPath
    $d = 2 * $r
    $p.AddArc($x, $y, $d, $d, 180, 90)
    $p.AddArc($x + $w - $d, $y, $d, $d, 270, 90)
    $p.AddArc($x + $w - $d, $y + $h - $d, $d, $d, 0, 90)
    $p.AddArc($x, $y + $h - $d, $d, $d, 90, 90)
    $p.CloseFigure()
    return $p
}

# Draws one icon of size $s; small sizes use a simplified design
function New-AxisIconBitmap([int]$s, [bool]$uninstall) {
    $bmp = New-Object System.Drawing.Bitmap $s, $s, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = 'AntiAlias'
    $g.PixelOffsetMode = 'HighQuality'
    $g.Clear([System.Drawing.Color]::Transparent)

    $m = [Math]::Max(1.0, $s * 0.03)
    $rect = New-RoundRect $m $m ($s - 2 * $m) ($s - 2 * $m) ($s * 0.22)
    $bgBrush = New-Object System.Drawing.Drawing2D.LinearGradientBrush (New-Object System.Drawing.RectangleF 0, 0, $s, $s), $bgTop, $bgBottom, 90
    $g.FillPath($bgBrush, $rect)
    $borderPen = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(150, $goldLine)), ([Math]::Max(1.0, $s / 48.0))
    $g.DrawPath($borderPen, $rect)

    $c = $s / 2.0
    $rad = $s * 0.33
    # sphere (32 px and up): outline plus meridian and equator
    if ($s -ge 32) {
        $sphPen = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(230, $goldLine)), ([Math]::Max(1.0, $s / 32.0))
        $g.DrawEllipse($sphPen, $c - $rad, $c - $rad, 2 * $rad, 2 * $rad)
        $thin = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(120, $goldLine)), ([Math]::Max(1.0, $s / 64.0))
        $g.DrawEllipse($thin, $c - $rad * 0.42, $c - $rad, 2 * $rad * 0.42, 2 * $rad)
        $g.DrawEllipse($thin, $c - $rad, $c - $rad * 0.34, 2 * $rad, 2 * $rad * 0.34)
    }
    # the X: gold gradient with a dark outline for contrast
    $xr = if ($s -ge 32) { $rad * 0.80 } else { $s * 0.30 }
    $w = if ($s -ge 32) { $s * 0.12 } else { [Math]::Max(2.0, $s * 0.17) }
    $xPath = New-Object System.Drawing.Drawing2D.GraphicsPath
    $xPath.AddLine($c - $xr, $c - $xr, $c + $xr, $c + $xr)
    $xPath.StartFigure()
    $xPath.AddLine($c + $xr, $c - $xr, $c - $xr, $c + $xr)
    if ($s -ge 24) {
        $outline = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(255, 0x10, 0x0C, 0x14)), ($w + [Math]::Max(1.5, $s / 40.0))
        $outline.StartCap = 'Flat'; $outline.EndCap = 'Flat'
        $g.DrawPath($outline, $xPath)
    }
    $goldBrush = New-Object System.Drawing.Drawing2D.LinearGradientBrush (New-Object System.Drawing.RectangleF 0, ($c - $xr), $s, (2 * $xr + 1)), $gold1, $gold2, 90
    $xPen = New-Object System.Drawing.Pen $goldBrush, $w
    $xPen.StartCap = 'Flat'; $xPen.EndCap = 'Flat'
    $g.DrawPath($xPen, $xPath)

    # uninstaller: red minus badge at the bottom right
    if ($uninstall) {
        $br = $s * 0.40
        $bx = $s - $br - $m * 0.5; $by = $s - $br - $m * 0.5
        $g.FillEllipse((New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(255, 0x14, 0x10, 0x18))), $bx - $s * 0.03, $by - $s * 0.03, $br + $s * 0.06, $br + $s * 0.06)
        $g.FillEllipse((New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(255, 0xD0, 0x3B, 0x3B))), $bx, $by, $br, $br)
        $mw = [Math]::Max(1.0, $br * 0.16)
        $g.FillRectangle([System.Drawing.Brushes]::White, $bx + $br * 0.24, $by + $br / 2.0 - $mw / 2.0, $br * 0.52, $mw)
    }
    $g.Dispose()
    return $bmp
}

function Get-PngBytes($bmp) {
    $ms = New-Object System.IO.MemoryStream
    $bmp.Save($ms, [System.Drawing.Imaging.ImageFormat]::Png)
    return ,([byte[]]$ms.ToArray())   # leading comma keeps PowerShell from unrolling the array
}

# 32-bit DIB entry (BITMAPINFOHEADER, bottom-up BGRA, empty AND mask)
function Get-DibBytes($bmp) {
    $s = $bmp.Width
    $ms = New-Object System.IO.MemoryStream
    $bw = New-Object System.IO.BinaryWriter $ms
    $bw.Write([int]40); $bw.Write([int]$s); $bw.Write([int]($s * 2)); $bw.Write([int16]1); $bw.Write([int16]32)
    $bw.Write([int]0); $bw.Write([int]0); $bw.Write([int]0); $bw.Write([int]0); $bw.Write([int]0); $bw.Write([int]0)
    for ($y = $s - 1; $y -ge 0; $y--) {
        for ($x = 0; $x -lt $s; $x++) {
            $px = $bmp.GetPixel($x, $y)
            $bw.Write([byte]$px.B); $bw.Write([byte]$px.G); $bw.Write([byte]$px.R); $bw.Write([byte]$px.A)
        }
    }
    $maskRow = [int]([Math]::Ceiling($s / 32.0) * 4)
    $bw.Write((New-Object byte[] ($maskRow * $s)))
    $bw.Flush()
    return ,([byte[]]$ms.ToArray())
}

function Write-Ico([string]$path, [bool]$uninstall) {
    $sizes = 16, 24, 32, 48, 64, 128, 256
    $images = @()
    foreach ($s in $sizes) {
        $bmp = New-AxisIconBitmap $s $uninstall
        [byte[]]$bytes = if ($s -ge 256) { Get-PngBytes $bmp } else { Get-DibBytes $bmp }
        $images += , @($s, $bytes)
        $bmp.Dispose()
    }
    $ms = New-Object System.IO.MemoryStream
    $bw = New-Object System.IO.BinaryWriter $ms
    $bw.Write([int16]0); $bw.Write([int16]1); $bw.Write([int16]$images.Count)
    $offset = 6 + 16 * $images.Count
    foreach ($img in $images) {
        $s = $img[0]; $len = $img[1].Length
        $bw.Write([byte]($(if ($s -ge 256) { 0 } else { $s }))); $bw.Write([byte]($(if ($s -ge 256) { 0 } else { $s })))
        $bw.Write([byte]0); $bw.Write([byte]0); $bw.Write([int16]1); $bw.Write([int16]32)
        $bw.Write([int]$len); $bw.Write([int]$offset)
        $offset += $len
    }
    foreach ($img in $images) { $bw.Write([byte[]]$img[1]) }
    $bw.Flush()
    [IO.File]::WriteAllBytes($path, $ms.ToArray())
    "geschrieben: $path ($($ms.Length) Byte)"
}

# installer header image 150x57 (the MUI header area is light)
function Write-HeaderBmp([string]$path) {
    $bmp = New-Object System.Drawing.Bitmap 150, 57, ([System.Drawing.Imaging.PixelFormat]::Format24bppRgb)
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = 'AntiAlias'; $g.TextRenderingHint = 'AntiAliasGridFit'
    $g.Clear([System.Drawing.Color]::White)
    $icon = New-AxisIconBitmap 44 $false
    $g.DrawImage($icon, 6, 6, 44, 44); $icon.Dispose()
    $f1 = New-Object System.Drawing.Font 'Segoe UI', 15, ([System.Drawing.FontStyle]::Bold), ([System.Drawing.GraphicsUnit]::Pixel)
    $f2 = New-Object System.Drawing.Font 'Segoe UI', 10, ([System.Drawing.FontStyle]::Regular), ([System.Drawing.GraphicsUnit]::Pixel)
    $g.DrawString('Axis X', $f1, (New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(255, 0x2B, 0x25, 0x31))), 56, 12)
    $g.DrawString('GM tool for SphereX', $f2, (New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(255, 0x8F, 0x62, 0x12))), 57, 32)
    $g.Dispose()
    $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Bmp)
    $bmp.Dispose()
    "geschrieben: $path"
}

$icons = Join-Path $root 'Setup\Icons'
Write-Ico (Join-Path $icons 'AxisX.ico') $false
Write-Ico (Join-Path $icons 'UnAxisX.ico') $true
Write-HeaderBmp (Join-Path $icons 'AxisX.bmp')
if ($App) { Write-Ico (Join-Path $root 'AxisX\res\AxisX.ico') $false }

if ($Preview) {
    $sizes = 256, 128, 64, 48, 32, 24, 16
    $sheet = New-Object System.Drawing.Bitmap 760, 340
    $g = [System.Drawing.Graphics]::FromImage($sheet)
    $g.Clear([System.Drawing.Color]::FromArgb(255, 0xF3, 0xF1, 0xF5))
    $g.FillRectangle((New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb(255, 0x1B, 0x17, 0x20))), 380, 0, 380, 340)
    foreach ($half in 0, 1) {
        $x = 10 + 380 * $half
        foreach ($s in $sizes) {
            $b = New-AxisIconBitmap $s ($half -eq 1 -and $s -eq 128)
            $y = if ($s -ge 128) { 10 } else { 280 - $s }
            $g.DrawImage($b, $x, $y, $s, $s); $x += $s + 8; $b.Dispose()
            if ($s -eq 128) { $x = 10 + 380 * $half; }
        }
    }
    $hdr = [System.Drawing.Image]::FromFile((Join-Path $icons 'AxisX.bmp'))
    $g.DrawImage($hdr, 10, 150, 150, 57); $hdr.Dispose()
    $g.Dispose(); $sheet.Save($Preview); $sheet.Dispose()
    "Vorschau: $Preview"
}
