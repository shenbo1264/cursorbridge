# CursorBridge

[English](README.md) · [直接下载 EXE](https://github.com/shenbo1264/cursorbridge/releases/download/v0.9.0/CursorBridge.exe) · [Release](https://github.com/shenbo1264/cursorbridge/releases/tag/v0.9.0)

**MIT 开源的 Windows x64 光标调整工具。** 选择目标程序，调整大小、样式和颜色，保存预设并使用快捷键。从《群星》起步，v0.9 增加实验性通用 Win32 后端，保留《群星》专属适配。方向是 YoloMouse 同类的开源工具，目前尚未实现完整功能对等。

## 下载与使用

1. 下载 **[CursorBridge.exe](https://github.com/shenbo1264/cursorbridge/releases/download/v0.9.0/CursorBridge.exe)**，双击打开设置。无需安装器、手动解压、Python 或单独下载 DLL。
2. 点击设置页上方或托盘中的 **选择目标程序…**，选中实际运行的 **64 位程序 EXE**。正常启动它，或点击 **启动目标程序**。
3. 调节大小、样式与颜色，回到目标使用。目标在前台、鼠标位于客户区时才应用调整。
4. 从面板恢复、托盘暂停或退出。更新前关闭目标程序：已加载的钩子驻留到目标退出。

EXE 内置钩子及 108 个原创光标文件，首次运行在 `%LOCALAPPDATA%/CursorBridge/runtime/` 准备版本化缓存，每次使用前逐文件与内置数据比对，不联网下载组件。设置与日志在 `%LOCALAPPDATA%/CursorBridge/`。普通用户只需一个 EXE；完整 ZIP 另含文档、兼容启动脚本和工坊脚本。

未指定目标时，已有用户保留 Steam／正在运行的《群星》发现。选择完整《群星》安装使用专属后端，其他有效 x64 EXE 使用通用后端。仅连接所选完整路径，不向所有前台应用自动加载钩子。

## v0.9.0 预览版

- 双击单文件启动，界面与托盘通用文案，可明确选择目标程序。
- 透明磨砂设置、原生控件、键盘焦点和实际像素预览。遵守系统透明／高对比设置，不支持时用实色背景。[材质与限制](docs/UI_DESIGN.md)。
- **1–96 像素**整数调节，建议从 24–32 开始；1 像素为极限选项，已处理热点越界。
- **12 套原创样式**：箭头／十字／圆环 × 白／青／琥珀／粉色，含轮廓与状态动画。
- 三个个人预设、目标／面板前台快捷键、可选 Shift、窗口内锁定。目前预设由各目标共享。
- 恢复／暂停、原句柄保护、缓存上限、心跳恢复、路径和位数检查。
- 中英双语：《群星》跟随游戏语言；通用程序跟随 Windows 用户界面语言，中文显示中文，其他显示英文。
- 可选《群星》事件／法令入口和本地设置页按钮。**仅订阅工坊无法调整光标，仍需伴侣程序。**

## 通用后端与 YoloMouse 对标

通用后端是**实验性功能**，已在独立原生 x64 宿主验证。拦截目标主模块导入的 `USER32!SetCursor`；走这条路线的程序可能可用。自行绘制／隐藏光标、动态获取 API、仅由其他模块设置光标、32 位程序及受保护进程不在当前支持范围内。未宣称所有游戏兼容或通过反作弊认证。

通用原样模式复制缩放观察到的 Windows 光标，必要时纠正热点，不保证任意原始 ANI 动画保留。《群星》识别本机九个 CUR/ANI 资源并保留原动画，不打包游戏素材。部分标准抓取、忙碌、禁止光标可保留状态样式；未知自定义光标使用基础样式，不假定友军／敌军含义。

光晕、发光、拖尾、放大镜、原图染色、导入／编辑器、逐光标绑定与自动按应用配置尚未实现。[功能对照](docs/YOLOMOUSE_COMPARISON.md)与[路线图](docs/ROADMAP.md)区分已完成和计划。

无驱动、服务、开机启动、遥测或网络客户端。以目标正常权限运行。预览版尚未签名，请从本项目 Release 下载并核对 SHA256。[安全说明](SECURITY.md)。

## 快捷键与参数

默认 Ctrl+Alt：**C** 设置、**↑/↓** 每次一像素、**0** 恢复、**1/2/3** 预设、**L** 锁定。冲突可勾选“快捷键加 Shift”。滑块支持方向键、Home/End、Page Up/Page Down（8 像素）。

```text
CursorBridge.exe
CursorBridge.exe --app-path "D:\Apps\MyApp.exe" --settings
CursorBridge.exe --game-path "D:\SteamLibrary\steamapps\common\Stellaris\stellaris.exe" --settings
CursorBridge.exe --background
CursorBridge.exe --launch
CursorBridge.exe --stop
CursorBridge.exe --data-dir "D:\CursorBridgeData"
```

`--app-path` 明确启用通用后端，`--game-path` 要求完整《群星》安装；面板选择自动判断适配类型。只向《群星》传入 `-skiploop`；需要专用启动器的应用继续使用正常入口。

## 工坊与开发

工坊包仅含原创脚本和十种语言文本，不替换 UOD／暗蓝 GUI，不覆盖 `00_on_actions.txt`。尚未代为发布工坊项目。[上传指南](docs/WORKSHOP_UPLOAD.md)。第三方设置页 GUI 生成结果仅在本机使用，不在公开包中。

构建需要 Windows x64、VS 2022 C++ Build Tools／Windows SDK、CMake 3.21+、Python 3.10+，无第三方钩子库。开发构建保留历史名 `StellarisCursor.exe`，公开单文件命名 `CursorBridge.exe`。

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
python tools/make_test_assets.py "build/Synthetic Game"
```

完整命令见[英文 README](README.md#build-and-verify)。测试只用独立宿主及原创素材，不加载存档或运行布局标记。[验证记录](docs/VALIDATION.md)说明范围。[贡献指南](CONTRIBUTING.md)。原创代码、脚本和素材采用 [MIT](LICENSE)，游戏素材与第三方 GUI 不在授权内。

本项目独立开发，与 Paradox Interactive、Steam 或 YoloMouse 无关联。
