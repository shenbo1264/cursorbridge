# CursorBridge

<img src="assets/branding/cursorbridge-a1.png" width="112" height="112" alt="CursorBridge A1 baby mouse mascot">

[简体中文](README.zh-CN.md) · [Download EXE](https://github.com/shenbo1264/cursorbridge/releases/download/v0.9.4/CursorBridge.exe) · [Release](https://github.com/shenbo1264/cursorbridge/releases/tag/v0.9.4)

**An independent MIT-licensed Windows x64 cursor customization tool.** Select a target application, resize its cursor and switch original themes without changing Windows desktop preferences. Started with Stellaris, v0.9 adds an **experimental general Win32 adapter** alongside the dedicated Stellaris adapter. The goal is a tool in the same category as YoloMouse; full feature parity is not claimed.

## Download and run

1. Download **[CursorBridge.exe](https://github.com/shenbo1264/cursorbridge/releases/download/v0.9.4/CursorBridge.exe)** and double-click. No installer, manual extraction, Python or separate DLL download is needed.
2. Click **Select target application…** in settings or the tray. Select the actual **64-bit application executable**. Click the new **Launch game / Launch & connect** button beside the selector, or start the target normally. If it is already running, the button connects to that process; after a failure, click **Connect game / Connect app** to retry.
3. Set size/style/color, then return to the target. Changes apply while it is foreground and the pointer is over its client area.
4. Minimize to the taskbar, or use Hide to tray / close the window to keep running. Left-click the tray icon to reopen settings; right-click for its menu. Quit app exits completely and restores the original cursor. The hook remains resident until the target exits; close the target before updating the tool.

The EXE embeds our hook and 108 original theme files. First launch prepares a versioned cache under `%LOCALAPPDATA%/CursorBridge/runtime/`; all cached files are compared against embedded bytes before use. Components are never downloaded. Preferences/logs live under `%LOCALAPPDATA%/CursorBridge/`. Ordinary users only need the EXE; the optional full ZIP includes documentation, legacy launchers and Stellaris scripts.

Existing users retain Steam/running-Stellaris discovery when no target is selected. A complete Stellaris installation uses its resource-aware adapter; another native x64 EXE uses the general adapter. Only the selected complete path is eligible, not every foreground application.

## No change inside the target?

**No Workshop mod is required to resize Stellaris cursors.** Check the footer: waiting means the selected target has not been found; a failed connection can be retried with the connection button. If paused, explicitly turn on Enable cursor adjustment. Editing size, style or presets preserves pause, including across restarts. Once connected, use Return to game / Return to app or switch to the target and move the pointer into its client area. The panel preview alone does not prove a connection.

A hotkey conflict only affects those shortcuts; the panel still works, and Add Shift can avoid a collision. Connection, pause and size remain visible separately from shortcut/save notices.

## v0.9.4 preview

- A1 baby-mouse app icon embedded in the EXE, settings/taskbar and tray, with nine native Windows sizes. [Artwork and provenance](docs/BRANDING.md).

- Real taskbar minimization, Hide to tray and explicit Quit app. Settings are no longer always on top and never automatically hide or reopen. Left-click the tray icon to open settings; right-click for its menu.
- Explicit enable switch; editing size, style or presets preserves pause, including across restarts.
- Launch/connect and retry directly from settings. Once connected, the same action returns to the target window without creating another instance. Settings remain after target exit.
- Accessible status text, separate helper notices, clear preset actions and hover help. Wheel resizing is restricted to the size area.

- Single-file launch, general interface/tray wording and explicit application selection.
- Frosted settings, native controls, keyboard focus and actual-pixel preview. Honors OS transparency/high contrast; opaque fallback where needed. [UI details](docs/UI_DESIGN.md).
- **1–96 px**, exact input and slider keyboard control. Start around 24–32 px; 1 px is extreme. Hotspots stay within the image.
- **12 original themes**: arrow/crosshair/ring × white/cyan/amber/pink, outlines and animated state variants.
- Three personal presets, target/panel-scoped hotkeys, optional Shift modifier and optional foreground-only confinement. Presets currently are shared across targets.
- Restore/pause, original-handle protection, bounded cache, heartbeat recovery and path/architecture checks.
- Chinese/English UI: Stellaris follows its game language; general applications follow Windows user UI language, Chinese or English fallback.
- Optional Stellaris event/edict and local settings-page button. **Workshop subscription alone cannot resize cursors.**

## Compatibility and YoloMouse scope

The general adapter is **experimental**, verified in our separate native x64 test host. It intercepts the target main module's `USER32!SetCursor` import. Software-drawn/hidden cursors, dynamically resolved APIs, calls made only in other modules, 32-bit applications and protected processes are unsupported by this backend. No universal-game compatibility or anti-cheat certification is claimed.

General original mode copies/scales observed Win32 cursors and clamps out-of-bounds hotspots. Arbitrary original ANI animation is not guaranteed. Stellaris recognizes nine installed CUR/ANI sources and preserves their native animation; no game artwork is bundled. Standard Win32 roles retain hand/busy/blocked theme variants. Unknown custom roles use the base theme without invented friendly/enemy meaning.

Trails, glow/halo, zoom, original-art recoloring, arbitrary import/editor, per-cursor binding and automatic per-app profiles are **not implemented**. [YoloMouse comparison](docs/YOLOMOUSE_COMPARISON.md) · [Roadmap](docs/ROADMAP.md).

No driver, service, autostart, telemetry or network client. Run at the target's normal privilege level. The preview is unsigned; obtain it from this repository's Releases and verify SHA256. [Security scope](SECURITY.md).

## Commands

```text
CursorBridge.exe
CursorBridge.exe --settings
CursorBridge.exe --background
CursorBridge.exe --launch
CursorBridge.exe --stop
CursorBridge.exe --app-path "D:\Apps\MyApp.exe" --settings
CursorBridge.exe --game-path "D:\SteamLibrary\steamapps\common\Stellaris\stellaris.exe" --settings
CursorBridge.exe --data-dir "D:\CursorBridgeData"
CursorBridge.exe --game-log "D:\CustomStellarisProfile\logs\game.log"
```

`--app-path` selects the general adapter; `--game-path` requires a complete Stellaris layout. Manual selection auto-detects the adapter. Launch supplies `-skiploop` only for Stellaris. Keep using the normal launcher where required. `--game-log` is specific to the Stellaris bridge. Invalid/incomplete options return code 2.

## Stellaris integration

The optional `workshop/` contains original event/edict scripts and ten locale folders. It adds uniquely named on-actions without replacing `00_on_actions.txt` or UOD/Dark Blue GUI files. No Workshop item is published yet. [Upload/local-install guide](docs/WORKSHOP_UPLOAD.md).

The single-player bridge accepts only whitelisted new event/log records. Start the companion first or reopen the edict. Multiplayer is untested; the mod affects checksums and achievements are not promised. The slider is the companion panel, not an engine-native setting.

`tools/create_settings_patch.py` generates a local settings button from matching installed UI source. Python 3.10+ is required only for this tool. Generated third-party GUI stays local, loads after its source and must be regenerated after UI updates. Do not upload it. [Architecture](docs/ARCHITECTURE.md).

## Build and verify

Windows x64, VS 2022 C++ Build Tools/Windows SDK, CMake 3.21+, Python 3.10+. No third-party hook library. Developer builds retain `StellarisCursor.exe`; public standalone releases use `CursorBridge.exe`.

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
python tools/make_test_assets.py "build/Synthetic Game"
$fixture = Join-Path (Get-Location) 'build\Synthetic Game\stellaris.exe'
$data = Join-Path (Get-Location) 'build\test-data'
$test = Start-Process 'build\bin\StellarisCursor.exe' -ArgumentList ('--self-test --game-path "{0}" --data-dir "{1}"' -f $fixture, $data) -PassThru -WindowStyle Hidden -Wait
$test.ExitCode
$generic = Start-Process 'build\bin\StellarisCursor.exe' -ArgumentList '--self-test-generic' -PassThru -WindowStyle Hidden -Wait
$generic.ExitCode
python tools/verify_standalone.py --build-dir build
```

Tests inject only the exact-path dedicated test host, never the layout marker or a user application. Reports under `build/logs` cover 40,700 Stellaris and 34,969 general-adapter checks. Six CTest groups cover bridge, language, paths, preferences, hidden native controls and isolated process connection. Single-file verification checks the compiled app icon as well as its embedded runtime. Counts do not replace long-session/real-game testing. [Validation scope](docs/VALIDATION.md). Packaging regenerates original cursor themes and checks a public-file allowlist.

[Contribute](CONTRIBUTING.md) · [Security](SECURITY.md). Remove personal paths from shared logs. Independent of Paradox Interactive, Steam and YoloMouse. Original code/scripts/fixtures/art use [MIT](LICENSE); game assets and generated third-party GUI do not.
