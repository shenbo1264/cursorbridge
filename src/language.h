#pragma once
#include "shared.h"
#include <fstream>
#include <regex>
#include <sstream>

enum class UiLanguage { English, Chinese };
inline UiLanguage ClassifyLanguage(std::string token){
    for(char& c:token)if(c>='A'&&c<='Z')c=(char)(c-'A'+'a');
    return token=="l_simp_chinese"||token=="l_trad_chinese"||token=="l_traditional_chinese"||token=="l_chinese"?UiLanguage::Chinese:UiLanguage::English;
}
inline std::string SettingsLanguage(const std::string& text){
    std::istringstream lines(text);std::string line;
    static const std::regex pattern(R"lang(^\s*language\s*=\s*"([A-Za-z0-9_]+)"\s*(?:#.*)?$)lang");
    while(std::getline(lines,line)){
        if(line.compare(0,3,"\xef\xbb\xbf")==0)line.erase(0,3);
        if(!line.empty()&&line.back()=='\r')line.pop_back();
        std::smatch match;if(std::regex_match(line,match,pattern))return match[1].str();
    }
    return {};
}
inline std::string PdxSettingsLanguage(const std::string& text){
    // Only the System/language entry is a language setting; never a graphics value.
    static const std::regex system(R"lang("System"\s*=\s*\{)lang");
    static const std::regex language(R"lang("language"\s*=\s*\{[^{}]*?value\s*=\s*"([A-Za-z0-9_]+)")lang");
    std::smatch match;if(!std::regex_search(text,match,system))return {};
    size_t first=(size_t)(match.position()+match.length()),last=first;int depth=1;bool quoted=false,escaped=false;
    for(;last<text.size();last++){
        char c=text[last];if(escaped){escaped=false;continue;}if(quoted&&c=='\\'){escaped=true;continue;}
        if(c=='"'){quoted=!quoted;continue;}if(quoted)continue;
        if(c=='{')depth++;else if(c=='}'&&--depth==0)break;
    }
    if(depth!=0)return {};
    std::string body=text.substr(first,last-first);return std::regex_search(body,match,language)?match[1].str():std::string();
}
inline std::string ReadSettingsText(const std::wstring& path){
    std::ifstream in(path,std::ios::binary);if(!in)return {};
    in.seekg(0,std::ios::end);auto length=in.tellg();if(length<=0||length>262144)return {};
    in.seekg(0);std::string text((size_t)length,'\0');return in.read(&text[0],length)?text:std::string();
}
class LanguageMonitor {
    std::wstring settings;
public:
    UiLanguage language=UiLanguage::English;
    std::string token;
    void SetSource(const std::wstring& path){settings=path;Refresh();}
    const std::wstring& Source()const{return settings;}
    bool Refresh(){
        std::string next=SettingsLanguage(ReadSettingsText(settings));
        if(next.empty())next=PdxSettingsLanguage(ReadSettingsText(Parent(settings)+L"\\pdx_settings.txt"));
        UiLanguage current=ClassifyLanguage(next);bool changed=current!=language;language=current;token=next;return changed;
    }
};

enum class UiText { WindowTitle, Title, Hint, Restored, Preview, DefaultSize, Restore, Done, Connected, Waiting, Launch, Slider, Pause, Enable, Reconnect, Exit, TrayWaiting, TrayTitle, On, Off, Pixel, ChooseGame, Count };
inline const wchar_t* UiString(UiLanguage language,UiText text){
    static const wchar_t* chinese[]={L"群星光标大小设置",L"群星 · 光标大小",L"拖动即生效 · 每步 1 像素 · 桌面鼠标保持原设置",L"已恢复原光标",L"实际像素预览",L"推荐 32 px",L"恢复游戏原光标",L"完成",L"已连接游戏 · 关闭面板后保留尺寸",L"等待游戏 · 可先预览并保存尺寸",L"启动《群星》",L"光标大小滑块（1–96 px）",L"暂停并恢复原光标",L"启用光标调整",L"重新尝试连接",L"退出工具并恢复原光标",L"群星光标：等待游戏",L"群星光标：",L"启用",L"暂停",L" 像素",L"选择《群星》安装位置…"};
    static const wchar_t* english[]={L"Stellaris Cursor Settings",L"Stellaris · Cursor Size",L"Live adjustment · 1 px steps · Desktop unchanged",L"Original restored",L"Actual pixel size",L"Default 32 px",L"Restore original",L"Done",L"Connected · Size is kept when closed",L"Waiting for Stellaris · Preview and save a size",L"Launch Stellaris",L"Cursor size slider (1–96 px)",L"Pause and restore original cursor",L"Enable cursor adjustment",L"Retry connection",L"Exit and restore original cursor",L"Stellaris cursor: waiting for game",L"Stellaris cursor: ",L"On",L"Paused",L" px",L"Select Stellaris installation…"};
    static_assert(sizeof(chinese)/sizeof(chinese[0])==(size_t)UiText::Count,"Chinese strings incomplete");
    static_assert(sizeof(english)/sizeof(english[0])==(size_t)UiText::Count,"English strings incomplete");
    return (language==UiLanguage::Chinese?chinese:english)[(size_t)text];
}
