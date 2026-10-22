# 地图 imagegen 重绘与按位置显示

## 2026-10-22 成都主城CD00重新绘制（进行中）

用户指出旧全城重建有误，要求主城也按本轮320分块方案重做。新计划位于`runtime/game/hd/maps/context-tiles/CD00-320/plan.json`，从原始2560×2400地图导出64个320×320窗口，目标10240×9600。旧版`imagegen/assembled/CD00-full-54600edd193aedd4.png`仅保留作历史比较；新方案完成前，不把旧版计作本轮主城完成。

## 2026-10-22 成都关联地图续作（完成）

新增完成 CDDK（城外入口）、CD10（府邸庭院）、CDKZ00/CDKZ02（客栈两层）、CDJL00/CDJL02（酒楼两层），共43个320×320上下文窗口。CD00主城和五剑堂已有绘图继续保留。范围按原始PLACE.BIN引用核对；CD00H、CDDL00未找到地点表引用，未计入实际地点完成范围。

内置imagegen原生输出1254×1254，倍率3.91875，最近邻微量对齐至1280坐标。每图保留原生文件、提示词、共享64像素图块库、直接拼图和1:1局部对照。未出现或被遮挡的库图块保留原像素回退，未计作AI恢复。游戏安装使用按位置直接拼图，新增130条双层区域记录；导入素材与拼图逐像素相同，前后景覆盖完整，保留原始透明掩码。

六图分别完成独立程序64帧显示、地图名命中核对与1280×800截图。室内含短按方向输入；城外和宅院使用隔离地图加载程序绕过入口等待。未替换正式可执行程序，原MAP/BLK档案和存档哈希未变；这不代表完整剧情或全部行走路线验收。分块色差、局部几何与招牌字形偏差仍存在，共享图块版另存比较，不以平滑掩盖接缝。

汇总：`runtime/game/hd/maps/context-tiles/chengdu-completion.json`；截图总览：同目录`chengdu-game-preview.png`；逐图报告：`imagegen/verification-reports/SCENE-context320-installation.json`及`chengdu-context-native/`。完整记录见`context-tiles/chengdu-notes.md`。清单及旧安装有逐图备份，其他场景记录保持原样。

## 2026-10-22 少林寺寺内320分块续作（进行中）

范围及地点映射保存在 `context-tiles/shaolin-interiors-scope.json`。正殿 SLS08 已完成6个原生1254×1254上下文窗口，输出3840×2240。直接拼图有墙顶偏移、香案附近跨块重影，因此本场景选择共享64像素图块重拼版导入，两版及历史生成均保留。共享库233槽中103槽AI恢复、122槽原像素回退、8槽透明；共享重拼保留结构，但会牺牲部分位置细节，未将回退计作AI恢复。

SLS08 安装报告为 `imagegen/verification-reports/SLS08-context320-installation.json`，已验证双层覆盖、导入像素一致、原档案与其他场景记录不变；独立原渲染器在地点99运行64帧、240格命中，存档哈希不变。截图位于 `SLS08-context320-native/preview.png`。这不是完整剧情或全图行走验收。塔林 SLS04 的15个窗口已准备，继续逐块重绘，其余场景尚未完成本轮320流程。


## 2026-10-22 临安关联地图续作（进行中）

按原始 PLACE.BIN 的城内出口、室内连接与后期临安场景确认范围，清单为本地 `imagegen/linan-scope.json`。保留已安装的 LAN00、FBT00、JJF、JHY；FSD 没有已确认的临安出入口关联，LANTL02/LANTL04 从其他地区引用，未因编号相邻或文件名前缀而纳入。

本次已完成并导入 DC00、LANMF02、KFJL00、KFJL02、KFKZ00、KFKZ02、MF70、MF10、MF20，共32个320像素上下文窗口。原生输出均为1254×1254（3.91875倍），使用最近邻微量对齐至四倍坐标；保留原生文件、提示词、共享64像素图块库、直接拼图和局部1:1对照。KFJL02楼梯下酒坛被误画成花的版本保留为历史，已修正后导入。MF70的一次连接失败已单块重试成功。

每张地图均有独立备份、原档案哈希校验、前后景完整覆盖及运行BMP与拼图逐像素核对。隔离渲染程序分别运行64帧，各记录240次新地图格命中，截图和报告位于 `imagegen/verification-reports/SCENE-context320-native/` 和 `SCENE-context320-installation.json`。测试设置人物坐标并跳过入口等待，不代表正常剧情、全路线或全部遮挡人工验收。正式游戏显示模块同版本的客栈二楼兼容性复测位于 `KFKZ02-context320-native-shipping-hd/`；未替换正式可执行程序或修改存档。

