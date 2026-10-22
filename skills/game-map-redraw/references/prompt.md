# 上下文 AI 原生细节重绘提示词

每个窗口使用自己的原始地图截图作为编辑目标。将场景标识和尺寸替换为实际值；默认输入320，目标1280。先检查输出原始文件尺寸，再判断是否能进入图块提取。地图原有黑色空白和补边应保留；仅对 plan.json 的有效区域拼图。

```text
Use case: style-transfer.
EDIT TARGET: attached exact 320x320 ORIGINAL GAME MAP CROP for SCENE.
Create a native 1280x1280 high-detail restoration, about 4x in each axis.
AI must redraw fine detail at output resolution, rather than merely enlarging,
blurring, smoothing or interpolating the original pixels.

SPATIAL REGISTRATION: each original 16x16 cell corresponds to a 64x64 output
cell, with no visible grid lines. Keep all object edges at 4 times their source
coordinates. Preserve tree counts, trunks, roots, small plants, rock chips,
stone boundaries, cracks, roof-tile rows, fence posts, steps, windows,
inscriptions, shadows, materials and colors. Preserve silhouettes, topology,
paths, stairs, architecture, oblique top-down projection and exact framing.
Match every border for adjoining map crops. Keep original empty areas empty.
If the input has an edge-padding band, retain it as padding; do not invent
new scene content there. No shifts, zoom, margins, redesign, extra objects,
removed objects, text overlays or watermarks.

DETAIL AND SHARPNESS: sharply resolved small stone facets, individual roof
tiles, wood grain, foliage clusters, thin branches and tiny ground grit.
High-quality hand-painted 2D game textures matching the original palette
and art direction. Dense fine detail with crisp edges at 100% view.
No depth of field, haze, bloom, painterly smearing, denoising blur or
out-of-focus regions. Increase resolved detail, not just edge sharpening.

Output only the faithful square map crop at native 1280x1280.
```

原始字形、关键招牌和出入口位置应另作目视核对。提示词是约束意图，不是无损或精确坐标已实现的证据。风格描述以当前游戏和用户要求为准，不把衡山中式建筑风格强加给其他题材。
