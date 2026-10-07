# CursorBridge v0.9.3 — predictable window and pause controls

Settings now have a real minimize button and a native taskbar lifecycle. Close / Hide to tray keeps the companion running; Quit app exits completely and restores the original target cursor. The window is no longer always on top and never automatically hides or reopens. Left-click the tray icon to reopen settings, right-click for the menu. Native minimize/close icons have descriptive accessibility names and hover help; the app uses its existing original cyan arrow artwork for its tray/window icon.

An explicit Enable cursor adjustment switch separates connection from activation. Editing size, style, presets or recommended size preserves pause; pause also survives restarts. Preview always reflects the chosen settings. Once connected, the launch/connect action becomes Return to game / Return to app without changing activation or launching a duplicate process. Connection and activation status remain visible separately from notices; failed saves no longer claim success.

Preset actions now say Apply and Save to. Wheel resizing is restricted to the size area. The Chinese and English interface, README and direct download links describe these interactions.

Validation: six local CTest groups passed, including 58 native-panel checks, 48 preference checks and 13 real connection checks against only an isolated native x64 host. Actual Chinese/English settings, pause followed by editing, minimize/restore, close/reopen and complete exit are checked separately on the desktop. Single-file launch and all 109 embedded runtime files are checked before publication. The hook DLL and ABI are unchanged; no new game compatibility is claimed.

---

设置页补齐真正的任务栏最小化。“关闭窗口／收起到托盘”保留后台运行，“退出程序”完全退出并恢复目标原光标。窗口不再强制置顶，不会自动隐藏或弹回；托盘左键打开设置，右键显示菜单。

新增独立的“启用光标调整”开关。暂停时修改尺寸、样式、预设或推荐尺寸均保持暂停，重启后保留暂停状态；预览仍显示待应用的设置。连接后的按钮改为“返回游戏／返回目标程序”，不会解除暂停或重复启动。连接状态与辅助提示分开显示，预设操作改为“应用／保存到”，滚轮仅在尺寸区改变大小，并补齐悬停帮助和可读取的状态文字。

六组本地测试通过，含 58 项面板、48 项偏好和 13 项真实进程连接检查；中文／英文布局和窗口操作另在实际界面验证。钩子及 ABI 保持原样。