当前基础素材包缺少部分原始图块识别记录，因此安装工具补充原像素、随调色板显示的识别图块，再按地图位置关联重绘图。其他地图没有被替换为本场景的共享AI图块。离线图块库中的受遮挡/未出现素材仍单列原像素回退，不计入AI恢复。

QL04、QL03各已完成6个原生窗口，分别输出3200×2560、3840×2560并导入；YC另完成14个原生窗口，输出8320×2112并导入。独立原渲染器各运行64帧、240格命中，原档案与存档校验通过。至此本轮新增12张、58个窗口。LANWYS00继续逐块处理，剩余场景尚未完成，不能将本批进度表述为临安全部完成。续作工具为 `tools/complete-context-map.py`、`tools/record-linan-redraw-status.py`；各地图 `context-tiles/SCENE-320/plan.json` 为逐块来源和登记状态依据。

## 2026-10-22 少林寺入口320分块版接入

SLS00 入口地图采用 `context-tiles/SLS00-320/context-redraw-4x.png` 直接上下文拼图重新接入。20张320×320原图由内置 imagegen 原生绘制为1254×1254（3.91875倍），以最近邻作约2.07%的坐标对齐，完整输出4480×6400。原生图、计划、提示词、64像素共享图块库、共享重拼图与100%局部对照均保留于该目录。共享库880槽中287槽来自AI候选、512槽回退原像素、81槽透明；回退包含未出现在本场景或被遮挡而无法恢复的素材，不计为AI完成。

导入备份于 `imagegen/backups/before-SLS00-context320-20261022T015846067755Z/`，登记72条前后景位置记录；切片像素与整图完全一致，原档案和其他场景记录保持不变。补充500个原图块定位参考用于按位置显示，它们是原像素参考，并非AI图块。寺内既有地图美术保留。

独立原渲染器在地点91、测试坐标(46,65)运行64帧，命中新场景240次并输出1280×800截图；存档哈希不变。安装及实机验证记录为 `imagegen/verification-reports/SLS00-context320-installation.json`，截图与日志位于 `SLS00-context320-native/`。已目视检查整图、牌坊文字与局部对照；分块仍有少量色调差异，AI局部轮廓可能偏离原图，尚未进行全图行走和完整剧情验收。

## 2026-10-21 衡山320分块版正式接入

HENS00 使用 `context-tiles/HENS00-320/context-redraw-4x.png` 的最新直接拼接整图接入游戏。20张320×320输入由内置AI原生绘制为1254×1254，仅作约2.07%最近邻坐标对齐；最终图5120×5760。运行导入按原坐标切成32张BMP、登记64条前后景记录，逐切片与来源整图像素完全一致，没有再次缩放。此次不替换大厅、房间或下山道路。

实际目录中原有 `hd/superres/` 已不存在，而位置素材关联依赖原始16×16图块的来源标记。因此仅为衡山补充480个非透明原始图块定位参考，文件位于 `maps/imagegen/tile-anchors/HENS00/`，登记在基础 `hd/manifest.tsv`。这些参考采用原像素最近邻显示，不是新的AI图块库；最终地图显示仍来自上述整张高清图。未恢复整套历史超分包或修改引擎。

正式程序以隐藏窗口主动请求绘制的方式运行64个世界帧，确认HENS00高清地图命中1535次，窗口、声音和关闭检查通过，存档文件哈希不变。实际1280×800截图和日志位于 `verification-reports/HENS00-context320-native/`；完整安装报告为 `verification-reports/HENS00-context320-installation.json`。这不等于全地图行走、所有遮挡边界或完整剧情的人工验收。

旧基础清单、场景清单及衡山旧参考/BMP备份于 `backups/before-HENS00-context320-20261021T070826Z/`。原始档案、程序和其他地图既有场景记录未由本次导入修改。

## 2026-10-21 成都主城

CD00 使用内置 imagegen 参照原图分为四个带 64 像素上下文的区域重绘，再融合为 10240×9600。府门、南城门和东侧杂货铺另按原图单独重绘，覆盖分区交界处的重影。原始生成图、区域计划、提示词及整图预览位于 `runtime/game/hd/maps/imagegen/` 的 `region-generated/CD00_*.png`、`generated/CD00-*.png`、`plans/CD00.json`、`prompt-CD00.txt` 和 `previews/CD00-full-preview.png`。

接入沿用按位置切片和原始前后景索引，不修改碰撞、事件或存档。资源校验与本地地图显示测试分别记录；本次范围为 CD00 主城，不含成都室内及其他关联场景，尚未完成主城实机行走和遮挡人工验收。生成美术仍可能有局部轮廓与文字偏差。

## 2026-10-21 少林寺寺内地图

