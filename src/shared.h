#pragma once
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <string>
#include <vector>
#include <stdint.h>

inline std::wstring gameDirectory;
static const DWORD CURSOR_MAGIC=0x53435552;
static constexpr DWORD CURSOR_ABI_VERSION=2;
static constexpr int CURSOR_MIN_SIZE=1, CURSOR_MAX_SIZE=96;
static constexpr int CURSOR_SIZE_COUNT=CURSOR_MAX_SIZE-CURSOR_MIN_SIZE+1;
inline bool ValidCursorSize(int size){return size>=CURSOR_MIN_SIZE&&size<=CURSOR_MAX_SIZE;}
inline int CursorSizeFromPosition(int x,int left,int right){
    if(right<=left)return 32;
    if(x<=left)return CURSOR_MIN_SIZE;if(x>=right)return CURSOR_MAX_SIZE;
    return CURSOR_MIN_SIZE+MulDiv(x-left,CURSOR_MAX_SIZE-CURSOR_MIN_SIZE,right-left);
}
struct Shared {
    DWORD magic,version,pid,reserved;
    volatile LONG enabled,size,hookSlots,mappings;
    volatile LONG64 heartbeat;
    volatile LONG calls,replaced,unmatched,lastInput,lastOutput,lastResource;
    volatile LONG theme,lockWindow,lockActive;
};
struct InitArgs { DWORD size; DWORD test; wchar_t targetExe[32768]; wchar_t resourceDirectory[32768]; };
inline std::wstring MapName(DWORD pid) { return L"Local\\StellarisCursor-"+std::to_wstring(pid); }
inline std::wstring ModulePath(HMODULE m=NULL) { wchar_t p[32768]={};GetModuleFileNameW(m,p,32768);return p; }
inline std::wstring Parent(const std::wstring& p) { return p.substr(0,p.find_last_of(L"\\/")); }
inline bool SamePath(const std::wstring& a,const std::wstring& b) { return _wcsicmp(a.c_str(),b.c_str())==0; }
inline std::wstring ResourcePathAt(const std::wstring& directory,int i) {
    static const wchar_t* names[]={L"normal.cur",L"selected.cur",L"dragselect.cur",L"grab.cur",L"grabbing.cur",L"busy.ani",L"no_move.ani",L"friendly_move.ani",L"attack_move.ani"};
    return directory+L"\\gfx\\cursors\\"+names[i];
}
inline std::wstring ResourcePath(int i) { return ResourcePathAt(gameDirectory,i); }
struct Signature {
    int w=0,h=0;DWORD x=0,y=0;uint64_t hash=14695981039346656037ULL;
    bool operator==(const Signature& s)const {return w==s.w&&h==s.h&&x==s.x&&y==s.y&&hash==s.hash;}
};
inline void HashBytes(Signature& s,const void* p,size_t n){auto b=(const unsigned char*)p;while(n--){s.hash^=*b++;s.hash*=1099511628211ULL;}}
inline bool CursorSignature(HCURSOR c,Signature& s) {
    if(!c)return false;ICONINFO ii={};if(!GetIconInfo(c,&ii))return false;
    bool ok=true;BITMAP color={},mask={};
    if(ii.hbmColor){ok=GetObjectW(ii.hbmColor,sizeof(color),&color)!=0;s.w=color.bmWidth;s.h=color.bmHeight;}
    else {ok=GetObjectW(ii.hbmMask,sizeof(mask),&mask)!=0;s.w=mask.bmWidth;s.h=mask.bmHeight/2;}
    s.x=ii.xHotspot;s.y=ii.yHotspot;
    for(HBITMAP b:{ii.hbmColor,ii.hbmMask})if(b){
        BITMAP m={};if(!GetObjectW(b,sizeof(m),&m)||m.bmWidth<=0||m.bmHeight<=0||m.bmWidth>512||m.bmHeight>1024){ok=false;continue;}
        size_t n=(size_t)m.bmWidthBytes*m.bmHeight;std::vector<unsigned char> pixels(n);
        if(GetBitmapBits(b,(LONG)n,pixels.data())!=(LONG)n){ok=false;continue;}
        HashBytes(s,&m.bmBitsPixel,sizeof(m.bmBitsPixel));HashBytes(s,pixels.data(),pixels.size());
    }
    if(ii.hbmColor)DeleteObject(ii.hbmColor);if(ii.hbmMask)DeleteObject(ii.hbmMask);
    return ok&&s.w>0&&s.h>0;
}
