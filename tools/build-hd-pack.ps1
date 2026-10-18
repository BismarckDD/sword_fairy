param(
    [string]$GameDirectory = (Join-Path $PSScriptRoot '../runtime/game'),
    [string]$OutputDirectory = (Join-Path $PSScriptRoot '../runtime/game/hd')
)
$ErrorActionPreference='Stop'
$game=(Resolve-Path -LiteralPath $GameDirectory).Path.TrimEnd('\','/')
$output=[IO.Path]::GetFullPath($OutputDirectory)
if($output -eq $game) {throw 'Choose a separate hd subdirectory, not the source game directory.'}
Add-Type -Path (Join-Path $PSScriptRoot 'hd_pack.cs')
$count=[JxHdPack]::Build($game,$output)
@"
# 高清素材包：1280×800

本目录由 tools/build-hd-pack.ps1 从本地原版资源生成，共 $count 张四倍素材。
images/：32 位无压缩 BMP，可独立编辑，必须保留文件名和像素尺寸。
references/：识别原图及处理调色板的参考数据，请勿编辑。
manifest.tsv：游戏读取的资源清单（UTF-8，无 BOM）。

默认生成素材采用逐像素整数四倍放大，保留原字体笔画、构图、配色和透明掩码。已导入的用户重绘素材及 truecolor 标记会保留；导入记录和备份见 imports/。
320×200 图片输出 1280×800，60×70 头像输出 240×280；局部图按各自尺寸乘四。
重新生成会保留已存在的 BMP，以免覆盖人工编辑；更新的参考文件与原游戏版本必须匹配。
游戏启动时自动读取；F8 切换高清/原像素显示；关闭游戏后将 hd 重命名可禁用整个素材包。
地图及打包动画尚未单独导出，运行时按原像素整数四倍显示。原游戏资源与存档不变。
"@ | Set-Content -LiteralPath (Join-Path $output 'README.md') -Encoding UTF8
Write-Output "Created $count four-times assets in $output"
