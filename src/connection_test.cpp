// Real button commands against only our hidden, application-owned test host.
// No user game or foreground input is used.
#define wWinMain CursorBridgeApplicationEntry
#include "controller.cpp"
#undef wWinMain
#include <iostream>

static int checks=0,failures=0;
static void Expect(bool result,const char* message){++checks;if(!result){++failures;std::cerr<<"FAIL: "<<message<<"\n";}}
static void ClickAction(){SendMessageW(panelWindow,WM_COMMAND,MAKEWPARAM(135,BN_CLICKED),(LPARAM)GetDlgItem(panelWindow,135));}
static std::vector<DWORD> FixtureProcesses(){
    std::vector<DWORD> ids;HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);if(snapshot==INVALID_HANDLE_VALUE)return ids;
    PROCESSENTRY32W entry={};entry.dwSize=sizeof(entry);
    if(Process32FirstW(snapshot,&entry))do{
        if(_wcsicmp(entry.szExeFile,L"Connection Target.exe"))continue;
        HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,entry.th32ProcessID);wchar_t path[32768]={};DWORD length=32768;
        if(process&&QueryFullProcessImageNameW(process,0,path,&length)&&SamePath(path,gameExe))ids.push_back(entry.th32ProcessID);
        if(process)CloseHandle(process);
    }while(Process32NextW(snapshot,&entry));CloseHandle(snapshot);return ids;
}
static void StopHost(DWORD pid){
    std::wstring name=L"Local\\CursorBridgeConnectionHost-"+std::to_wstring(pid);
    HANDLE process=OpenProcess(SYNCHRONIZE,FALSE,pid),stop=NULL;
    for(int i=0;i<50&&!stop;i++){stop=OpenEventW(EVENT_MODIFY_STATE,FALSE,name.c_str());if(!stop)Sleep(20);}
    if(stop){SetEvent(stop);CloseHandle(stop);}if(process){WaitForSingleObject(process,5000);CloseHandle(process);}
}
int wmain(){
    SetProcessDPIAware();INITCOMMONCONTROLSEX controls={sizeof(controls),ICC_BAR_CLASSES};InitCommonControlsEx(&controls);
    wchar_t temporary[MAX_PATH]={};GetTempPathW(MAX_PATH,temporary);
    auto fixture=std::filesystem::path(temporary)/(L"CursorBridge-connection-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64()));
    if(!std::filesystem::create_directory(fixture))return 2;
    binDir=Parent(ModulePath());gameExe=(fixture/L"Connection Target.exe").wstring();gameDirectory=fixture.wstring();
    std::filesystem::copy_file(std::filesystem::path(binDir)/L"StellarisCursorConnectionHost.exe",gameExe);
    genericAdapter=true;explicitGameExe=true;settingsPath=(fixture/L"settings.ini").wstring();logPath=(fixture/L"controller.log").wstring();
    uiLanguage.SetSource(L"");desiredSize=13;desiredTheme=0;enabled=false;OpenSettings(false);
    Expect(FindGame()==0,"isolated target is not initially running");
    ClickAction();DWORD launched=FindGame();
    for(int i=0;i<50&&!launched;i++){Sleep(20);launched=FindGame();}
    Expect(launched!=0,"button launches selected executable when absent");
    // A repeated launch request must not create another instance while pending.
    ClickAction();Expect(FindGame()==launched&&FixtureProcesses().size()==1,"pending launch keeps exactly one process");
    Tick();Expect(attached&&attachedPid==launched&&state&&state->enabled&&state->size==13,"timer connects launched host and publishes selected size");
    Expect(!launchDeadline&&!IsWindowEnabled(GetDlgItem(panelWindow,135)),"connected action is disabled after successful launch");
    HCURSOR original=LoadCursorW(NULL,IDC_ARROW);Signature before,after;CursorSignature(original,before);
    DWORD same=attachedPid;RestoreCursor();ClickAction();
    Expect(attachedPid==same&&state&&state->enabled&&enabled,"action resumes paused connection without relaunch");
    CursorSignature(original,after);Expect(before==after,"controller action leaves desktop cursor resource unchanged");
    StopHost(launched);Tick();Expect(!attached&&panelWindow&&foundTargetPid==0,"target exit keeps settings and launch action available");
    // Start independently, emulate an earlier failed attempt, then invoke the
    // same visible action: this must retry that PID rather than start a copy.
    STARTUPINFOW startup={};startup.cb=sizeof(startup);PROCESS_INFORMATION info={};std::wstring command=L"\""+gameExe+L"\"";
    bool started=CreateProcessW(gameExe.c_str(),command.data(),NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,gameDirectory.c_str(),&startup,&info)!=FALSE;
    Expect(started,"independent target fixture starts");
    if(started){
        failedPid=info.dwProcessId;foundTargetPid=info.dwProcessId;enabled=false;SyncPanel();
        Expect(IsWindowEnabled(GetDlgItem(panelWindow,135))!=FALSE,"failed connection allows a manual retry");
        ClickAction();Expect(attachedPid==info.dwProcessId&&failedPid==0&&state&&state->enabled,"button retries already running PID and enables adjustment");
        DWORD retryPid=attachedPid;ClickAction();Expect(attachedPid==retryPid&&FindGame()==retryPid&&FixtureProcesses().size()==1,"connected action does not replace or duplicate host");
        StopHost(info.dwProcessId);CloseHandle(info.hThread);CloseHandle(info.hProcess);Tick();
    }
    for(DWORD pid:FixtureProcesses())StopHost(pid);
    CloseSettings();Disconnect();
    auto resolved=std::filesystem::weakly_canonical(fixture);
    if(resolved.parent_path()!=std::filesystem::weakly_canonical(std::filesystem::path(temporary))||resolved.filename()!=fixture.filename())return 2;
    std::filesystem::remove_all(resolved);
    std::cout<<checks<<" connection checks; "<<failures<<" failures\n";return failures?1:0;
}
