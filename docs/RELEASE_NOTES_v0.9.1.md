# CursorBridge v0.9.1 — clearer connection and pause status

The settings footer now always identifies whether the target is connected, waiting or failed, and whether cursor adjustment is enabled or paused. A shortcut conflict or saved-preset notice no longer replaces that information. When connected and enabled, the footer shows the configured size and reminds you to return to the target.

The companion can resize Stellaris without a Workshop subscription. The optional mod only provides in-game entry points. Moving the slider re-enables adjustment after restoring the original cursor.

Validation: all five CTest groups passed, including four new native-panel status checks. On the installed Stellaris 4.5.2 main menu, the native cursor changed from a 48px source to 24px and then 13px; observed bitmap signatures matched the resized game resources. The updated companion connected to the same game, retained 13px after returning to it, and persisted that preference. No savegame was opened and no mod playset was changed.

This release changes the controller's status presentation, not its hook or ABI. Generic Win32 support remains experimental. For normal updates, close the target and old companion before replacing the EXE.

---

设置页底部现在始终显示等待、已连接或连接失败，并说明调整处于启用还是暂停状态；快捷键冲突和预设保存提示不会再遮住连接状态。已连接且启用时会显示尺寸，并提醒切回目标程序。

调整《群星》的光标无需订阅工坊模组；可选模组只提供游戏内入口。恢复原光标后，拖动滑块即可重新启用。

已通过五组自动测试及主菜单实测：48px 原始光标成功切换为 24px、13px，实际光标位图与对应尺寸的游戏资源一致。新版连接、返回游戏和 13px 设置保存也已验证。未读取存档、未改动模组播放集。通用 Win32 后端仍为实验性功能。
