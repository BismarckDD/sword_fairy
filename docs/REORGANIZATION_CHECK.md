# 源码目录整理验收

日期：2026-10-16。环境：Windows x64、PowerShell 7、Zig 0.16.0。

- `All` 构建通过：游戏核心及 GUI 启动器，启用 C11、Wall、Wextra、Werror。
- `Events` 构建通过：事件研究库。
- 从无关工作目录启动，`--verify-production`、`--verify-window`、`--verify-audio` 均返回 0，报告均为 PASS。
- 正常启动显示游戏窗口，没有可见控制台；窗口测试完成 64 个世界帧和 2 次光盘音频提交。
- 音频测试完成 1 次 waveOut PCM 提交，验证设备接受数据，不将其表述为人工听音验收。
- 原资源仅放入被忽略的本地 `runtime/`；提交清单与源码压缩包不包含该目录、编译产物、缓存、存档或原游戏资源。
- `scripts/check-source.py` 核对迁移文件哈希、公共文件类型、大小及常见私人路径和令牌格式。

本次验证针对目录迁移、构建和启动/窗口/音频。未重新执行五条结局全流程；此前冻结版本的结果见 VALIDATION.md，不能视为此次新二进制的全部测试结果。

## 本次构建 SHA256

- `jx_native.exe`：`3bede1257b59582258edf2ecf8025ee523f355165a599b138cb4c72e18065a05`
- `game_launcher.exe`：`eaa8cb1ff493a1ac505fee848d5622e1075ef8efdf199302c4fdb9537a1e15b0`
- `events.dll`：`18605143e94fa7b7c551045ae762a9a6c98b7f15b44aa325c4219900548d2422`
