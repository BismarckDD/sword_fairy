# 剑侠情缘 DOS 原生还原工程

使用 C 与 Win32 实现的原生运行工程，以及对原游戏逻辑、图像和资源格式的研究代码。项目仍在还原与验证中，函数名、结构体名及部分字段名由研究者命名；它不是原作者源码。

**仓库只提供代码，不包含原游戏、字体、图像、音乐、语音、存档或内存快照。构建程序无需游戏资源，运行需要自行准备原版资源。当前暂未设置许可证。**

## 快速构建

支持 Windows x64。已实测编译器为 Zig 0.16.0，使用 PowerShell，无需 Python 即可构建。

```powershell
# zig 已加入 PATH 时
./scripts/build.ps1

# 或显式指定编译器
./scripts/build.ps1 -Zig 'C:/tools/zig/zig.exe'
```

产物为 `build/jx_native.exe` 和 `build/game_launcher.exe`。构建使用 C11 和 `-Wall -Wextra -Werror`，不依赖原研究环境中的固定编译器路径。

## 准备本地运行目录

准备你自己的原版游戏资源、匹配的 `memory.bin` 和光盘音乐文件，然后执行：

```powershell
./scripts/prepare-runtime.ps1 -GameDirectory 'C:/games/JXQY' `
  -MemorySnapshot 'C:/games/JXQY/save_tool/memory.bin' `
  -MediaDirectory 'C:/games'
./runtime/game/启动游戏.exe
```

`runtime/` 只用于本地运行，已被 Git 忽略。脚本不会导入原存档，也不会覆盖已有运行目录。新存档位于 `runtime/game/main_decompiled/userdata/`。

没有 `memory.bin` 时，参见 [构建与资源准备说明](docs/BUILDING.md)。使用的游戏版本必须与还原代码匹配，其他版本尚未验证。

## 目录

```text
src/engine/             还原逻辑、桥接接口与早期研究代码
src/platform/windows/  Win32 窗口、音频、光盘后端、资源宿主和启动器
tests/routes/          宿主验证使用的纯文本路线头文件
scripts/               构建、本地资源准备、源码检查
tools/                 隔离内存提取研究工具
docs/                  构建说明、来源记录和验证范围
.github/               Issue 与 Pull Request 模板
```

只保留实际编译依赖的路线头文件，历史快照、重复路线、编译缓存和生成图片不属于公共源码目录。

## 验证范围

此前冻结的运行版本验证过五条实际结局、完整片尾、战败读档、恢复后通关，以及窗口、输入、声音和保存菜单。原版 CPU 对照已覆盖从启动到完整酒楼剧情与战斗。这些结论仅适用于各自记录的版本和测试条件。

本仓库整理后的构建需单独验收；不能把旧版本的全部结果直接归给新的二进制。尚未证明整游戏逐指令等价或全部玩法组合。战斗测试按用户要求将 17 种补给设为 32767，普通启动不会启用测试补给修改。详见 [验证说明](docs/VALIDATION.md)。

## 贡献与授权状态

请先阅读 [贡献说明](CONTRIBUTING.md) 和 [来源与授权状态](docs/PROVENANCE.md)。不要提交原游戏文件、存档、内存快照、生成二进制或私人路径。

当前按维护者要求不添加许可证；在授权范围明确、许可证确定前，不将“公开可见”表述为已授予开源使用许可。

