# CursorBridge v0.8.0 — Frosted glass settings preview

The settings panel now has a light frosted surface, rounded controls, a larger size readout, a quieter cursor preview and toggle switches. The 1–96 px range, 12 original styles, presets, shortcuts, restoration and Chinese/English behavior are retained.

- Windows 11 22H2+ requests native Desktop Acrylic. Windows can show a solid fallback when inactive or in power saving. Disabled transparency, high contrast and unsupported Windows use an opaque surface.
- Clearer grouping and spacing, native keyboard controls, visible focus, accessible preview button, draggable header and close button.
- No new external runtime, Apple assets, OS setting changes or Workshop dependencies. The application still supports Stellaris 4.5.1 on Windows x64 only and still needs a companion.
- Five automated test groups, including 30 isolated native-panel checks. Chinese/English window captures and control trees inspected. Desktop click automation could not activate the panel on the test desktop; foreground blur and full mouse/drag interaction of the new skin still need manual confirmation. This release remains a preview. See [design and verification](https://github.com/shenbo1264/cursorbridge/blob/v0.8.0/docs/UI_DESIGN.md).

Download `CursorBridge-Stellaris-windows-x64.zip`, extract it completely, then open `Open Settings.cmd`. The source archives are for developers. Existing preferences are reused; stop the old companion before starting the new one. The cursor hook implementation and shared protocol are unchanged from v0.7.
