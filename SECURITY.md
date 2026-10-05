# Security and supported scope

The current adapter targets Windows x64 Stellaris only. The controller verifies the exact target executable path, name, x64 process and installed cursor resource layout before loading its own DLL. The DLL checks the provided target identity again. A same-name executable and copied assets are not a cryptographic publisher signature: this tool is not an authentication or sandbox system.

The hook changes only the main module's `USER32!SetCursor` import. There is no kernel driver, arbitrary process picker, global mouse hook, executable patch on disk, network client or downloaded executable command. Bridge log input is parsed against a small command whitelist and never evaluated as a shell command. A four-second heartbeat timeout causes recognized cursors to revert on the next cursor/refresh operation. The DLL remains resident until the game exits; quit/restart the game before replacing DLL versions.

Windows desktop preferences are not changed. Hard crashes, unusual fullscreen/input states and unrecognized/software-rendered cursors can affect observed behavior. Do not use untested adapters with protected multiplayer games or assume compatibility with anti-cheat software. Linux, macOS and arbitrary games are unsupported in this release.

The first release is unsigned. Obtain binaries from the project's GitHub Releases, verify SHA256, or build them yourself. Open source enables inspection; it does not guarantee absence of defects or antivirus alerts. Never disable security protections to run the program.

Please report ordinary reproducible defects through GitHub issues, with sensitive data removed. For a security vulnerability use GitHub's private vulnerability reporting when enabled. Do not include credentials, savegames or personally identifying logs in public issues.
