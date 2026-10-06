#pragma once
#include "shared.h"

// ClipCursor is desktop-wide: only own an unrestricted clip, and do not undo a
// different restriction installed by the game or another program afterwards.
class WindowLock {
    RECT ownedRect={}; bool owned=false;
    static bool Equal(const RECT& a,const RECT& b){return EqualRect(&a,&b)!=FALSE;}
public:
    void Release(){
        if(owned){RECT current={};if(GetClipCursor(&current)&&Equal(current,ownedRect))ClipCursor(NULL);owned=false;}
    }
    bool Update(HWND window){
        if(!window){Release();return false;}
        RECT next={};if(!GetClientRect(window,&next)||next.right<=next.left||next.bottom<=next.top){Release();return false;}
        MapWindowPoints(window,NULL,(POINT*)&next,2);
        RECT current={};if(!GetClipCursor(&current)){Release();return false;}
        RECT desktop={GetSystemMetrics(SM_XVIRTUALSCREEN),GetSystemMetrics(SM_YVIRTUALSCREEN),0,0};
        desktop.right=desktop.left+GetSystemMetrics(SM_CXVIRTUALSCREEN);desktop.bottom=desktop.top+GetSystemMetrics(SM_CYVIRTUALSCREEN);
        if(owned&&!Equal(current,ownedRect))owned=false;
        if(!owned&&!Equal(current,desktop))return Equal(current,next); // retain someone else's clip
        if(ClipCursor(&next)){ownedRect=next;owned=true;return true;}
        Release();return false;
    }
};
