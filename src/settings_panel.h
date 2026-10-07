#pragma once
#include <windowsx.h>
#include "glass_style.h"

static HWND panelWindow=NULL,sizeEdit=NULL,sizeSlider=NULL,shapeCombo=NULL,colorCombo=NULL,lockCheck=NULL,shiftCheck=NULL,enableCheck=NULL,panelTooltip=NULL;
static std::wstring panelTooltips[141];
static HCURSOR panelCursor=NULL;static int panelCursorSize=0,panelCursorTheme=-1,previewResource=0,panelCursorResource=-1;
static UINT panelDpi=96;static bool panelSyncing=false,hotkeyError=false;
static HFONT panelFont=NULL;static std::wstring panelNotice;
static HBRUSH panelBackground=NULL;
static HFONT panelNumberFont=NULL;
static bool panelGlass=false,panelContrast=false;
static const int PanelWidth=640,PanelHeight=768;
static COLORREF PanelFg(){return panelContrast?GetSysColor(COLOR_WINDOWTEXT):RGB(29,36,49);}
static COLORREF PanelMuted(){return panelContrast?GetSysColor(COLOR_WINDOWTEXT):RGB(43,55,70);}
static COLORREF PanelAccent(){return panelContrast?GetSysColor(COLOR_HIGHLIGHT):RGB(0,105,219);}
static COLORREF PanelSurface(){return panelContrast?GetSysColor(COLOR_WINDOW):RGB(250,251,253);}
static bool PanelChinese(){return uiLanguage.language==UiLanguage::Chinese;}
static const wchar_t* PanelLabel(const wchar_t* cn,const wchar_t* en){return PanelChinese()?cn:en;}
static const wchar_t* LaunchButtonLabel(){
    if(connecting)return PanelLabel(L"正在连接…",L"Connecting…");
    if(attached)return genericAdapter?PanelLabel(L"返回目标程序",L"Return to app"):PanelLabel(L"返回游戏",L"Return to game");
    if(launchDeadline)return PanelLabel(L"正在启动…",L"Starting…");
    if(foundTargetPid)return genericAdapter?PanelLabel(L"连接程序",L"Connect app"):PanelLabel(L"连接游戏",L"Connect game");
    return genericAdapter?PanelLabel(L"启动并连接",L"Launch & connect"):PanelLabel(L"启动并连接游戏",L"Launch game");
}
static void SyncLaunchButton(){
    if(!panelWindow)return;HWND button=GetDlgItem(panelWindow,135);if(!button)return;
    wchar_t current[128]={};GetWindowTextW(button,current,128);
    const wchar_t* label=LaunchButtonLabel();bool active=!connecting&&!launchDeadline;
    HWND chooser=GetDlgItem(panelWindow,134);bool canChoose=!connecting&&!launchDeadline;
    if((IsWindowEnabled(chooser)!=FALSE)!=canChoose)EnableWindow(chooser,canChoose);
    if(wcscmp(current,label)){SetWindowTextW(button,label);InvalidateRect(panelWindow,NULL,FALSE);}
    if((IsWindowEnabled(button)!=FALSE)!=active)EnableWindow(button,active);
}
static std::wstring PanelStatus(){
    if(connecting)return PanelLabel(L"正在连接目标进程…",L"Connecting to target process…");
    if(attached)return std::wstring(PanelLabel(L"已连接 · ",L"Connected · "))+std::to_wstring(desiredSize)+L" px · "+
        (enabled?PanelLabel(L"调整已开启",L"Adjustment on"):PanelLabel(L"调整已暂停",L"Adjustment paused"));
    if(launchDeadline)return PanelLabel(L"正在启动目标 · 等待自动连接",L"Starting target · Waiting to connect");
    if(failedPid)return PanelLabel(L"连接失败 · 请点击连接按钮重试",L"Connection failed · Use Connect to retry");
    return enabled?Text(UiText::Waiting):PanelLabel(L"等待目标程序 · 调整已暂停",L"Waiting for target · Adjustment paused");
}
static std::wstring PanelDetail(){
    if(preferencesSaveFailed)return PanelLabel(L"设置保存失败，请检查设置文件夹是否可写。",L"Settings could not be saved. Check that the settings folder is writable.");
    if(!panelNotice.empty())return panelNotice;
    if(hotkeyError)return PanelLabel(L"部分快捷键已被占用；可勾选“快捷键加 Shift”避开冲突。",L"Some hotkeys are unavailable. Try adding Shift to avoid the conflict.");
    if(!enabled)return PanelLabel(L"暂停期间可预调样式；主动开启“启用光标调整”后生效。",L"Preview and edit while paused. Turn on Enable cursor adjustment to apply.");
    return PanelLabel(L"设置即时保存 · 关闭窗口后仍在托盘运行",L"Settings save automatically · Closing the window keeps the app in the tray");
}
static void SetControlTextIfChanged(HWND child,const std::wstring& value){
    wchar_t current[512]={};GetWindowTextW(child,current,512);
    if(value!=current)SetWindowTextW(child,value.c_str());
}
static void SyncPanelStatus(){
    if(!panelWindow)return;
    SetControlTextIfChanged(GetDlgItem(panelWindow,139),PanelStatus());
    SetControlTextIfChanged(GetDlgItem(panelWindow,140),PanelDetail());
}
static int P(int value){return MulDiv(value,(int)panelDpi,96);}
static RECT PanelRect(int left,int top,int right,int bottom){return {P(left),P(top),P(right),P(bottom)};}
static void CloseSettings(){if(panelWindow)DestroyWindow(panelWindow);}
static void ReturnToTarget(){
    struct Target {DWORD pid;HWND window;};Target target={attachedPid,NULL};
    EnumWindows([](HWND candidate,LPARAM data)->BOOL{
        auto selected=(Target*)data;DWORD pid=0;GetWindowThreadProcessId(candidate,&pid);
        if(pid==selected->pid&&IsWindowVisible(candidate)&&!GetWindow(candidate,GW_OWNER)){selected->window=candidate;return FALSE;}return TRUE;
    },(LPARAM)&target);
    if(target.window){if(IsIconic(target.window))ShowWindow(target.window,SW_RESTORE);SetForegroundWindow(target.window);}
    else panelNotice=PanelLabel(L"目标已连接，但当前没有可切换的窗口。",L"Target connected, but no window is available to switch to.");
}
static void SyncPanel(){
    if(!panelWindow||panelSyncing)return;panelSyncing=true;
    SyncLaunchButton();SyncPanelStatus();
    SendMessageW(enableCheck,BM_SETCHECK,enabled?BST_CHECKED:BST_UNCHECKED,0);
    if(GetFocus()!=sizeEdit)SetWindowTextW(sizeEdit,std::to_wstring(desiredSize).c_str());
    SendMessageW(sizeSlider,TBM_SETPOS,TRUE,desiredSize);
    SendMessageW(shapeCombo,CB_SETCURSEL,ThemeShape(desiredTheme),0);
    SendMessageW(colorCombo,CB_SETCURSEL,ThemeColor(desiredTheme),0);EnableWindow(colorCombo,desiredTheme!=0);
    SendMessageW(lockCheck,BM_SETCHECK,lockWindow?BST_CHECKED:BST_UNCHECKED,0);
    SendMessageW(shiftCheck,BM_SETCHECK,hotkeyModifiers&MOD_SHIFT?BST_CHECKED:BST_UNCHECKED,0);
    panelSyncing=false;RedrawWindow(panelWindow,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN);
}
static void ApplySize(int size,bool persist){
    if(!ValidCursorSize(size))return;desiredSize=size;PublishPreferences();
    if(persist)SaveSize();SyncPanel();
}
static void SetAdjustmentEnabled(bool value){enabled=value;SaveSize();PublishPreferences();SyncPanel();}
static void RestoreCursor(){SetAdjustmentEnabled(false);}
static void ApplyPreset(int slot){
    static const int defaults[]={24,32,48};CursorPreset preset=ReadPreset(settingsPath,L"Preset"+std::to_wstring(slot),{defaults[slot-1],0,false});
    desiredSize=preset.size;desiredTheme=preset.theme;lockWindow=preset.lock;SaveSize();PublishPreferences();SyncPanel();
}
static void PanelPreview(){
    int size=desiredSize,theme=desiredTheme;
    if(panelCursor&&panelCursorSize==size&&panelCursorTheme==theme&&panelCursorResource==previewResource)return;
    int resource=previewResource;
    if(theme&&resource>0&&resource<3){
        HCURSOR base=LoadCursorFromFileW(ResourcePath(0).c_str()),variant=LoadCursorFromFileW(ResourcePath(resource).c_str());Signature a,b;
        if(base&&variant&&CursorSignature(base,a)&&CursorSignature(variant,b)&&a==b)resource=0;
        if(base)DestroyCursor(base);if(variant)DestroyCursor(variant);
    }
    std::wstring asset=theme?ThemeResourcePath(binDir,theme,resource):ResourcePath(resource);
    HCURSOR next=genericAdapter&&!theme?ResizeNativeCursor(LoadCursorW(NULL,IDC_ARROW),size):
        (HCURSOR)LoadImageW(NULL,asset.c_str(),IMAGE_CURSOR,size,size,LR_LOADFROMFILE);
    if(!next){if(panelCursor){DestroyCursor(panelCursor);panelCursor=NULL;}panelCursorTheme=-1;return;}
    if(GetCursor()==panelCursor)SetCursor(LoadCursorW(NULL,IDC_ARROW));
    if(panelCursor)DestroyCursor(panelCursor);panelCursor=next;panelCursorSize=size;panelCursorTheme=theme;panelCursorResource=previewResource;
}

