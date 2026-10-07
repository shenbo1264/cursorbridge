#pragma once
#include "shared.h"
#include <shlobj.h>
#include <filesystem>
#include <fstream>
#include <regex>

inline std::wstring KnownDirectory(REFKNOWNFOLDERID id){
    PWSTR raw=nullptr;std::wstring result;
    if(SUCCEEDED(SHGetKnownFolderPath(id,0,NULL,&raw))&&raw)result=raw;
    if(raw)CoTaskMemFree(raw);return result;
}
inline std::wstring DefaultGameProfile(){auto dir=KnownDirectory(FOLDERID_Documents);return dir.empty()?L"":dir+L"\\Paradox Interactive\\Stellaris";}
inline std::wstring DefaultDataDirectory(){auto dir=KnownDirectory(FOLDERID_LocalAppData);return dir.empty()?L"":dir+L"\\CursorBridge";}
inline bool FileExists(const std::wstring& path){DWORD attr=GetFileAttributesW(path.c_str());return attr!=INVALID_FILE_ATTRIBUTES&&!(attr&FILE_ATTRIBUTE_DIRECTORY);}
inline std::wstring AbsolutePath(const std::wstring& path){
    if(path.empty())return {};std::error_code error;auto absolute=std::filesystem::absolute(std::filesystem::path(path),error);
    return error?std::wstring():absolute.lexically_normal().wstring();
}
// Validate a user-selected native executable without running it. Bound all
// reads; reject DLLs, malformed headers and architectures our hook cannot use.
inline bool ValidApplicationExecutable(const std::wstring& exe){
    if(exe.empty()||!std::filesystem::path(exe).is_absolute()||
       _wcsicmp(std::filesystem::path(exe).extension().c_str(),L".exe"))return false;
    std::ifstream in(exe,std::ios::binary);if(!in)return false;
    in.seekg(0,std::ios::end);auto length=in.tellg();in.seekg(0);
    IMAGE_DOS_HEADER dos={};if(!in.read((char*)&dos,sizeof(dos))||dos.e_magic!=IMAGE_DOS_SIGNATURE||dos.e_lfanew<(LONG)sizeof(dos)||dos.e_lfanew>1048576||length<dos.e_lfanew+(std::streamoff)sizeof(IMAGE_NT_HEADERS64))return false;
    in.seekg(dos.e_lfanew);IMAGE_NT_HEADERS64 nt={};
    return in.read((char*)&nt,sizeof(nt))&&nt.Signature==IMAGE_NT_SIGNATURE&&nt.FileHeader.Machine==IMAGE_FILE_MACHINE_AMD64&&
        (nt.FileHeader.Characteristics&IMAGE_FILE_EXECUTABLE_IMAGE)&&!(nt.FileHeader.Characteristics&IMAGE_FILE_DLL)&&nt.OptionalHeader.Magic==IMAGE_NT_OPTIONAL_HDR64_MAGIC;
}
inline bool ValidGameExecutable(const std::wstring& exe){
    if(exe.empty()||!std::filesystem::path(exe).is_absolute()||_wcsicmp(std::filesystem::path(exe).filename().c_str(),L"stellaris.exe")||!FileExists(exe))return false;
    for(int i=0;i<9;i++)if(!FileExists(ResourcePathAt(Parent(exe),i)))return false;
    return true;
}
inline std::wstring Utf8Path(const std::string& value){
    int length=MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,value.data(),(int)value.size(),NULL,0);if(length<=0)return {};
    std::wstring result(length,L'\0');MultiByteToWideChar(CP_UTF8,MB_ERR_INVALID_CHARS,value.data(),(int)value.size(),&result[0],length);return result;
}
inline std::vector<std::wstring> SteamLibraries(const std::string& text){
    std::vector<std::wstring> result;
    // Support modern "path" entries and legacy numeric library entries. Bound callers' reads.
    static const std::regex entry(R"vdf("(?:path|[0-9]+)"\s*"((?:\\.|[^"\\])*)")vdf");
    for(std::sregex_iterator it(text.begin(),text.end(),entry),end;it!=end;++it){
        std::string escaped=(*it)[1].str(),decoded;
        for(size_t i=0;i<escaped.size();i++){if(escaped[i]=='\\'&&i+1<escaped.size()&&(escaped[i+1]=='\\'||escaped[i+1]=='"'))i++;decoded+=escaped[i];}
        auto path=Utf8Path(decoded);if(!path.empty()&&std::filesystem::path(path).is_absolute())result.push_back(AbsolutePath(path));
    }return result;
}
inline std::wstring SteamRoot(){
    wchar_t value[32768]={};DWORD size=sizeof(value);
    if(RegGetValueW(HKEY_CURRENT_USER,L"Software\\Valve\\Steam",L"SteamPath",RRF_RT_REG_SZ,NULL,value,&size)==ERROR_SUCCESS)return AbsolutePath(value);
    size=sizeof(value);
    if(RegGetValueW(HKEY_LOCAL_MACHINE,L"SOFTWARE\\Valve\\Steam",L"InstallPath",RRF_RT_REG_SZ|RRF_SUBKEY_WOW6432KEY,NULL,value,&size)==ERROR_SUCCESS)return AbsolutePath(value);
    return {};
}
inline std::wstring DiscoverInstalledGame(){
    auto root=SteamRoot();if(root.empty())return {};std::vector<std::wstring> libraries={root};
    std::ifstream file(root+L"\\steamapps\\libraryfolders.vdf",std::ios::binary);
    if(file){file.seekg(0,std::ios::end);auto count=file.tellg();if(count>0&&count<1048576){file.seekg(0);std::string text((size_t)count,'\0');if(file.read(&text[0],count)){auto more=SteamLibraries(text);libraries.insert(libraries.end(),more.begin(),more.end());}}}
    for(auto& library:libraries){auto exe=library+L"\\steamapps\\common\\Stellaris\\stellaris.exe";if(ValidGameExecutable(exe))return exe;}
    return {};
}
