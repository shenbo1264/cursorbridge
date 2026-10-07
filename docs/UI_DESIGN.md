# Frosted glass settings — v0.8

## Direction / 设计方向

The settings window borrows the hierarchy of Apple's materials: a frosted outer surface, a quieter size/preview area, and restrained blue emphasis on the slider, switches and completion action. Text remains dark and readable; the preview uses an opaque well so tiny cursors remain visible. This is an independently implemented Windows interface, not Apple's Liquid Glass renderer.

新版采用浅色磨砂背景、细边缘、圆角控件与更清楚的留白。尺寸读数和预览放在最显眼的位置，样式／配色、窗口锁定、个人预设与快捷键按操作顺序排列。玻璃效果服务于层次，文字和预览优先保证清晰。

## Implementation

- Windows 11 build 22621 or later: documented `DWMWA_SYSTEMBACKDROP_TYPE` with `DWMSBT_TRANSIENTWINDOW` requests Desktop Acrylic. The DWM frame extends across the client area, and a premultiplied-alpha bitmap preserves the material behind our drawing.
- Windows 10, unsupported attributes or disabled system transparency: opaque light surface. High contrast requests an opaque surface and uses system foreground/background/accent colors. Windows can also choose its own fallback for power saving or an inactive window. We do not change Windows settings.
- Native edit, trackbar, combo, auto-checkbox and button controls remain responsible for values, focus and input. Subclass painting adds rounded surfaces, switches, hover/pressed feedback and visible focus outlines. Native edit drawing has its alpha repaired to prevent unreadable pale text over DWM glass.
- System Segoe UI and Microsoft YaHei UI fonts; no Apple font/assets or additional framework/runtime installation. Windows' built-in GDI+, DWM and common controls provide the rendering.
- The header can be dragged; the close button and Escape close the panel. Layout fits the current monitor work area. The preview is drawn at actual cursor pixels, independently of the panel's layout scaling.
- No desktop screenshots are taken by the application, no background blur runs in our own loop, and no new game/mod content is loaded. Only the small open settings window requests the system material. Desktop Acrylic can still cost GPU/power while visible.

## Verification and limits

Release builds use warnings as errors. A dedicated hidden native-panel test checks 30 cases: edit boundaries and normalization, native slider Home/End/arrow notifications, shape/color changes, disabled colors for the original pointer, toggles, complete preset persistence, restoration, preview cycling, language changes, rendering, close and reopen. It operates only on its own isolated controls and preferences; it does not inject user input or alter desktop settings.

Chinese and English window captures were inspected, including the 1–96 range, complete footer and accessibility control tree. Captures show the inactive solid fallback: Windows intentionally makes background Acrylic solid when the window deactivates. Automated desktop clicking could not activate this panel on the test desktop (`failed to activate captured window`); foreground blur appearance and full mouse/drag interaction with this revision are still pending manual confirmation. A successful DWM request establishes availability, not that Windows displays translucency in every context. Per-monitor DPI transitions, contrast themes and Windows 10 fallback also need visual checks on those configurations.

## Primary references

- [Apple HIG — Materials](https://developer.apple.com/design/human-interface-guidelines/materials): visual hierarchy, legibility and restrained use of glass.
- [Apple — Adopting Liquid Glass](https://developer.apple.com/documentation/technologyoverviews/adopting-liquid-glass): appearance and accessibility adaptations.
- [Microsoft — DWM_SYSTEMBACKDROP_TYPE](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwm_systembackdrop_type): documented Acrylic selection and OS support.
- [Microsoft — Custom Window Frame Using DWM](https://learn.microsoft.com/en-us/windows/win32/dwm/customframe): frame extension and alpha handling.
- [Microsoft — Acrylic material](https://learn.microsoft.com/en-us/windows/apps/design/style/acrylic): inactive, transparency, power-saving and high-contrast fallback; GPU cost.
