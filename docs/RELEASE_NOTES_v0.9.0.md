# CursorBridge v0.9.0 — single-file EXE and general Win32 preview

## 中文

- **直接下载 CursorBridge.exe，双击打开设置。** 内置组件自动在用户目录准备并逐文件校验，不需要手动解压或另放 DLL。
- 设置页增加“选择目标程序”，托盘改为“启动目标程序”等通用文案，并显示目标 EXE 名称。
- 新增实验性 **Windows x64 原生光标后端**，明确选定程序完整路径后尝试连接；保留《群星》专属资源／工坊适配。
- 保留 1–96 像素、12 套原创主题、三个个人预设、快捷键及可选前台窗口锁定。修复 1 像素热点越界和外部滑块值不同步。
- 修正 Windows 11 新版的透明通道，沿用磨砂设置界面。材质依系统和兼容性回退，部分原生 accent 参数属于实验性实现。
- 完整中英 README、功能对照、架构与验证说明；提供 SHA256。

通用后端仅覆盖目标主模块的 Win32 SetCursor 导入。自绘光标、32 位、动态／其他模块调用及受保护进程不在当前范围内；未宣称所有游戏兼容。通用原光标 ANI 保留不作保证。预设当前跨目标共享。光晕、拖尾、编辑器、逐光标绑定和自动按应用配置仍在路线图中。

## English

Download **CursorBridge.exe** and double-click to open settings. The single-file build embeds its hook and original artwork, preparing and verifying a local runtime automatically without downloads or manual extraction.

Adds an explicit application selector and general tray wording, plus an experimental native Win32 x64 adapter alongside Stellaris. Retains 1–96 px, 12 original themes, three presets, contextual hotkeys and opt-in confinement. Corrects extreme-size hotspots, external slider synchronization and the Windows 11 redirection alpha channel.

General compatibility is limited to the selected main-module SetCursor import. Software cursors, 32-bit/protected targets and other import routes are not supported; arbitrary original ANI preservation is not guaranteed. Presets remain shared. This is an **unsigned preview**, not full YoloMouse feature parity or universal-game certification.

## Downloads

- **CursorBridge.exe** — recommended single-file program.
- **CursorBridge-windows-x64.zip** — optional program/documentation/legacy-launcher package.
- **CursorBridge-Stellaris-workshop-upload.zip** — original Stellaris scripts/localization; companion required. No Steam Workshop item has been published.
- **SHA256SUMS.txt** — hashes for all three assets.

## Validation

Windows x64 Release build with warnings-as-errors; five native unit groups; independent 40,700-check Stellaris and 34,969-check general-adapter suites; EXE-only launch from a Unicode/spaced folder with all 109 embedded files verified; Workshop script/localization and public-package guards. See repository `docs/VALIDATION.md` for actual results and untested scope. General tests use our dedicated host, not real third-party game certification.
