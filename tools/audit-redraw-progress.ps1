param([string]$GameDirectory=(Join-Path $PSScriptRoot '../runtime/game'),[string]$OutputDirectory=(Join-Path $PSScriptRoot '../runtime/ai-redraw'))
$ErrorActionPreference='Stop'
$game=(Resolve-Path -LiteralPath $GameDirectory).Path
$output=(Resolve-Path -LiteralPath $OutputDirectory).Path
$encoding=[Text.Encoding]::GetEncoding(936)
$actors=[IO.File]::ReadAllBytes((Join-Path $game 'MAN.DAT'))
$places=[IO.File]::ReadAllBytes((Join-Path $game 'PLACE.BIN'))
if($actors.Length%68 -or $places.Length%2784){throw 'Unexpected character table layout'}
$refs=[Collections.Generic.List[object]]::new()
function Add-Actor($bytes,$at,$kind,$scene,$slot){
 $name=($encoding.GetString($bytes,$at,12) -split "`0")[0]
 $sprite=[BitConverter]::ToInt16($bytes,$at+14)
 if($kind -eq 'npc' -and !$name -and $sprite -le 0){return}
 $refs.Add([pscustomobject]@{kind=$kind;scene=$scene;slot=$slot;name=$name;sprite=$sprite;source_offset=$at+14})
}
for($i=0;$i -lt $actors.Length/68;$i++){Add-Actor $actors ($i*68) actor '' $i}
for($scene=0;$scene -lt $places.Length/2784;$scene++){
 for($npc=0;$npc -lt 30;$npc++){Add-Actor $places ($scene*2784+864+$npc*64) npc $scene $npc}
}
$refs | Export-Csv -LiteralPath (Join-Path $output 'character-sprites.csv') -NoTypeInformation -Encoding UTF8
$walk=@(Import-Csv (Join-Path $output 'walk-source/inventory.tsv') -Delimiter "`t")
$maps=@(Import-Csv (Join-Path $output 'map-source/inventory.tsv') -Delimiter "`t")
$battle=@(Import-Csv (Join-Path $output 'battle-source/inventory.tsv') -Delimiter "`t")
$missing=@($refs | Where-Object {$_.sprite -ge 0 -and ($walk[[int]$_.sprite].status -ne 'exported')} | Select-Object sprite -Unique)
$candidates=@(Get-ChildItem -LiteralPath (Join-Path $output 'candidates') -Filter '*.png' | ForEach-Object {
 [ordered]@{file=$_.Name;sha256=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash;status=if($_.Name -eq 'walk_001-v1.png'){'rejected-invalid-source'}else{'candidate-not-installed'}}
})
$report=[ordered]@{
 schema=1
 exported=[ordered]@{walk=@($walk | Where-Object status -eq exported).Count;maps=@($maps | Where-Object status -eq exported).Count;battle=@($battle | Where-Object status -eq exported).Count;battle_frames=($battle | Measure-Object frames -Sum).Sum}
 unresolved_maps=@($maps | Where-Object status -eq error)
 referenced_missing_walk=$missing
 candidates=$candidates
 installed_new_redraws=0
 limitations=@('Map PNGs contain static layers only; actors and scripted objects remain separate.','No full-game visual acceptance.','Original transparency masks still determine runtime silhouettes.','Whole-sheet image generation does not guarantee per-frame registration.','Character reference audit is static; script changes may add runtime sprites.')
}
$report | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $output 'progress.json') -Encoding UTF8
$report.exported
Write-Output "Referenced sprite IDs without export: $($missing.Count)"
