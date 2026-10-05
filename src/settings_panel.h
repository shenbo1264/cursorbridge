#pragma once
#include <windowsx.h>

static HWND panelWindow=NULL;
static HCURSOR panelCursor=NULL;
static int panelCursorSize=0;
static bool panelDragging=false;
static UINT panelDpi=96;
static int P(int value){return MulDiv(value,(int)panelDpi,96);}
static RECT PanelRect(int left,int top,int right,int bottom){return {P(left),P(top),P(right),P(bottom)};}
static bool InPanelRect(int x,int y,const RECT& r){POINT p={x,y};return PtInRect(&r,p)!=FALSE;}
static void ApplySize(int size,bool persist){
    if(!ValidCursorSize(size))return;
    desiredSize=size;enabled=true;
    if(state){InterlockedExchange(&state->size,size);InterlockedExchange(&state->enabled,1);InterlockedExchange64(&state->heartbeat,GetTickCount64());}
    if(persist)SaveSize();
    if(panelWindow)InvalidateRect(panelWindow,NULL,FALSE);
}
static void RestoreCursor(){
    enabled=false;if(state)InterlockedExchange(&state->enabled,0);
    if(panelWindow)InvalidateRect(panelWindow,NULL,FALSE);
}
static void PanelPreview(){
    int size=enabled?desiredSize:48;
    if(panelCursor&&panelCursorSize==size)return;
    HCURSOR next=(HCURSOR)LoadImageW(NULL,ResourcePath(0).c_str(),IMAGE_CURSOR,size,size,LR_LOADFROMFILE);
    if(!next)return;
    if(GetCursor()==panelCursor)SetCursor(next);
    if(panelCursor)DestroyCursor(panelCursor);
    panelCursor=next;panelCursorSize=size;
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
    PanelFill(dc,client,RGB(16,30,49));PanelFill(dc,PanelRect(0,0,460,4),accent);
    PanelText(dc,Text(UiText::Title),PanelRect(24,14,405,50),20,fg);
    PanelText(dc,L"×",PanelRect(418,10,450,48),24,muted,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    PanelText(dc,Text(UiText::Hint),PanelRect(24,50,438,76),12,muted);
    PanelText(dc,enabled?std::to_wstring(desiredSize)+L" px":Text(UiText::Restored),PanelRect(24,84,310,122),26,accent);
    RECT preview=PanelRect(322,80,436,186);PanelFill(dc,preview,RGB(28,47,68));
    PanelPreview();if(panelCursor)DrawIconEx(dc,(preview.left+preview.right-panelCursorSize)/2,(preview.top+preview.bottom-panelCursorSize)/2,panelCursor,panelCursorSize,panelCursorSize,0,NULL,DI_NORMAL);
    PanelText(dc,Text(UiText::Preview),PanelRect(322,189,436,211),11,muted,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    const int left=P(28),right=P(292),y=P(147);
    int knob=left+MulDiv(desiredSize-CURSOR_MIN_SIZE,right-left,CURSOR_MAX_SIZE-CURSOR_MIN_SIZE);
    PanelFill(dc,{left,y-P(2),right,y+P(2)},RGB(64,88,115));
    PanelFill(dc,{left,y-P(2),knob,y+P(2)},accent);
    HBRUSH b=CreateSolidBrush(accent);HGDIOBJ previous=SelectObject(dc,b);Ellipse(dc,knob-P(8),y-P(8),knob+P(8),y+P(8));SelectObject(dc,previous);DeleteObject(b);
    PanelText(dc,std::to_wstring(CURSOR_MIN_SIZE)+L" px",PanelRect(24,166,100,191),12,muted);
    PanelText(dc,std::to_wstring(CURSOR_MAX_SIZE)+L" px",PanelRect(216,166,296,191),12,muted,DT_RIGHT|DT_SINGLELINE|DT_VCENTER);
    PanelFill(dc,PanelRect(24,221,141,255),RGB(38,69,99));PanelFill(dc,PanelRect(153,221,309,255),RGB(38,69,99));PanelFill(dc,PanelRect(321,221,436,255),RGB(38,69,99));
    PanelText(dc,Text(UiText::DefaultSize),PanelRect(24,221,141,255),13,fg,DT_CENTER|DT_SINGLELINE|DT_VCENTER);
    PanelText(dc,Text(UiText::Restore),PanelRect(153,221,309,255),13,fg,DT_CENTER|DT_SINGLELINE|DT_VCENTER);
    PanelText(dc,Text(UiText::Done),PanelRect(321,221,436,255),13,fg,DT_CENTER|DT_SINGLELINE|DT_VCENTER);
    PanelText(dc,Text(attached?UiText::Connected:UiText::Waiting),PanelRect(24,267,438,292),11,muted);
    BitBlt(target,0,0,client.right,client.bottom,dc,0,0,SRCCOPY);
    SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);EndPaint(h,&ps);
}
static LRESULT CALLBACK PanelProcedure(HWND h,UINT m,WPARAM w,LPARAM l){
    if(m==WM_MOUSEACTIVATE)return MA_NOACTIVATE;
    if(m==WM_ERASEBKGND)return 1;
    if(m==WM_PAINT){PaintPanel(h);return 0;}
    if(m==WM_SETCURSOR&&LOWORD(l)==HTCLIENT){PanelPreview();if(panelCursor)SetCursor(panelCursor);return TRUE;}
    if(m==WM_LBUTTONDOWN){
        int x=GET_X_LPARAM(l),y=GET_Y_LPARAM(l);
        if(InPanelRect(x,y,PanelRect(16,126,305,166))){panelDragging=true;SetCapture(h);ApplySize(CursorSizeFromPosition(x,P(28),P(292)),false);}
        else if(InPanelRect(x,y,PanelRect(24,221,141,255)))ApplySize(32,true);
        else if(InPanelRect(x,y,PanelRect(153,221,309,255)))RestoreCursor();
        else if(InPanelRect(x,y,PanelRect(321,221,436,255))||InPanelRect(x,y,PanelRect(418,10,450,48)))DestroyWindow(h);
        return 0;
    }
    if(m==WM_MOUSEMOVE&&panelDragging){ApplySize(CursorSizeFromPosition(GET_X_LPARAM(l),P(28),P(292)),false);PanelPreview();if(panelCursor)SetCursor(panelCursor);return 0;}
    if(m==WM_LBUTTONUP&&panelDragging){ApplySize(CursorSizeFromPosition(GET_X_LPARAM(l),P(28),P(292)),true);panelDragging=false;ReleaseCapture();return 0;}
    if(m==WM_CAPTURECHANGED&&panelDragging){panelDragging=false;SaveSize();return 0;}
    if(m==WM_CLOSE){DestroyWindow(h);return 0;}
    if(m==WM_DESTROY){if(panelDragging){panelDragging=false;SaveSize();}panelWindow=NULL;if(panelCursor){SetCursor(LoadCursorW(NULL,IDC_ARROW));DestroyCursor(panelCursor);panelCursor=NULL;panelCursorSize=0;}return 0;}
    return DefWindowProcW(h,m,w,l);
}
static void OpenSettings(){
    if(panelWindow){ShowWindow(panelWindow,SW_SHOWNOACTIVATE);return;}
    WNDCLASSW cls={};cls.hInstance=GetModuleHandleW(NULL);cls.lpfnWndProc=PanelProcedure;cls.lpszClassName=L"StellarisCursorSettings";RegisterClassW(&cls);
    HDC screen=GetDC(NULL);panelDpi=(UINT)GetDeviceCaps(screen,LOGPIXELSX);ReleaseDC(NULL,screen);
    RECT bounds;SystemParametersInfoW(SPI_GETWORKAREA,0,&bounds,0);
    HWND game=GetForegroundWindow();DWORD pid=0;GetWindowThreadProcessId(game,&pid);
    if(attachedPid&&pid==attachedPid){GetClientRect(game,&bounds);MapWindowPoints(game,NULL,(POINT*)&bounds,2);}
    panelWindow=CreateWindowExW(WS_EX_APPWINDOW|WS_EX_NOACTIVATE|WS_EX_TOPMOST,cls.lpszClassName,Text(UiText::WindowTitle),WS_POPUP|WS_BORDER,
        bounds.left+(bounds.right-bounds.left-P(460))/2,bounds.top+(bounds.bottom-bounds.top-P(306))/2,P(460),P(306),NULL,NULL,cls.hInstance,NULL);
    if(panelWindow){ShowWindow(panelWindow,SW_SHOWNOACTIVATE);UpdateWindow(panelWindow);Log(L"打开 1–96 像素实时滑块设置。");}
}
static void PanelTick(){
    if(!panelWindow||panelDragging)return;
    HWND foreground=GetForegroundWindow();DWORD pid=0;GetWindowThreadProcessId(foreground,&pid);
    if(attachedPid&&pid!=attachedPid&&foreground!=panelWindow)ShowWindow(panelWindow,SW_HIDE);
    else if(!IsWindowVisible(panelWindow))ShowWindow(panelWindow,SW_SHOWNOACTIVATE);
}
