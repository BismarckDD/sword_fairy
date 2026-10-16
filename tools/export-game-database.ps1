param(
    [string]$GameDirectory = (Join-Path $PSScriptRoot '../runtime/game'),
    [string]$OutputDirectory = (Join-Path $PSScriptRoot '../runtime/analysis')
)
$ErrorActionPreference = 'Stop'
$encoding = [Text.Encoding]::GetEncoding(936)
New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
function Read-Word($Bytes, $Offset) { [BitConverter]::ToInt16($Bytes, $Offset) }
function Read-Name($Bytes, $Offset) {
    $length = 0
    while ($length -lt 12 -and $Bytes[$Offset + $length] -ne 0) { $length++ }
    $encoding.GetString($Bytes, $Offset, $length)
}
function Animation-Path($Number, $Enemy) {
    if ($Number -lt 0) { return $null }
    if (!$Enemy) { $Number -= 100 }
    # Original loader formats width 3 and retains only the first 3 characters.
    $numberText = ('{0,3}' -f $Number).Replace(' ', '0').Substring(0,3)
    if ($Enemy) { "fight/enemy$numberText.anm" } else { "fight/frien$numberText.anm" }
}
$specs = @(@('MAN.DAT',68,16,'actors'), @('ENEMY.DAT',30,64,'enemies'),
           @('SKILL.DAT',28,64,'skills'), @('THING.DAT',28,64,'items'))
$database = [ordered]@{}
foreach ($spec in $specs) {
    $bytes = [IO.File]::ReadAllBytes((Join-Path $GameDirectory $spec[0]))
    if ($bytes.Length -ne $spec[1]*$spec[2]) { throw "Unexpected file size: $($spec[0])" }
    $records = @(for ($id=0; $id -lt $spec[2]; $id++) {
        $base = $id*$spec[1]
        $words = [ordered]@{}
        for ($offset=12; $offset -lt $spec[1]; $offset+=2) {
            $words[('0x{0:X2}' -f $offset)] = Read-Word $bytes ($base+$offset)
        }
        $row = [ordered]@{ id=$id; name=(Read-Name $bytes $base); file_offset=$base;
            raw_hex=[BitConverter]::ToString($bytes,$base,$spec[1]); words_signed_le=$words }
        switch ($spec[3]) {
            'actors' {
                foreach ($pair in @(@('hp',16),@('mp',18),@('max_hp',20),@('max_mp',22),
                    @('attack',24),@('defense',26),@('level',34),@('equipment_id',38))) {
                    $row[$pair[0]] = Read-Word $bytes ($base+$pair[1])
                }
                $row['experience']=[BitConverter]::ToUInt16($bytes,$base+30)
                $row['skill_slots']=@($bytes[($base+52)..($base+67)] | ForEach-Object { if ($_ -ge 128) {-1} else {[int]$_} })
            }
            'enemies' {
                foreach ($pair in @(@('hp',14),@('attack',16),@('defense',18),
                    @('attack_animation_id',20),@('skill_id',24),@('experience_reward',28))) {
                    $row[$pair[0]]=Read-Word $bytes ($base+$pair[1])
                }
            }
            'skills' {
                $row['power']=Read-Word $bytes ($base+16)
                $row['mp_cost']=Read-Word $bytes ($base+20)
                $row['animation_id']=Read-Word $bytes ($base+24)
                $row['sound_id']=Read-Word $bytes ($base+26)
                foreach ($enemy in @($false,$true)) {
                    $key=if($enemy){'enemy_animation'}else{'friend_animation'}
                    $path=Animation-Path $row['animation_id'] $enemy
                    $row[$key]=$path
                    $row[$key+'_exists']=($null -ne $path -and (Test-Path -LiteralPath (Join-Path $GameDirectory $path)))
                }
            }
            'items' {
                $row['legacy_category']=if($id -eq 0){'none'}elseif($id -lt 12){'equipment'}elseif($id -lt 38){'special_handler'}elseif($id -lt 59){'consumable'}else{'battle_throwable_branch'}
                foreach ($pair in @(@('hp_or_throw_damage',14),@('max_hp_gain',16),@('mp_gain',18),
                    @('max_mp_gain',20),@('attack_bonus_or_gain',22),@('defense_bonus_or_gain',24))) {
                    $row[$pair[0]]=Read-Word $bytes ($base+$pair[1])
                }
            }
        }
        [pscustomobject]$row
    })
    $database[$spec[3]]=$records
}
$database | ConvertTo-Json -Depth 10 | Set-Content -Encoding UTF8 (Join-Path $OutputDirectory 'database.json')
$lines = @('# 本地游戏数据解析', '', '名称按 GB936 解码；ID 从 0 开始。未确认字段保留原始字节与有符号小端 WORD。', '',
    '## 技能与动画', '', '| ID | 名称 | 威力 | 内力 | 动画ID | 音效ID | 主角动画 | 敌人动画 |', '|---|---|---|---|---|---|---|---|')
foreach ($r in $database.skills) {
    $friend=if($r.friend_animation_exists){$r.friend_animation}else{'无对应文件'}
    $enemy=if($r.enemy_animation_exists){$r.enemy_animation}else{'无对应文件'}
    $lines += "| $($r.id) | $($r.name) | $($r.power) | $($r.mp_cost) | $($r.animation_id) | $($r.sound_id) | $friend | $enemy |"
}
$lines += @('', '## 角色初始数据', '', '| ID | 名称 | 等级 | 生命 | 内力 | 攻击 | 防御 | 装备ID | 技能ID |', '|---|---|---|---|---|---|---|---|---|')
foreach ($r in $database.actors) {
    $skills=($r.skill_slots | Where-Object {$_ -ge 0}) -join ','
    $lines += "| $($r.id) | $($r.name) | $($r.level) | $($r.hp)/$($r.max_hp) | $($r.mp)/$($r.max_mp) | $($r.attack) | $($r.defense) | $($r.equipment_id) | $skills |"
}
foreach ($key in @('enemies','items')) {
    $lines += @('', "## $key 原始字段", '', '| ID | 名称 | +0C起的有符号WORD（顺序排列） |', '|---|---|---|')
    foreach ($r in $database[$key]) { $lines += "| $($r.id) | $($r.name) | $($r.words_signed_le.Values -join ', ') |" }
}
$lines | Set-Content -Encoding UTF8 (Join-Path $OutputDirectory 'database-report.md')
Write-Output "Exported 208 records to $OutputDirectory"
