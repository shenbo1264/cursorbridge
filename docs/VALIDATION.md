# Validation scope — v0.9.2

## v0.9.2 launch/connect button

- Six local CTest groups passed: 42 native-panel checks and 12 real connection checks in addition to the existing regression groups.
- The new settings button launched only our unique temporary x64 fixture, the timer connected it and published 13px, and an already running fixture with a previously failed PID connected through the same button. Paused adjustment resumed; exact-path process enumeration verified no duplicate instances. The desktop cursor resource was unchanged, and the settings panel remained after fixture exit.
- Native controls cover Chinese/English labels, generic application wording, starting/connecting disabled states and prevention of target switching during a pending operation.
- The actual Chinese panel was visually inspected: the new button fits beside the target selector, and the current user preference is retained. No live third-party game was launched for this change; the hook and ABI remain the v0.9.1 baseline.

## v0.9.1 status and live-target verification

- All five local CTest groups passed; native-panel checks increased from 32 to 36. Active connection and size remain visible during a shortcut conflict; Chinese and English paused states and failed connections are covered.
- In the installed Stellaris 4.5.2 main menu, the 48px source cursor changed to 24px then 13px through the companion panel. Read-only GetCursorInfo/GetIconInfo observations matched the corresponding resized game resources, including dimensions and bitmap hashes.
- The updated controller reconnected to that game and applied 13px after returning from the panel. The 13px preference was written to the normal settings file. The test game was exited from its main menu; no save was opened and no playset was edited.
- This patch does not change the hook DLL or ABI. These checks do not establish new generic-game compatibility or long-session stability.

## v0.9.0 hook and packaging baseline

- MSVC Windows x64 Release build, warnings-as-errors: passed.
- Five CTest groups: passed, including 32 hidden native-panel checks and 16 path checks.
- Dedicated native host / synthetic Stellaris assets: **40,700 checks, zero failures**.
- Dedicated generic Win32 x64 host: **34,969 checks, zero failures**. Seven standard/unknown source roles, all 96 sizes and 13 original/theme modes; dimensions, hotspots, pixel/signature equivalence, unchanged sources, pause, invalid input and heartbeat expiry.
- EXE copied alone into a Unicode/spaced folder: startup and exact-path stop passed; all **109 embedded runtime files** matched the generated manifest's size/SHA256. Target was an explicitly selected never-run copy of our own test EXE, avoiding real application attachment.
- Workshop scripts/localization and public-file/package guards: required before publication. GitHub Actions reruns native, general and single-file checks for this source/tag.

General tests run in our dedicated host with pointer-context bypass, not a real third-party game. They do not establish universal compatibility, foreground/input behavior for every engine, software-cursor support, anti-cheat approval or long-session stability. The general adapter's foreground/confinement worker shares the existing tested implementation, but new third-party applications require their own testing.

The owner accepted the frosted material revision. Earlier development checks observed a colored backdrop through blurred boundaries; final general-selector interactions were not visually rechecked. Windows 10/contrast/power saving/unusual DPI and every activation transition remain unverified. Historical v0.8 JPGs show its older gray fallback, not v0.9 glass. See [UI details](UI_DESIGN.md).

The ABI is now 3: target mode, single-file runtime and general recognition changed; original theme hotspots were adjusted to keep native 1px rounding inside the image. Restart a connected target before using the new build. No user savegame or mod playset was changed for current release checks.

## Historical v0.8 checks

## New settings panel

- Windows x64 Release build with warnings as errors: passed.
- Five local CTest groups: passed, including **30 native-panel checks** for boundary values, slider keyboard notifications, selections, toggles, presets, restoration, preview, language changes, rendering and close/reopen.
- Chinese and English captures inspected: all controls and bottom actions visible, native value/checkbox/combo accessibility tree present. Desktop Acrylic request succeeded on the Windows 11 test host.
- The screenshots show the inactive solid fallback. Desktop input automation returned `failed to activate captured window`, so this revision's active blur and full mouse/drag interaction are **not manually verified**. Windows 10, high contrast and unusual DPI configurations require separate visual checks. Details: [UI design](UI_DESIGN.md).
- Cursor hook, shared ABI, game adapter and Workshop scripts are unchanged, except the Workshop descriptor version. Existing v0.7 checks below are retained as historical evidence, not relabeled as new manual testing. Source CI reruns the synthetic native suite for this tag.

