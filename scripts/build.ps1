param([string]$Zig = 'zig', [ValidateSet('All','Game','Launcher','Events')][string]$Target = 'All')
$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path $PSScriptRoot -Parent
$outputRoot = Join-Path $repoRoot 'build'
New-Item -ItemType Directory -Force $outputRoot | Out-Null
$zigCommand = Get-Command $Zig -ErrorAction Stop
$compiler = $zigCommand.Source
$priorCache = $env:ZIG_GLOBAL_CACHE_DIR
try {
    $env:ZIG_GLOBAL_CACHE_DIR = Join-Path $repoRoot '.cache/zig'
    $sources = @(Get-ChildItem -LiteralPath (Join-Path $repoRoot 'src/engine') -Filter 'events_*.c' | Sort-Object Name | ForEach-Object { $_.FullName })
    $platformRoot = Join-Path $repoRoot 'src/platform/windows'
    $common = @('-std=c11','-Wall','-Wextra','-Werror')
    if ($Target -in @('All','Game')) {
        $hdObject = Join-Path $outputRoot 'native_hd.o'
        & $compiler cc @common -O2 -c (Join-Path $platformRoot 'native_hd.c') '-I' (Join-Path $repoRoot 'src/engine') '-o' $hdObject
        if ($LASTEXITCODE -ne 0) { throw 'HD presentation compilation failed' }
        $platform = @('resource_host.c','native_window.c','native_audio.c','native_cd.c','native_saves.c') | ForEach-Object { Join-Path $platformRoot $_ }
        $platform += $hdObject
        & $compiler cc @common @sources @platform '-I' (Join-Path $repoRoot 'src/engine') '-I' (Join-Path $repoRoot 'tests/routes') '-lgdi32' '-luser32' '-lwinmm' '-Wl,/Brepro' '-Wl,--build-id=none' '-o' (Join-Path $outputRoot 'jx_native.exe')
        if ($LASTEXITCODE -ne 0) { throw 'Game compilation failed' }
    }
    if ($Target -in @('All','Launcher')) {
        & $compiler cc @common '-municode' (Join-Path $platformRoot 'game_launcher.c') '-luser32' '-Wl,/subsystem:windows' '-Wl,/Brepro' '-Wl,--build-id=none' '-o' (Join-Path $outputRoot 'game_launcher.exe')
        if ($LASTEXITCODE -ne 0) { throw 'Launcher compilation failed' }
    }
    if ($Target -eq 'Events') {
        & $compiler cc @common '-shared' @sources '-Wl,--export-all-symbols' '-o' (Join-Path $outputRoot 'events.dll')
        if ($LASTEXITCODE -ne 0) { throw 'Event library compilation failed' }
    }
} finally { $env:ZIG_GLOBAL_CACHE_DIR = $priorCache }
Write-Output "Build complete: $outputRoot"

