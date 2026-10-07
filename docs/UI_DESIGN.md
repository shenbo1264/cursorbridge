# Frosted settings — v0.9 preview

The panel borrows Apple's material hierarchy: a frosted outer surface, restrained blue controls and a readable opaque cursor-preview well. This is an independent Windows implementation, not Apple's Liquid Glass renderer. Native edit, trackbar, combo, checkbox and button controls retain keyboard/value/accessibility behavior; GDI+ subclass painting provides rounded surfaces and focus feedback.

## Material implementation

- Requests documented Desktop Acrylic through DWM, with the frame extended across the client area and premultiplied-alpha drawing.
- Windows 11 24H2+ requires `DWMWA_REDIRECTIONBITMAP_ALPHA` (39) to honor the redirection bitmap's alpha. Version 0.9 explicitly requests it; older Windows reject unsupported attributes without preventing launch.
- A dynamically resolved native `SetWindowCompositionAttribute` accent backdrop supplies a controllable frosted tint. **Accent-policy constants are not a public compatibility contract**; this backend is experimental. Failure retains the documented DWM route. Future Windows updates may require a change.
- Respects user transparency/high-contrast preferences. Disabled transparency, high contrast or unavailable composition uses an opaque readable surface. No Windows settings are changed. Windows may also apply its own power/inactive fallback.
- Uses Windows GDI+, DWM, common controls and installed Segoe UI/Microsoft YaHei UI fonts. No desktop screenshots/background-capture blur loop, Apple assets or extra UI framework is installed.

The header remains draggable, Escape/close dismiss the panel, and the preview is in actual pixels independently of layout scaling. The application selector is now visible near the header. Layout fits the monitor work area; per-monitor DPI transitions need more coverage.

## Verification limits

The v0.8 captures in `docs/images/glass-panel-*.jpg` are **historical**, showing its gray fallback, not evidence for v0.9 transparency. During v0.9 development a colored native test backdrop visibly showed through with blurred boundaries, and numeric/slider updates were exercised. The final material revision was inspected and accepted by the project owner; a complete automated visual matrix was not performed.

The hidden native panel suite now has **32 checks**: value boundaries, normalization, slider keyboard and external-value synchronization, selections, toggles, presets, restoration, preview, language changes, rendering and close/reopen. This verifies our controls, not OS blur. General-application selection/branding was added afterward and builds under warnings-as-errors. Windows 10, contrast themes, power saving, unusual DPI and all activation transitions are not visually certified. A successful API return alone does not prove the displayed material is translucent.

## Primary references

- [Apple HIG — Materials](https://developer.apple.com/design/human-interface-guidelines/materials).
- [Microsoft — DWMWINDOWATTRIBUTE](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwmwindowattribute): redirection bitmap alpha and OS support.
- [Microsoft — DWM_SYSTEMBACKDROP_TYPE](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwm_systembackdrop_type).
- [Microsoft — Custom Window Frame](https://learn.microsoft.com/en-us/windows/win32/dwm/customframe).
- [Microsoft — SetWindowCompositionAttribute](https://learn.microsoft.com/en-us/windows/win32/dwm/setwindowcompositionattribute): dynamically resolved API; Microsoft recommends documented DWM attributes instead.
- [Microsoft — Acrylic](https://learn.microsoft.com/en-us/windows/apps/design/style/acrylic): OS fallback and GPU cost.
