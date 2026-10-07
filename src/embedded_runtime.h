#pragma once
#ifdef CURSORBRIDGE_STANDALONE
#include "runtime_manifest.h"
#include <cstring>

// RCDATA is read from this executable, never fetched from a server. Compare
// existing cache bytes against the embedded resource before loading the DLL.
inline bool RuntimeFileMatches(const std::filesystem::path& path,const BYTE* data,DWORD size){
    DWORD attributes=GetFileAttributesW(path.c_str());
    if(attributes==INVALID_FILE_ATTRIBUTES||(attributes&FILE_ATTRIBUTE_REPARSE_POINT))return false;
    std::ifstream file(path,std::ios::binary);if(!file)return false;
    file.seekg(0,std::ios::end);if(file.tellg()!=(std::streamoff)size)return false;file.seekg(0);
    std::vector<char> bytes(size);return file.read(bytes.data(),size)&&memcmp(bytes.data(),data,size)==0;
}
inline bool RuntimeDirectorySafe(const std::filesystem::path& path){
    DWORD attributes=GetFileAttributesW(path.c_str());return attributes!=INVALID_FILE_ATTRIBUTES&&(attributes&FILE_ATTRIBUTE_DIRECTORY)&&!(attributes&FILE_ATTRIBUTE_REPARSE_POINT);
}
inline std::wstring PrepareEmbeddedRuntime(){
    auto local=KnownDirectory(FOLDERID_LocalAppData);if(local.empty())return {};
    std::filesystem::path root=std::filesystem::path(local)/L"CursorBridge"/L"runtime"/RuntimeKey;
    std::error_code error;std::filesystem::create_directories(root,error);if(error)return {};
    // Refuse redirecting any application-owned cache directory through a link.
    for(auto path=root;path!=std::filesystem::path(local);path=path.parent_path())if(!RuntimeDirectorySafe(path))return {};
    HMODULE module=GetModuleHandleW(NULL);
    for(const auto& item:EmbeddedFiles){
        HRSRC resource=FindResourceW(module,MAKEINTRESOURCEW(item.id),RT_RCDATA);
        if(!resource||SizeofResource(module,resource)!=item.size)return {};
        auto data=(const BYTE*)LockResource(LoadResource(module,resource));if(!data)return {};
        auto path=root/item.path;std::filesystem::create_directories(path.parent_path(),error);if(error)return {};
        for(auto parent=path.parent_path();parent!=root;parent=parent.parent_path())if(!RuntimeDirectorySafe(parent))return {};
        if(RuntimeFileMatches(path,data,item.size))continue;
        DWORD attributes=GetFileAttributesW(path.c_str());if(attributes!=INVALID_FILE_ATTRIBUTES&&(attributes&FILE_ATTRIBUTE_REPARSE_POINT))return {};
        auto temporary=path;temporary+=L"."+std::to_wstring(GetCurrentProcessId())+L"."+std::to_wstring(GetTickCount64())+L".tmp";
        HANDLE output=CreateFileW(temporary.c_str(),GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);if(output==INVALID_HANDLE_VALUE)return {};
        DWORD written=0;bool complete=WriteFile(output,data,item.size,&written,NULL)&&written==item.size&&FlushFileBuffers(output);CloseHandle(output);
        if(!complete||!MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)){DeleteFileW(temporary.c_str());return {};}
        if(!RuntimeFileMatches(path,data,item.size))return {};
    }
    return root.wstring();
}
#endif