## Existing cursor/adapter baseline — v0.7


| Check | Result | What it establishes |
| --- | --- | --- |
| Windows x64 MSVC Release build, warnings treated as errors | Passed locally | Source builds with the documented compiler |
| Native hook regression with original synthetic CUR/ANI fixtures | 40,700 checks, zero failures | All nine resources × 96 sizes in original mode and 12 themes; rendered pixels, hotspots, animated second frames, return semantics, invalid values and restoration |
| Native hook regression with installed Stellaris 4.5.1 resources | 40,700 checks, zero failures | Real resource recognition/scaling and deterministic handling of indistinguishable normal/selected/dragselect pointers |
| Bridge parser/tail regression | 153 checks, zero failures | Whitelisting, partial lines, rotation, repeated requests and bounded input |
| Language and active profile regression | 134 checks, zero failures | Chinese/English strings and fallback, settings precedence and actual command-line profile reading |
| Installation path regression | 13 checks, zero failures | Steam VDF, Unicode/spaces, Known Folders and missing-resource rejection |
| Preferences regression | 38 checks, zero failures | Theme mapping, three saved slots, invalid persisted values and modifier configuration |

Native suites run in an exact-path dedicated test process, including the real-resource run. Synthetic fixtures now have distinct bitmap signatures for all nine roles. The game itself uses identical files for normal/selected/dragselect: their first matching base shape is the honest expected behavior of this adapter. Original cursor handles/signatures are not modified. Cached handles are revalidated to handle Windows recycling a destroyed cursor handle.

## Manual Windows / Stellaris checks

Tests used isolated profiles with UOD, Dark Blue and the previously verified dependency set, without editing the normal enabled-mod list or user settings.

- Chinese windowed game and Chinese panel; numeric entry, arrow shortcut, Tab navigation and Home/End endpoints 1/96. An input of 97 left the saved size at 96 and reverted when focus left the field.
- Shape selection and cyan palette; saved preset 1 (24 px / cyan crosshair / confinement on), switched to unused preset 2 (32 px / original / confinement off), then restored the complete first preset.
- Default Ctrl+Alt+L was occupied on this machine. The panel reported that binding; enabling the saved Shift modifier removed the conflict. Ctrl+Alt+Shift+C opened the panel and Up changed 24 to 25 px. The restore shortcut worked in the panel and in the game.
- Actual game cursor measured at 25×25 and 24×24, with hotspot and bitmap hashes matching the generated theme loaded at the same size. Diagnostics were DPI-aware; screenshots alone were not used to infer hardware cursor dimensions.
- Windowed confinement covered the game's screen-coordinate client bounds. Opening the panel released it to the full desktop rectangle. Restoration disabled confinement and returned the original 48×48 game pointer.
- English game configured for fullscreen with borderless disabled; English panel and actual 24×24 cyan crosshair verified. When fullscreen minimized on panel focus, closing the final panel automatically restored the game to the foreground. The restore shortcut returned the original 48×48 pointer.
- Abrupt termination of our own test companion while the game remained foreground: after heartbeat expiry the DLL released its confinement flag and restored the 48×48 original without restarting the game. Reconnection also succeeded. This does not establish behavior after every OS-level failure.
- Chinese and English final-panel screenshots are included. The application remains Stellaris-specific; built-in styles are not arbitrary user-import support.

The previous v0.6 smoke coverage of the unchanged event/log bridge includes single-player startup/edict opening and a locally generated UOD/Dark Blue settings button, with repeated requests on the same game date. Version 0.7 does not change those scripts except the descriptor version. The ten localization folders still pass 16-key parity and script validation. No third-party GUI is distributed.

The normal settings and enabled-mod-list hashes were checked before and after these tests. No savegame was loaded or modified. These are main-menu and isolated-process checks, not a lengthy campaign or multiplayer certification. Long sessions, unusual multi-monitor/DPI setups, other games, software cursors and future game versions still need specific coverage.

## Release checks

The package excludes local logs/profiles, test binaries and proprietary game resources. Every generated theme asset is independently regenerated and compared byte-for-byte by the packager before inclusion. ZIP CRCs and a per-file SHA256 manifest are verified. Public source CI and the downloaded Release asset are checked during publication; see the tagged release for the corresponding source and checksums.
