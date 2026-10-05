# Workshop upload design / 工坊上传方案

## Two deliverables / 两种载体

| Destination | Contents | Player action |
| --- | --- | --- |
| Steam Workshop | `workshop/`: original event/edict scripts, 10 locale folders, descriptor and original preview | Subscribe and enable the in-game entry |
| GitHub Releases | Windows x64 companion ZIP, original source repository, checksum file | Download, extract and run the companion |

Recommended Workshop title: **CursorBridge｜群星光标大小 · 1–96 px**. Put “requires companion app” prominently in the opening paragraph and preview image. Categories: Utilities / Graphics, subject to the launcher's available tags. Supported script version: `4.5.*`; tested executable: 4.5.1. Do not claim a pure-subscription solution or universal compatibility.

The primary download link is the actual tagged [GitHub Release](https://github.com/shenbo1264/cursorbridge/releases/tag/v0.6.0). The direct ZIP URL is `https://github.com/shenbo1264/cursorbridge/releases/download/v0.6.0/CursorBridge-Stellaris-windows-x64.zip`. Use both in the description: the page contains notes/checksums and the ZIP link goes straight to the program. Do not use a `releases/latest` link while the release is marked prerelease; GitHub's latest endpoint may skip it.

## Upload-ready text / 可直接粘贴的简介

- Chinese: [WORKSHOP_DESCRIPTION.zh.bbcode](WORKSHOP_DESCRIPTION.zh.bbcode)
- English: [WORKSHOP_DESCRIPTION.en.bbcode](WORKSHOP_DESCRIPTION.en.bbcode)

For a single public item, place the Chinese section first and English after it, or use Steam's localized description controls if available in the uploader. The `preview.png` is original artwork, with the size range, Windows scope and companion requirement. Use fresh app screenshots for the gallery; avoid advertising proposed theme/animation features as shipped.

## Register a local item before uploading / 上传前注册本地模组

1. Keep a separate upload copy of the `workshop/` folder, for example `CursorBridge Workshop`. Do not put the companion binaries or your generated UI patch in it.
2. Use the Paradox Launcher mod creation/import flow for the current installed launcher to create a local mod. The launcher's menu labels may vary. It produces an outer `.mod` registration file and a content folder containing `descriptor.mod`.
3. Copy only the upload-ready `workshop/` contents into that content folder. In the outer registration file keep the actual local `path` supplied by the launcher; the inner descriptor intentionally has no machine-specific path or Workshop ID.
4. Enable and test it with the companion. Confirm the edict/start event appears, the slider opens, size changes apply, restoration works, and the description clearly says the program is required.
5. In the launcher uploader, select the local item, preview and title; paste the BBCode description. Begin with a private or unlisted visibility if the installed uploader offers it, inspect the resulting page, then make public when ready.
6. After upload, retain the real `remote_file_id` / outer descriptor for future updates. Do not invent an ID or publish a duplicate item for every version.

Only the owner performs the final Steam publication at this stage; this project prepares the package and text. No Workshop ID/link is claimed until Steam has actually created the item.

If manually installing the local core, create an outer file in the game's active user profile `mod/` directory with its content directory's absolute path:

```text
name="CursorBridge - Stellaris Cursor Size"
path="D:/Mods/CursorBridge/workshop"
supported_version="4.5.*"
```

Then use the launcher to add it to a chosen playset. The companion's tray works without any playset change. Local tests should use an isolated `-userdir`; never rewrite another user's enabled-mod list.

## Optional native settings entry / 可选原生设置页入口

This is an advanced local patch, not a second automatically compatible Workshop item. Use Python 3.10+:

```text
python tools/create_settings_patch.py --gui-source "D:/SteamLibrary/steamapps/workshop/content/281990/1623423360/interface/settings_view.gui" --output "D:/Mods/CursorBridgeLocalSettings" --font font_text_20
```

Register its generated directory as a separate local mod. Load it after UOD, Dark Blue and the core CursorBridge mod. It adds only our button, but the full generated GUI file takes precedence at load time; any later mod replacing that page must be accounted for. Regenerate into a new folder after UI updates. Vanilla needs its own layout check and `cg_16b` font selection; it is not the validated UOD setup.

No third-party GUI is uploaded with the project. Our generator refuses to overwrite an existing output folder or the source UI.

## Release and update discipline / 后续更新

Use the same GitHub repository and Workshop item. Tag tested versions, attach a clean binary ZIP and SHA256 sums, update the description's release and asset links, and list actual compatibility changes. The application does not auto-download or run updates. Close the game and companion before replacing a build.

Primary references: [GitHub Releases documentation](https://docs.github.com/en/repositories/releasing-projects-on-github/about-releases) and [Steam Workshop documentation](https://partner.steamgames.com/doc/features/workshop/implementation). Steam's documentation describes Workshop content/metadata submission; the exact Stellaris launcher flow must be checked against its installed version.
