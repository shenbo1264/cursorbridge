# Architecture and boundaries

`shared.h` defines the narrow size/mapping protocol and cursor bitmap signatures. `install_paths.h` provides the Stellaris layout adapter and Windows Known Folder/Steam discovery. `game_profile.h` reads the active game's bounded command line through SDK-declared process fields to resolve `-userdir`. `language.h` handles language preference precedence and visible strings.

`controller.cpp` validates the selected x64 process and loads only the DLL next to the controller. Remote addresses are resolved by module + export RVA rather than assuming ASLR addresses match. Initialization arguments carry the exact executable and resource directory, with bounded UTF-16 storage and a checked structure size. `cursor_hook.cpp` validates them again and replaces the main module's SetCursor import.

The hook recognizes installed cursor resources by bitmap/hotspot signatures. It loads requested sizes through Win32 `LoadImage`, preserving the installed assets and animation, instead of rewriting the assets. Only recognized handles are replaced. SetCursor's previous-handle semantics are translated back for the game. A cache retains three recent sizes per original and at most 32 resized cursor handles overall. Unknown pointers and invalid sizes pass through.

The worker refreshes about every 250 ms; the controller heartbeat updates every second. When the controller exits, restoration is requested; after heartbeat expiry, the next relevant hook/refresh also passes through the original. The hook stays resident until game exit. Abnormal process failure cannot guarantee an immediate visual update before Windows/game input refreshes.

`settings_panel.h` draws a small floating panel. `bridge.h` tails new records from the active single-player game log, with bounded reads/line storage and whitelisted commands. It ignores old requests on connection, recognizes file rotation and does not execute script text or programs. The Workshop package communicates through those event/log records and supplies Chinese text or English fallback across ten supported locale folders.

The core Workshop package does not replace a GUI. The optional local generator adds one effect button to a settings page supplied by the user. Such a local patch overrides that specific file, so it must load after its source UI and be regenerated after updates. Only the generator and our button effect are distributable; a generated copy of a third-party GUI is not part of the public package.

A pure Workshop implementation was investigated on Windows Stellaris 4.5.1: both ordinary cursor overlays and `replace_path` probes failed to change a marked cursor while an unrelated menu-text control confirmed that the test mod was loaded. Read-only engine inspection found direct Win32 cursor-file loading. This evidence supports the current companion design; it is not proof that all future engine versions can never expose a cursor-setting API. No proprietary binary/disassembly or extracted resources are distributed here.

Custom game adapters, software-rendered cursor interception, overlay-based animation, themes and universal process support are not implemented yet.
