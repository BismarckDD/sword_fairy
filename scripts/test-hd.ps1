param([string]$Zig='zig')
$ErrorActionPreference='Stop'
$root=Split-Path $PSScriptRoot -Parent
Push-Location $root
$priorCache=$env:ZIG_GLOBAL_CACHE_DIR
try {
    $compiler=(Get-Command $Zig -ErrorAction Stop).Source
    $env:ZIG_GLOBAL_CACHE_DIR=Join-Path $root '.cache/zig'
    New-Item -ItemType Directory -Force build | Out-Null
    $common=@('-std=c11','-Wall','-Wextra','-Werror','-I','src/engine','-I','src/platform/windows')
    & $compiler cc @common -O2 -c src/platform/windows/native_hd.c -o build/native_hd_test.o
    if($LASTEXITCODE -ne 0){throw 'HD adapter compilation failed'}
    & $compiler cc @common tests/native_hd.c build/native_hd_test.o -o build/native_hd_test.exe
    if($LASTEXITCODE -ne 0){throw 'HD test compilation failed'}
    & ./build/native_hd_test.exe
    if($LASTEXITCODE -ne 0){throw 'HD adapter verification failed'}
    & $compiler cc @common tests/native_hd_maps.c build/native_hd_test.o -o build/native_hd_maps.exe
    if($LASTEXITCODE -ne 0){throw 'Map scene test compilation failed'}
    & ./build/native_hd_maps.exe
    if($LASTEXITCODE -ne 0){throw 'Map scene verification failed'}
    & $compiler cc @common tests/native_hd_walk.c build/native_hd_test.o -o build/native_hd_walk.exe
    if($LASTEXITCODE -ne 0){throw 'Handpaint test compilation failed'}
    & ./build/native_hd_walk.exe
    if($LASTEXITCODE -ne 0){throw 'Handpaint verification failed'}
    $sources=@(Get-ChildItem src/engine -Filter events_*.c | ForEach-Object FullName)
    foreach($name in @('native_window_hd','native_window_timing')) {
        & $compiler cc @common @sources "tests/$name.c" src/platform/windows/native_window.c src/platform/windows/native_saves.c build/native_hd_test.o -lgdi32 -luser32 -lwinmm -o "build/$name.exe"
        if($LASTEXITCODE -ne 0){throw "$name compilation failed"}
        & "./build/$name.exe"
        if($LASTEXITCODE -ne 0){throw "$name verification failed"}
    }
} finally { $env:ZIG_GLOBAL_CACHE_DIR=$priorCache; Pop-Location }
