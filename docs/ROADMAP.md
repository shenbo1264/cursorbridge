# Roadmap / 后续路线

## Shipped foundation / 已完成的基础

Windows x64 Stellaris adapter; 1–96 px resizing; native art/hotspot/animation preservation; local preferences; Chinese/English UI; Steam/running-process/manual path discovery; single-player Workshop event/edict bridge; original synthetic fixtures and automated native regression tests.

## Next: a stronger Stellaris release / 先把《群星》版做好

- More long-session and fullscreen/multi-monitor testing, clear error/status reporting and better update/reconnect handling.
- Keyboard-accessible controls and numeric size input; cursor lost/too-small recovery shortcut scoped to the supported game.
- Portable settings-page patch generation with layout checks for each UI variant, avoiding shipped copies of UOD or other GUI files.
- Signed release binaries when a suitable signing process is available; reproducible build evidence and more game-version fixtures.

## Then: explicit game adapters / 再扩展到其他游戏

Separate process validation, cursor identification and any supported engine integration into adapters. Add games one at a time with restore/focus/size tests. A game using Win32 hardware cursors may be suitable; games drawing their own cursors need a different implementation and explicit review. No promise of working everywhere and no anti-cheat bypass.

## Later: original themes and motion / 原创样式与动效

An original/licensed theme-pack format for shapes, outline, color and contrast; per-game profiles; size/hotspot previews; optional native animation packs and frame timing controls. Import validation, asset rights and resource limits come before sharing packs. Existing Stellaris animations are already preserved; these planned features mean **user-defined** styles and animations.

The long-term direction is an independently built, open-source cursor customization tool in the same broad category as YoloMouse. Current Releases remain honestly labeled with the games and features that were actually tested. Milestones are priorities, not promised dates.
