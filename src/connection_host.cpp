// A private native x64 fixture for launch/connect integration tests. It has no
// visible window, accepts no UI input and exits on an application-owned event.
#include "shared.h"
int WINAPI wWinMain(HINSTANCE,HINSTANCE,wchar_t*,int){
    std::wstring name=L"Local\\CursorBridgeConnectionHost-"+std::to_wstring(GetCurrentProcessId());
    HANDLE stop=CreateEventW(NULL,TRUE,FALSE,name.c_str());if(!stop)return 2;
    SetCursor(NULL); // Retain a real main-module SetCursor import for the hook.
    DWORD result=WaitForSingleObject(stop,20000);CloseHandle(stop);
    return result==WAIT_OBJECT_0?0:1;
}
