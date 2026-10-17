param(
    [string]$GameDirectory = (Join-Path $PSScriptRoot '../runtime/game'),
    [string]$OutputDirectory = (Join-Path $PSScriptRoot '../runtime/hd-analysis')
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
$source = (Resolve-Path -LiteralPath $GameDirectory).Path
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$output = (Resolve-Path -LiteralPath $OutputDirectory).Path
if ($output -eq $source -or $output.StartsWith($source + [IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) {
    throw 'Output must be outside the source game directory.'
}
$preview = Join-Path $output 'previews'
New-Item -ItemType Directory -Force -Path $preview | Out-Null
$files = @(Get-ChildItem -LiteralPath $source -File)
foreach ($folder in @('FIGHT','VOC')) {
    $path = Join-Path $source $folder
    if (Test-Path -LiteralPath $path) { $files += @(Get-ChildItem -LiteralPath $path -File -Recurse) }
}
$records = @(); $exports = @()
foreach ($file in ($files | Sort-Object FullName)) {
    $relative = $file.FullName.Substring($source.Length+1).Replace('\','/')
    $record = [ordered]@{ source=$relative; bytes=$file.Length; extension=$file.Extension.ToUpperInvariant(); status='not-decoded' }
    if ($file.Extension -ieq '.IMG') {
        $bytes = [IO.File]::ReadAllBytes($file.FullName)
        if ($bytes.Length -ge 784 -and [Text.Encoding]::ASCII.GetString($bytes,0,4) -eq 'IMGV') {
            $width = [int][BitConverter]::ToUInt16($bytes,6)+1
            $height = [int][BitConverter]::ToUInt16($bytes,8)+1
            $area = [long]$width*$height
            $payload = $bytes.Length-784
            # Only export exact raw indexed-image layouts; packed formats stay unclassified.
            if ($width -le 4096 -and $height -le 4096 -and $payload -gt 0 -and $payload % $area -eq 0) {
                $frames = [int]($payload/$area)
                $record.width=$width; $record.height=$height; $record.frames=$frames
                $record.status='raw-indexed-preview'
                $paletteStart = $bytes.Length-768
                $colors = New-Object 'int[]' 256
                for ($i=0;$i -lt 256;$i++) {
                    $p=$paletteStart+$i*3
                    $r=[Math]::Min(255,[int]$bytes[$p]*4)
                    $g=[Math]::Min(255,[int]$bytes[$p+1]*4)
                    $b=[Math]::Min(255,[int]$bytes[$p+2]*4)
                    $colors[$i]=[Drawing.Color]::FromArgb(255,$r,$g,$b).ToArgb()
                }
                for ($frame=0;$frame -lt $frames;$frame++) {
                    $pixels=New-Object 'int[]' ([int]$area)
                    $start=16+$frame*$area
                    for ($i=0;$i -lt $area;$i++) { $pixels[$i]=$colors[$bytes[$start+$i]] }
                    $bitmap=New-Object Drawing.Bitmap $width,$height
                    try {
                        $rect=New-Object Drawing.Rectangle 0,0,$width,$height
                        $data=$bitmap.LockBits($rect,[Drawing.Imaging.ImageLockMode]::WriteOnly,[Drawing.Imaging.PixelFormat]::Format32bppArgb)
                        try { [Runtime.InteropServices.Marshal]::Copy($pixels,0,$data.Scan0,$pixels.Length) }
                        finally { $bitmap.UnlockBits($data) }
                        $name=($relative -replace '[/\\.]','_')+('_{0:D3}.png' -f $frame)
                        $bitmap.Save((Join-Path $preview $name),[Drawing.Imaging.ImageFormat]::Png)
                        $exports += [ordered]@{source=$relative; frame=$frame; preview="previews/$name"; width=$width; height=$height}
                    } finally { $bitmap.Dispose() }
                }
            } else { $record.status='unsupported-layout' }
        } else { $record.status='unsupported-header' }
    }
    $records += [pscustomobject]$record
}
[ordered]@{schema=1; target=@{width=1280;height=960}; files=$records; previews=$exports} |
    ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $output 'inventory.json') -Encoding UTF8
$html = @('<!doctype html><meta charset="utf-8"><title>素材盘点</title><style>body{background:#202020;color:#eee;font:16px sans-serif}main{display:flex;flex-wrap:wrap;gap:16px}figure{margin:0;padding:12px;background:#333}img{max-width:480px;image-rendering:pixelated}small{display:block}</style><h1>原版素材预览（尚未重绘）</h1><p>透明索引暂按原色显示；调色板与多帧排列需在游戏内核对。</p><main>')
foreach ($item in $exports) {
    $label=[Net.WebUtility]::HtmlEncode("$($item.source) / $($item.frame) / $($item.width)×$($item.height)")
    $html += "<figure><img loading='lazy' src='$($item.preview)'><small>$label</small></figure>"
}
$html += '</main>'
$html | Set-Content -LiteralPath (Join-Path $output 'index.html') -Encoding UTF8
Write-Output "Inventoried $($records.Count) files; exported $($exports.Count) previews to $output"
