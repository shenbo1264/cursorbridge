# CursorBridge v0.9.4 — A1 mouse mascot

CursorBridge now uses its selected A1 baby-mouse mascot as the embedded Windows EXE icon, settings/taskbar icon and tray icon. The Windows ICO includes nine sizes from 16 to 256 px, preserving the complete selected artwork and background. Large and small window icons are loaded separately at the system's icon sizes. Icons no longer depend on unpacked cursor themes.

The unmodified 1254-square source PNG, exact generation prompt, ICO and [branding provenance](BRANDING.md) are included in source and the optional full ZIP. README download links point to this release. The EXE remains the only file needed to run; no image-generation service or image library is used at runtime or during a normal build.

Validation: six local CTest groups passed, including 62 native-panel checks and the existing connection/preferences groups. The compiled EXE's nine image resources match the checked-in ICO byte-for-byte; Windows extracts both its default large and small icons. EXE-only startup from a Unicode/spaced directory, all 109 embedded runtime files and exact-path shutdown passed. The hook and ABI remain unchanged; existing size, style and pause preferences are retained. No new third-party-game coverage is claimed.

---

新增用户选定的 A1“小鼠团子”图标，统一用于 EXE、设置窗口／任务栏与托盘。内置 16–256 像素的九种尺寸，保留原图完整画面和背景；程序图标不再依赖解包的光标样式文件。

源码及完整 ZIP 附原始 PNG、生成提示词、ICO 与来源说明。仍然只需下载 EXE 即可运行，不增加运行时依赖，原有尺寸、样式及暂停设置继续保留。六组本地测试、九种尺寸资源比对、Windows 大小图标提取及单文件启动检查通过。
