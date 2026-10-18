param(
    [Parameter(Mandatory=$true)][string]$SourceDirectory,
    [string]$HdDirectory=(Join-Path $PSScriptRoot '../runtime/game/hd')
)
$ErrorActionPreference='Stop'
$source=(Resolve-Path -LiteralPath $SourceDirectory).Path
$hd=(Resolve-Path -LiteralPath $HdDirectory).Path
if($source -eq $hd){throw 'Source and destination must differ'}
Add-Type -AssemblyName System.Drawing
$assemblies=@([Drawing.Bitmap].Assembly.Location,[Drawing.Color].Assembly.Location)
$drawingDirectory=Split-Path ([Drawing.Bitmap].Assembly.Location) -Parent
$assemblies+=@(Get-ChildItem -LiteralPath $drawingDirectory -Filter 'System.Private.Windows*.dll' | ForEach-Object FullName)
Add-Type -Path (Join-Path $PSScriptRoot 'hd_import.cs') -ReferencedAssemblies $assemblies
$stamp=Get-Date -Format 'yyyyMMdd-HHmmss'
$work=Join-Path $hd "imports/$stamp"
foreach($folder in @('staging','previews','review','backup')){New-Item -ItemType Directory -Force (Join-Path $work $folder) | Out-Null}
$manifestPath=Join-Path $hd 'manifest.tsv'
Copy-Item -LiteralPath $manifestPath -Destination (Join-Path $work 'backup/manifest.tsv')
$lines=@(Get-Content -LiteralPath $manifestPath)
$entries=@{}
for($i=0;$i -lt $lines.Count;$i++){
    if($lines[$i].StartsWith('#') -or !$lines[$i]){continue}
    $fields=$lines[$i].Split("`t")
    if($fields.Count -lt 5){throw 'Invalid manifest'}
    if($fields[0] -notmatch '^[A-Z0-9_]+$' -or $fields[3] -notmatch '^references/[A-Z0-9_]+\.idx$' -or $fields[4] -notmatch '^images/[A-Z0-9_]+\.bmp$'){throw 'Unsafe manifest path or key'}
    $entries[$fields[0]]=[pscustomobject]@{line=$i;key=$fields[0];width=[int]$fields[1];height=[int]$fields[2];reference=$fields[3];image=$fields[4]}
}
$files=@(Get-ChildItem -LiteralPath (Join-Path $source 'FACE') -Filter '*.png' -File)
$faces=@(foreach($file in $files){if($file.BaseName -match '^upscale_face(\d+)(.*)$'){
    [pscustomobject]@{file=$file;id=[int]$Matches[1];revision=($Matches[2].Length -gt 0);hash=(Get-FileHash -LiteralPath $file.FullName).Hash}
}})
$emptyHashes=@{}
foreach($face in $faces){
    $key='FACE_INF_{0:D3}' -f $face.id
    if($entries.ContainsKey($key) -and [JxHdImport]::Empty((Join-Path $hd $entries[$key].reference))){$emptyHashes[$face.hash]=$true}
}
$selected=[Collections.Generic.List[object]]::new();$records=[Collections.Generic.List[object]]::new()
foreach($group in $faces | Group-Object id){
    # A separately saved revision takes priority; keep the other version for review.
    $ordered=@($group.Group | Sort-Object @{Expression='revision';Descending=$true},@{Expression={$_.file.LastWriteTime};Descending=$true})
    for($i=0;$i -lt $ordered.Count;$i++){
        $face=$ordered[$i];$key='FACE_INF_{0:D3}' -f $face.id;$reason=''
        if(!$entries.ContainsKey($key)){$reason='no_original_reference'}
        elseif([JxHdImport]::Empty((Join-Path $hd $entries[$key].reference))){$reason='original_is_all_transparent'}
        elseif($emptyHashes.ContainsKey($face.hash)){$reason='identical_to_art_generated_for_empty_slot'}
        elseif($i -gt 0){$reason='alternate_revision_not_activated'}
        if($reason){
            Copy-Item -LiteralPath $face.file.FullName -Destination (Join-Path $work ('review/'+$face.file.Name))
            $records.Add([pscustomobject]@{source=$face.file.FullName;key=$key;status=$reason;sha256=$face.hash;processing='not activated'})
        }else{$selected.Add([pscustomobject]@{file=$face.file;key=$key;align=$false;hash=$face.hash})}
    }
}
foreach($file in Get-ChildItem -LiteralPath (Join-Path $source 'FIGHT') -Filter '*.png' -File){
    $key='FIGHT_'+$file.BaseName.ToUpperInvariant()+'_IMG_000'
    if($entries.ContainsKey($key)){$selected.Add([pscustomobject]@{file=$file;key=$key;align=($file.BaseName -match '^SHADE_');hash=(Get-FileHash -LiteralPath $file.FullName).Hash})}
    else{
        Copy-Item -LiteralPath $file.FullName -Destination (Join-Path $work ('review/'+$file.Name))
        $records.Add([pscustomobject]@{source=$file.FullName;key=$key;status='no_original_reference';sha256=(Get-FileHash -LiteralPath $file.FullName).Hash;processing='not activated'})
    }
}
# Complete all conversions before replacing the active images.
foreach($item in $selected){
    $entry=$entries[$item.key]
    $processed=[JxHdImport]::Convert($item.file.FullName,(Join-Path $hd $entry.reference),
        (Join-Path $work "staging/$($item.key).bmp"),(Join-Path $work "previews/$($item.key).png"),$entry.width,$entry.height,$item.align)
    $records.Add([pscustomobject]@{source=$item.file.FullName;key=$item.key;status='imported';sha256=$item.hash;processing=$processed})
}
foreach($item in $selected){
    $entry=$entries[$item.key];$destination=Join-Path $hd $entry.image
    Copy-Item -LiteralPath $destination -Destination (Join-Path $work "backup/$($item.key).bmp")
    Copy-Item -LiteralPath (Join-Path $work "staging/$($item.key).bmp") -Destination $destination -Force
    $lines[$entry.line]=@($entry.key,$entry.width,$entry.height,$entry.reference,$entry.image,'truecolor') -join "`t"
}
[IO.File]::WriteAllLines($manifestPath,$lines,[Text.UTF8Encoding]::new($false))
$records | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $work 'import-report.json') -Encoding UTF8
$records | Export-Csv -LiteralPath (Join-Path $work 'import-report.csv') -NoTypeInformation -Encoding UTF8
$summary=@('# 用户高清素材导入','',"本次启用 $($selected.Count) 张，其他 $($records.Count-$selected.Count) 张保存在 review/。",'',
    'images/ 是游戏实际加载的 BMP；本目录 previews/ 提供处理后的透明 PNG，backup/ 保存被替换的旧文件与清单。',
    '头像统一 240×280；ZD 背景统一 1280×800；SHADE 统一 880×592，并将非背景主体对齐原角色包围框。保留原透明掩码，不修改原文件。',
    '导入图片使用 truecolor 颜色策略，保持新图颜色，仅跟随整体淡入淡出；已有普通四倍素材仍沿用原调色板策略。',
    '优先启用同编号后缀修订版，其余版本保留待核对。原图全透明的编号不启用新人物图，与这些图完全重复的其他编号也暂不启用。',
    '', '| 来源文件 | 目标编号 | 状态 | 处理 |','|---|---|---|---|')
foreach($record in $records){$summary+="| $([IO.Path]::GetFileName($record.source)) | $($record.key) | $($record.status) | $($record.processing) |"}
$summary | Set-Content -LiteralPath (Join-Path $work 'README.md') -Encoding UTF8
Write-Output "Imported=$($selected.Count); review=$($records.Count-$selected.Count); report=$work"
