#pragma once
#include "shared.h"
#include <functional>

enum class BridgeKind { None, Size, Restore, Settings };
struct BridgeCommand { BridgeKind kind=BridgeKind::None; int size=0; };

inline BridgeCommand ParseBridgeLine(std::string line) {
    if(!line.empty()&&line.back()=='\r')line.pop_back();
    // Stellaris suppresses repeated script log text on the same game date.
    // Its event-selection record is unique per displayed player event and is not suppressed.
    bool sliderEvent=line.find("Event scursor.101 added info about event selection. selectedOption ")!=std::string::npos;
    const std::string selection=sliderEvent?"Event scursor.101 added info about event selection. selectedOption ":"Event scursor.100 added info about event selection. selectedOption ";
    size_t event=line.find(selection);
    if(event!=std::string::npos) {
        std::string prefix=line.substr(0,event);
        if(prefix.empty()||prefix[0]!='['||prefix.find("[eventcommands.cpp:")==std::string::npos||prefix.size()<3||prefix.substr(prefix.size()-3)!="]: ")return {};
        std::string value=line.substr(event+selection.size());
        const std::string human=", human 1, playerEventId ";
        if(value.size()<1+human.size()+1||value[0]<'0'||value[0]>'4'||value.substr(1,human.size())!=human)return {};
        std::string id=value.substr(1+human.size());
        if(id.size()>10||id.find_first_not_of("0123456789")!=std::string::npos)return {};
        if(sliderEvent){if(value[0]=='0')return {BridgeKind::Settings,0};return {};}
        int choice=value[0]-'0';if(choice==4)return {BridgeKind::Restore,0};
        static const int sizes[]={24,32,40,48};return {BridgeKind::Size,sizes[choice]};
    }
    const std::string marker="SCURSOR_V1_";
    size_t start=line.find(marker);
    // Require a logger prefix and one complete terminal token, never a substring command.
    if(start==std::string::npos||start==0||line[0]!='['||line.substr(0,start).find("]: ")==std::string::npos)return {};
    if(line.find(marker,start+1)!=std::string::npos)return {};
    std::string token=line.substr(start);
    for(int size:{24,32,40,48})if(token=="SCURSOR_V1_SIZE_"+std::to_string(size))return {BridgeKind::Size,size};
    if(token=="SCURSOR_V1_RESTORE")return {BridgeKind::Restore,0};
    if(token=="SCURSOR_V1_OPEN_SETTINGS")return {BridgeKind::Settings,0};
    const std::string request="SCURSOR_V1_OPEN_SETTINGS_N_";
    if(token.compare(0,request.size(),request)==0){
        std::string number=token.substr(request.size());
        if(number.empty()||number.size()>40||number[0]<'1'||number[0]>'9')return {};
        // Numeric UI nonce only, including common localized thousands separators.
        for(size_t i=0;i<number.size();i++){
            char c=number[i];if((c>='0'&&c<='9')||c=='.'||c==','||c==' ')continue;
            if(number.compare(i,2,"\xc2\xa0")==0){i++;continue;}
            if(number.compare(i,3,"\xe2\x80\xaf")==0){i+=2;continue;}
            return {};
        }
        return {BridgeKind::Settings,0};
    }
    return {};
}

class BridgeTail {
    std::wstring path;
    ULONGLONG offset=0;
    BY_HANDLE_FILE_INFORMATION identity={};
    bool haveIdentity=false,discardLine=false;
    std::string pending;
    static bool SameFile(const BY_HANDLE_FILE_INFORMATION& a,const BY_HANDLE_FILE_INFORMATION& b) {
        return a.dwVolumeSerialNumber==b.dwVolumeSerialNumber&&a.nFileIndexHigh==b.nFileIndexHigh&&a.nFileIndexLow==b.nFileIndexLow;
    }
    HANDLE Open()const{return CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);}
public:
    void Reset(const std::wstring& file) {
        path=file;offset=0;haveIdentity=false;discardLine=false;pending.clear();
        HANDLE h=Open();if(h==INVALID_HANDLE_VALUE)return;
        LARGE_INTEGER size;
        if(GetFileInformationByHandle(h,&identity)&&GetFileSizeEx(h,&size)) {haveIdentity=true;offset=(ULONGLONG)size.QuadPart;}
        CloseHandle(h); // Ignore historical requests when connecting or reconnecting.
    }
    void Poll(const std::function<void(const BridgeCommand&)>& apply) {
        if(path.empty())return;
        HANDLE h=Open();if(h==INVALID_HANDLE_VALUE)return;
        BY_HANDLE_FILE_INFORMATION current={};LARGE_INTEGER size={};
        if(!GetFileInformationByHandle(h,&current)||!GetFileSizeEx(h,&size)){CloseHandle(h);return;}
        if(!haveIdentity||!SameFile(current,identity)||(ULONGLONG)size.QuadPart<offset){offset=0;pending.clear();discardLine=false;}
        identity=current;haveIdentity=true;
        LARGE_INTEGER position;position.QuadPart=(LONGLONG)offset;
        if(!SetFilePointerEx(h,position,NULL,FILE_BEGIN)){CloseHandle(h);return;}
        // Bound disk reads and line storage; large unrelated logs cannot block the UI timer.
        char buffer[65536];DWORD read=0;
        if(ReadFile(h,buffer,sizeof(buffer),&read,NULL)) {
            offset+=read;
            for(DWORD i=0;i<read;i++) {
                char c=buffer[i];
                if(c=='\n') {
                    if(!discardLine){BridgeCommand command=ParseBridgeLine(pending);if(command.kind!=BridgeKind::None)apply(command);}
                    pending.clear();discardLine=false;
                } else if(!discardLine) {
                    if(pending.size()>=4096){pending.clear();discardLine=true;}else pending.push_back(c);
                }
            }
        }
        CloseHandle(h);
    }
};
