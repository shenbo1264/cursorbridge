#include "shared.h"
#include "install_paths.h"
#include "preferences.h"
#include "window_lock.h"
#include <algorithm>

static HMODULE selfModule;
static Shared* state;
static HANDLE mapHandle;
static bool isTest=false;
static bool bypassContext=false;
static bool genericAdapter=false;
static DWORD ownerPid;
static SRWLOCK lock=SRWLOCK_INIT;
static HCURSOR(WINAPI* realSetCursor)(HCURSOR)=::SetCursor;
struct Reference {int resource;Signature signature;};
struct SizedCursor {HCURSOR cursor=nullptr;int size=0,theme=0;ULONGLONG used=0;};
struct Replacement {HCURSOR original;int resource,originalWidth;Signature signature;SizedCursor sized[3];};
static std::vector<Reference> refs;
static std::vector<Replacement> cache;
static std::vector<void**> slots;
static HCURSOR lastOriginal;
static unsigned allocatedCursors=0;
static ULONGLONG cacheClock=0;
static void ReleaseSized(Replacement& owner,SizedCursor& sized){
    if(!sized.cursor)return;
    if(GetCursor()==sized.cursor)realSetCursor(owner.original);
    DestroyCursor(sized.cursor);sized={};allocatedCursors--;
}
static void TrimCache(){
    if(allocatedCursors<32)return;
    Replacement* owner=nullptr;SizedCursor* oldest=nullptr;
    for(auto& c:cache)for(auto& h:c.sized)if(h.cursor&&(!oldest||h.used<oldest->used)){oldest=&h;owner=&c;}
    if(owner&&oldest)ReleaseSized(*owner,*oldest);
}

