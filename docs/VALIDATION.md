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

Manual game smoke results are recorded separately before release. The scope is a fresh single-player test profile, not a lengthy campaign or multiplayer session. The original user's normal profile and mod list are not changed during these checks.

Still needing broader coverage: long sessions, unusual DPI/multi-monitor configurations, other operating systems, software cursors, future game versions, protected multiplayer games, and settings-page layouts outside the tested UOD/Dark Blue combination. Other games and custom theme/animation packs are roadmap items.
