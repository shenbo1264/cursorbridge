// Exercise our native controls in a hidden, isolated panel. No input injection,
// external game/process, desktop preference changes or UI automation is used.
#define wWinMain CursorBridgeApplicationEntry
#include "controller.cpp"
#undef wWinMain
#include <iostream>

static int checks=0,failures=0;
static void Expect(bool result,const char* label){++checks;if(!result){++failures;std::cerr<<"FAIL: "<<label<<"\n";}}
static void Command(int id,int code=BN_CLICKED){SendMessageW(panelWindow,WM_COMMAND,MAKEWPARAM(id,code),(LPARAM)GetDlgItem(panelWindow,id));}
static std::wstring ControlText(HWND child){wchar_t text[256]={};GetWindowTextW(child,text,256);return text;}
int wmain(){
    SetProcessDPIAware();INITCOMMONCONTROLSEX controls={sizeof(controls),ICC_BAR_CLASSES};InitCommonControlsEx(&controls);
    wchar_t temporary[MAX_PATH]={};GetTempPathW(MAX_PATH,temporary);
    auto fixture=std::filesystem::path(temporary)/(L"CursorBridge-panel-test-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64()));
    if(!std::filesystem::create_directory(fixture))return 2;settingsPath=(fixture/L"settings.ini").wstring();logPath=(fixture/L"test.log").wstring();
    binDir=Parent(ModulePath());desiredSize=24;desiredTheme=6;lockWindow=false;enabled=true;
    uiLanguage.language=UiLanguage::Chinese;OpenSettings(false);
    attached=(HANDLE)1;hotkeyError=true;
    Expect(PanelStatus().find(L"已连接")!=std::wstring::npos&&PanelStatus().find(L"24 px")!=std::wstring::npos,"shortcut warning retains connection and applied size");
    enabled=false;Expect(PanelStatus().find(L"调整已暂停")!=std::wstring::npos,"paused connection explains re-enabling adjustment");
    uiLanguage.language=UiLanguage::English;Expect(PanelStatus().find(L"Paused")!=std::wstring::npos,"English pause status");
    attached=NULL;failedPid=42;Expect(PanelStatus().find(L"Connection failed")!=std::wstring::npos,"failure is distinct from waiting");
    failedPid=0;hotkeyError=false;enabled=true;uiLanguage.language=UiLanguage::Chinese;
    Expect(panelWindow!=NULL,"panel created");if(!panelWindow)return 1;
    Expect(!IsWindowVisible(panelWindow),"test panel stays hidden");
    Expect(ControlText(sizeEdit)==L"24","initial numeric value");
    Expect(SendMessageW(sizeSlider,TBM_GETRANGEMIN,0,0)==1&&SendMessageW(sizeSlider,TBM_GETRANGEMAX,0,0)==96,"full slider range");
    SetWindowTextW(sizeEdit,L"1");Expect(desiredSize==1,"lower endpoint accepts native edit notification");
    SetWindowTextW(sizeEdit,L"96");Expect(desiredSize==96,"upper endpoint accepts native edit notification");
    SetWindowTextW(sizeEdit,L"97");Expect(desiredSize==96,"out-of-range text rejected");
    Command(100,EN_KILLFOCUS);Expect(ControlText(sizeEdit)==L"96","invalid input normalizes on blur");
    SetWindowTextW(sizeEdit,L"");Expect(desiredSize==96,"empty edit does not overwrite preference");Command(100,EN_KILLFOCUS);
    SendMessageW(sizeSlider,WM_KEYDOWN,VK_HOME,0);Expect(desiredSize==1,"native slider Home still applies value");
    SendMessageW(sizeSlider,WM_KEYDOWN,VK_END,0);Expect(desiredSize==96,"native slider End still applies value");
    SendMessageW(sizeSlider,WM_KEYDOWN,VK_LEFT,0);Expect(desiredSize==95,"native slider arrow still applies value");
    SendMessageW(sizeSlider,TBM_SETPOS,TRUE,37);Expect(desiredSize==37,"external slider value applies preference");
    Expect(ControlText(sizeEdit)==L"37","external slider value synchronizes numeric edit");
    SendMessageW(shapeCombo,CB_SETCURSEL,3,0);SendMessageW(colorCombo,CB_SETCURSEL,2,0);Command(102,CBN_SELCHANGE);
    Expect(desiredTheme==11,"native style/color selection changes theme");
    SendMessageW(shapeCombo,CB_SETCURSEL,0,0);Command(102,CBN_SELCHANGE);Expect(!IsWindowEnabled(colorCombo),"original cursor disables color");
    SendMessageW(shapeCombo,CB_SETCURSEL,2,0);SendMessageW(colorCombo,CB_SETCURSEL,2,0);Command(102,CBN_SELCHANGE);Expect(IsWindowEnabled(colorCombo)!=FALSE,"custom cursor enables color");
    SendMessageW(lockCheck,BM_SETCHECK,BST_CHECKED,0);Command(104);Expect(lockWindow,"confinement checkbox applies");
    SendMessageW(shiftCheck,BM_SETCHECK,BST_CHECKED,0);Command(105);Expect((hotkeyModifiers&MOD_SHIFT)!=0,"Shift modifier applies");
    ApplySize(27,true);Command(121);ApplySize(44,true);Command(111);Expect(desiredSize==27&&desiredTheme==7&&lockWindow,"preset round trip keeps all fields");
    Command(130);Expect(desiredSize==32&&enabled,"recommended size enables cursor");
    Command(131);Expect(!enabled,"restore disables override");
    int previousPreview=previewResource;Command(106);Expect(previewResource==(previousPreview+1)%9,"preview accessible button cycles state");
    uiLanguage.language=UiLanguage::English;LocalizePanel();Expect(ControlText(GetDlgItem(panelWindow,132))==L"Done","English action label");
    Expect(ControlText(shiftCheck)==L"Add Shift to hotkeys","English toggle label");
    Expect(SendMessageW(shapeCombo,CB_GETCURSEL,0,0)==2,"language change retains style selection");
    uiLanguage.language=UiLanguage::Chinese;LocalizePanel();Expect(ControlText(GetDlgItem(panelWindow,132))==L"完成","Chinese action label");
    CursorPreset saved=ReadPreset(settingsPath,L"Cursor");Expect(saved.size==32&&saved.theme==7&&saved.lock,"preference persistence survives UI refresh");
    // Paint all controls in their own window DC: catches renderer/subclass
    // recursion and invalid resources without pretending to test system blur.
    for(int id:{100,101,102,103,104,105,106,111,121,132,133}){SendMessageW(GetDlgItem(panelWindow,id),WM_PAINT,0,0);}
    SendMessageW(panelWindow,WM_PAINT,0,0);Expect(IsWindow(panelWindow)!=FALSE,"native rendering completes");
    Command(133);Expect(panelWindow==NULL,"close action releases panel");
    OpenSettings(false);Expect(panelWindow!=NULL&&ControlText(sizeEdit)==L"32","reopen keeps selected size");CloseSettings();
    Expect(panelFont==NULL&&panelNumberFont==NULL&&panelBackground==NULL,"close releases renderer resources");
    auto resolved=std::filesystem::weakly_canonical(fixture);
    if(resolved.parent_path()!=std::filesystem::weakly_canonical(std::filesystem::path(temporary))||resolved.filename()!=fixture.filename())return 2;
    std::filesystem::remove_all(resolved);
    std::cout<<checks<<" panel checks; "<<failures<<" failures\n";return failures?1:0;
}
