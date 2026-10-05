#pragma once
#include "shared.h"
#include <winternl.h>
#include <shellapi.h>
#include <filesystem>

inline std::wstring UserDirectoryFromCommandLine(const std::wstring& command){
    int count=0;wchar_t** args=CommandLineToArgvW(command.c_str(),&count);if(!args)return {};
    std::wstring path;
    for(int i=1;i<count;i++){
        if(_wcsnicmp(args[i],L"-userdir=",9)==0)path=args[i]+9;
        else if(_wcsicmp(args[i],L"-userdir")==0&&i+1<count)path=args[++i];
    }
    LocalFree(args);
    if(path.empty()||!std::filesystem::path(path).is_absolute())return {};
    return std::filesystem::path(path).lexically_normal().wstring();
}
inline std::wstring ProcessUserDirectory(HANDLE process){
    // SDK-declared PEB/RTL_USER_PROCESS_PARAMETERS fields, bounded read-only access.
    using Query=NTSTATUS(NTAPI*)(HANDLE,PROCESSINFOCLASS,PVOID,ULONG,PULONG);
    auto query=(Query)GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"NtQueryInformationProcess");
    PROCESS_BASIC_INFORMATION basic={};ULONG returned=0;
    if(!query||query(process,ProcessBasicInformation,&basic,sizeof(basic),&returned)<0)return {};
    PRTL_USER_PROCESS_PARAMETERS params=nullptr;SIZE_T bytes=0;
    if(!ReadProcessMemory(process,(BYTE*)basic.PebBaseAddress+offsetof(PEB,ProcessParameters),&params,sizeof(params),&bytes)||!params)return {};
    UNICODE_STRING command={};
    if(!ReadProcessMemory(process,(BYTE*)params+offsetof(RTL_USER_PROCESS_PARAMETERS,CommandLine),&command,sizeof(command),&bytes)||!command.Buffer||!command.Length||command.Length%sizeof(wchar_t))return {};
    std::wstring text(command.Length/sizeof(wchar_t),L'\0');
    if(!ReadProcessMemory(process,command.Buffer,&text[0],command.Length,&bytes)||bytes!=command.Length)return {};
    return UserDirectoryFromCommandLine(text);
}
