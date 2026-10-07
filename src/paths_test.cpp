#include "install_paths.h"
#include <iostream>

int wmain(){
    int checks=0,failed=0;auto check=[&](bool pass,const char* name){checks++;if(!pass){failed++;std::cerr<<name<<"\n";}};
    auto modern=SteamLibraries(R"vdf("libraryfolders" { "0" { "path" "C:\\Program Files (x86)\\Steam" "apps" { "281990" "1234" } } "1" { "path" "D:\\Games 中文\\Steam" } })vdf");
    check(modern.size()==2,"modern_libraries_ignore_app_entries");
    if(modern.size()==2){check(modern[0]==L"C:\\Program Files (x86)\\Steam","escaped_backslashes_and_spaces");check(modern[1]==L"D:\\Games 中文\\Steam","UTF8_library_name");}
    auto legacy=SteamLibraries(R"vdf("LibraryFolders" { "1" "F:\\SteamLibrary" "2" "relative" "path" "" })vdf");
    check(legacy.size()==1&&legacy[0]==L"F:\\SteamLibrary","legacy_numeric_entries_and_relative_rejection");
    check(SteamLibraries("\"path\" \"D:\\Broken").empty(),"incomplete_VDF_ignored");
    check(Utf8Path(std::string(1,'\xff')).empty(),"invalid_UTF8_rejected");
    check(!DefaultGameProfile().empty()&&!DefaultDataDirectory().empty(),"Windows_known_folders_available");
    check(!ValidGameExecutable(L"relative\\stellaris.exe"),"relative_executable_rejected");
    check(ValidApplicationExecutable(ModulePath()),"own_native_x64_executable_accepted");
    check(!ValidApplicationExecutable(L"relative.exe"),"generic_relative_path_rejected");
    check(ResourcePathAt(L"C:\\Game",8)==L"C:\\Game\\gfx\\cursors\\attack_move.ani","game_adapter_cursor_path");
    wchar_t temp[32768]={};GetTempPathW(32768,temp);
    std::wstring fixture=std::wstring(temp)+L"CursorBridge-path-test-"+std::to_wstring(GetCurrentProcessId())+L" 中文";
    std::filesystem::create_directories(fixture+L"\\gfx\\cursors");
    auto write=[](const std::wstring& p){std::ofstream f(p,std::ios::binary);f<<"fixture, not executable";};
    write(fixture+L"\\stellaris.exe");
    check(!ValidGameExecutable(fixture+L"\\stellaris.exe"),"partial_installation_rejected");
    for(int i=0;i<9;i++)write(ResourcePathAt(fixture,i));
    check(ValidGameExecutable(fixture+L"\\stellaris.exe"),"complete_fixture_with_spaces_and_Unicode");
    write(fixture+L"\\other.exe");check(!ValidGameExecutable(fixture+L"\\other.exe"),"wrong_executable_name_rejected");
    check(!ValidApplicationExecutable(fixture+L"\\other.exe"),"generic_non_PE_executable_rejected");
    std::filesystem::remove(ResourcePathAt(fixture,8));
    check(!ValidGameExecutable(fixture+L"\\stellaris.exe"),"missing_animated_cursor_rejected");
    // Delete only the unique fixture we just created under GetTempPath.
    std::filesystem::remove_all(fixture);
    std::cout<<"Paths: "<<checks<<" checks, "<<failed<<" failures\n";return failed?1:0;
}
