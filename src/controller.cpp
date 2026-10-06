#include "shared.h"
#include "bridge.h"
#include "language.h"
#include "game_profile.h"
#include "install_paths.h"
#include "preferences.h"
#include <commctrl.h>
#include <commdlg.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <fstream>
#include <sstream>

static HWND windowHandle;
static HANDLE attached=NULL,mapHandle=NULL;
static Shared* state=NULL;
static DWORD attachedPid=0,failedPid=0;
static int desiredSize=32;
static int desiredTheme=0;
static bool lockWindow=false;
static UINT hotkeyModifiers=MOD_CONTROL|MOD_ALT;
static bool enabled=true;
static std::wstring binDir,logPath;
static std::wstring gameExe,defaultGameLog,gameLog,configuredGameLog,dataDirectory,settingsPath;
static bool explicitGameExe=false;
static bool explicitGameLog=false;
static LanguageMonitor uiLanguage;
static const wchar_t* Text(UiText key){return UiString(uiLanguage.language,key);}
static BridgeTail bridge;
static NOTIFYICONDATAW tray={};
static int IndexSize(int i){static const int sizes[]={24,32,40,48};return sizes[i];}
static void SaveSize(){WritePreset(settingsPath,L"Cursor",{desiredSize,desiredTheme,lockWindow});}
static void PublishPreferences(){
    if(!state)return;
    InterlockedExchange(&state->size,desiredSize);InterlockedExchange(&state->theme,desiredTheme);
    InterlockedExchange(&state->lockWindow,lockWindow?1:0);InterlockedExchange(&state->enabled,enabled?1:0);
    InterlockedExchange64(&state->heartbeat,GetTickCount64());
}
static void Log(const std::wstring& text) {
    int n=WideCharToMultiByte(CP_UTF8,0,text.c_str(),(int)text.size(),NULL,0,NULL,NULL);std::string utf8(n,'\0');
    WideCharToMultiByte(CP_UTF8,0,text.c_str(),(int)text.size(),&utf8[0],n,NULL,NULL);
    std::ofstream f(logPath,std::ios::app);SYSTEMTIME t;GetLocalTime(&t);
    f<<t.wHour<<":"<<t.wMinute<<":"<<t.wSecond<<" "<<utf8<<"\n";
}
#include "settings_panel.h"
#include "hotkeys.h"
static void RefreshUiLanguage(){
    if(!uiLanguage.Refresh())return;
    SetWindowTextW(windowHandle,Text(UiText::WindowTitle));
    if(panelWindow){SetWindowTextW(panelWindow,Text(UiText::WindowTitle));LocalizePanel();InvalidateRect(panelWindow,NULL,FALSE);}
}
static HANDLE OpenValidated(DWORD pid,bool test) {
    HANDLE h=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ|PROCESS_VM_WRITE|PROCESS_VM_OPERATION|PROCESS_CREATE_THREAD|SYNCHRONIZE,FALSE,pid);
    if(!h)return NULL;
    wchar_t p[32768]={};DWORD n=32768;
    bool good=QueryFullProcessImageNameW(h,0,p,&n)&&SamePath(p,test?binDir+L"\\StellarisCursorTest.exe":gameExe);
    BOOL wow=FALSE;if(!IsWow64Process(h,&wow)||wow)good=false;
    if(!good){CloseHandle(h);return NULL;}return h;
}
static uintptr_t ModuleBase(DWORD pid,const std::wstring& path) {
    for(int attempt=0;attempt<100;attempt++) {
        HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);
        if(snap==INVALID_HANDLE_VALUE){Sleep(20);continue;}
        MODULEENTRY32W entry={};entry.dwSize=sizeof(entry);uintptr_t result=0;
        if(Module32FirstW(snap,&entry))do{if(SamePath(entry.szExePath,path)){result=(uintptr_t)entry.modBaseAddr;break;}}while(Module32NextW(snap,&entry));
        CloseHandle(snap);if(result)return result;Sleep(20);
    }
    return 0;
}
static bool RemoteCall(HANDLE process,uintptr_t function,void* argument,DWORD& result) {
    HANDLE thread=CreateRemoteThread(process,NULL,0,(LPTHREAD_START_ROUTINE)function,argument,0,NULL);
    if(!thread)return false;
    bool complete=WaitForSingleObject(thread,15000)==WAIT_OBJECT_0;
    bool good=complete&&GetExitCodeThread(thread,&result);CloseHandle(thread);return good;
}
static bool Connect(DWORD pid,bool test=false,bool windowTest=false) {
    HANDLE h=OpenValidated(pid,test);if(!h){Log(L"连接失败：目标路径/位数/权限检查未通过。");return false;}
    std::wstring dll=binDir+L"\\StellarisCursorHook.dll";
    std::wstring localKernel=ModulePath(GetModuleHandleW(L"kernel32.dll"));
    uintptr_t remoteKernel=ModuleBase(pid,localKernel);
    // Resolve the loader by module + RVA; never assume identical ASLR addresses.
    FARPROC loader=GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"LoadLibraryW");
    HMODULE owner=NULL;GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)loader,&owner);
    uintptr_t ownerRemote=ModuleBase(pid,ModulePath(owner));
    if(!remoteKernel||!ownerRemote){Log(L"找不到目标加载器模块。");CloseHandle(h);return false;}
    size_t length=(dll.size()+1)*sizeof(wchar_t);
    void* remote=VirtualAllocEx(h,NULL,length,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);DWORD exitCode=0;
    if(!remote||!WriteProcessMemory(h,remote,dll.c_str(),length,NULL)){Log(L"写入 DLL 路径失败，Win32="+std::to_wstring(GetLastError()));if(remote)VirtualFreeEx(h,remote,0,MEM_RELEASE);CloseHandle(h);return false;}
    bool loaded=RemoteCall(h,ownerRemote+((uintptr_t)loader-(uintptr_t)owner),remote,exitCode);
    if(!loaded){Log(L"加载线程未完成；保留参数内存直到目标退出，避免超时后释放被使用的内存。");CloseHandle(h);return false;}
    VirtualFreeEx(h,remote,0,MEM_RELEASE);
    uintptr_t base=ModuleBase(pid,dll);if(!base){Log(L"DLL 没有加载到目标进程，线程返回="+std::to_wstring(exitCode));CloseHandle(h);return false;}
    HMODULE local=LoadLibraryExW(dll.c_str(),NULL,DONT_RESOLVE_DLL_REFERENCES);
    FARPROC init=local?GetProcAddress(local,"Initialize"):NULL;
    uintptr_t rva=init?(uintptr_t)init-(uintptr_t)local:0;
    if(local)FreeLibrary(local);if(!rva){Log(L"找不到 Initialize 导出。");CloseHandle(h);return false;}
    HANDLE mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(Shared),MapName(pid).c_str());
    DWORD mapError=GetLastError();
    Shared* s=mapping?(Shared*)MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(Shared)):NULL;
    if(!s){Log(L"共享内存创建失败，Win32="+std::to_wstring(GetLastError()));if(mapping)CloseHandle(mapping);CloseHandle(h);return false;}
    if(mapError!=ERROR_ALREADY_EXISTS){ZeroMemory(s,sizeof(*s));s->magic=CURSOR_MAGIC;s->version=CURSOR_ABI_VERSION;s->pid=pid;}
    if(s->magic!=CURSOR_MAGIC||s->version!=CURSOR_ABI_VERSION||s->pid!=pid){UnmapViewOfFile(s);CloseHandle(mapping);CloseHandle(h);return false;}
    InterlockedExchange(&s->size,desiredSize);InterlockedExchange(&s->enabled,enabled?1:0);InterlockedExchange64(&s->heartbeat,GetTickCount64());
    InterlockedExchange(&s->theme,desiredTheme);InterlockedExchange(&s->lockWindow,lockWindow?1:0);
    InitArgs args={};args.size=sizeof(args);args.test=test?(windowTest?2U:1U):0U;
    wcsncpy_s(args.targetExe,(test?binDir+L"\\StellarisCursorTest.exe":gameExe).c_str(),_TRUNCATE);
    wcsncpy_s(args.resourceDirectory,gameDirectory.c_str(),_TRUNCATE);
    remote=VirtualAllocEx(h,NULL,sizeof(args),MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    bool written=remote&&WriteProcessMemory(h,remote,&args,sizeof(args),NULL);
    bool called=written&&RemoteCall(h,base+rva,remote,exitCode);
    if(called||!written){if(remote)VirtualFreeEx(h,remote,0,MEM_RELEASE);}
    if(!called||exitCode!=1||s->hookSlots<=0){Log(L"连接失败：Initialize 返回="+std::to_wstring(exitCode)+L"，样本="+std::to_wstring(s->mappings)+L"，入口="+std::to_wstring(s->hookSlots));s->enabled=0;UnmapViewOfFile(s);CloseHandle(mapping);CloseHandle(h);return false;}
    attached=h;state=s;mapHandle=mapping;attachedPid=pid;
    if(!test){
        std::wstring profile=ProcessUserDirectory(h);
        if(profile.empty())profile=Parent(Parent(gameLog));
        if(!explicitGameLog)gameLog=profile+L"\\logs\\game.log";
        uiLanguage.SetSource(profile+L"\\settings.txt");
        if(panelWindow){SetWindowTextW(panelWindow,Text(UiText::WindowTitle));LocalizePanel();InvalidateRect(panelWindow,NULL,FALSE);}
        bridge.Reset(gameLog);Log(L"游戏内设置入口已就绪；接收本次连接后的尺寸、恢复及滑块面板请求。");
        Log(L"语言配置来源："+uiLanguage.Source()+L"；显示 "+(uiLanguage.language==UiLanguage::Chinese?L"中文":L"English"));
    }
    Log(L"已连接 PID "+std::to_wstring(pid)+L"；IAT SetCursor 入口 "+std::to_wstring(s->hookSlots)+L"，资源样本 "+std::to_wstring(s->mappings));return true;
}
static void Disconnect() {
    if(state){InterlockedExchange(&state->enabled,0);InterlockedExchange64(&state->heartbeat,GetTickCount64());Sleep(300);UnmapViewOfFile(state);state=NULL;}
    if(mapHandle){CloseHandle(mapHandle);mapHandle=NULL;}if(attached){CloseHandle(attached);attached=NULL;}attachedPid=0;
}
static DWORD FindGame() {
    HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);if(snap==INVALID_HANDLE_VALUE)return 0;
    PROCESSENTRY32W entry={};entry.dwSize=sizeof(entry);DWORD pid=0;
    if(Process32FirstW(snap,&entry))do{
        if(_wcsicmp(entry.szExeFile,L"stellaris.exe"))continue;
        HANDLE query=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,entry.th32ProcessID);
        wchar_t path[32768]={};DWORD count=32768;
        bool good=query&&WaitForSingleObject(query,0)==WAIT_TIMEOUT&&QueryFullProcessImageNameW(query,0,path,&count)&&ValidGameExecutable(path);
        if(query)CloseHandle(query);
        if(!good||(explicitGameExe&&!SamePath(path,gameExe)))continue;
        gameExe=path;gameDirectory=Parent(gameExe);pid=entry.th32ProcessID;break;
    }while(Process32NextW(snap,&entry));
    CloseHandle(snap);return pid;
}
static void ChooseGame(){
    wchar_t path[32768]={};OPENFILENAMEW dialog={};dialog.lStructSize=sizeof(dialog);dialog.hwndOwner=windowHandle;
    dialog.lpstrFilter=L"Stellaris (stellaris.exe)\0stellaris.exe\0";dialog.lpstrFile=path;dialog.nMaxFile=32768;
    dialog.Flags=OFN_FILEMUSTEXIST|OFN_PATHMUSTEXIST|OFN_NOCHANGEDIR;
    if(!GetOpenFileNameW(&dialog))return;
    if(!ValidGameExecutable(path)){MessageBoxW(NULL,uiLanguage.language==UiLanguage::Chinese?L"请选择完整安装目录中的 stellaris.exe（需包含 gfx/cursors）。":L"Select stellaris.exe from a complete installation including gfx/cursors.",L"CursorBridge",MB_OK|MB_ICONINFORMATION);return;}
    if(attached&&!SamePath(path,gameExe)){MessageBoxW(NULL,uiLanguage.language==UiLanguage::Chinese?L"请先退出当前游戏，再切换安装目录。":L"Exit the connected game before switching installations.",L"CursorBridge",MB_OK);return;}
    gameExe=AbsolutePath(path);gameDirectory=Parent(gameExe);explicitGameExe=true;failedPid=0;
    WritePrivateProfileStringW(L"Game",L"Executable",gameExe.c_str(),settingsPath.c_str());
}
static void Tick() {
    if(attached&&WaitForSingleObject(attached,0)==WAIT_OBJECT_0){Log(L"游戏已退出，释放本地连接。");if(panelWindow)DestroyWindow(panelWindow);Disconnect();gameLog=explicitGameLog?configuredGameLog:defaultGameLog;uiLanguage.SetSource(Parent(Parent(gameLog))+L"\\settings.txt");failedPid=0;}
    if(!attached){DWORD pid=FindGame();if(pid&&pid!=failedPid){if(!Connect(pid))failedPid=pid;}}
    PublishPreferences();
    RefreshUiLanguage();
    std::wstring label=attached?std::wstring(Text(UiText::TrayTitle))+std::to_wstring(desiredSize)+L" px "+Text(enabled?UiText::On:UiText::Off):Text(UiText::TrayWaiting);
    wcsncpy_s(tray.szTip,label.c_str(),_TRUNCATE);Shell_NotifyIconW(NIM_MODIFY,&tray);
}
static void BridgeTick() {
    if(!attached||!state||WaitForSingleObject(attached,0)!=WAIT_TIMEOUT)return;
    bridge.Poll([](const BridgeCommand& command){
        if(command.kind==BridgeKind::Settings){OpenSettings();return;}
        if(command.kind==BridgeKind::Size){if(desiredSize==command.size&&enabled)return;desiredSize=command.size;enabled=true;SaveSize();Log(L"游戏内按钮："+std::to_wstring(desiredSize)+L" 像素。");}
        else if(command.kind==BridgeKind::Restore){if(!enabled)return;enabled=false;Log(L"游戏内按钮：恢复原光标。");}
        PublishPreferences();SyncPanel();
    });
}
static void LaunchGame(){
    if(FindGame())return;if(!ValidGameExecutable(gameExe))ChooseGame();
    if(ValidGameExecutable(gameExe))ShellExecuteW(NULL,L"open",gameExe.c_str(),L"-skiploop",gameDirectory.c_str(),SW_SHOWNORMAL);
}
static void Menu() {
    RefreshUiLanguage();
    HMENU menu=CreatePopupMenu();AppendMenuW(menu,MF_STRING,1,Text(UiText::Launch));
    AppendMenuW(menu,MF_STRING,5,Text(UiText::Slider));
    AppendMenuW(menu,MF_STRING,6,Text(UiText::ChooseGame));
    AppendMenuW(menu,MF_STRING,2,Text(enabled?UiText::Pause:UiText::Enable));
    for(int i=0;i<4;i++){int size=IndexSize(i);AppendMenuW(menu,MF_STRING|(size==desiredSize?MF_CHECKED:0),10+i,(std::to_wstring(size)+Text(UiText::Pixel)).c_str());}
    AppendMenuW(menu,MF_STRING,3,Text(UiText::Reconnect));AppendMenuW(menu,MF_SEPARATOR,0,NULL);AppendMenuW(menu,MF_STRING,4,Text(UiText::Exit));
    POINT p;GetCursorPos(&p);SetForegroundWindow(windowHandle);int cmd=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_RIGHTBUTTON,p.x,p.y,0,windowHandle,NULL);DestroyMenu(menu);
    if(cmd==1)LaunchGame();
    else if(cmd==5)OpenSettings();
    else if(cmd==6)ChooseGame();
    else if(cmd==2){enabled=!enabled;PublishPreferences();SyncPanel();}
    else if(cmd>=10&&cmd<=13)ApplySize(IndexSize(cmd-10),true);
    else if(cmd==3)failedPid=0;
    else if(cmd==4)DestroyWindow(windowHandle);
    if(cmd!=4)Tick();
}
static LRESULT CALLBACK Procedure(HWND h,UINT m,WPARAM w,LPARAM l) {
    if(m==WM_HOTKEY){HandleHotkey((int)w);return 0;}
    if(m==WM_APP+4){OpenSettings();return 0;}
    if(m==WM_APP+3){LaunchGame();return 0;}
    if(m==WM_CLOSE){DestroyWindow(h);return 0;}
    if(m==WM_TIMER){if(w==2){BridgeTick();PanelTick();RefreshHotkeys();}else Tick();return 0;}
    if(m==WM_APP+1&&(l==WM_RBUTTONUP||l==WM_LBUTTONUP)){Menu();return 0;}
    if(m==WM_DESTROY){ClearHotkeys();if(panelWindow)DestroyWindow(panelWindow);KillTimer(h,1);KillTimer(h,2);Disconnect();Shell_NotifyIconW(NIM_DELETE,&tray);PostQuitMessage(0);return 0;}
    return DefWindowProcW(h,m,w,l);
}
static int SelfTest(bool windowTest=false) {
    std::wstring host=binDir+L"\\StellarisCursorTest.exe";
    std::wstring cmd=L"\""+host+L"\" --game-dir \""+gameDirectory+L"\""+(windowTest?L" --refresh":L"");STARTUPINFOW startup={};startup.cb=sizeof(startup);PROCESS_INFORMATION pi={};
    if(!CreateProcessW(host.c_str(),&cmd[0],NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,binDir.c_str(),&startup,&pi))return 2;
    bool ok=Connect(pi.dwProcessId,true,windowTest);DWORD exitCode=99;
    if(ok){for(int i=0;i<3000;i++){InterlockedExchange64(&state->heartbeat,GetTickCount64());if(WaitForSingleObject(pi.hProcess,100)==WAIT_OBJECT_0)break;}GetExitCodeProcess(pi.hProcess,&exitCode);}
    Disconnect();CloseHandle(pi.hThread);CloseHandle(pi.hProcess);Log(exitCode==0?L"独立进程自测通过。":L"独立进程自测失败。");return ok&&exitCode==0?0:1;
}
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,wchar_t*,int) {
    SetProcessDPIAware();binDir=Parent(ModulePath());dataDirectory=DefaultDataDirectory();
    defaultGameLog=DefaultGameProfile()+L"\\logs\\game.log";gameLog=defaultGameLog;
    bool launch=false,showSettings=false,stop=false,selfTest=false,windowTest=false;
    int argCount=0;wchar_t** args=CommandLineToArgvW(GetCommandLineW(),&argCount);
    if(!args)return 2;
    for(int i=1;i<argCount;i++){
        std::wstring option=args[i];
        if(option==L"--game-log"&&i+1<argCount){configuredGameLog=gameLog=AbsolutePath(args[++i]);explicitGameLog=true;}
        else if(option==L"--game-path"&&i+1<argCount){gameExe=AbsolutePath(args[++i]);explicitGameExe=true;}
        else if(option==L"--data-dir"&&i+1<argCount)dataDirectory=AbsolutePath(args[++i]);
        else if(option==L"--launch")launch=true;
        else if(option==L"--settings")showSettings=true;
        else if(option==L"--stop")stop=true;
        else if(option==L"--self-test")selfTest=true;
        else if(option==L"--self-test-window"){selfTest=true;windowTest=true;}
        else {LocalFree(args);return 2;}
    }LocalFree(args);
    if(dataDirectory.empty())return 2;
    std::error_code error;std::filesystem::create_directories(dataDirectory+L"\\logs",error);if(error)return 2;
    logPath=dataDirectory+L"\\logs\\controller.log";settingsPath=dataDirectory+L"\\settings.ini";
    if(!explicitGameExe){wchar_t saved[32768]={};GetPrivateProfileStringW(L"Game",L"Executable",L"",saved,32768,settingsPath.c_str());if(ValidGameExecutable(saved)){gameExe=saved;explicitGameExe=true;}else gameExe=DiscoverInstalledGame();}
    if(explicitGameExe&&!ValidGameExecutable(gameExe)){Log(L"Invalid Stellaris installation. Check --game-path.");return 2;}
    gameDirectory=Parent(gameExe);
    uiLanguage.SetSource(Parent(Parent(gameLog))+L"\\settings.txt");
    if(selfTest){if(!ValidGameExecutable(gameExe))return 2;return SelfTest(windowTest);}
    if(stop){
        HWND existing=FindWindowW(L"StellarisCursorController",NULL);DWORD pid=0;GetWindowThreadProcessId(existing,&pid);
        HANDLE process=pid?OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,pid):NULL;
        wchar_t path[32768]={};DWORD n=32768;
        bool stopped=true;
        if(process&&QueryFullProcessImageNameW(process,0,path,&n)&&SamePath(path,ModulePath())){
            PostMessageW(existing,WM_CLOSE,0,0);stopped=WaitForSingleObject(process,20000)==WAIT_OBJECT_0;
        }
        if(process)CloseHandle(process);return stopped?0:1;
    }
    HANDLE single=CreateMutexW(NULL,TRUE,L"Local\\StellarisCursorToolController");if(GetLastError()==ERROR_ALREADY_EXISTS){if(launch)PostMessageW(FindWindowW(L"StellarisCursorController",NULL),WM_APP+3,0,0);if(showSettings)PostMessageW(FindWindowW(L"StellarisCursorController",NULL),WM_APP+4,0,0);CloseHandle(single);return 0;}
    CursorPreset preferences=ReadPreset(settingsPath,L"Cursor");desiredSize=preferences.size;desiredTheme=preferences.theme;lockWindow=preferences.lock;
    hotkeyModifiers=ReadHotkeyModifiers(settingsPath);
    INITCOMMONCONTROLSEX controls={sizeof(controls),ICC_BAR_CLASSES};InitCommonControlsEx(&controls);
    WNDCLASSW wc={};wc.lpfnWndProc=Procedure;wc.hInstance=instance;wc.lpszClassName=L"StellarisCursorController";RegisterClassW(&wc);
    windowHandle=CreateWindowExW(0,wc.lpszClassName,Text(UiText::WindowTitle),0,0,0,0,0,NULL,NULL,instance,NULL);
    tray.cbSize=sizeof(tray);tray.hWnd=windowHandle;tray.uID=1;tray.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;tray.uCallbackMessage=WM_APP+1;tray.hIcon=LoadIconW(NULL,IDI_APPLICATION);wcsncpy_s(tray.szTip,Text(UiText::TrayWaiting),_TRUNCATE);Shell_NotifyIconW(NIM_ADD,&tray);
    SetTimer(windowHandle,1,1000,NULL);SetTimer(windowHandle,2,200,NULL);Tick();Log(L"CursorBridge started; validating the Stellaris executable and cursor resources before attaching.");
    if(launch)LaunchGame();
    if(showSettings)OpenSettings();
    MSG msg;while(GetMessageW(&msg,NULL,0,0)>0){if(panelWindow&&IsDialogMessageW(panelWindow,&msg))continue;TranslateMessage(&msg);DispatchMessageW(&msg);}CloseHandle(single);return 0;
}
