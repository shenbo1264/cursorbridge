#pragma once
#include <windowsx.h>

static HWND panelWindow=NULL,sizeEdit=NULL,sizeSlider=NULL,shapeCombo=NULL,colorCombo=NULL,lockCheck=NULL,shiftCheck=NULL;
static HCURSOR panelCursor=NULL;static int panelCursorSize=0,panelCursorTheme=-1,previewResource=0,panelCursorResource=-1;
static UINT panelDpi=96;static bool panelSyncing=false,hotkeyError=false;
static HFONT panelFont=NULL;static std::wstring panelNotice;
static HBRUSH panelBackground=NULL;
static int P(int value){return MulDiv(value,(int)panelDpi,96);}
static RECT PanelRect(int left,int top,int right,int bottom){return {P(left),P(top),P(right),P(bottom)};}
static void CloseSettings(){
    HWND closing=panelWindow;if(!closing)return;
    bool returnToGame=GetForegroundWindow()==closing&&attached&&WaitForSingleObject(attached,0)==WAIT_TIMEOUT;
    struct Target {DWORD pid;HWND window;};Target target={attachedPid,NULL};
    if(returnToGame)EnumWindows([](HWND candidate,LPARAM data)->BOOL{
        auto target=(Target*)data;DWORD pid=0;GetWindowThreadProcessId(candidate,&pid);
        if(pid==target->pid&&IsWindowVisible(candidate)&&!GetWindow(candidate,GW_OWNER)){target->window=candidate;return FALSE;}return TRUE;
    },(LPARAM)&target);
    DestroyWindow(closing);
    if(target.window){ShowWindow(target.window,SW_RESTORE);SetForegroundWindow(target.window);}
}
static void SyncPanel(){
    if(!panelWindow||panelSyncing)return;panelSyncing=true;
    if(GetFocus()!=sizeEdit)SetWindowTextW(sizeEdit,std::to_wstring(desiredSize).c_str());
    SendMessageW(sizeSlider,TBM_SETPOS,TRUE,desiredSize);
    SendMessageW(shapeCombo,CB_SETCURSEL,ThemeShape(desiredTheme),0);
    SendMessageW(colorCombo,CB_SETCURSEL,ThemeColor(desiredTheme),0);EnableWindow(colorCombo,desiredTheme!=0);
    SendMessageW(lockCheck,BM_SETCHECK,lockWindow?BST_CHECKED:BST_UNCHECKED,0);
    SendMessageW(shiftCheck,BM_SETCHECK,hotkeyModifiers&MOD_SHIFT?BST_CHECKED:BST_UNCHECKED,0);
    panelSyncing=false;InvalidateRect(panelWindow,NULL,FALSE);
}
static void ApplySize(int size,bool persist){
    if(!ValidCursorSize(size))return;desiredSize=size;enabled=true;PublishPreferences();
    if(persist)SaveSize();SyncPanel();
}
static void RestoreCursor(){enabled=false;PublishPreferences();SyncPanel();}
static void ApplyPreset(int slot){
    static const int defaults[]={24,32,48};CursorPreset preset=ReadPreset(settingsPath,L"Preset"+std::to_wstring(slot),{defaults[slot-1],0,false});
    desiredSize=preset.size;desiredTheme=preset.theme;lockWindow=preset.lock;enabled=true;SaveSize();PublishPreferences();SyncPanel();
}
static void PanelPreview(){
    int size=enabled?desiredSize:48,theme=enabled?desiredTheme:0;
    if(panelCursor&&panelCursorSize==size&&panelCursorTheme==theme&&panelCursorResource==previewResource)return;
    int resource=previewResource;
    if(theme&&resource>0&&resource<3){
        HCURSOR base=LoadCursorFromFileW(ResourcePath(0).c_str()),variant=LoadCursorFromFileW(ResourcePath(resource).c_str());Signature a,b;
        if(base&&variant&&CursorSignature(base,a)&&CursorSignature(variant,b)&&a==b)resource=0;
        if(base)DestroyCursor(base);if(variant)DestroyCursor(variant);
    }
    std::wstring asset=theme?ThemeResourcePath(binDir,theme,resource):ResourcePath(resource);
    HCURSOR next=(HCURSOR)LoadImageW(NULL,asset.c_str(),IMAGE_CURSOR,size,size,LR_LOADFROMFILE);
    if(!next){if(panelCursor){DestroyCursor(panelCursor);panelCursor=NULL;}panelCursorTheme=-1;return;}
    if(GetCursor()==panelCursor)SetCursor(LoadCursorW(NULL,IDC_ARROW));
    if(panelCursor)DestroyCursor(panelCursor);panelCursor=next;panelCursorSize=size;panelCursorTheme=theme;panelCursorResource=previewResource;
}
static void PanelText(HDC dc,const std::wstring& text,RECT box,int points,COLORREF color,UINT flags=DT_LEFT|DT_SINGLELINE|DT_VCENTER){
    HFONT font=CreateFontW(-P(points),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Microsoft YaHei UI");
    HGDIOBJ old=SelectObject(dc,font);SetTextColor(dc,color);SetBkMode(dc,TRANSPARENT);
    DrawTextW(dc,text.c_str(),-1,&box,flags);SelectObject(dc,old);DeleteObject(font);
}
static void PanelFill(HDC dc,RECT box,COLORREF color){HBRUSH brush=CreateSolidBrush(color);FillRect(dc,&box,brush);DeleteObject(brush);}
static void PaintPanel(HWND h){
    PAINTSTRUCT ps;HDC target=BeginPaint(h,&ps);RECT client;GetClientRect(h,&client);
    HDC dc=CreateCompatibleDC(target);HBITMAP bitmap=CreateCompatibleBitmap(target,client.right,client.bottom);HGDIOBJ old=SelectObject(dc,bitmap);
    const COLORREF fg=RGB(222,236,250),muted=RGB(150,178,207),accent=RGB(103,192,255);
    PanelFill(dc,client,RGB(16,30,49));PanelFill(dc,PanelRect(0,0,560,4),accent);
    PanelText(dc,Text(UiText::Title),PanelRect(24,14,500,50),20,fg);
    PanelText(dc,Text(UiText::Hint),PanelRect(24,50,540,76),12,muted);
    PanelText(dc,enabled?(uiLanguage.language==UiLanguage::Chinese?L"尺寸 (px)":L"Size (px)"):Text(UiText::Restored),PanelRect(24,86,145,119),16,accent);
    RECT preview=PanelRect(360,82,536,210);PanelFill(dc,preview,RGB(28,47,68));
    PanelPreview();UINT frame=enabled&&desiredTheme&&previewResource>=5?(UINT)((GetTickCount64()/133)%(previewResource==5?8:2)):0;
    if(panelCursor)DrawIconEx(dc,(preview.left+preview.right-panelCursorSize)/2,(preview.top+preview.bottom-panelCursorSize)/2,panelCursor,panelCursorSize,panelCursorSize,frame,NULL,DI_NORMAL);
    std::wstring previewLabel=std::wstring(Text((UiText)((int)UiText::PreviewNormal+previewResource)))+(uiLanguage.language==UiLanguage::Chinese?L" · 点击切换预览":L" · Click to preview");
    PanelText(dc,previewLabel,PanelRect(360,183,536,210),11,muted,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    PanelText(dc,L"1 px",PanelRect(24,168,100,192),12,muted);PanelText(dc,L"96 px",PanelRect(256,168,338,192),12,muted,DT_RIGHT|DT_SINGLELINE|DT_VCENTER);
    PanelText(dc,Text(UiText::Styles),PanelRect(24,211,270,234),12,muted);
    PanelText(dc,Text(UiText::Colors),PanelRect(290,211,535,234),11,muted);
    PanelText(dc,Text(UiText::Presets),PanelRect(24,318,535,344),12,muted);
    std::wstring keys=Text(UiText::Keys),keys2=Text(UiText::Keys2);
    if(hotkeyModifiers&MOD_SHIFT){keys.replace(0,8,L"Ctrl+Alt+Shift");keys2.replace(0,8,L"Ctrl+Alt+Shift");}
    PanelText(dc,keys,PanelRect(24,412,538,440),12,muted);
    PanelText(dc,keys2,PanelRect(24,440,355,463),11,muted);
    PanelText(dc,panelNotice.empty()?Text(hotkeyError?UiText::KeyError:attached?UiText::Connected:UiText::Waiting):panelNotice,PanelRect(24,511,538,537),11,accent);
    BitBlt(target,0,0,client.right,client.bottom,dc,0,0,SRCCOPY);
    SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);EndPaint(h,&ps);
}
static HWND PanelControl(const wchar_t* cls,const wchar_t* label,DWORD style,int id,int x,int y,int width,int height){
    HWND child=CreateWindowExW(wcscmp(cls,L"EDIT")==0?WS_EX_CLIENTEDGE:0,cls,label,WS_CHILD|WS_VISIBLE|WS_TABSTOP|style,P(x),P(y),P(width),P(height),panelWindow,(HMENU)(INT_PTR)id,GetModuleHandleW(NULL),NULL);
    SendMessageW(child,WM_SETFONT,(WPARAM)panelFont,TRUE);return child;
}
static void LocalizePanel(){
    if(!panelWindow)return;panelSyncing=true;
    SendMessageW(shapeCombo,CB_RESETCONTENT,0,0);SendMessageW(colorCombo,CB_RESETCONTENT,0,0);
    for(UiText label:{UiText::Original,UiText::Arrow,UiText::Cross,UiText::Ring})SendMessageW(shapeCombo,CB_ADDSTRING,0,(LPARAM)Text(label));
    for(UiText label:{UiText::White,UiText::Cyan,UiText::Amber,UiText::Pink})SendMessageW(colorCombo,CB_ADDSTRING,0,(LPARAM)Text(label));
    SetWindowTextW(lockCheck,Text(UiText::Lock));
    SetWindowTextW(shiftCheck,uiLanguage.language==UiLanguage::Chinese?L"快捷键加 Shift":L"Add Shift to hotkeys");
    for(int slot=1;slot<=3;slot++){
        SetWindowTextW(GetDlgItem(panelWindow,110+slot),(std::to_wstring(slot)+L" · "+Text(UiText::Load)).c_str());
        SetWindowTextW(GetDlgItem(panelWindow,120+slot),(std::wstring(Text(UiText::Save))+L" "+std::to_wstring(slot)).c_str());
    }
    SetWindowTextW(GetDlgItem(panelWindow,130),Text(UiText::DefaultSize));SetWindowTextW(GetDlgItem(panelWindow,131),Text(UiText::Restore));SetWindowTextW(GetDlgItem(panelWindow,132),Text(UiText::Done));
    panelSyncing=false;SyncPanel();
}
static LRESULT CALLBACK PanelProcedure(HWND h,UINT m,WPARAM w,LPARAM l){
    if(m==WM_ERASEBKGND)return 1;
    if(m==WM_PAINT){PaintPanel(h);return 0;}
    if(m==WM_CTLCOLORSTATIC){HDC dc=(HDC)w;SetTextColor(dc,RGB(222,236,250));SetBkColor(dc,RGB(16,30,49));return (LRESULT)panelBackground;}
    if(m==WM_LBUTTONDOWN){POINT point={GET_X_LPARAM(l),GET_Y_LPARAM(l)};RECT preview=PanelRect(360,82,536,210);if(PtInRect(&preview,point)){previewResource=(previewResource+1)%9;InvalidateRect(h,NULL,FALSE);}return 0;}
    if(m==WM_HSCROLL&&(HWND)l==sizeSlider){ApplySize((int)SendMessageW(sizeSlider,TBM_GETPOS,0,0),true);return 0;}
    if(m==WM_MOUSEWHEEL){ApplySize(desiredSize+(GET_WHEEL_DELTA_WPARAM(w)>0?1:-1),true);return 0;}
    if(m==WM_COMMAND){
        if(panelSyncing)return 0;int id=LOWORD(w),notification=HIWORD(w);
        if(id==IDCANCEL){CloseSettings();return 0;}
        if(id==100&&notification==EN_CHANGE){
            wchar_t value[16]={};GetWindowTextW(sizeEdit,value,16);wchar_t* end=NULL;long size=wcstol(value,&end,10);
            if(*value&&end&&!*end&&ValidCursorSize((int)size))ApplySize((int)size,true);
        }else if(id==100&&notification==EN_KILLFOCUS){panelSyncing=true;SetWindowTextW(sizeEdit,std::to_wstring(desiredSize).c_str());panelSyncing=false;}
        else if((id==102||id==103)&&notification==CBN_SELCHANGE){
            int shape=(int)SendMessageW(shapeCombo,CB_GETCURSEL,0,0),color=(int)SendMessageW(colorCombo,CB_GETCURSEL,0,0);
            if(shape>=0&&shape<=3&&color>=0&&color<=3){desiredTheme=ThemeFromSelection(shape,color);enabled=true;SaveSize();PublishPreferences();SyncPanel();}
        }else if(id==104&&notification==BN_CLICKED){lockWindow=SendMessageW(lockCheck,BM_GETCHECK,0,0)==BST_CHECKED;SaveSize();PublishPreferences();SyncPanel();}
        else if(id==105&&notification==BN_CLICKED){bool shift=SendMessageW(shiftCheck,BM_GETCHECK,0,0)==BST_CHECKED;hotkeyModifiers=MOD_CONTROL|MOD_ALT|(shift?MOD_SHIFT:0);WritePrivateProfileStringW(L"Hotkeys",L"AddShift",shift?L"1":L"0",settingsPath.c_str());SyncPanel();}
        else if(id>=111&&id<=113&&notification==BN_CLICKED){panelNotice.clear();ApplyPreset(id-110);}
        else if(id>=121&&id<=123&&notification==BN_CLICKED){WritePreset(settingsPath,L"Preset"+std::to_wstring(id-120),{desiredSize,desiredTheme,lockWindow});panelNotice=std::wstring(Text(UiText::Saved))+L" "+std::to_wstring(id-120);SyncPanel();}
        else if(id==130&&notification==BN_CLICKED)ApplySize(32,true);
        else if(id==131&&notification==BN_CLICKED)RestoreCursor();
        else if(id==132&&notification==BN_CLICKED)CloseSettings();
        return 0;
    }
    if(m==WM_CLOSE){CloseSettings();return 0;}
    if(m==WM_KEYDOWN&&w==VK_ESCAPE){CloseSettings();return 0;}
    if(m==WM_DESTROY){panelWindow=NULL;if(panelCursor){DestroyCursor(panelCursor);panelCursor=NULL;}panelCursorSize=0;panelCursorTheme=-1;if(panelFont){DeleteObject(panelFont);panelFont=NULL;}if(panelBackground){DeleteObject(panelBackground);panelBackground=NULL;}return 0;}
    return DefWindowProcW(h,m,w,l);
}
static void OpenSettings(){
    if(panelWindow){ShowWindow(panelWindow,SW_SHOW);SetForegroundWindow(panelWindow);return;}
    WNDCLASSW cls={};cls.hInstance=GetModuleHandleW(NULL);cls.lpfnWndProc=PanelProcedure;cls.lpszClassName=L"StellarisCursorSettings";cls.hCursor=LoadCursorW(NULL,IDC_ARROW);RegisterClassW(&cls);
    HDC screen=GetDC(NULL);panelDpi=(UINT)GetDeviceCaps(screen,LOGPIXELSX);ReleaseDC(NULL,screen);
    HWND foreground=GetForegroundWindow();HMONITOR monitor=MonitorFromWindow(foreground,MONITOR_DEFAULTTONEAREST);MONITORINFO display={};display.cbSize=sizeof(display);GetMonitorInfoW(monitor,&display);RECT bounds=display.rcWork;
    panelWindow=CreateWindowExW(WS_EX_APPWINDOW|WS_EX_TOPMOST,cls.lpszClassName,Text(UiText::WindowTitle),WS_POPUP|WS_BORDER|WS_CLIPCHILDREN,
        bounds.left+(bounds.right-bounds.left-P(560))/2,bounds.top+(bounds.bottom-bounds.top-P(550))/2,P(560),P(550),NULL,NULL,cls.hInstance,NULL);
    if(!panelWindow)return;
    panelBackground=CreateSolidBrush(RGB(16,30,49));
    panelFont=CreateFontW(-P(13),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Microsoft YaHei UI");
    sizeEdit=PanelControl(L"EDIT",L"",ES_NUMBER|ES_AUTOHSCROLL,100,150,86,82,32);SendMessageW(sizeEdit,EM_SETLIMITTEXT,2,0);
    sizeSlider=PanelControl(TRACKBAR_CLASSW,L"",TBS_HORZ|TBS_NOTICKS,101,24,129,314,36);SendMessageW(sizeSlider,TBM_SETRANGE,TRUE,MAKELPARAM(1,96));SendMessageW(sizeSlider,TBM_SETPAGESIZE,0,8);
    shapeCombo=PanelControl(L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_VSCROLL,102,24,238,246,180);
    colorCombo=PanelControl(L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_VSCROLL,103,290,238,246,170);
    lockCheck=PanelControl(L"BUTTON",L"",BS_AUTOCHECKBOX,104,24,288,510,27);
    shiftCheck=PanelControl(L"BUTTON",L"",BS_AUTOCHECKBOX,105,370,439,168,25);
    for(int slot=1;slot<=3;slot++){int x=24+(slot-1)*175;PanelControl(L"BUTTON",L"",BS_PUSHBUTTON,110+slot,x,348,160,29);PanelControl(L"BUTTON",L"",BS_PUSHBUTTON,120+slot,x,380,160,28);}
    PanelControl(L"BUTTON",L"",BS_PUSHBUTTON,130,24,474,147,32);PanelControl(L"BUTTON",L"",BS_PUSHBUTTON,131,185,474,192,32);PanelControl(L"BUTTON",L"",BS_DEFPUSHBUTTON,132,391,474,145,32);
    panelNotice.clear();LocalizePanel();ShowWindow(panelWindow,SW_SHOW);UpdateWindow(panelWindow);SetForegroundWindow(panelWindow);SetFocus(sizeSlider);Log(L"Opened cursor settings: exact size, themes, presets and confinement.");
}
static void PanelTick(){
    if(!panelWindow)return;HWND foreground=GetForegroundWindow();DWORD pid=0;GetWindowThreadProcessId(foreground,&pid);
    if(previewResource>=5)InvalidateRect(panelWindow,NULL,FALSE);
    if(attachedPid&&pid!=attachedPid&&foreground!=panelWindow&&!IsChild(panelWindow,foreground))ShowWindow(panelWindow,SW_HIDE);
    else if(!IsWindowVisible(panelWindow))ShowWindow(panelWindow,SW_SHOWNOACTIVATE);
}
