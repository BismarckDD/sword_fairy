# 构建与运行

## 环境

- Windows x64。
- PowerShell；本机验证使用 PowerShell 7。
- Zig 0.16.0，加入 PATH 或使用 `-Zig` 指定 `zig.exe`。
- 仅内存提取工具需要 Python、Unicorn、Capstone；它们不是游戏运行依赖。

不附带编译器、Python 环境或第三方库副本。

## 构建目标

在仓库根目录执行；也可以从任意目录以脚本绝对路径调用：

```powershell
./scripts/build.ps1 -Zig 'C:/tools/zig/zig.exe' -Target All
./scripts/build.ps1 -Target Game
./scripts/build.ps1 -Target Launcher
./scripts/build.ps1 -Target Events
```

`All` 构建游戏核心与 GUI 启动器；`Events` 生成供研究使用的 `build/events.dll`。独立的早期研究 `.c` 文件也保留在 `src/engine/`，主程序按原构建范围编译 `events_*.c`。

编译缓存位于 `.cache/zig/`，产物位于 `build/`，都不会提交。不同源码路径可能影响断言中的文件名，不承诺跨路径逐字节相同。

## 原版资源

原资源不随仓库分发。本地运行需要：

- 匹配版本的 `MAIN.EXE` 和原游戏数据文件。
- `FIGHT/`、`VOC/` 完整目录。
- 无扩展名字体 `ASC16`、`ASCF16`、`HZF`、`HZK16`。
- 1 MiB 的 `save_tool/memory.bin`。
- `AudioCD.cue` 及它引用的 `AudioCD.bin`。

已有研究快照时，直接用 `-MemorySnapshot` 指定。可选提取流程：

```powershell
python -m venv .venv
./.venv/Scripts/python.exe -m pip install -r tools/requirements.txt
# 进入你自己的原版游戏目录；下行使用工具的绝对路径
C:/projects/jxqy-native/.venv/Scripts/python.exe C:/projects/jxqy-native/tools/extract_memory.py
```

工具在当前游戏目录生成 `save_tool/memory.bin`，在 Unicorn 内隔离解释装载过程，不在操作系统中执行 DOS 游戏。它针对特定样本的研究装载状态，不生成可运行的脱壳 EXE；不要把提取结果提交仓库。

原已验证 `MAIN.EXE` 的 SHA256：`04e6ccb8bd83d487a8043921620ad944a6d8a07be61a09697ae6589bea0282c8`。其他版本不保证兼容。

## 本地部署

```powershell
./scripts/prepare-runtime.ps1 -GameDirectory 'C:/games/JXQY' `
  -MemorySnapshot 'C:/games/JXQY/save_tool/memory.bin' `
  -MediaDirectory 'C:/games'
```

生成结构：

```text
runtime/
  AudioCD.cue
  AudioCD.bin
  game/
    启动游戏.exe
    MAIN.EXE、原数据文件、字体、FIGHT/、VOC/
    save_tool/memory.bin
    main_decompiled/build/jx_native.exe
    main_decompiled/userdata/
    main_decompiled/analysis/
```

`main_decompiled` 是运行宿主目前要求的内部目录名，与公共源码目录布局无关。音乐相对于 `game/` 向上一层查找。请保留该结构。

脚本拒绝覆盖已有 `runtime/`。升级已有运行目录时，先备份存档，然后只替换游戏核心和启动器；不要用资源准备脚本覆盖玩家目录。

## 检查

```powershell
python ./scripts/check-source.py
./runtime/game/启动游戏.exe --verify-production
./runtime/game/启动游戏.exe --verify-window
./runtime/game/启动游戏.exe --verify-audio
```

从 PowerShell 自动等待 GUI 程序时应使用 `Start-Process -Wait -PassThru`，检查返回码及 `runtime/game/main_decompiled/analysis/` 中的报告。窗口测试会显示并关闭本任务的游戏窗口。不能仅凭程序退出码推断主线或结局通过。

## Git 提交前

```powershell
git status --short
git add --dry-run .
python ./scripts/check-source.py
```

提交列表应只有源码、脚本、文档及仓库配置。`runtime/`、`build/`、`.cache/`、`.venv/` 和资源文件都不应出现。许可证当前留空，不上传 `LICENSE` 占位授权文件。

