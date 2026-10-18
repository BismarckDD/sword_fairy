param(
    [string]$GameDirectory=(Join-Path $PSScriptRoot '../runtime/game'),
    [string]$OutputDirectory=(Join-Path $PSScriptRoot '../runtime/portrait-audit')
)
$ErrorActionPreference='Stop'
$game=(Resolve-Path -LiteralPath $GameDirectory).Path
$repo=Split-Path $PSScriptRoot -Parent
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$output=(Resolve-Path -LiteralPath $OutputDirectory).Path
$encoding=[Text.Encoding]::GetEncoding(936)
$face=[IO.File]::ReadAllBytes((Join-Path $game 'FACE.INF'))
$actors=[IO.File]::ReadAllBytes((Join-Path $game 'MAN.DAT'))
$places=[IO.File]::ReadAllBytes((Join-Path $game 'PLACE.BIN'))
if($face.Length%4200 -ne 0 -or $actors.Length%68 -ne 0 -or $places.Length%2784 -ne 0){throw 'Unexpected resource layout'}
$count=[int]($face.Length/4200)
$references=[Collections.Generic.List[object]]::new()
function Add-Reference([int]$id,[string]$kind,[string]$source,[string]$detail){
    if($id -ge 0 -and $id -lt $count){$references.Add([pscustomobject]@{portrait=$id;kind=$kind;source=$source;detail=$detail})}
}
function Read-Name([byte[]]$bytes,[int]$at){return ($encoding.GetString($bytes,$at,12) -split "`0")[0]}
for($i=0;$i -lt $actors.Length/68;$i++){
    $at=$i*68;$id=[BitConverter]::ToInt16($actors,$at+12)
    Add-Reference $id 'actor_table' ('MAN.DAT@0x{0:X}' -f ($at+12)) ("actor=$i name="+(Read-Name $actors $at))
}
for($scene=0;$scene -lt $places.Length/2784;$scene++){
    for($npc=0;$npc -lt 30;$npc++){
        $at=$scene*2784+864+$npc*64;$name=Read-Name $places $at;$id=[BitConverter]::ToInt16($places,$at+12)
        # Empty zero-initialized slots are padding, not 5824 independent uses of face 0.
        if($name.Length -gt 0 -or $id -gt 0){Add-Reference $id 'npc_table' ('PLACE.BIN@0x{0:X}' -f ($at+12)) "scene=$scene npc=$npc name=$name"}
    }
}

# Match the original XOR51 chapter loader and then its two-byte speaker parser.
$archive=[IO.File]::ReadAllBytes((Join-Path $game 'TXT.DAT'))
$index=[IO.File]::ReadAllBytes((Join-Path $game 'TXT.IDX'))
if($index.Length%16){throw 'Unexpected TXT.IDX layout'}
$dialogues=[Collections.Generic.List[object]]::new()
$parserWarnings=[Collections.Generic.List[string]]::new()
for($entry=0;$entry -lt $index.Length/16;$entry++){
    $chapter=Read-Name $index ($entry*16)
    $position=[int][BitConverter]::ToUInt32($index,$entry*16+12)
    $buffer=[Collections.Generic.List[byte]]::new();$reading=$false;$trim=$false;$record='';$terminated=$false
    while($position -lt $archive.Length){
        $raw=$archive[$position++];if($raw -eq 0x4b){$terminated=$true;break}
        $value=[byte]($raw -bxor 0x51)
        if($value -eq 13 -or $value -eq 10){$trim=$true;continue}
        if($trim -and $value -eq 32){continue};$trim=$false
        if(!$reading -and $value -ge 48 -and $value -le 57){
            if($position+3 -gt $archive.Length){throw 'Truncated chapter header'}
            $record=[string][char]$value+[char]($archive[$position] -bxor 0x51)+[char]($archive[$position+1] -bxor 0x51)
            $position+=3;$reading=$true;continue
        }
        if($value -eq 120){
            $bytes=$buffer.ToArray();$dialogues.Add([pscustomobject]@{chapter=$chapter;record=$record;bytes=$bytes})
            $buffer.Clear();$position++;$reading=$false
        }else{$buffer.Add($value)}
    }
    if(!$terminated){$parserWarnings.Add("$chapter has no raw 4B terminator")}
}
foreach($dialogue in $dialogues){
    $bytes=$dialogue.bytes;$cursor=0
    while($cursor -lt $bytes.Length -and $bytes[$cursor] -ne 0){
        if($cursor+1 -ge $bytes.Length){$parserWarnings.Add("$($dialogue.chapter):$($dialogue.record) short prefix at $cursor");break}
        $first=$bytes[$cursor];$second=$bytes[$cursor+1]
        if($first -ge 48 -and $first -le 57 -and $second -ge 48 -and $second -le 57){
            $id=($first-48)*10+$second-48
            if($id -ge 30){Add-Reference $id 'dialogue_prefix' "$($dialogue.chapter):$($dialogue.record)" "fragment_offset=$cursor explicit=$id (may be suppressed by caller npc=-1)"}
        }
        $cursor+=2
        while($cursor -lt $bytes.Length -and $bytes[$cursor] -ne 0 -and !($bytes[$cursor] -gt 33 -and $bytes[$cursor] -lt 128)){$cursor++}
    }
}

