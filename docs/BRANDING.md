# CursorBridge mascot — A1

![A1 mouse mascot](../assets/branding/cursorbridge-a1.png)

The project owner selected **A1, 小鼠团子 (baby mouse)** from six independently generated candidates. A friendly mouse connects the product's cursor purpose with a small, approachable utility. Its lake-blue body, deep-indigo features and warm-apricot square background are preserved exactly as selected, including incidental shading and highlights.

## Source and reproducibility

- Master: `assets/branding/cursorbridge-a1.png`, the unmodified native **1254 × 1254** PNG.
- Master SHA256: `215ba93d0780d53700732366c20941e2d62afe46140243981cdd05f2e6ad93bb`.
- Prompt: [exact A1 prompt](../assets/branding/a1-prompt.txt). The prompt requested approximately 1536 square; the service returned 1254 square and the original was retained without enlargement.
- Provider: the built-in `image_gen` tool. Its metadata did not disclose the actual model; no model identity is claimed. Constraints were sent in the main prompt. A1 was generated once and selected by the owner; it was not redrawn or repaired.
- Windows derivative: `assets/branding/cursorbridge.ico`, **16, 20, 24, 32, 40, 48, 64, 128 and 256 px** frames encoded from the complete master. Only size/format conversion is applied; no crop, background removal, recoloring or new drawing.

The master and derivative are distributed under the project's [MIT license](../LICENSE). This is original project artwork generated with an AI tool; it contains no bundled game artwork.

## Developer use

The ICO is checked into source, so ordinary CMake and CI builds require **no image library**. To regenerate it from the unchanged master, install Pillow in your development environment and run `python tools/make_app_icon.py` (the original derivative used Pillow 12.1.1). This does not modify the master PNG. Runtime icon resource 101 is embedded into the executable independently from the existing hook/theme cache.

The application loads separate Windows large and small icons for its settings/controller windows and tray. Its own cursor size/style preferences do not affect its app icon. Release verification compares all nine compiled resource images byte-for-byte with the checked-in ICO, and checks default large/small extraction through Windows [ExtractIconExW](https://learn.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-extracticonexw). Actual taskbar/tray rendering can additionally depend on Windows scaling and icon caching.

---

项目所有者选定 A1“小鼠团子”作为程序形象。保留原图的完整构图、配色与背景，仅制作 Windows 所需的九种尺寸 ICO。程序文件、设置窗口与托盘使用同一形象；应用图标与用户选择的游戏光标样式互不影响。原始 PNG、生成提示词及 ICO 均随源码公开，普通构建无需 Pillow。
