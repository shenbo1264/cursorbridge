# Validation scope — v0.6.0

This release was prepared from an earlier Stellaris-only prototype and then rebuilt with runtime paths and Windows Known Folders. No prior local prototype archives are used as public release assets.

| Check | Result | What it establishes |
| --- | --- | --- |
| Windows x64 MSVC Release build, warnings treated as errors | Passed locally | Source builds with the documented compiler |
| Native regression with original synthetic CUR/ANI fixtures | 4,771 checks, zero failures | All 96 sizes, rendering, animated frame preservation, return semantics, invalid values and recovery |
| Native regression with installed Stellaris 4.5.1 resources | 4,771 checks, zero failures | The portable adapter still recognizes and scales the real nine resources |
| Bridge parser/tail regression | 153 checks, zero failures | Whitelisting, partial lines, rotation, repeated requests and bounded input |
| Language and active profile regression | 80 checks, zero failures | Chinese/English fallback, settings precedence and actual command-line profile reading |
| Installation path regression | 13 checks, zero failures | Modern/legacy Steam VDF, Unicode/spaces, invalid input, Known Folders and missing-resource rejection |

The two native runs use a dedicated test process. They do not certify all savegames or arbitrary mod stacks. CI runs the synthetic suite without downloading any game assets. Public runtime packages exclude the test host and development reports.

Manual smoke checks in a fresh single-player profile with UOD, Dark Blue and their existing dependencies passed: main menu and new-game flow, active `-userdir` detection, Chinese slider, 1/96 panel endpoints, 16×16 actual in-game cursor, persistence, startup-event opening, locally generated settings button, repeated N_1/N_2 requests on the same game date, and restoration to the 48×48 original cursor. No CursorBridge script errors were found in the game error log. Existing third-party UI warnings are outside this check's scope.

The normal user's settings and enabled-mod-list file hashes were checked and remained unchanged. The scope is a fresh test profile, not a lengthy campaign or multiplayer session. An initially incomplete isolated UI profile exited before loading even without the companion; tests proceeded with the complete previously validated UI dependency set. This is why broad playset compatibility is not claimed.

The English game main menu and English floating panel were also checked in a separate profile with the final controller build; Chinese and English screenshots are included. Non-Chinese fallback is additionally covered by language classification tests. An explicit log override remains configured when the game exits, instead of reverting to a different profile.

Still needing broader coverage: long sessions, unusual DPI/multi-monitor configurations, other operating systems, software cursors, future game versions, protected multiplayer games, and settings-page layouts outside the tested UOD/Dark Blue combination. Other games and custom theme/animation packs are roadmap items.
