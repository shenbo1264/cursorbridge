# YoloMouse comparison / 功能对照

Reviewed 2026-10-06 against YoloMouse's [official help](https://dragonrisegames.com/yolomouse/help), [Steam feature list](https://store.steampowered.com/app/1283970/YoloMouse/) and [official release log](https://dragonrisegames.com/yolomouse/log), which lists 1.23.1 dated 2026-09-26. These are a feature inventory, not independently verified performance or anti-cheat claims. CursorBridge uses its own code and original artwork.

| Capability / 能力 | CursorBridge 0.7 |
| --- | --- |
| Original cursor resizing / 原光标缩放 | 1–96 px, exact integer input, keyboard-accessible slider; original artwork/hotspots/ANI retained |
| Replacement shapes and colors / 替换样式与配色 | 12 original sets: arrow/crosshair/ring × white/cyan/amber/pink, plus game original; outline included |
| State-specific pointers / 状态区分 | Nine source files recognized; friendly green, attack red, blocked amber; spinning busy ANI and pulsing movement ANI. In 4.5.1, normal/selected/dragselect are identical and share the base pointer; grab/grabbing remain distinct |
| Presets / 预设 | Three user slots, each stores size/theme/confinement; persistent local configuration |
| Hotkeys / 快捷键 | Game/panel-scoped Ctrl+Alt+C, Up/Down, 0, 1/2/3 and L; collision warning and optional Shift modifier; letter bindings are fixed in this release |
| Window confinement / 窗口内锁定 | Opt-in, client area only; released on focus loss, pause, controller expiry/exit; does not override an existing different clip |
| Language / 语言 | Chinese for Chinese game settings, English fallback for every other language |
| Original recoloring/inversion / 原光标染色或反色 | Not implemented; palettes apply to our replacement sets |
| Trails, zoom, halo, glow, mirror, jiggle / 拖尾、放大镜、光晕、发光、镜像、摆动物理 | Not implemented; require an independently tested rendering backend, including exclusive-fullscreen behavior |
| User CUR/ANI import, editor, sharing / 导入、编辑器与分享 | Not implemented; own set generator is developer tooling, not an end-user editor or arbitrary import support |
| Per-cursor size/style overrides / 按单个光标指定配置 | Not implemented; one set applies to the nine recognized roles |
| Arbitrary apps/Windows/other games / 任意应用、桌面与其他游戏 | Stellaris-only adapter; desktop cursor settings unchanged |
| Multiple monitors / 多显示器 | Panel opens on foreground monitor, confinement follows screen-coordinate client bounds; unusual DPI/multi-monitor setups still need coverage |
| Sensitivity toggle / 灵敏度切换 | Not implemented; changing global Windows sensitivity would affect unrelated apps and is not our current game-local backend |
| Linux/Proton | Not implemented |

## Controls / 操作

All shortcuts use **Ctrl+Alt**, or **Ctrl+Alt+Shift** when **Add Shift to hotkeys** is checked in the panel. This alternate combination is saved separately from cursor presets and can avoid occupied shortcuts. **C** opens settings, **Up/Down** changes size by one pixel, **0** restores the game original and turns off confinement, **1/2/3** loads a preset, **L** toggles confinement. Shortcuts are registered while the connected game or its settings panel is foreground and unregistered on the next 200 ms check after focus loss. Commands recheck foreground before applying. A short registration transition may still consume a shortcut immediately after switching applications; no unrelated application command is intentionally executed. If a binding is already occupied, the panel warns and remains usable.

The slider supports arrows, Home/End and Page Up/Down (8 px); use Tab to reach controls. Sizes outside 1–96 do not apply and the previous valid value is restored when leaving the box. Save/load buttons use three local slots. Defaults for unused slots are 24/32/48 px with original artwork and confinement off.

Confinement is **off by default**. The original game pointer is restored while the floating panel has focus. Return to the game to apply the selected size/style. A fullscreen game may minimize when the panel takes focus; closing the panel restores the connected game automatically when the panel was foreground. The confinement worker runs in the target process and checks the companion heartbeat: an abnormal companion exit releases its owned clip after about four seconds plus the refresh interval. It avoids removing a different clip installed by another application. This is a bounded recovery policy, not a promise of immediate recovery after all OS failures.

## Next implementation priorities / 后续优先级

1. Validated complete CUR/ANI packs, per-role overrides and a simple original theme editor; no imported game/IP assets bundled.
2. Configurable hotkeys and profiles through the panel.
3. Independent overlay backend for halo/glow/trails; verify exclusive fullscreen, multiple displays, focus and frame timing before advertising those effects.
4. Explicit adapters for additional games with appropriate process validation and restore tests. Linux/Proton requires a separate backend.

This update closes the practical first group of gaps. It does **not** claim complete YoloMouse feature parity, universal compatibility, a performance advantage or anti-cheat certification.
