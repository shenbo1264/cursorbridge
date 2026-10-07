# Security and supported scope

Version 0.9 supports the dedicated Stellaris adapter and an opt-in experimental native Win32 x64 adapter. The selected application is checked for an absolute EXE path and bounded AMD64/PE32+ headers, then the running process's complete path and architecture are rechecked. The DLL validates the supplied identity again. The Stellaris adapter also checks its resource layout. These checks are not publisher authentication or a sandbox.

The hook changes the selected main module's `USER32!SetCursor` import. There is no driver, global mouse hook, executable patch on disk, network client or downloaded executable command. Application selection is explicit; no automatic arbitrary foreground injection is performed. The single-file EXE extracts only its embedded hook and original themes into a versioned user cache, checking bytes and refusing application-owned reparse directories/files. The log bridge is Stellaris-only and parses whitelisted commands without shell evaluation. Heartbeat expiry passes through original cursors on the next relevant operation. The hook remains resident until target exit; restart the target before updating.

Windows desktop preferences are not changed. Hard crashes and unusual fullscreen/input states can affect recovery. General Win32 compatibility is experimental; software cursors, 32-bit applications and protected processes are outside this backend's scope. Linux/macOS and anti-cheat compatibility are not certified. Use only permitted target applications.

The first release is unsigned. Obtain binaries from the project's GitHub Releases, verify SHA256, or build them yourself. Open source enables inspection; it does not guarantee absence of defects or antivirus alerts. Never disable security protections to run the program.

Please report ordinary reproducible defects through GitHub issues, with sensitive data removed. For a security vulnerability use GitHub's private vulnerability reporting when enabled. Do not include credentials, savegames or personally identifying logs in public issues.
