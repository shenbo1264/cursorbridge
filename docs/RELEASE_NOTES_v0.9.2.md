# CursorBridge v0.9.2 — launch or connect from settings

A new button beside the target selector starts the selected game/application when it is closed, or connects to its existing process when it is running. Automatic connection still operates after launch. Failed connections can be retried from this button; a paused connection offers Enable adjustment. Connected and starting states prevent repeated launches. Target selection is temporarily disabled during launch/connection, and launch errors appear in the footer.

The button follows the existing Chinese/English language choice. Stellaris shows game wording; other selected applications retain general application wording. Settings stay available after the target exits, allowing another launch from the same panel. The hook DLL and ABI are unchanged.

Validation: six local CTest groups passed, including 42 native-panel checks and 12 new connection checks against an isolated native x64 fixture. The actual settings button launched the hidden fixture, automatic connection published the chosen size, manual connection retried an already running PID, pause/resume worked, and process enumeration confirmed no duplicate target. The Chinese layout was visually checked on the desktop. No new third-party game compatibility is claimed.

---

“选择目标程序”右侧新增启动／连接按钮：目标未运行时启动它，运行时连接已有进程。启动后仍会自动连接；连接失败可直接重试，暂停时可点击“启用光标调整”。启动中和已连接状态会防止重复启动；启动失败会在底部提示。目标退出后设置页保留，可再次启动。

按钮延续中英双语规则，对《群星》使用游戏文案，对其他目标使用通用程序文案。六组测试全部通过，包括 42 项面板校验与 12 项真实进程连接校验；使用独立测试程序验证启动、自动连接、重试、恢复启用和防重复实例。中文布局已在实际界面检查。此版本未更换钩子或 ABI。
