#include "language.h"
#include "game_profile.h"
#include <fstream>
#include <sstream>

int wmain(){
    int checks=0,failed=0;std::ostringstream rows;
    auto check=[&](bool pass,const char* name){checks++;if(!pass)failed++;rows<<"{\"name\":\""<<name<<"\",\"passed\":"<<(pass?"true":"false")<<"},\n";};
    for(const std::string& value:{"l_simp_chinese","l_trad_chinese","l_traditional_chinese","l_chinese","L_SIMP_CHINESE"})check(ClassifyLanguage(value)==UiLanguage::Chinese,"Chinese_variants_use_Chinese");
    for(const std::string& value:{"l_english","l_french","l_german","l_spanish","l_russian","l_polish","l_braz_por","l_japanese","l_korean","unknown","","l_simp_chinese_extra"})check(ClassifyLanguage(value)==UiLanguage::English,"every_other_language_uses_English");
    check(SettingsLanguage("\xef\xbb\xbf" "language=\"l_simp_chinese\"\r\ngraphics={}\r\n")=="l_simp_chinese","settings_UTF8_BOM_CRLF");
    check(SettingsLanguage("# language=\"l_simp_chinese\"\n language = \"l_german\" # comment\n")=="l_german","comments_and_spaces");
    for(const std::string& value:{"language=Chinese","language=\"l_simp_chinese\" arbitrary","language=\"l_simp_chinese","not_language=\"l_simp_chinese\""})check(SettingsLanguage(value).empty(),"malformed_or_wrong_key_ignored");
    check(PdxSettingsLanguage("\"Graphics\"={value=\"l_simp_chinese\"}\n\"System\"={\"other\"={value=\"foo\"}\"language\"={version=0 value=\"l_english\"}}")=="l_english","PDX_System_language_after_other_entries");
    check(PdxSettingsLanguage("\"Graphics\"={\"language\"={value=\"l_simp_chinese\"}}").empty(),"PDX_wrong_section_ignored");
    check(PdxSettingsLanguage("\"System\"={\"language\"={value=\"l_simp_chinese\"").empty(),"PDX_incomplete_write_ignored");
    for(int n=0;n<(int)UiText::Count;n++){
        check(wcslen(UiString(UiLanguage::Chinese,(UiText)n))>0,"Chinese_visible_string_present");
        check(wcslen(UiString(UiLanguage::English,(UiText)n))>0,"English_visible_string_present");
    }
    std::wstring profile=L"E:\\Game Profiles\\中文\\";
    check(UserDirectoryFromCommandLine(L"\"E:\\Game\\stellaris.exe\" \"-userdir=E:\\Game Profiles\\中文\"")==L"E:\\Game Profiles\\中文","quoted_equals_profile");
    check(UserDirectoryFromCommandLine(L"game.exe -userdir \"E:\\Game Profiles\\English\"")==L"E:\\Game Profiles\\English","separate_profile_argument");
    check(UserDirectoryFromCommandLine(L"game.exe -userdir=relative").empty(),"relative_profile_not_guessed");
    check(UserDirectoryFromCommandLine(L"game.exe -another-dir=E:\\Wrong").empty(),"other_argument_not_profile");
    check(ProcessUserDirectory(GetCurrentProcess())==L"E:\\Game Profiles\\中文","SDK_read_actual_process_command_line");
    std::wstring logs=Parent(Parent(ModulePath()))+L"\\logs",fixtures=logs+L"\\language_fixtures_v04";
    CreateDirectoryW(logs.c_str(),NULL);CreateDirectoryW(fixtures.c_str(),NULL);
    std::wstring flat=fixtures+L"\\settings.txt",pdx=fixtures+L"\\pdx_settings.txt";
    auto write=[](const std::wstring& path,const std::string& value){std::ofstream out(path,std::ios::binary|std::ios::trunc);out<<value;};
    write(flat,"language=\"l_simp_chinese\"\n");write(pdx,"\"System\"={\"language\"={value=\"l_english\"}}");
    LanguageMonitor monitor;monitor.SetSource(flat);check(monitor.language==UiLanguage::Chinese,"active_settings_wins_over_stale_launcher_value");
    write(flat,"language=\"l_english\"\n");check(monitor.Refresh()&&monitor.language==UiLanguage::English,"live_preference_change_to_English");
    write(flat,"language=\"l_german\"\n");check(!monitor.Refresh()&&monitor.token=="l_german"&&monitor.language==UiLanguage::English,"other_language_keeps_English");
    write(flat,"unrelated=1\n");write(pdx,"\"System\"={\"language\"={value=\"l_simp_chinese\"}}");
    check(monitor.Refresh()&&monitor.language==UiLanguage::Chinese,"launcher_fallback_only_when_active_language_missing");
    monitor.SetSource(fixtures+L"\\missing\\settings.txt");check(monitor.language==UiLanguage::English,"missing_profile_defaults_to_English");
    std::string data=rows.str();if(data.size()>=2)data.erase(data.size()-2,1);
    std::ofstream out(logs+L"\\selftest-language.json");out<<"{\"passed\":"<<(failed==0?"true":"false")<<",\"checks\":"<<checks<<",\"failed\":"<<failed<<",\"cases\":["<<data<<"]}\n";
    return failed?1:0;
}