SLS01、SLS02、SLS03、SLS04、SLS05、SLS06、SLS07、SLS08、SLS10、SLS12、SLS14 使用内置 imagegen 参照各自原图完整重绘，包括大殿外院、塔林、佛殿内景、两侧院落、兵器堂、藏经楼及僧房。沿用 SLS00 入口的高清手绘风格。SLS06 已移除初稿中新增的四条长凳后再导入。

原始输出保存于 `runtime/game/hd/maps/imagegen/generated/SLS*-full.png`；逐图提示词为 `prompt-SLS*-full.txt`，兵器堂修正提示词为 `prompt-SLS06-fix.txt`。导入沿用 `scene-chunks`，运行资源按原图四倍尺寸生成，背景与前景共 338 条按位置显示记录。既有 SLS00 入口不重复生成，原碰撞、事件和前后景索引不改动。

整图已做目视检查，逐图资源校验保存在 `scene-verification/SLS*.json`，本批记录见 `verification-reports/SLS-interiors-installation.json`。生成画面仍可能有局部轮廓、材质及小物件偏差；尚未完成游戏内逐场景行走和遮挡人工验收。

## 2026-10-20 成都五剑堂

WJT00 外围树林、WJT02 庭院、WJT03/WJT04 侧厅及 WJT05 正厅分别使用内置 imagegen，参照各自原图完整重绘，再按原图四倍尺寸切片接入。两张侧厅原图存在少量墙面差异，因此各自生成。保留原碰撞、事件、前后景索引和遮挡掩码；生成美术仍可能与原轮廓有局部偏差。

原始输出位于 `runtime/game/hd/maps/imagegen/generated/WJT*-full.png`，提示词见 `prompt-WJT.txt`，导入脚本见 `install-wjt.py`，备份与校验记录见 `backups/before-WJT-*` 和 `verification-reports/WJT-installation.json`。

2026-10-21 补充真实资源运行验证：地点 169（外围）、170（庭院）、171/172（左右侧厅）、173（正厅）分别运行 64 个世界帧并注入短按方向键，检查地图命中、人物移动及 1280×800 截图。运行记录与截图见 `verification-reports/WJT-native/report.json`；出入口几何切换另见同目录 `doors.json`。图片目视检查未发现明显断缝。左右侧厅实际都引用 WJT03；WJT04 没有 PLACE.BIN 引用，仅完成重绘打包，不能宣称该素材已经实际显示。

验证程序位于 `build/wjt-validation/`，使用正式地图、渲染代码与高清资源，测试窗口隐藏并显式请求绘制。人物起点为测试设置；正厅强制连战入口仅在独立测试程序中跳过，以便检查美术。出入口测试另启用五剑堂通行标记并跳过侧厅/正厅入口剧情，仅验证原坐标和切换逻辑。未修改正式游戏程序、原始资源或存档；这些检查不等同于正常第五章剧情、通行条件或全流程人工验收。初次直接进入连战与通行条件不足的日志保留在验证目录。

按用户选择，新增地图美术使用内置 imagegen 直接重绘，保留原版构图、造型、路径和建筑位置。Real-ESRGAN 的本轮试算已经停止，没有安装到游戏。既有超分包仍为尚未重绘的地图提供回退显示。

现已扩展至全部可解析场景，按地图逐张生成与验收，不代表全游戏地图已经完成。具体覆盖与安装状态以本地 `runtime/game/hd/maps/imagegen/world-progress.json`、`scene-verification/`、`plans/` 和 `installation.json` 为准。衡山 HENS00 使用根目录的 `region-jobs.json`；其他地图各有独立计划。BXD10 与 BXD12 原始图片完全相同，复用同一张生成图。

## 资源与重建

所有新增地图原图、生成图片、提示词、安装图片、校验与备份放在 `runtime/game/hd/maps/` 下，不纳入源码分发。

- `source/`：原始地图和图块表。191 个场景可以解析；BXD14 和 KF02 的原资源引用异常另列在 inventory.tsv。
- `imagegen/generated/`：保存内置 imagegen 的原始输出及完整场景。
- `imagegen/region-source/`、`region-generated/`：大地图的带重叠区域及对应生成图。
- `imagegen/region-jobs.json`：区域位置、覆盖范围、图片哈希及生成进度。
- `imagegen/prompt-*.txt`：实际采用的提示词。
- `imagegen/assembled/`：按区域融合的完整高清地图，文件名包含版本哈希。
- `imagegen/scenes/`：按地图、版本及位置保存的原始索引参考，进入对应区域时加载。
- `imagegen/pixels/`：游戏读取的无压缩 32 位 BMP，按内容去重。
- `imagegen/scene-manifest.tsv`：按地图名和原始位置登记场景区域。

衡山原图为 1280×1440，以带 32 像素上下文的区域分别生成，随后渐变融合重叠部分，输出 5120×5760。开场区域另保留较高细节的完整绘制。BXD12 原图为 320×224，运行图片为 1280×896。重绘仍需检查道路、台阶和树木边界的视觉偏差，不能把自动校验当作全剧情人工验收。

