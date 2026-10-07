# Roadmap / 后续路线

## Shipped foundation / 已完成的基础

Single-file Windows x64 EXE; explicit experimental general Win32 adapter plus dedicated Stellaris adapter; 1–96 px resizing, exact input and keyboard controls; native art/hotspot/animation preservation; 12 original shape/color sets with native animation and semantic states; three saved presets; game-scoped hotkeys; optional foreground-only confinement with heartbeat recovery; Chinese/English UI; Steam/running-process/manual path discovery; single-player Workshop event/edict bridge; original synthetic fixtures and automated native regression tests. See the [YoloMouse comparison](YOLOMOUSE_COMPARISON.md).

## Next: stronger general application support / 完善通用应用适配

- Experimental Win32 x64 adapter and single-file delivery shipped in v0.9. Validate real applications, dynamic/module import routes, fullscreen/multi-monitor recovery and better status reporting.
- Automatic per-application profiles and per-cursor bindings, including remembered size/style choices for each target.
- Editable hotkey bindings and per-role size/style overrides in the panel.
- Portable settings-page patch generation with layout checks for each UI variant, avoiding shipped copies of UOD or other GUI files.
- Signed release binaries when a suitable signing process is available; reproducible build evidence and more game-version fixtures.

## Engine adapters / 游戏引擎适配

Separate process validation, cursor identification and any supported engine integration into adapters. Add games one at a time with restore/focus/size tests. A game using Win32 hardware cursors may be suitable; games drawing their own cursors need a different implementation and explicit review. No promise of working everywhere and no anti-cheat bypass.

## Later: original themes and motion / 原创样式与动效

A validated original/licensed CUR/ANI theme-pack format; per-game profiles; an end-user editor and frame timing controls. Built-in original themes and native animations are now available. Import validation, asset rights and resource limits come before sharing arbitrary packs. An independent rendering backend is needed before adding halo/glow/trails/zoom in exclusive fullscreen; its compatibility and performance must be measured.

The long-term direction is an independently built, open-source cursor customization tool in the same broad category as YoloMouse. Current Releases remain honestly labeled with the games and features that were actually tested. Milestones are priorities, not promised dates.
