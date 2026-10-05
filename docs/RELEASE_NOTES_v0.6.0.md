# CursorBridge v0.6.0 — Stellaris public preview

**Windows x64 / Stellaris 4.5.1. Companion program required.**

## Download / 下载

- Players: **CursorBridge-Stellaris-windows-x64.zip** — extract the whole archive and run **Open Settings.cmd**. Keep the EXE and DLL together in `bin`.
- Workshop uploader: **CursorBridge-Stellaris-workshop-upload.zip** — scripts, localization, descriptor and original preview only. This does not contain a standalone cursor resizing engine.
- **SHA256SUMS.txt** — checksums for both files. GitHub's automatic source archives are for developers.

下载玩家程序包后完整解压，双击 **Open Settings.cmd**，再正常启动《群星》。工坊上传包只提供游戏内入口，仍需要运行配套程序。程序未签名，源码采用 MIT 协议，可自行检查与构建。

## Included / 本版内容

Live 1–96 px sizing, original game cursor/animation preservation, restore/pause/exit controls, saved size preference, Chinese UI for Chinese game settings and English fallback for other languages. Steam library discovery, running-installation detection, manual executable selection and redirected Windows Known Folder support remove the prototype's private machine paths.

Optional single-player event/edict integration is included. A Python generator can create a local native settings-page button from an installed UI; generated third-party GUI files are not bundled or redistributable project content.

## Validation / 验证

4,771 native checks passed with original synthetic fixtures, and separately with installed Stellaris resources. Bridge (153), language/profile (80) and installation paths (13) passed. GitHub Windows CI independently builds and runs the synthetic suite and validates ten localization folders. Actual single-player UI/resize/restore integration was smoke-tested; details are in docs/VALIDATION.md.

## Limits / 当前边界

This is a **public preview**, not a universal cursor replacement program. Linux/macOS, arbitrary games, user-defined themes/animations, protected multiplayer support and long-session certification are not implemented. Existing animations are preserved; custom animation packs are future work. The Workshop script mod affects the checksum. The slider is a floating companion panel, and the settings button is an optional locally generated patch.

Close the game and companion before replacing DLL versions. No game assets, old local development logs, personal paths, drivers or installer are included.

Source: https://github.com/shenbo1264/cursorbridge
中文说明: https://github.com/shenbo1264/cursorbridge/blob/main/README.zh-CN.md