static bool Active() {
    return state&&state->enabled&&GetTickCount64()-(ULONGLONG)state->heartbeat<4000;
}
static bool GamePointerContext() {
    if(bypassContext)return true;
    HWND window=GetForegroundWindow();DWORD pid=0;GetWindowThreadProcessId(window,&pid);if(pid!=ownerPid)return false;
    POINT p;if(!GetCursorPos(&p))return false;
    // Avoid cross-thread hit-testing while the game's UI thread is loading.
    RECT client;if(!GetClientRect(window,&client)||!ScreenToClient(window,&p))return false;
    return PtInRect(&client,p)!=FALSE;
}
static HCURSOR Resolve(HCURSOR input,bool active) {
    if(!input)return input;
    // Translate our own handles back before evaluating a new size or disable request.
    for(auto& c:cache)for(auto& h:c.sized)if(h.cursor&&h.cursor==input)input=c.original;
    lastOriginal=input;
    if(!active)return input;
    Replacement* found=nullptr;
    for(auto& c:cache)if(c.original==input){found=&c;break;}
    // Windows may recycle a destroyed cursor's numeric handle. Revalidate its
    // artwork instead of silently applying the previous cursor's semantic role.
    if(found){
        Signature current;
        if(!CursorSignature(input,current)||!(current==found->signature)){
            for(auto& h:found->sized)ReleaseSized(*found,h);
            size_t index=(size_t)(found-cache.data());cache.erase(cache.begin()+index);found=nullptr;
        }
    }
    if(!found){
        Signature signature;if(!CursorSignature(input,signature))return input;
        int resource=-1;for(auto& r:refs)if(r.signature==signature){resource=r.resource;break;}
        if(resource<0&&genericAdapter)resource=0; // Unknown roles use the base theme; do not invent game semantics.
        if(resource<0){InterlockedIncrement(&state->unmatched);return input;}
        if(cache.size()>=64)return input;
        cache.push_back({input,resource,signature.w,signature,{}});found=&cache.back();
    }
    int size=state->size,theme=state->theme;if(!ValidCursorSize(size)||!ValidTheme(theme))return input;
    SizedCursor* sized=nullptr;
    for(auto& h:found->sized)if(h.cursor&&h.size==size&&h.theme==theme){sized=&h;break;}
    if(!sized) {
        // ANI handles contain many frame handles: retain only three recent sizes per original.
        for(auto& h:found->sized)if(!h.cursor){sized=&h;break;}
        if(!sized){sized=&found->sized[0];for(auto& h:found->sized)if(h.used<sized->used)sized=&h;ReleaseSized(*found,*sized);}
        TrimCache();
        std::wstring asset=theme?ThemeResourcePath(Parent(ModulePath(selfModule)),theme,found->resource):ResourcePath(found->resource);
        HCURSOR h=genericAdapter&&!theme?ResizeNativeCursor(input,size):
            (HCURSOR)LoadImageW(NULL,asset.c_str(),IMAGE_CURSOR,size,size,LR_LOADFROMFILE);
        Signature s;if(!h||!CursorSignature(h,s)||s.w!=size||s.h!=size){if(h)DestroyCursor(h);return input;}
        *sized={h,size,theme,0};
        allocatedCursors++;
    }
    sized->used=++cacheClock;
    InterlockedExchange(&state->lastInput,found->originalWidth);InterlockedExchange(&state->lastOutput,size);
    InterlockedExchange(&state->lastResource,found->resource);
    InterlockedIncrement(&state->replaced);
    return sized->cursor;
}
static HCURSOR WINAPI SmallSetCursor(HCURSOR input) {
    if(state)InterlockedIncrement(&state->calls);
    bool active=Active()&&GamePointerContext();
    AcquireSRWLockExclusive(&lock);
    HCURSOR replacement=Resolve(input,active);
    HCURSOR previous=realSetCursor(replacement);
    // Preserve SetCursor's return semantics: game code sees its original handle.
    for(auto& c:cache)for(auto& h:c.sized)if(h.cursor&&h.cursor==previous)previous=c.original;
    ReleaseSRWLockExclusive(&lock);return previous;
}
static DWORD WINAPI RefreshWorker(void*) {
    MSG queue;PeekMessageW(&queue,NULL,0,0,PM_NOREMOVE);
    LONG lastEnabled=-1,lastSize=-1,lastTheme=-1;bool previousContext=false;WindowLock windowLock;
    for(;;) {
        Sleep(250);LONG enabled=Active()?1:0;LONG size=state->size,theme=state->theme;
        HWND lockTarget=GetForegroundWindow();DWORD lockPid=0;GetWindowThreadProcessId(lockTarget,&lockPid);
        bool lockRequested=enabled&&state->lockWindow&&lockPid==ownerPid&&!bypassContext;
        InterlockedExchange(&state->lockActive,windowLock.Update(lockRequested?lockTarget:NULL)?1:0);
        bool context=GamePointerContext();
        if(bypassContext)continue;
        if(!context){previousContext=false;continue;}
        if(enabled==lastEnabled&&size==lastSize&&theme==lastTheme&&previousContext)continue;
        HWND foreground=GetForegroundWindow();DWORD tid=GetWindowThreadProcessId(foreground,NULL);
        // Refresh on the game's input queue; never set the cursor over the desktop.
        if(AttachThreadInput(GetCurrentThreadId(),tid,TRUE)) {
            AcquireSRWLockExclusive(&lock);
            CURSORINFO current={};current.cbSize=sizeof(current);
            HCURSOR input=GetCursorInfo(&current)?current.hCursor:lastOriginal;
            if(input)realSetCursor(Resolve(input,enabled!=0));
            ReleaseSRWLockExclusive(&lock);
            AttachThreadInput(GetCurrentThreadId(),tid,FALSE);
            InterlockedIncrement((volatile LONG*)&state->reserved);
            lastEnabled=enabled;lastSize=size;lastTheme=theme;previousContext=true;
        }else {
            InterlockedExchange((volatile LONG*)&state->reserved,(LONG)(0x80000000U|GetLastError()));
        }
    }
}
static bool PatchMainImport() {
    BYTE* base=(BYTE*)GetModuleHandleW(NULL);auto dos=(IMAGE_DOS_HEADER*)base;
    if(dos->e_magic!=IMAGE_DOS_SIGNATURE)return false;
    auto nt=(IMAGE_NT_HEADERS64*)(base+dos->e_lfanew);
    if(nt->Signature!=IMAGE_NT_SIGNATURE||nt->OptionalHeader.Magic!=IMAGE_NT_OPTIONAL_HDR64_MAGIC)return false;
    DWORD rva=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
    if(!rva)return false;
    for(auto desc=(IMAGE_IMPORT_DESCRIPTOR*)(base+rva);desc->Name;desc++) {
        if(_stricmp((char*)(base+desc->Name),"user32.dll")||!desc->OriginalFirstThunk)continue;
        auto names=(IMAGE_THUNK_DATA64*)(base+desc->OriginalFirstThunk);
        auto imports=(IMAGE_THUNK_DATA64*)(base+desc->FirstThunk);
        for(;names->u1.AddressOfData;names++,imports++) {
            if(IMAGE_SNAP_BY_ORDINAL64(names->u1.Ordinal))continue;
            auto name=(IMAGE_IMPORT_BY_NAME*)(base+names->u1.AddressOfData);
            if(strcmp((char*)name->Name,"SetCursor"))continue;
            void** slot=(void**)&imports->u1.Function;DWORD old;
            if(!VirtualProtect(slot,sizeof(void*),PAGE_READWRITE,&old))continue;
            void* previous=InterlockedExchangePointer((void* volatile*)slot,(void*)&SmallSetCursor);
            realSetCursor=(HCURSOR(WINAPI*)(HCURSOR))previous;
            DWORD ignored;VirtualProtect(slot,sizeof(void*),old,&ignored);slots.push_back(slot);
        }
    }
    return !slots.empty();
}
extern "C" __declspec(dllexport) DWORD WINAPI Initialize(void* param) {
    if(state)return 1;
    if(!param||((InitArgs*)param)->size!=sizeof(InitArgs))return 0;
    InitArgs args=*(InitArgs*)param;
    if(args.test>2||args.generic>1||!wmemchr(args.targetExe,0,32768)||!wmemchr(args.resourceDirectory,0,32768))return 0;
    std::wstring path=ModulePath();std::wstring testPath=Parent(ModulePath(selfModule))+L"\\StellarisCursorTest.exe";
    isTest=args.test!=0&&SamePath(path,testPath);
    bypassContext=isTest&&args.test==1;
    genericAdapter=args.generic!=0;
    if(!SamePath(path,args.targetExe))return 0;
    gameDirectory=AbsolutePath(args.resourceDirectory);
    if(genericAdapter){if((args.test&&!isTest)||!ValidApplicationExecutable(path)||!SamePath(gameDirectory,Parent(path)))return 0;}
    else if(isTest){if(!ValidGameExecutable(gameDirectory+L"\\stellaris.exe"))return 0;}
    else if(args.test||!ValidGameExecutable(path)||!SamePath(gameDirectory,Parent(path)))return 0;
    ownerPid=GetCurrentProcessId();
    mapHandle=OpenFileMappingW(FILE_MAP_ALL_ACCESS,FALSE,MapName(ownerPid).c_str());
    if(!mapHandle)return 0;
    state=(Shared*)MapViewOfFile(mapHandle,FILE_MAP_ALL_ACCESS,0,0,sizeof(Shared));
    if(!state||state->magic!=CURSOR_MAGIC||state->version!=CURSOR_ABI_VERSION||state->pid!=ownerPid){state=nullptr;return 0;}
    // References are loaded through unpatched Win32 APIs, using untouched game assets.
    if(genericAdapter){
        struct SystemRole {LPCWSTR name;int role;};
        for(auto source:{SystemRole{IDC_ARROW,0},SystemRole{IDC_HAND,3},SystemRole{IDC_SIZEALL,4},SystemRole{IDC_WAIT,5},SystemRole{IDC_APPSTARTING,5},SystemRole{IDC_NO,6}}){
            HCURSOR h=LoadCursorW(NULL,source.name);Signature signature;
            if(h&&CursorSignature(h,signature))refs.push_back({source.role,signature});
        }
    }else for(int i=0;i<9;i++) {
        HCURSOR h=LoadCursorFromFileW(ResourcePath(i).c_str());Signature signature;
        if(h&&CursorSignature(h,signature))refs.push_back({i,signature});
        if(h)DestroyCursor(h);
        h=(HCURSOR)LoadImageW(NULL,ResourcePath(i).c_str(),IMAGE_CURSOR,32,32,LR_LOADFROMFILE);signature=Signature();
        if(h&&CursorSignature(h,signature))refs.push_back({i,signature});
        if(h)DestroyCursor(h);
    }
    state->mappings=(LONG)refs.size();
    if((!genericAdapter&&refs.empty())||!PatchMainImport()){state->enabled=0;return 0;}
    state->hookSlots=(LONG)slots.size();
    HANDLE worker=CreateThread(NULL,0,RefreshWorker,NULL,0,NULL);if(worker)CloseHandle(worker);
    return 1;
}
BOOL WINAPI DllMain(HINSTANCE h,DWORD why,void*) {
    if(why==DLL_PROCESS_ATTACH){selfModule=h;DisableThreadLibraryCalls(h);}return TRUE;
}
