# Validation scope — v0.7.0

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
