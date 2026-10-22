# 剑侠情缘 DOS 还原工程适配器

## 输入与场景识别

项目根目录由 `--project-root` 传入。输入位于：

```text
runtime/game/MAP.DAT, MAP.IDX
runtime/game/BLK.DAT, BLK.IDX
runtime/game/COMMON.PAL
runtime/game/hd/maps/catalog.json
runtime/game/hd/maps/source/SCENE.png
```

catalog 的 `scenes` 条目提供 `name,width,height,block,palette,status,indices`。宽高以16像素图块计，`indices` 为已解析的背景、前景两层，共 `2*width*height` 项；仅处理 `status=ready`。按现有场景说明、PLACE.BIN 引用或用户指明的编号消除歧义。

缺少 catalog 或原图导出时，先检查项目自己的导出工具，不能编造地图。当前工程 `tools/remaster-assets.py prepare` 生成超分研究目录的清单，地图导出另见 `tools/export-world-art.ps1`；这些不是本技能自动运行的安装或推理步骤。现有 `hd/maps/catalog.json` 是地图阶段整理后的目录，应保持它与原档案和原图匹配。任何补导出先输出到独立目录再核对。

## 资源格式中的关键语义

- IDX 每项16字节：12字节ASCII资源名和4字节小端数据偏移。
- BLK 头为三个小端16位数：图块数、宽、高；本适配器只接受16×16。后续每块256索引字节，按4个VGA平面排列。行列还原为 `reshape(4,16,4).transpose(1,2,0).reshape(16,16)`。
- 调色板从768字节 COMMON.PAL 开始，用 MAP 中对应调色板记录的 `offset+502:offset+787` 覆盖字节480:765。VGA色值乘4并截断到255。
- 索引255透明；前景图块编号1表示无前景。本约定是该引擎特有的，不能推广到其他游戏。
- 相同索引图块会跨场景共享，但可能使用不同调色板。候选包按场景保留，不能只因哈希相同就覆盖所有地图的最终RGB。

## 可复用命令

命令中的 `SKILL_DIR` 指本技能目录，`PROJECT_ROOT` 是当前工程，`OUTPUT` 是新的或已经属于同一任务的输出目录。脚本默认320分块；省略输出时使用 `runtime/game/hd/maps/context-tiles/SCENE-320/`。为比较改版结果可显式传新目录。

```powershell
python "$SKILL_DIR/scripts/map_redraw.py" prepare --project-root "$PROJECT_ROOT" --scene HENS00 --output "$OUTPUT"
python "$SKILL_DIR/scripts/map_redraw.py" status --project-root "$PROJECT_ROOT" --scene HENS00 --output "$OUTPUT"
# 用内置 imagegen 编辑 source/ 中的原始窗口后：
python "$SKILL_DIR/scripts/map_redraw.py" register --project-root "$PROJECT_ROOT" --scene HENS00 --output "$OUTPUT" --job HENS00_320_0 --source "原生生成文件.png"
# 所有窗口实际落盘后：
python "$SKILL_DIR/scripts/map_redraw.py" finish --project-root "$PROJECT_ROOT" --scene HENS00 --output "$OUTPUT" --allow-near4x
python "$SKILL_DIR/scripts/map_redraw.py" verify --project-root "$PROJECT_ROOT" --scene HENS00 --output "$OUTPUT"
python "$SKILL_DIR/scripts/compare.py" --work "$OUTPUT" --region 320,0,128,128
```

`--region` 使用原始像素坐标，可重复传入有代表性的屋顶、树木、山石或台阶区域。提供 `--baseline 旧方案完整拼图.png` 可生成“旧方案/新直接拼图/新图块重拼”对照；否则第一栏为原像素四倍显示。该对照不缩放最终图像裁剪，所以能判断细节是否糊。

`prepare` 会验证并复用已有计划，避免丢失登记状态；不同来源或分块设置要求新目录。`register` 将原始PNG按内容哈希存储，修改该任务的选中版本并保留历史路径；**登记操作顺序执行**，避免多个进程同时修改 plan.json。脚本不调用图像模型，也不上传素材。

## 输出与核验

- `source/`、`plan.json`：真实原图窗口、有效范围、输入哈希、原档案哈希及任务状态。
- `generated/`：未缩放的 AI 原始PNG与历史版本；提示词由执行者保存为 `prompts.txt`，实际工具输出来源可另存 `generation-provenance.json`。
- `context-redraw-4x.png`：直接上下文拼图；`reconstructed-shared-tiles-4x.png`：共享图块重拼。尺寸均为原图两轴各四倍。
- `tiles/`、`replacement-library-64.png`、`candidate-pack/`：64×64素材、图块表、32位无压缩BMP及原始索引参考。
- `report.json`、`validation.json`：原生尺寸、实际倍率、对齐方式、候选来源、覆盖、哈希、图块像素保留及档案完整性。
- `previews/100-percent-detail.png`：1:1局部对照；`previews/comparison.png` 是整图缩略图，不作为清晰度证据。

缺失任何生成窗口时 `finish` 会失败，不使用原图或其他方案填补并冒充整图完成。库中没有完整可见候选的图块使用原像素回退并单列；这不代表该图块已经 AI 重绘。

## 已验证实例与游戏接入

衡山 HENS00 原图1280×1440，320窗口横向起点0/320/640/960、纵向0/320/640/960/1120，共20块。原生生成均1254×1254（3.91875倍）；约2.07%最近邻对齐后得到5120×5760。424个非透明图块可恢复，56个非透明图块回退，3个透明槽位。数字仅描述这个实例，其他地图从实际报告读取。

本工程同时有 `hd/superres/manifest.tsv` 的共享图块和 `hd/maps/imagegen/scene-manifest.tsv` 的按位置场景覆盖。后者可能优先显示，单换共享图块不一定能在游戏看到变化。用户要求接入时，选择请求的最终图，通过项目已有 `tools/import-imagegen-maps.py scene-chunks` 和 `verify-scenes` 等流程打包，备份有关清单/资源，再做地图命中与实际截图检查。不要照抄一个地图的测试结果为其他地图背书。碰撞、事件、原始档案与存档不属于美术重绘变更。

大地图一次只处理一个场景；完整四倍RGB图每像素需3字节，另有拼图和参考图缓存。内存不足时分区/流式适配，不能偷偷降低输出分辨率。