衡山场景范围依据 `PLACE.BIN` 开场场景与出口关系确认：HENS00 主地图、HSMF 大厅、MF00 房间、HSWYS 下山道路。原版后期场景复用同名地图时也会读取对应新美术。该范围及逐场景记录见本地 `imagegen/hengshan-scope.json`，不要仅依据 HS 文件名前缀把其他地区并入衡山。大厅右侧墙面、床和书架的拼接修正单独保存为 `detail_overrides`，重建时一并应用。

`tools/import-imagegen-maps.py` 只登记、缩放、拼接、切片和验证已经生成的图片，不调用超分模型或图像生成 API。

```powershell
python tools/import-imagegen-maps.py register-region --batch HENS00_0_0 --source <imagegen输出.png>
python tools/import-imagegen-maps.py assemble --scene HENS00
python tools/import-imagegen-maps.py scene-chunks --scene HENS00 --source <assembled输出.png>
python tools/import-imagegen-maps.py verify-scenes
python tools/import-imagegen-maps.py progress
```

需要 NumPy 与 Pillow。图片生成使用内置 imagegen；不需要把本地模型或 API 密钥加入导入工具。

## 为什么按地图位置加载

原游戏重复使用同一个 16×16 图块。直接把整张生成图切成这些共享图块，会让不同位置的新细节互相覆盖，产生重复纹理、错接和缝隙。

新增显示适配在原引擎完成一个地图格的背景、人物和前景合成后，核对原始图块，再关联到完整重绘图的对应位置。相同原图块在地图不同位置可以显示不同的手绘细节。图片参考仍分别保存原始背景与前景索引，人物、遮挡、显存页面复制、截图恢复和淡出继续使用既有显示路径；不修改剧情、坐标、碰撞、原档案或存档。

场景分成不超过原始 320×192 的区域，以复用现有 BMP 缓存和来源标记。清单每行依次为地图名称、原图像素 X/Y、原始宽/高、参考路径、BMP 路径，使用制表符分隔。路径相对 hd；宽、高和起点必须是 16 的整数倍。地图名匹配时统一为 ASCII 大写。

游戏需重启以读取新图片；F8 可切换原像素对照。将 `scene-manifest.tsv` 改名可单独禁用新场景，而保留原有高清人物与其他素材。没有有效场景记录时保留既有显示；图片损坏时允许回退原像素。

## 验证

临安城主地图 LAN00 已完成 25 个重叠区域的内置 imagegen 重绘，拼接为 10240×9600，并安装为 208 条按位置区分的背景/前景记录。尺寸、哈希、完整覆盖与原资源校验通过；高清、人物、窗口和计时回归通过。独立地图验证程序在城外与城内各运行 64 帧，分别记录 176、240 次新地图格命中，并检查城内 1280×800 截图。此项仅覆盖临安城主地图，不包含各室内场景，也不代表全部剧情与行走路线人工验收。原图、提示词、拼接图及截图保留于本地地图目录，验证记录为 `imagegen/verification-reports/LAN00-native-probe.json`。

`scripts/test-hd.ps1` 已包含 `tests/native_hd_maps.c`，检查相同原图块在两个位置显示不同图片、背景/前景透明掩码、相机偏移、名称大小写、淡出、VGA 页面复制、未知覆盖、错误参考和地图隔离。其余人物、菜单、窗口及计时回归仍保持原有检查。

真实资源开场检查需记录 `HD imagegen scene` 的地图名与命中数量，并检查 1280×800 实际游戏截图；仅有素材数量增加不能证明新地图实际显示。源码构建不包含任何生成美术。

## 宋营与金营地图

本地新增 16 张内置 imagegen 完整地图重绘：宋营外景 SY00、SY10，岳飞主帐 SY04、韩世忠主帐 SY06，以及 SY02、SY03、SY08、SY09；金营 JY00、JY10、JY20、JY30、JY40、JY50、JY70、JY90。整图按原坐标切片，运行尺寸为原图宽高各四倍。原始生成尺寸另存于各地图计划，不将运行尺寸表述为模型原生输出尺寸。

原始输出和提示词保存在 `imagegen/generated/` 与 `imagegen/prompt-*.txt`；安装与完整性报告为 `imagegen/verification-reports/camps-hd.json`，替换前清单与计划保存在 `imagegen/backups/camps-20261020/`。JY00 去除生成图的空白上下黑边后再对齐；部分夜景和低纹理边角经整图目视对照后使用已知坐标切片，未降低通用导入器的自动匹配阈值。原版地图档案、前后景索引与遮罩保持不变。资源完整性及地图显示回归通过，尚未逐场景进行完整剧情实机验收。
