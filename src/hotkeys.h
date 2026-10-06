#pragma once
static bool hotkeysRegistered=false;
static UINT registeredModifiers=0;
static bool HotkeyContext(){
    if(!attachedPid)return false;HWND foreground=GetForegroundWindow();DWORD pid=0;GetWindowThreadProcessId(foreground,&pid);
    return pid==attachedPid||foreground==panelWindow;
}
static void ClearHotkeys(){for(int id=201;id<=208;id++)UnregisterHotKey(windowHandle,id);hotkeysRegistered=false;}
static void RefreshHotkeys(){
    bool context=HotkeyContext();if(context==hotkeysRegistered&&registeredModifiers==hotkeyModifiers)return;
    if(hotkeysRegistered)ClearHotkeys();
    if(!context){ClearHotkeys();return;}
    const UINT keys[]={'C',VK_UP,VK_DOWN,'0','1','2','3','L'};hotkeyError=false;
    for(int i=0;i<8;i++)if(!RegisterHotKey(windowHandle,201+i,hotkeyModifiers|MOD_NOREPEAT,keys[i])){hotkeyError=true;Log(L"Hotkey unavailable: key="+std::to_wstring(keys[i])+L", error="+std::to_wstring(GetLastError()));}
    hotkeysRegistered=true;registeredModifiers=hotkeyModifiers;if(panelWindow)InvalidateRect(panelWindow,NULL,FALSE);
}
static void HandleHotkey(int id){
    if(!HotkeyContext())return;
    if(id==201)OpenSettings();
    else if(id==202)ApplySize(desiredSize+1,true);
    else if(id==203)ApplySize(desiredSize-1,true);
    else if(id==204){lockWindow=false;SaveSize();RestoreCursor();}
    else if(id>=205&&id<=207)ApplyPreset(id-204);
    else if(id==208){lockWindow=!lockWindow;SaveSize();PublishPreferences();SyncPanel();}
}
