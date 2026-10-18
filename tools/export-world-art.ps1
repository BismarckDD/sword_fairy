param(
 [string]$GameDirectory=(Join-Path $PSScriptRoot '../runtime/game'),
 [string]$OutputDirectory=(Join-Path $PSScriptRoot '../runtime/ai-redraw/map-source')
)
$ErrorActionPreference='Stop'
if($PSVersionTable.PSEdition -eq 'Core'){
 & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $PSCommandPath -GameDirectory $GameDirectory -OutputDirectory $OutputDirectory
 if($LASTEXITCODE -ne 0){throw 'World export failed'}
 return
}
Add-Type -AssemblyName System.Drawing
$game=(Resolve-Path -LiteralPath $GameDirectory).Path
$output=[IO.Path]::GetFullPath($OutputDirectory)
if($output -eq $game -or $output.StartsWith($game+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)){throw 'Output must be outside the game directory'}
New-Item -ItemType Directory -Force -Path $output | Out-Null
Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition (Get-Content -LiteralPath (Join-Path $PSScriptRoot 'world_art_export.cs') -Raw)
[WorldArtExport]::Run($game,$output)
