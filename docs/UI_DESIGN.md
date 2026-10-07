# Frosted settings — v0.9 preview

The panel borrows Apple's material hierarchy: a frosted outer surface, restrained blue controls and a readable opaque cursor-preview well. This is an independent Windows implementation, not Apple's Liquid Glass renderer. Native edit, trackbar, combo, checkbox and button controls retain keyboard/value/accessibility behavior; GDI+ subclass painting provides rounded surfaces and focus feedback.

## Material implementation

- Requests documented Desktop Acrylic through DWM, with the frame extended across the client area and premultiplied-alpha drawing.
- Windows 11 24H2+ requires `DWMWA_REDIRECTIONBITMAP_ALPHA` (39) to honor the redirection bitmap's alpha. Version 0.9 explicitly requests it; older Windows reject unsupported attributes without preventing launch.
- A dynamically resolved native `SetWindowCompositionAttribute` accent backdrop supplies a controllable frosted tint. **Accent-policy constants are not a public compatibility contract**; this backend is experimental. Failure retains the documented DWM route. Future Windows updates may require a change.
- Respects user transparency/high-contrast preferences. Disabled transparency, high contrast or unavailable composition uses an opaque readable surface. No Windows settings are changed. Windows may also apply its own power/inactive fallback.
- Uses Windows GDI+, DWM, common controls and installed Segoe UI/Microsoft YaHei UI fonts. No desktop screenshots/background-capture blur loop, Apple assets or extra UI framework is installed.

The header remains draggable. Minimize keeps a taskbar window; Escape/close/Hide to tray dismiss settings while the companion stays running; Quit app exits completely. Settings are not forced above other apps and never automatically hide or reappear. Preview reflects chosen settings even while paused, in actual pixels independently of layout scaling. The application selector is now visible near the header. Layout fits the monitor work area; per-monitor DPI transitions need more coverage.

## Verification limits

The v0.8 captures in `docs/images/glass-panel-*.jpg` are **historical**, showing its gray fallback, not evidence for v0.9 transparency. During v0.9 development a colored native test backdrop visibly showed through with blurred boundaries, and numeric/slider updates were exercised. The final material revision was inspected and accepted by the project owner; a complete automated visual matrix was not performed.

The hidden native panel suite now has **58 checks**: value boundaries, normalization, slider keyboard and external-value synchronization, selections, toggles, presets, pause persistence, paused edits/preview, wheel scope, accessible status text, window styles, language changes, rendering and close/reopen. This verifies our controls, not OS blur. General-application selection/branding was added afterward and builds under warnings-as-errors. Windows 10, contrast themes, power saving, unusual DPI and all activation transitions are not visually certified. A successful API return alone does not prove the displayed material is translucent.

## Primary references

- [Apple HIG — Materials](https://developer.apple.com/design/human-interface-guidelines/materials).
- [Microsoft — DWMWINDOWATTRIBUTE](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwmwindowattribute): redirection bitmap alpha and OS support.
- [Microsoft — DWM_SYSTEMBACKDROP_TYPE](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwm_systembackdrop_type).
- [Microsoft — Custom Window Frame](https://learn.microsoft.com/en-us/windows/win32/dwm/customframe).
- [Microsoft — SetWindowCompositionAttribute](https://learn.microsoft.com/en-us/windows/win32/dwm/setwindowcompositionattribute): dynamically resolved API; Microsoft recommends documented DWM attributes instead.
- [Microsoft — Acrylic](https://learn.microsoft.com/en-us/windows/apps/design/style/acrylic): OS fallback and GPU cost.

## v0.9.3 interaction review

An explicit Enable cursor adjustment toggle separates connection from activation. Size/style/preset edits preserve pause; the state survives restarts. Once connected, the launch/connect button becomes Return to game/app. Closing settings no longer steals target focus. Current status and helper notices use separate native static text controls. Native minimize/close buttons use Microsoft Segoe MDL2 Assets glyphs with descriptive accessibility names and tooltips. The tray/taskbar icon derives from our existing original cyan arrow artwork. Three slots clearly distinguish Apply from Save to.

The current audit captured and inspected Chinese and English settings locally, including pause/edit, minimize/restore, close/reopen and explicit exit. No desktop screenshot, user profile or game artwork is published with this release. Screenshot review and native keyboard/accessibility inspection do not establish full screen-reader compliance. Windows 10, high contrast and multi-monitor DPI transitions remain outside this revision’s manual verification.

- [Microsoft — Segoe MDL2 Assets icons](https://learn.microsoft.com/en-us/windows/apps/design/iconography/segoe-ui-symbol-font) (ChromeMinimize E921, ChromeClose E8BB).
- [Microsoft — Notification area](https://learn.microsoft.com/en-us/windows/win32/uxguide/winenv-notification).
