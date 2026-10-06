#pragma once
#include "shared.h"

struct CursorPreset { int size=32,theme=0; bool lock=false; };
inline bool ValidTheme(int theme){return theme>=0&&theme<=12;}
inline int ThemeFromSelection(int shape,int color){return shape==0?0:1+(shape-1)*4+color;}
inline int ThemeShape(int theme){return theme==0?0:1+(theme-1)/4;}
inline int ThemeColor(int theme){return theme==0?0:(theme-1)%4;}
inline UINT ReadHotkeyModifiers(const std::wstring& path){return MOD_CONTROL|MOD_ALT|(GetPrivateProfileIntW(L"Hotkeys",L"AddShift",0,path.c_str())==1?MOD_SHIFT:0);}
inline std::wstring ThemeResourcePath(const std::wstring& binaryDirectory,int theme,int resource){
    std::wstring original=ResourcePathAt(L"",resource);
    return binaryDirectory+L"\\assets\\themes\\"+std::to_wstring(theme)+original.substr(original.find_last_of(L"\\"));
}
inline CursorPreset ReadPreset(const std::wstring& path,const std::wstring& section,CursorPreset fallback={}){
    int size=GetPrivateProfileIntW(section.c_str(),L"Size",fallback.size,path.c_str());
    int theme=GetPrivateProfileIntW(section.c_str(),L"Theme",fallback.theme,path.c_str());
    int lock=GetPrivateProfileIntW(section.c_str(),L"Lock",fallback.lock?1:0,path.c_str());
    return {ValidCursorSize(size)?size:fallback.size,ValidTheme(theme)?theme:fallback.theme,lock==1};
}
inline void WritePreset(const std::wstring& path,const std::wstring& section,const CursorPreset& preset){
    WritePrivateProfileStringW(section.c_str(),L"Size",std::to_wstring(preset.size).c_str(),path.c_str());
    WritePrivateProfileStringW(section.c_str(),L"Theme",std::to_wstring(preset.theme).c_str(),path.c_str());
    WritePrivateProfileStringW(section.c_str(),L"Lock",preset.lock?L"1":L"0",path.c_str());
}
