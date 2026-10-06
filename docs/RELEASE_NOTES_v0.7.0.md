# CursorBridge v0.7.0 — Shapes, presets and game controls

**Windows x64 / Stellaris 4.5.1. Companion program required.**

## New / 新增

- Twelve independently generated arrow/crosshair/ring sets in white, cyan, amber and pink, with a dark outline. Friendly/attack/blocked states retain semantic colors, grab states retain different shapes and busy/movement cursors have native ANI animation.
- Exact 1–96 px numeric entry and a keyboard-accessible slider: arrows, Home/End, Page Up/Down and Tab navigation.
- Three persistent size/theme/confinement presets; unused slots start at 24/32/48 px in original mode.
- Foreground-scoped shortcuts for the panel, size, presets, restoration and confinement. An optional Shift modifier avoids occupied bindings; unavailable shortcuts are reported.
- Opt-in client-area cursor confinement with focus-loss and heartbeat recovery.
- Chinese/English controls, theme preview and state cycling. The desktop pointer remains unchanged.
- Closing the focused panel restores a still-connected fullscreen game automatically.
- Cache signature revalidation protects against recycled Win32 cursor handles. ABI v2 rejects mismatched old mappings.

新增 12 套原创光标、精确数字输入、键盘操作、3 个个人预设、快捷键及备用 Shift 组合、前台窗口锁定和状态动画预览。游戏处于前台时应用设置；切出游戏或暂停时解除工具拥有的锁定。

## Upgrade / 更新

Close the game and companion, extract the **entire** player ZIP and keep `bin/assets/themes` next to the EXE/DLL. Start **Open Settings.cmd**, then start the game normally. Existing size settings remain valid. No installer or Python is needed to run the program. Binaries remain unsigned.

先退出游戏与工具，完整解压玩家 ZIP，保留 `bin/assets/themes`。再运行 **Open Settings.cmd** 并正常启动游戏。原有尺寸配置可以继续使用，运行程序无需 Python。

## Download files / 下载文件

- `CursorBridge-Stellaris-windows-x64.zip`: companion, original themes, optional entry mod and bilingual documentation.
- `CursorBridge-Stellaris-workshop-upload.zip`: original scripts/localization/preview only; requires the companion.
- `SHA256SUMS.txt`: checksums. Automatic source archives are for developers.

## Validation and limits / 验证与边界

See [validation](https://github.com/shenbo1264/cursorbridge/blob/v0.7.0/docs/VALIDATION.md) and the [YoloMouse feature comparison](https://github.com/shenbo1264/cursorbridge/blob/v0.7.0/docs/YOLOMOUSE_COMPARISON.md). The native suite exercises **40,700 checks** per fixture. Stellaris 4.5.1's normal/selected/dragselect assets are identical, so this bitmap adapter uses one basic shape for those three states. This is a Stellaris public preview, not full YoloMouse parity: other games, arbitrary pack import, an end-user editor and overlay effects remain future work. No game artwork is included. The packager regenerates and verifies all original theme assets before shipping.
