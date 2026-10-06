#include "preferences.h"
#include <filesystem>
#include <iostream>
int wmain(){
    int checks=0,failed=0;auto check=[&](bool good){checks++;if(!good)failed++;};
    for(int theme=0;theme<=12;theme++){check(ValidTheme(theme));check(ThemeFromSelection(ThemeShape(theme),ThemeColor(theme))==theme);}
    check(!ValidTheme(-1));check(!ValidTheme(13));
    wchar_t temporary[MAX_PATH]={};GetTempPathW(MAX_PATH,temporary);
    std::wstring file=std::wstring(temporary)+L"CursorBridge-prefs-"+std::to_wstring(GetCurrentProcessId())+L".ini";
    check(!std::filesystem::exists(file));
    for(int slot=1;slot<=3;slot++){
        std::wstring section=L"Preset"+std::to_wstring(slot);CursorPreset expected={slot*24,slot*4,slot==2};
        WritePreset(file,section,expected);CursorPreset actual=ReadPreset(file,section);
        check(actual.size==expected.size&&actual.theme==expected.theme&&actual.lock==expected.lock);
    }
    WritePrivateProfileStringW(L"Cursor",L"Size",L"999",file.c_str());WritePrivateProfileStringW(L"Cursor",L"Theme",L"-1",file.c_str());WritePrivateProfileStringW(L"Cursor",L"Lock",L"2",file.c_str());
    CursorPreset repaired=ReadPreset(file,L"Cursor");check(repaired.size==32&&repaired.theme==0&&!repaired.lock);
    CursorPreset missing=ReadPreset(file,L"Missing",{48,12,true});check(missing.size==48&&missing.theme==12&&missing.lock);
    check(ReadHotkeyModifiers(file)==(MOD_CONTROL|MOD_ALT));
    WritePrivateProfileStringW(L"Hotkeys",L"AddShift",L"1",file.c_str());check(ReadHotkeyModifiers(file)==(MOD_CONTROL|MOD_ALT|MOD_SHIFT));
    WritePrivateProfileStringW(L"Hotkeys",L"AddShift",L"2",file.c_str());check(ReadHotkeyModifiers(file)==(MOD_CONTROL|MOD_ALT));
    check(DeleteFileW(file.c_str())!=FALSE);std::cout<<checks<<" checks, "<<failed<<" failed\n";return failed?1:0;
}
