param([Parameter(Mandatory=$true)][string]$GameDirectory, [string]$MemorySnapshot = '', [string]$MediaDirectory = '')
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$gameRoot = (Resolve-Path -LiteralPath $GameDirectory).Path
$runtimeRoot = Join-Path $repoRoot 'runtime'
if (Test-Path -LiteralPath $runtimeRoot) { throw 'runtime already exists. Preserve it and choose a fresh checkout; this script never overwrites player saves.' }
if (!(Test-Path -LiteralPath (Join-Path $gameRoot 'MAIN.EXE'))) { throw 'MAIN.EXE is missing from GameDirectory' }
if (!$MemorySnapshot) { $MemorySnapshot = Join-Path $gameRoot 'save_tool/memory.bin' }
$snapshot = (Resolve-Path -LiteralPath $MemorySnapshot).Path
if ((Get-Item -LiteralPath $snapshot).Length -ne 1048576) { throw 'Expected a 1 MiB memory.bin snapshot' }
foreach ($name in @('jx_native.exe','game_launcher.exe')) {
    if (!(Test-Path -LiteralPath (Join-Path $repoRoot "build/$name"))) { throw 'Build the game and launcher first' }
}
$destination = Join-Path $runtimeRoot 'game'
foreach ($folder in @('save_tool','main_decompiled/build','main_decompiled/userdata','main_decompiled/analysis')) {
    New-Item -ItemType Directory -Force (Join-Path $destination $folder) | Out-Null
}
$extensions = @('.ALW','.ANM','.BIN','.CFG','.DAT','.EXE','.FLI','.FLM','.GO','.ICO','.IDX','.IMG','.INF','.PAL','.TXT')
foreach ($file in Get-ChildItem -LiteralPath $gameRoot -File) {
    if ($file.Name -match '^SAVE[0-4]\.DAT$' -or $file.Name -match '^AudioCD\.') { continue }
    if (($extensions -contains $file.Extension.ToUpperInvariant() -and $file.Name -match '^[A-Za-z0-9_~]{1,8}\.[A-Za-z0-9]{1,3}$') -or $file.Name -in @('ASC16','ASCF16','HZF','HZK16')) {
        Copy-Item -LiteralPath $file.FullName -Destination (Join-Path $destination $file.Name)
    }
}
foreach ($folder in @('FIGHT','VOC')) {
    Copy-Item -LiteralPath (Join-Path $gameRoot $folder) -Destination $destination -Recurse
}
Copy-Item -LiteralPath $snapshot -Destination (Join-Path $destination 'save_tool/memory.bin')
Copy-Item -LiteralPath (Join-Path $repoRoot 'build/jx_native.exe') -Destination (Join-Path $destination 'main_decompiled/build/jx_native.exe')
Copy-Item -LiteralPath (Join-Path $repoRoot 'build/game_launcher.exe') -Destination (Join-Path $destination '启动游戏.exe')
if (!$MediaDirectory) {
    $MediaDirectory = if (Test-Path -LiteralPath (Join-Path $gameRoot 'AudioCD.cue')) { $gameRoot } else { Split-Path $gameRoot -Parent }
}
foreach ($name in @('AudioCD.cue','AudioCD.bin')) {
    Copy-Item -LiteralPath (Join-Path $MediaDirectory $name) -Destination (Join-Path $runtimeRoot $name)
}
Write-Output "Local runtime prepared: $destination"

