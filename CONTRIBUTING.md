# Contributing

Please open an issue describing the user-facing problem before a large feature or new game adapter. Small fixes can use a pull request directly.

Build with CMake and MSVC on Windows x64. Run CTest and the native self-test with the original synthetic fixtures described in README. Keep warnings enabled as errors. Include the observed result and the scope tested; do not label an adapter universal based on one game.

Keep game selection, profile discovery, language handling, bridge commands and cursor replacement separate. New adapters need an explicit process identity, resource recognition and tests for restoring the original state. Prefer supported game extension APIs whenever available. Do not add blanket process injection, anti-cheat bypass, telemetry or a required online service.

Use the language tables for all visible companion strings. Chinese should read naturally, and every other game language should use the English fallback. Validate the Workshop localization BOM and key parity with `python tools/validate_workshop.py`.

Only contribute code and assets you have the right to license. No extracted game cursors, copied commercial cursor packs, third-party GUI files, personal logs, secrets or local absolute paths belong in Git. The fixture generator and project cover are original work. Generated settings patches remain local and are not distributable project assets.

By contributing original work you agree it can be distributed under the project's MIT license. For dependency code, include its license and discuss compatibility first.