static void PanelText(Gdiplus::Graphics& g,const std::wstring& text,RECT box,int size,COLORREF color,bool center=false,bool semibold=false,bool wrap=false){
    using namespace Gdiplus;
    Font font(PanelChinese()?L"Microsoft YaHei UI":L"Segoe UI",(float)P(size),semibold?FontStyleBold:FontStyleRegular,UnitPixel);
    SolidBrush brush(GlassColor(color));StringFormat format;
    format.SetAlignment(center?StringAlignmentCenter:StringAlignmentNear);format.SetLineAlignment(StringAlignmentCenter);
    if(!wrap)format.SetFormatFlags(StringFormatFlagsNoWrap);format.SetTrimming(StringTrimmingEllipsisCharacter);
    RectF rect((float)box.left,(float)box.top,(float)(box.right-box.left),(float)(box.bottom-box.top));
    g.DrawString(text.c_str(),(INT)text.size(),&font,rect,&format,&brush);
}
static void PanelLine(Gdiplus::Graphics& g,int y){
    Gdiplus::Pen pen(GlassColor(PanelMuted(),panelContrast?255:35),(float)P(1));g.DrawLine(&pen,P(28),P(y),P(612),P(y));
}
static void PaintPanel(HWND h){
    PAINTSTRUCT ps;HDC target=BeginPaint(h,&ps);RECT client;GetClientRect(h,&client);
    GlassCanvas canvas(target,client.right,client.bottom);auto& g=*canvas.graphics;
    if(panelGlass)g.Clear(Gdiplus::Color(66,255,255,255));
    else g.Clear(GlassColor(panelContrast?GetSysColor(COLOR_WINDOW):RGB(238,241,247)));
    GlassRound(g,PanelRect(1,1,639,PanelHeight-1),(float)P(20),Gdiplus::Color(0,0,0,0),Gdiplus::Color(140,255,255,255),(float)P(1));
    PanelText(g,L"CursorBridge",PanelRect(28,17,520,51),25,PanelFg(),false,false);
    GlassRound(g,PanelRect(20,96,620,282),(float)P(18),GlassColor(PanelSurface(),panelGlass?225:255),GlassColor(RGB(255,255,255)));
    GlassRound(g,PanelRect(38,142,130,190),(float)P(10),GlassColor(PanelSurface()),GlassColor(GetFocus()==sizeEdit?PanelAccent():RGB(207,214,224)),(float)P(1));
    PanelText(g,L"px",PanelRect(142,146,190,185),17,PanelMuted());
    PanelText(g,L"1 px",PanelRect(40,240,120,264),12,PanelMuted());
    PanelText(g,L"96 px",PanelRect(321,240,378,264),12,PanelMuted());
    PanelText(g,PanelLabel(L"光标样式",L"Cursor style"),PanelRect(28,299,305,326),13,PanelMuted());
    PanelText(g,PanelLabel(L"颜色",L"Color"),PanelRect(334,299,610,326),13,PanelMuted());
    PanelLine(g,438);
    PanelText(g,PanelLabel(L"个人预设",L"Your presets"),PanelRect(28,449,610,476),14,PanelFg(),false,true);
    std::wstring keys=Text(UiText::Keys),keys2=Text(UiText::Keys2);
    if(hotkeyModifiers&MOD_SHIFT){keys.replace(0,8,L"Ctrl+Alt+Shift");keys2.replace(0,8,L"Ctrl+Alt+Shift");}
    PanelText(g,keys,PanelRect(28,558,612,584),12,PanelMuted());
    PanelText(g,keys2,PanelRect(28,584,612,610),12,PanelMuted());
    PanelLine(g,651);
    canvas.Present();EndPaint(h,&ps);
}
static void PaintPanelControl(HWND child){
    PAINTSTRUCT ps;HDC target=BeginPaint(child,&ps);RECT r;GetClientRect(child,&r);
    GlassCanvas canvas(target,r.right,r.bottom);auto& g=*canvas.graphics;
    int id=GetDlgCtrlID(child);bool focus=GetFocus()==child,active=IsWindowEnabled(child)!=FALSE;
    bool pressed=(SendMessageW(child,BM_GETSTATE,0,0)&BST_PUSHED)!=0;
    POINT pointer;GetCursorPos(&pointer);ScreenToClient(child,&pointer);bool hover=PtInRect(&r,pointer)!=FALSE;
    if(panelContrast)g.Clear(GlassColor(PanelSurface()));
    else if(id==101||id==106||id==137)g.Clear(GlassColor(PanelSurface(),panelGlass?225:255));
    else g.Clear(panelGlass?Gdiplus::Color(66,255,255,255):GlassColor(RGB(238,241,247)));
    RECT inset={P(1),P(1),r.right-P(1),r.bottom-P(1)};
    COLORREF text=active?PanelFg():panelContrast?GetSysColor(COLOR_GRAYTEXT):PanelMuted();
    if(id==101){
        RECT thumb;SendMessageW(child,TBM_GETTHUMBRECT,0,(LPARAM)&thumb);
        int x=(thumb.left+thumb.right)/2,y=(std::max)(P(12),(std::min)((int)r.bottom-P(12),(int)(thumb.top+thumb.bottom)/2));
        RECT rail={P(10),y-P(3),r.right-P(10),y+P(3)};
        GlassRound(g,rail,(float)P(3),GlassColor(RGB(217,224,235)));
        RECT fill=rail;fill.right=(std::max)((int)fill.left+P(6),x);
        GlassRound(g,fill,(float)P(3),GlassColor(PanelAccent()));
        Gdiplus::SolidBrush shadow(Gdiplus::Color(30,38,55,83));g.FillEllipse(&shadow,x-P(12),y-P(10),P(24),P(24));
        Gdiplus::SolidBrush knob(GlassColor(RGB(255,255,255)));g.FillEllipse(&knob,x-P(11),y-P(11),P(22),P(22));
        Gdiplus::Pen edge(GlassColor(focus?PanelAccent():RGB(195,204,218)),(float)P(focus?2:1));g.DrawEllipse(&edge,x-P(11),y-P(11),P(22),P(22));
    }else if(id==106){
        GlassRound(g,inset,(float)P(14),GlassColor(panelContrast?PanelSurface():RGB(235,239,246)),GlassColor(focus?PanelAccent():RGB(218,225,235)),(float)P(1));
        // A quiet cross-grid provides context for very small cursors.
        Gdiplus::Pen grid(GlassColor(PanelMuted(),25),(float)P(1));
        g.DrawLine(&grid,r.right/2,P(28),r.right/2,r.bottom-P(46));g.DrawLine(&grid,P(30),P(70),r.right-P(30),P(70));
        PanelPreview();UINT frame=desiredTheme&&previewResource>=5?(UINT)((GetTickCount64()/133)%(previewResource==5?8:2)):0;
        if(panelCursor)canvas.Icon(panelCursor,(r.right-panelCursorSize)/2,(P(140)-panelCursorSize)/2,panelCursorSize,frame);
        PanelText(g,Text((UiText)((int)UiText::PreviewNormal+previewResource)),RECT{P(4),P(126),r.right-P(4),P(147)},12,PanelFg(),true);
        PanelText(g,PanelLabel(L"点击切换预览",L"Click to preview"),RECT{P(4),P(148),r.right-P(4),P(167)},10,PanelMuted(),true);
    }else if(id==102||id==103){
        GlassRound(g,inset,(float)P(10),GlassColor(PanelSurface()),GlassColor(focus?PanelAccent():RGB(206,215,228)),(float)P(focus?2:1));
        wchar_t title[256]={};GetWindowTextW(child,title,256);
        PanelText(g,title,RECT{P(12),0,r.right-P(35),r.bottom},13,text);
        Gdiplus::Pen pen(GlassColor(text),(float)P(1));int x=r.right-P(19),y=r.bottom/2;
        g.DrawLine(&pen,x-P(4),y-P(2),x,y+P(2));g.DrawLine(&pen,x,y+P(2),x+P(4),y-P(2));
    }else if(id==104||id==105||id==137){
        bool checked=SendMessageW(child,BM_GETCHECK,0,0)==BST_CHECKED;
        int top=((int)r.bottom-P(24))/2;RECT pill={r.right-P(48),top,r.right-P(4),top+P(24)};
        GlassRound(g,pill,(float)P(12),GlassColor(checked?PanelAccent():RGB(193,202,215)));
        Gdiplus::SolidBrush knob(GlassColor(RGB(255,255,255)));
        g.FillEllipse(&knob,checked?r.right-P(26):r.right-P(46),top+P(2),P(20),P(20));
        wchar_t title[256]={};GetWindowTextW(child,title,256);
        PanelText(g,title,RECT{P(4),0,r.right-P(62),r.bottom},13,text);
        if(focus)GlassRound(g,inset,(float)P(8),Gdiplus::Color(0,0,0,0),GlassColor(PanelAccent()),(float)P(2));
    }else if(id==139||id==140){
        wchar_t title[512]={};GetWindowTextW(child,title,512);
        PanelText(g,title,r,id==139?13:11,id==140&&(preferencesSaveFailed||hotkeyError)?RGB(176,49,44):PanelMuted(),false,id==139,id==140);
    }else{
        bool primary=id==135;
        COLORREF surface=!active?RGB(230,234,241):primary?PanelAccent():pressed?RGB(220,230,243):hover?RGB(232,239,248):RGB(255,255,255);
        if(panelContrast)surface=primary?GetSysColor(COLOR_HIGHLIGHT):GetSysColor(COLOR_WINDOW);
        GlassRound(g,inset,(float)P(id==133?13:10),GlassColor(surface),GlassColor(focus?PanelAccent():RGB(209,217,229)),(float)P(focus?2:1));
        wchar_t title[256]={};GetWindowTextW(child,title,256);
        if(id==133||id==136){
            using namespace Gdiplus;
            Font font(L"Segoe MDL2 Assets",(float)P(12),FontStyleRegular,UnitPixel);SolidBrush brush(GlassColor(text));StringFormat format;
            format.SetAlignment(StringAlignmentCenter);format.SetLineAlignment(StringAlignmentCenter);
            RectF box((float)inset.left,(float)inset.top,(float)(inset.right-inset.left),(float)(inset.bottom-inset.top));
            g.DrawString(id==136?L"\uE921":L"\uE8BB",1,&font,box,&format,&brush);
        }else PanelText(g,title,inset,13,primary&&active?(panelContrast?GetSysColor(COLOR_HIGHLIGHTTEXT):RGB(255,255,255)):text,true,primary);
    }
    canvas.Present();EndPaint(child,&ps);
}
static LRESULT CALLBACK PanelControlProcedure(HWND h,UINT m,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR){
    if(m==WM_PAINT){
        int id=GetDlgCtrlID(h);
        if(id==100){
            PAINTSTRUCT ps;HDC target=BeginPaint(h,&ps);RECT r;GetClientRect(h,&r);
            GlassCanvas canvas(target,r.right,r.bottom);canvas.graphics->Clear(GlassColor(PanelSurface()));
            DefSubclassProc(h,WM_PRINTCLIENT,(WPARAM)canvas.NativeDC(),PRF_CLIENT|PRF_ERASEBKGND);
            canvas.MakeOpaque();canvas.Present();EndPaint(h,&ps);
        }else PaintPanelControl(h);
        return 0;
    }
    if(m==WM_ERASEBKGND)return 1;
    if(m==WM_MOUSEMOVE){TRACKMOUSEEVENT track={sizeof(track),TME_LEAVE,h,0};TrackMouseEvent(&track);InvalidateRect(h,NULL,FALSE);}
    if(m==WM_MOUSELEAVE||m==WM_SETFOCUS||m==WM_KILLFOCUS||m==WM_ENABLE)InvalidateRect(h,NULL,FALSE);
    LRESULT result=DefSubclassProc(h,m,w,l);
    if(h==sizeSlider&&m==TBM_SETPOS&&!panelSyncing)ApplySize((int)SendMessageW(h,TBM_GETPOS,0,0),true);
    if(m==WM_LBUTTONDOWN||m==WM_LBUTTONUP||m==WM_KEYDOWN||m==WM_KEYUP||m==BM_SETCHECK||m==BM_SETSTATE||m==CB_SETCURSEL||m==WM_SETTEXT)InvalidateRect(h,NULL,FALSE);
    return result;
}
static HWND PanelControl(const wchar_t* cls,const wchar_t* label,DWORD style,int id,int x,int y,int width,int height){
    HWND child=CreateWindowExW(0,cls,label,WS_CHILD|WS_VISIBLE|(wcscmp(cls,L"STATIC")?WS_TABSTOP:0)|style,P(x),P(y),P(width),P(height),panelWindow,(HMENU)(INT_PTR)id,GetModuleHandleW(NULL),NULL);
    SendMessageW(child,WM_SETFONT,(WPARAM)panelFont,TRUE);
    if(wcscmp(cls,L"BUTTON")==0||wcscmp(cls,TRACKBAR_CLASSW)==0)SetWindowTheme(child,L"",L"");
    SetWindowSubclass(child,PanelControlProcedure,1,0);
    return child;
}
static void PanelTip(int id,const wchar_t* cn,const wchar_t* en){
    if(!panelTooltip)return;panelTooltips[id]=PanelLabel(cn,en);
    TOOLINFOW info={};info.cbSize=sizeof(info);info.hwnd=panelWindow;info.uFlags=TTF_IDISHWND|TTF_SUBCLASS;
    info.uId=(UINT_PTR)GetDlgItem(panelWindow,id);info.lpszText=panelTooltips[id].data();
    SendMessageW(panelTooltip,TTM_DELTOOLW,0,(LPARAM)&info);SendMessageW(panelTooltip,TTM_ADDTOOLW,0,(LPARAM)&info);
}
static void LocalizePanel(){
    if(!panelWindow)return;panelSyncing=true;
    auto target=std::filesystem::path(gameExe).filename().wstring();
    SetWindowTextW(GetDlgItem(panelWindow,134),(std::wstring(Text(UiText::ChooseGame))+(target.empty()?L"":L" · "+target)).c_str());
    SendMessageW(shapeCombo,CB_RESETCONTENT,0,0);SendMessageW(colorCombo,CB_RESETCONTENT,0,0);
    for(UiText label:{UiText::Original,UiText::Arrow,UiText::Cross,UiText::Ring})SendMessageW(shapeCombo,CB_ADDSTRING,0,(LPARAM)Text(label));
    for(UiText label:{UiText::White,UiText::Cyan,UiText::Amber,UiText::Pink})SendMessageW(colorCombo,CB_ADDSTRING,0,(LPARAM)Text(label));
    SetWindowTextW(enableCheck,PanelLabel(L"启用光标调整",L"Enable cursor adjustment"));
    SetWindowTextW(lockCheck,Text(UiText::Lock));
    SetWindowTextW(shiftCheck,PanelLabel(L"快捷键加 Shift",L"Add Shift to hotkeys"));
    SetWindowTextW(sizeEdit, std::to_wstring(desiredSize).c_str());
    SetWindowTextW(sizeSlider,PanelLabel(L"光标尺寸，1 至 96 像素",L"Cursor size, 1 to 96 pixels"));
    SetWindowTextW(GetDlgItem(panelWindow,106),PanelLabel(L"切换光标状态预览",L"Cycle cursor state preview"));
    for(int slot=1;slot<=3;slot++){
        SetWindowTextW(GetDlgItem(panelWindow,110+slot),(std::wstring(PanelLabel(L"应用 ",L"Apply "))+std::to_wstring(slot)).c_str());
        SetWindowTextW(GetDlgItem(panelWindow,120+slot),(std::wstring(PanelLabel(L"保存到 ",L"Save to "))+std::to_wstring(slot)).c_str());
    }
    SetWindowTextW(GetDlgItem(panelWindow,130),Text(UiText::DefaultSize));SetWindowTextW(GetDlgItem(panelWindow,131),Text(UiText::Restore));SetWindowTextW(GetDlgItem(panelWindow,132),PanelLabel(L"收起到托盘",L"Hide to tray"));
    SetWindowTextW(GetDlgItem(panelWindow,133),PanelLabel(L"关闭窗口，继续在托盘运行",L"Close window and keep running in tray"));
    SetWindowTextW(GetDlgItem(panelWindow,136),PanelLabel(L"最小化到任务栏",L"Minimize to taskbar"));
    SetWindowTextW(GetDlgItem(panelWindow,138),PanelLabel(L"退出程序",L"Quit app"));
    PanelTip(136,L"最小化到任务栏，光标调整继续运行。",L"Minimize to the taskbar. Cursor adjustment keeps running.");
    PanelTip(133,L"关闭设置窗口；点击托盘图标可重新打开。",L"Close settings. Click the tray icon to reopen them.");
    PanelTip(137,L"关闭后保持暂停，调整尺寸、样式或应用预设不会自动开启。",L"Pause stays off while editing size, style or presets. Turn on explicitly to resume.");
    PanelTip(131,L"暂停光标调整，目标程序恢复使用自己的光标。",L"Pause adjustment and let the target use its own cursor.");
    PanelTip(132,L"收起设置，后台继续运行；点击托盘图标可重新打开。",L"Hide settings and keep running. Click the tray icon to reopen.");
    PanelTip(138,L"完全退出，恢复目标程序原光标。",L"Exit completely and restore the target's original cursor.");
    for(int slot=1;slot<=3;slot++){
        PanelTip(110+slot,L"应用这个预设的尺寸、样式和窗口锁定设置，不改变暂停状态。",L"Apply saved size, style and confinement without changing pause state.");
        PanelTip(120+slot,L"将当前尺寸、样式和窗口锁定设置保存到这个预设。",L"Save current size, style and confinement to this preset.");
    }
    panelSyncing=false;SyncPanel();
}
static void RefreshPanelMaterial(){
    if(!panelWindow)return;panelContrast=GlassHighContrast();panelGlass=GlassApplyBackdrop(panelWindow);
    if(panelBackground)DeleteObject(panelBackground);panelBackground=CreateSolidBrush(PanelSurface());
    RedrawWindow(panelWindow,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN);
}
static LRESULT CALLBACK PanelProcedure(HWND h,UINT m,WPARAM w,LPARAM l){
    if(m==WM_NCCALCSIZE&&w)return 0;
    if(m==WM_NCHITTEST){POINT point={GET_X_LPARAM(l),GET_Y_LPARAM(l)};ScreenToClient(h,&point);if(point.y<P(50)&&point.x<P(514))return HTCAPTION;}
    if(m==WM_ERASEBKGND)return 1;
    if(m==WM_PAINT){PaintPanel(h);return 0;}
    if(m==WM_ACTIVATE)RefreshPanelMaterial();
    if(m==WM_SETTINGCHANGE||m==WM_THEMECHANGED||m==WM_DWMCOMPOSITIONCHANGED){RefreshPanelMaterial();return 0;}
    if(m==WM_CTLCOLORSTATIC||m==WM_CTLCOLOREDIT||m==WM_CTLCOLORLISTBOX){HDC dc=(HDC)w;SetTextColor(dc,PanelFg());SetBkColor(dc,PanelSurface());return (LRESULT)panelBackground;}
    if(m==WM_HSCROLL&&(HWND)l==sizeSlider){ApplySize((int)SendMessageW(sizeSlider,TBM_GETPOS,0,0),true);return 0;}
    if(m==WM_MOUSEWHEEL){
        POINT point={GET_X_LPARAM(l),GET_Y_LPARAM(l)};ScreenToClient(h,&point);RECT sizeArea=PanelRect(30,142,388,264);
        if(PtInRect(&sizeArea,point)){ApplySize(desiredSize+(GET_WHEEL_DELTA_WPARAM(w)>0?1:-1),true);return 0;}
    }
    if(m==WM_COMMAND){
        if(panelSyncing)return 0;int id=LOWORD(w),notification=HIWORD(w);
        if(id==IDCANCEL){CloseSettings();return 0;}
        if(id==134&&notification==BN_CLICKED){ChooseGame();return 0;}
        if(id==135&&notification==BN_CLICKED){StartOrConnect();return 0;}
        if(id==136&&notification==BN_CLICKED){ShowWindow(h,SW_MINIMIZE);return 0;}
        if(id==137&&notification==BN_CLICKED){panelNotice.clear();SetAdjustmentEnabled(SendMessageW(enableCheck,BM_GETCHECK,0,0)==BST_CHECKED);return 0;}
        if(id==138&&notification==BN_CLICKED){PostMessageW(windowHandle,WM_CLOSE,0,0);return 0;}
        if(id==100&&notification==EN_CHANGE){
            wchar_t value[16]={};GetWindowTextW(sizeEdit,value,16);wchar_t* end=NULL;long size=wcstol(value,&end,10);
            if(*value&&end&&!*end&&ValidCursorSize((int)size))ApplySize((int)size,true);
        }else if(id==100&&notification==EN_KILLFOCUS){panelSyncing=true;SetWindowTextW(sizeEdit,std::to_wstring(desiredSize).c_str());panelSyncing=false;InvalidateRect(h,NULL,FALSE);}
        else if(id==100&&notification==EN_SETFOCUS)InvalidateRect(h,NULL,FALSE);
        else if(id==106&&notification==BN_CLICKED){previewResource=(previewResource+1)%9;InvalidateRect(GetDlgItem(h,106),NULL,FALSE);}
        else if((id==102||id==103)&&notification==CBN_SELCHANGE){
            int shape=(int)SendMessageW(shapeCombo,CB_GETCURSEL,0,0),color=(int)SendMessageW(colorCombo,CB_GETCURSEL,0,0);
            if(shape>=0&&shape<=3&&color>=0&&color<=3){desiredTheme=ThemeFromSelection(shape,color);SaveSize();PublishPreferences();SyncPanel();}
        }else if(id==104&&notification==BN_CLICKED){lockWindow=SendMessageW(lockCheck,BM_GETCHECK,0,0)==BST_CHECKED;SaveSize();PublishPreferences();SyncPanel();}
        else if(id==105&&notification==BN_CLICKED){bool shift=SendMessageW(shiftCheck,BM_GETCHECK,0,0)==BST_CHECKED;hotkeyModifiers=MOD_CONTROL|MOD_ALT|(shift?MOD_SHIFT:0);WritePrivateProfileStringW(L"Hotkeys",L"AddShift",shift?L"1":L"0",settingsPath.c_str());SyncPanel();}
        else if(id>=111&&id<=113&&notification==BN_CLICKED){panelNotice.clear();ApplyPreset(id-110);}
        else if(id>=121&&id<=123&&notification==BN_CLICKED){bool saved=WritePreset(settingsPath,L"Preset"+std::to_wstring(id-120),{desiredSize,desiredTheme,lockWindow});
            panelNotice=saved?std::wstring(PanelLabel(L"已保存到预设 ",L"Saved to preset "))+std::to_wstring(id-120):PanelLabel(L"预设保存失败，请检查设置文件夹是否可写。",L"Preset could not be saved. Check that the settings folder is writable.");SyncPanel();}
        else if(id==130&&notification==BN_CLICKED)ApplySize(32,true);
        else if(id==131&&notification==BN_CLICKED)RestoreCursor();
        else if((id==132||id==133)&&notification==BN_CLICKED)CloseSettings();
        return 0;
    }
    if(m==WM_CLOSE){CloseSettings();return 0;}
    if(m==WM_KEYDOWN&&w==VK_ESCAPE){CloseSettings();return 0;}
    if(m==WM_DESTROY){panelWindow=NULL;panelTooltip=NULL;if(panelCursor){DestroyCursor(panelCursor);panelCursor=NULL;}panelCursorSize=0;panelCursorTheme=-1;
        if(panelFont){DeleteObject(panelFont);panelFont=NULL;}if(panelNumberFont){DeleteObject(panelNumberFont);panelNumberFont=NULL;}
        if(panelBackground){DeleteObject(panelBackground);panelBackground=NULL;}return 0;}
    return DefWindowProcW(h,m,w,l);
}
static void OpenSettings(bool show=true){
    if(panelWindow){if(show){ShowWindow(panelWindow,IsIconic(panelWindow)?SW_RESTORE:SW_SHOW);SetForegroundWindow(panelWindow);}return;}
    static GlassRuntime runtime;if(!runtime.token){Log(L"Settings renderer could not initialize.");return;}
    WNDCLASSW cls={};cls.hInstance=GetModuleHandleW(NULL);cls.lpfnWndProc=PanelProcedure;cls.lpszClassName=L"StellarisCursorSettings";cls.hCursor=LoadCursorW(NULL,IDC_ARROW);cls.hIcon=applicationIcon;RegisterClassW(&cls);
    HDC screen=GetDC(NULL);panelDpi=(UINT)GetDeviceCaps(screen,LOGPIXELSX);ReleaseDC(NULL,screen);
    HWND foreground=GetForegroundWindow();HMONITOR monitor=MonitorFromWindow(foreground,MONITOR_DEFAULTTONEAREST);MONITORINFO display={};display.cbSize=sizeof(display);GetMonitorInfoW(monitor,&display);RECT bounds=display.rcWork;
    // Fit a small work area without clipping actions; cursor preview stays in
    // actual screen pixels, independently from the layout scale.
    UINT fit=(UINT)(std::max)(72L,(std::min)((bounds.bottom-bounds.top-32)*96/PanelHeight,(bounds.right-bounds.left-32)*96/PanelWidth));
    panelDpi=(std::min)(panelDpi,fit);
    panelWindow=CreateWindowExW(WS_EX_APPWINDOW,cls.lpszClassName,Text(UiText::WindowTitle),WS_POPUP|WS_BORDER|WS_SYSMENU|WS_MINIMIZEBOX|WS_CLIPCHILDREN,
        bounds.left+(bounds.right-bounds.left-P(PanelWidth))/2,bounds.top+(bounds.bottom-bounds.top-P(PanelHeight))/2,P(PanelWidth),P(PanelHeight),NULL,NULL,cls.hInstance,NULL);
    if(!panelWindow)return;
    SetWindowPos(panelWindow,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_FRAMECHANGED);
    RefreshPanelMaterial();
    panelFont=CreateFontW(-P(13),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,CLEARTYPE_QUALITY,DEFAULT_PITCH,L"Microsoft YaHei UI");
    panelNumberFont=CreateFontW(-P(30),0,0,0,FW_SEMIBOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,L"Segoe UI");
    PanelControl(L"BUTTON",L"",BS_PUSHBUTTON,134,28,53,392,32);
    PanelControl(L"BUTTON",L"",BS_PUSHBUTTON,135,432,53,180,32);
    enableCheck=PanelControl(L"BUTTON",L"",BS_AUTOCHECKBOX,137,40,108,348,28);
    sizeEdit=PanelControl(L"EDIT",L"",ES_NUMBER|ES_AUTOHSCROLL|ES_CENTER,100,42,146,84,40);SendMessageW(sizeEdit,EM_SETLIMITTEXT,2,0);SendMessageW(sizeEdit,WM_SETFONT,(WPARAM)panelNumberFont,TRUE);
    sizeSlider=PanelControl(TRACKBAR_CLASSW,L"",TBS_HORZ|TBS_NOTICKS|TBS_FIXEDLENGTH,101,30,198,358,40);SendMessageW(sizeSlider,TBM_SETTHUMBLENGTH,P(24),0);SendMessageW(sizeSlider,TBM_SETRANGE,TRUE,MAKELPARAM(1,96));SendMessageW(sizeSlider,TBM_SETPAGESIZE,0,8);
    PanelControl(L"BUTTON",L"",BS_PUSHBUTTON,106,408,106,202,170);
    shapeCombo=PanelControl(L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_VSCROLL,102,28,331,278,200);
    colorCombo=PanelControl(L"COMBOBOX",L"",CBS_DROPDOWNLIST|WS_VSCROLL,103,334,331,278,200);
    SendMessageW(shapeCombo,CB_SETITEMHEIGHT,(WPARAM)-1,P(30));SendMessageW(colorCombo,CB_SETITEMHEIGHT,(WPARAM)-1,P(30));
    lockCheck=PanelControl(L"BUTTON",L"",BS_AUTOCHECKBOX,104,28,387,584,36);
    for(int slot=1;slot<=3;slot++){int x=28+(slot-1)*202;PanelControl(L"BUTTON",L"",BS_PUSHBUTTON,110+slot,x,480,180,34);PanelControl(L"BUTTON",L"",BS_PUSHBUTTON,120+slot,x,522,180,28);}
    shiftCheck=PanelControl(L"BUTTON",L"",BS_AUTOCHECKBOX,105,28,609,584,36);
    PanelControl(L"STATIC",L"",SS_LEFT,139,28,656,584,25);
    PanelControl(L"STATIC",L"",SS_LEFT,140,28,683,584,29);
    PanelControl(L"BUTTON",L"",BS_PUSHBUTTON,130,28,720,136,32);PanelControl(L"BUTTON",L"",BS_PUSHBUTTON,131,178,720,136,32);
    PanelControl(L"BUTTON",L"",BS_PUSHBUTTON,132,328,720,136,32);PanelControl(L"BUTTON",L"",BS_PUSHBUTTON,138,478,720,136,32);
    PanelControl(L"BUTTON",L"",BS_PUSHBUTTON,136,524,20,40,32);PanelControl(L"BUTTON",L"",BS_PUSHBUTTON,133,572,20,40,32);
    panelTooltip=CreateWindowExW(WS_EX_TOPMOST,TOOLTIPS_CLASSW,NULL,WS_POPUP|TTS_ALWAYSTIP|TTS_NOPREFIX,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,panelWindow,NULL,cls.hInstance,NULL);
    SendMessageW(panelTooltip,TTM_SETMAXTIPWIDTH,0,P(360));
    panelNotice.clear();LocalizePanel();
    if(show){ShowWindow(panelWindow,SW_SHOW);UpdateWindow(panelWindow);SetForegroundWindow(panelWindow);SetFocus(sizeSlider);}
    Log(panelGlass?(glassAccentActive?L"Opened glass settings with native frosted accent backdrop.":L"Opened glass settings with native Desktop Acrylic."):L"Opened glass settings with opaque accessibility/compatibility fallback.");
    Log(L"Window alpha channel request HRESULT="+std::to_wstring((unsigned long)glassAlphaResult));
}
static void PanelTick(){
    if(!panelWindow)return;SyncPanelStatus();
    if(IsWindowVisible(panelWindow)&&!IsIconic(panelWindow)&&previewResource>=5)InvalidateRect(GetDlgItem(panelWindow,106),NULL,FALSE);
}