# Read confirmed literal portrait-transition calls and NPC +12 writes from source.
foreach($file in Get-ChildItem -LiteralPath (Join-Path $repo 'src/engine') -Filter 'events_*.c'){
    $lineNumber=0
    foreach($line in Get-Content -LiteralPath $file.FullName){
        $lineNumber++
        foreach($match in [regex]::Matches($line,'portrait_transition\(c,\s*\d+,\s*(\d+)\)')){
            Add-Reference ([int]$match.Groups[1].Value) 'script_literal' "$($file.Name):$lineNumber" $match.Value
        }
        foreach($match in [regex]::Matches($line,'jx_set_word\(c,\s*\(uint16_t\)\(0x889c\s*\+\s*npc\*86\s*\+\s*12\),\s*(\d+)\)')){
            Add-Reference ([int]$match.Groups[1].Value) 'script_assignment' "$($file.Name):$lineNumber" $match.Value
        }
    }
}

$rows=@(for($id=0;$id -lt $count;$id++){
    $colors=[Collections.Generic.HashSet[byte]]::new();$visible=0
    for($i=$id*4200;$i -lt ($id+1)*4200;$i++){[void]$colors.Add($face[$i]);if($face[$i] -ne 255){$visible++}}
    $hits=@($references | Where-Object portrait -eq $id)
    [pscustomobject]@{id=$id;asset=('FACE_INF_{0:D3}' -f $id);transparent_only=($visible -eq 0);visible_pixels=$visible;distinct_indices=$colors.Count;
        statically_referenced=($hits.Count -gt 0);reference_count=$hits.Count;kinds=(@($hits.kind | Sort-Object -Unique) -join ',');
        evidence=(@($hits | ForEach-Object {"$($_.source) $($_.detail)"}) -join ' | ')}
})
$rows | Export-Csv -LiteralPath (Join-Path $output 'portraits.csv') -Encoding UTF8 -NoTypeInformation
$references | Export-Csv -LiteralPath (Join-Path $output 'references.csv') -Encoding UTF8 -NoTypeInformation
$references | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $output 'references.json') -Encoding UTF8
$used=@($rows | Where-Object statically_referenced);$empty=@($rows | Where-Object transparent_only)
$used.asset | Set-Content -LiteralPath (Join-Path $output 'referenced-ids.txt') -Encoding UTF8
$rows | Where-Object {!$_.statically_referenced -and !$_.transparent_only} | Select-Object -ExpandProperty asset |
    Set-Content -LiteralPath (Join-Path $output 'nonempty-unreferenced-ids.txt') -Encoding UTF8
$empty.asset | Set-Content -LiteralPath (Join-Path $output 'empty-ids.txt') -Encoding UTF8
$report=@('# FACE.INF 引用与空白检查','','编号从零开始；原图每张 60×70、4200 字节，255 为透明色。',
    '',"总数：$count；有图像内容：$($count-$empty.Count)；全透明：$($empty.Count)；静态引用：$($used.Count)；已引用但全透明：$(@($used | Where-Object transparent_only).Count)。",
    "解码章节：$($index.Length/16)；文本记录：$($dialogues.Count)；解析警告：$($parserWarnings.Count)。",'',
    '此清单证明数据或源码中存在引用，不能证明对应剧情可达或在完整实机流程中显示。对话调用 npc=-1 会抑制头像。空 NPC 的零填充不计入引用；场景 154 的命名占位 **FACE 则明确保留。',
    '未发现引用不等于形式化证明永不使用；自定义存档、资源修改和未建模的内存写入不在保证范围内。没有删除任何图片或调整原始编号。',
    '', '## 指定编号','','| 编号 | 全透明 | 非透明像素 | 静态引用 |','|---|---|---|---|')
foreach($row in $rows | Where-Object {$_.id -ge 257 -and $_.id -le 259}){$report+="| $($row.asset) | $($row.transparent_only) | $($row.visible_pixels) | $($row.reference_count) |"}
$report+=@('','## 静态引用清单','','| 编号 | 有图像内容 | 引用类型 | 名称或来源 |','|---|---|---|---|')
foreach($row in $used){
    $details=@($references | Where-Object portrait -eq $row.id)
    $names=@($details | ForEach-Object {if($_.detail -match 'name=(.*)$'){$Matches[1]}else{$_.source}} | Sort-Object -Unique)
    $report+="| $($row.asset) | $(!$row.transparent_only) | $($row.kinds) | $($names -join '、') |"
}
$report+=@('','## 全透明编号','',($empty.asset -join '、'),'','## 未计入静态自动分析的路径','',
    '角色和 NPC 的运行时替换、现场实际出现次数、存档内修改后的头像号。已人工检查的直接绝对头像字段写入只有结局将主角设为 -1；已识别的脚本 NPC 头像替换为 263；结局双头像直接指定 178、250。',
    '对话的两字节十进制前缀只能直接表示 30–99，00–29 表示 NPC 槽位，a/b/c/d 从角色或 NPC 表读取。不能把任意出现的十进制常量当作头像，例如角色 +14 字段的 259 是精灵编号。')
if($parserWarnings.Count){$report+=@('','## 解析警告','');$report+=$parserWarnings}
$report | Set-Content -LiteralPath (Join-Path $output 'report.md') -Encoding UTF8
Write-Output "Portraits=$count; nonempty=$($count-$empty.Count); empty=$($empty.Count); referenced=$($used.Count); empty referenced=$(@($used | Where-Object transparent_only).Count); parser warnings=$($parserWarnings.Count)"
Write-Output $output
