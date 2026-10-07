#include "shared.h"
#include "preferences.h"
#include <fstream>
#include <sstream>
#include <filesystem>

static int failed=0,checks=0;
static std::ostringstream details;
static void Check(bool good,const char* label) {
    checks++;if(!good)failed++;details<<"    {\"name\":\""<<label<<"\",\"passed\":"<<(good?"true":"false")<<"},\n";
}
static bool SameDrawing(HCURSOR a,HCURSOR b,int frame,int size) {
    HDC dc=GetDC(NULL),ma=CreateCompatibleDC(dc),mb=CreateCompatibleDC(dc);
    BITMAPINFO info={};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=size;info.bmiHeader.biHeight=-size;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
    void* pa=NULL,*pb=NULL;HBITMAP ba=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pa,NULL,0),bb=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pb,NULL,0);
    if(!ma||!mb||!ba||!bb){if(ba)DeleteObject(ba);if(bb)DeleteObject(bb);if(ma)DeleteDC(ma);if(mb)DeleteDC(mb);ReleaseDC(NULL,dc);return false;}
    HGDIOBJ oa=SelectObject(ma,ba),ob=SelectObject(mb,bb);memset(pa,0x7f,size*size*4);memset(pb,0x7f,size*size*4);
    bool ok=DrawIconEx(ma,0,0,a,size,size,frame,NULL,DI_NORMAL)&&DrawIconEx(mb,0,0,b,size,size,frame,NULL,DI_NORMAL);
    GdiFlush();ok=ok&&memcmp(pa,pb,size*size*4)==0;
    SelectObject(ma,oa);SelectObject(mb,ob);DeleteObject(ba);DeleteObject(bb);DeleteDC(ma);DeleteDC(mb);ReleaseDC(NULL,dc);return ok;
}
static int VisibleWidth(){CURSORINFO info={};info.cbSize=sizeof(info);Signature s;return GetCursorInfo(&info)&&CursorSignature(info.hCursor,s)?s.w:0;}
static bool RefreshTest(Shared* s) {
    HCURSOR original=LoadCursorFromFileW(ResourcePath(0).c_str());Signature before;CursorSignature(original,before);
    HWND prior=GetForegroundWindow();POINT pointer;GetCursorPos(&pointer);
    WNDCLASSW cls={};cls.lpfnWndProc=DefWindowProcW;cls.hInstance=GetModuleHandleW(NULL);cls.hCursor=original;cls.lpszClassName=L"StellarisCursorRefreshTest";RegisterClassW(&cls);
    HWND window=CreateWindowExW(0,cls.lpszClassName,L"光标恢复测试（约 4 秒后自动关闭）",WS_OVERLAPPEDWINDOW,400,250,520,300,NULL,NULL,cls.hInstance,NULL);
    Check(window!=NULL,"refresh_test_window_created");
    ShowWindow(window,SW_SHOWNORMAL);
    MSG queue;PeekMessageW(&queue,NULL,0,0,PM_NOREMOVE);
    DWORD foregroundThread=GetWindowThreadProcessId(prior,NULL);
    bool linked=foregroundThread!=GetCurrentThreadId()&&AttachThreadInput(GetCurrentThreadId(),foregroundThread,TRUE);
    SetForegroundWindow(window);
    if(linked)AttachThreadInput(GetCurrentThreadId(),foregroundThread,FALSE);
    ULONGLONG until=GetTickCount64()+500;
    do{while(PeekMessageW(&queue,NULL,0,0,PM_REMOVE)){TranslateMessage(&queue);DispatchMessageW(&queue);}Sleep(10);}while(GetTickCount64()<until);
    bool focused=GetForegroundWindow()==window;
    Check(focused,"refresh_test_window_has_foreground");
    if(focused) {
        POINT center={220,120};ClientToScreen(window,&center);SetCursorPos(center.x,center.y);
        SetCursor(original);Sleep(500);Check(VisibleWidth()==32,"foreground_actual_32px");
        InterlockedExchange(&s->enabled,0);Sleep(600);Check(GetForegroundWindow()==window,"foreground_during_restore");Check(VisibleWidth()==before.w,"worker_restores_while_UI_thread_blocked");
        InterlockedExchange(&s->size,24);InterlockedExchange(&s->enabled,1);Sleep(600);Check(GetForegroundWindow()==window,"foreground_during_resize");Check(VisibleWidth()==24,"worker_applies_size_without_game_mouse_event");
        InterlockedExchange(&s->enabled,0);Sleep(600);Check(GetForegroundWindow()==window,"foreground_during_exit");Check(VisibleWidth()==before.w,"worker_restores_on_exit_request");
    }
    if(GetForegroundWindow()==window){SetCursorPos(pointer.x,pointer.y);if(prior)SetForegroundWindow(prior);}
    DestroyWindow(window);DestroyCursor(original);return focused;
}
static int GenericTest(Shared* s,const std::wstring& logs){
    struct Source {LPCWSTR name;int role;};
    for(auto source:{Source{IDC_ARROW,0},Source{IDC_HAND,3},Source{IDC_SIZEALL,4},Source{IDC_WAIT,5},Source{IDC_APPSTARTING,5},Source{IDC_NO,6},Source{IDC_CROSS,0}}){
        HCURSOR original=LoadCursorW(NULL,source.name);Signature before;Check(CursorSignature(original,before),"generic_source_load");
        for(int theme=0;theme<=12;theme++)for(int size=1;size<=96;size++){
            InterlockedExchange64(&s->heartbeat,(LONG64)GetTickCount64());
            InterlockedExchange(&s->theme,theme);InterlockedExchange(&s->size,size);InterlockedExchange(&s->enabled,1);
            SetCursor(original);HCURSOR actual=GetCursor();Signature after;
            Check(CursorSignature(actual,after)&&after.w==size&&after.h==size,"generic_requested_dimensions");
            Check(after.x<(DWORD)size&&after.y<(DWORD)size,"generic_hotspot_bounds");
            HCURSOR expected=theme?(HCURSOR)LoadImageW(NULL,ThemeResourcePath(Parent(ModulePath()),theme,source.role).c_str(),IMAGE_CURSOR,size,size,LR_LOADFROMFILE):
                (HCURSOR)CopyImage(original,IMAGE_CURSOR,size,size,0);
            Signature wanted;bool validExpected=expected&&CursorSignature(expected,wanted);
            if(!theme){wanted.x=(std::min)(wanted.x,(DWORD)size-1);wanted.y=(std::min)(wanted.y,(DWORD)size-1);}
            Check(validExpected&&after==wanted,"generic_original_or_theme_artwork");
            Check(expected&&SameDrawing(actual,expected,0,size),"generic_rendered_pixels");
            if(expected)DestroyCursor(expected);
        }
        InterlockedExchange(&s->enabled,0);SetCursor(original);Check(GetCursor()==original,"generic_pause_restores_original");
        Signature intact;Check(CursorSignature(original,intact)&&intact==before,"generic_system_source_unchanged");
    }
    HCURSOR arrow=LoadCursorW(NULL,IDC_ARROW);
    InterlockedExchange(&s->enabled,1);InterlockedExchange(&s->size,0);SetCursor(arrow);Check(GetCursor()==arrow,"generic_invalid_size_fails_open");
    InterlockedExchange(&s->size,24);InterlockedExchange64(&s->heartbeat,(LONG64)(GetTickCount64()-5000));SetCursor(arrow);Check(GetCursor()==arrow,"generic_expired_heartbeat_restores_original");
    SetCursor(NULL);std::string rows=details.str();if(rows.size()>=2)rows.erase(rows.size()-2,1);
    std::ofstream output(logs+L"\\selftest-generic.json");output<<"{\"passed\":"<<(failed==0?"true":"false")<<",\"checks\":"<<checks<<",\"failed\":"<<failed<<",\"cases\":[\n"<<rows<<"]}\n";
    return failed?1:0;
}
int wmain(int argc,wchar_t** argv) {
    bool refresh=false,generic=false;
    for(int i=1;i<argc;i++){if(wcscmp(argv[i],L"--game-dir")==0&&i+1<argc)gameDirectory=argv[++i];else if(wcscmp(argv[i],L"--refresh")==0)refresh=true;else if(wcscmp(argv[i],L"--generic")==0)generic=true;else return 2;}
    if(gameDirectory.empty()&&!generic)return 2;
    SetProcessDPIAware();DWORD pid=GetCurrentProcessId();std::wstring logs=Parent(Parent(ModulePath()))+L"\\logs";
    std::filesystem::create_directories(logs);
    HANDLE map=NULL;Shared* s=NULL;
    for(int n=0;n<150;n++){
        map=OpenFileMappingW(FILE_MAP_ALL_ACCESS,FALSE,MapName(pid).c_str());
        if(map){s=(Shared*)MapViewOfFile(map,FILE_MAP_ALL_ACCESS,0,0,sizeof(Shared));if(s&&s->hookSlots>0)break;if(s)UnmapViewOfFile(s);CloseHandle(map);map=NULL;s=NULL;}
        Sleep(100);
    }
    if(!s)return 2;
    Check(s->magic==CURSOR_MAGIC&&s->pid==pid,"remote_payload_initialized");
    Check(s->hookSlots>0,"SetCursor_IAT_hook_installed");
    if(generic){int result=GenericTest(s,logs);UnmapViewOfFile(s);CloseHandle(map);return result;}
    if(refresh){RefreshTest(s);std::string rows=details.str();if(rows.size()>=2)rows.erase(rows.size()-2,1);std::ofstream output(logs+L"\\selftest-refresh.json");output<<"{\"passed\":"<<(failed==0?"true":"false")<<",\"checks\":"<<checks<<",\"failed\":"<<failed<<",\"worker_refreshes\":"<<s->reserved<<",\"cases\":[\n"<<rows<<"]}\n";UnmapViewOfFile(s);CloseHandle(map);return failed?1:0;}
    for(int n=0;n<9;n++) {
        HCURSOR original=LoadCursorFromFileW(ResourcePath(n).c_str());Signature before;bool valid=original&&CursorSignature(original,before);
        Check(valid,"original_resource_load");if(!valid)continue;
        for(int size=CURSOR_MIN_SIZE;size<=CURSOR_MAX_SIZE;size++) {
            InterlockedExchange(&s->size,size);InterlockedExchange(&s->enabled,1);
            SetCursor(original);HCURSOR actual=GetCursor();Signature after;
            Check(CursorSignature(actual,after)&&after.w==size&&after.h==size,"replacement_size");
            Check(after.x<(DWORD)size&&after.y<(DWORD)size,"scaled_hotspot_stays_inside_cursor");
            HCURSOR expected=(HCURSOR)LoadImageW(NULL,ResourcePath(n).c_str(),IMAGE_CURSOR,size,size,LR_LOADFROMFILE);Signature desired;
            Check(expected&&CursorSignature(expected,desired)&&after==desired,"original_artwork_and_hotspot_preserved");
            Check(SameDrawing(actual,expected,0,size),"rendered_pixels_match_native_load");
            if(n>=5)Check(SameDrawing(actual,expected,1,size),"animated_second_frame_preserved");
            if(expected)DestroyCursor(expected);
            HCURSOR previous=SetCursor(original);Check(previous==original,"previous_cursor_handle_semantics_preserved");
        }
        for(int invalid:{0,-1,97,2147483647}){
            InterlockedExchange(&s->size,invalid);SetCursor(original);Signature rejected;
            Check(CursorSignature(GetCursor(),rejected)&&rejected==before,"out_of_range_size_fails_open");
        }
        InterlockedExchange(&s->enabled,0);SetCursor(original);Signature disabled;
        Check(CursorSignature(GetCursor(),disabled)&&disabled==before,"disable_restores_original_size");
        Signature intact;Check(CursorSignature(original,intact)&&intact==before,"original_handle_not_mutated");
        SetCursor(NULL);DestroyCursor(original);
    }
    // Exercise every native theme at every supported size, including animated
    // movement/busy cursors and size/theme cache eviction under repeated changes.
    HCURSOR themeOriginals[9]={};for(int resource=0;resource<9;resource++)themeOriginals[resource]=LoadCursorFromFileW(ResourcePath(resource).c_str());
    int canonicalResources[9]={};
    // Some installed-game roles are byte-identical (normal/selected/dragselect
    // in 4.5.1). A bitmap-only adapter must use the first matching role; it must
    // not invent a distinction that the engine's loaded pointer does not expose.
    for(int resource=0;resource<9;resource++){
        canonicalResources[resource]=resource;Signature input;CursorSignature(themeOriginals[resource],input);bool matched=false;
        for(int candidate=0;candidate<9&&!matched;candidate++)for(int size:{0,32}){
            HCURSOR probe=size?(HCURSOR)LoadImageW(NULL,ResourcePath(candidate).c_str(),IMAGE_CURSOR,size,size,LR_LOADFROMFILE):LoadCursorFromFileW(ResourcePath(candidate).c_str());
            Signature signature;if(probe&&CursorSignature(probe,signature)&&signature==input){canonicalResources[resource]=candidate;matched=true;}
            if(probe)DestroyCursor(probe);if(matched)break;
        }
    }
    for(int theme=1;theme<=12;theme++)for(int resource=0;resource<9;resource++){
        HCURSOR original=themeOriginals[resource];
        Check(original!=NULL,"theme_original_loaded");if(!original)continue;
        for(int size=1;size<=96;size++){
            InterlockedExchange(&s->theme,theme);InterlockedExchange(&s->size,size);InterlockedExchange(&s->enabled,1);
            SetCursor(original);HCURSOR actual=GetCursor();Signature after,desired;
            HCURSOR expected=(HCURSOR)LoadImageW(NULL,ThemeResourcePath(Parent(ModulePath()),theme,canonicalResources[resource]).c_str(),IMAGE_CURSOR,size,size,LR_LOADFROMFILE);
            std::string caseId="_theme"+std::to_string(theme)+"_resource"+std::to_string(resource)+"_size"+std::to_string(size);
            Check(expected&&CursorSignature(expected,desired)&&CursorSignature(actual,after)&&after==desired,("theme_size_hotspot_and_pixels"+caseId).c_str());
            Check(SameDrawing(actual,expected,0,size),("theme_render_matches_generated_art"+caseId).c_str());
            if(resource>=5)Check(SameDrawing(actual,expected,1,size),"theme_animation_second_frame");
            Check(SetCursor(original)==original,"theme_previous_handle_semantics");
            if(expected)DestroyCursor(expected);
        }
        InterlockedExchange(&s->enabled,0);SetCursor(original);Check(GetCursor()==original,"theme_disable_restores_original_handle");
        SetCursor(NULL);
    }
    for(HCURSOR original:themeOriginals)if(original)DestroyCursor(original);
    InterlockedExchange(&s->theme,13);InterlockedExchange(&s->enabled,1);
    HCURSOR invalidTheme=LoadCursorFromFileW(ResourcePath(0).c_str());SetCursor(invalidTheme);Check(GetCursor()==invalidTheme,"invalid_theme_fails_open");SetCursor(NULL);DestroyCursor(invalidTheme);
    InterlockedExchange(&s->theme,0);
    InterlockedExchange(&s->enabled,1);InterlockedExchange(&s->size,32);
    HCURSOR windows=LoadCursorW(NULL,IDC_ARROW);SetCursor(windows);Check(GetCursor()==windows,"unrecognized_Windows_cursor_untouched");
    HCURSOR original=LoadCursorFromFileW(ResourcePath(0).c_str());Signature before;CursorSignature(original,before);
    LONG64 saved=InterlockedExchange64(&s->heartbeat,0);SetCursor(original);Signature expired;
    Check(CursorSignature(GetCursor(),expired)&&expired==before,"heartbeat_expiry_fails_open");InterlockedExchange64(&s->heartbeat,saved);
    SetCursor(NULL);DestroyCursor(original);
    std::string rows=details.str();if(rows.size()>=2)rows.erase(rows.size()-2,1);
    std::ofstream output(logs+L"\\selftest-native.json");
    output<<"{\n  \"passed\":"<<(failed==0?"true":"false")<<",\n  \"checks\":"<<checks<<",\n  \"failed\":"<<failed<<",\n  \"hook_slots\":"<<s->hookSlots<<",\n  \"replaced\":"<<s->replaced<<",\n  \"unmatched\":"<<s->unmatched<<",\n  \"cases\":[\n"<<rows<<"  ]\n}\n";
    UnmapViewOfFile(s);CloseHandle(map);return failed?1:0;
}
