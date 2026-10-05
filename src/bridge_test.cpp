#include "bridge.h"
#include <fstream>
#include <sstream>

int wmain() {
    int checks=0,failed=0;std::ostringstream rows;
    auto check=[&](bool good,const char* name){checks++;if(!good)failed++;rows<<"{\"name\":\""<<name<<"\",\"passed\":"<<(good?"true":"false")<<"},\n";};
    std::string prefix="[22:40:00][effect_impl.cpp:123]: ";
    for(int size:{24,32,40,48}) {
        BridgeCommand cmd=ParseBridgeLine(prefix+"SCURSOR_V1_SIZE_"+std::to_string(size)+"\r");
        check(cmd.kind==BridgeKind::Size&&cmd.size==size,"accepted_whitelisted_size");
    }
    check(ParseBridgeLine(prefix+"SCURSOR_V1_RESTORE").kind==BridgeKind::Restore,"accepted_restore");
    check(ParseBridgeLine(prefix+"SCURSOR_V1_OPEN_SETTINGS").kind==BridgeKind::Settings,"accepted_settings_panel_request");
    for(const std::string& number:{"1","2","1000","1,000","1.000","1 000"})check(ParseBridgeLine(prefix+"SCURSOR_V1_OPEN_SETTINGS_N_"+number).kind==BridgeKind::Settings,"accepted_numeric_native_settings_request");
    for(const std::string& number:{"","0","-1","1;run=anything","1 trailing","[variable]"})check(ParseBridgeLine(prefix+"SCURSOR_V1_OPEN_SETTINGS_N_"+number).kind==BridgeKind::None,"rejected_invalid_native_settings_request");
    const std::string sliderPrefix="[23:01:28][eventcommands.cpp:88]: Event scursor.101 added info about event selection. selectedOption ";
    check(ParseBridgeLine(sliderPrefix+"0, human 1, playerEventId 19").kind==BridgeKind::Settings,"mapped_slider_event_open_button");
    for(const std::string& suffix:{"1, human 1, playerEventId 19","2, human 1, playerEventId 19","0, human 0, playerEventId 19"})
        check(ParseBridgeLine(sliderPrefix+suffix).kind==BridgeKind::None,"ignored_other_slider_event_buttons");
    check(CursorSizeFromPosition(-100,28,292)==1&&CursorSizeFromPosition(1000,28,292)==96,"slider_clamps_at_bounds");
    check(CursorSizeFromPosition(10,10,10)==32,"slider_degenerate_track_is_safe");
    for(int size=CURSOR_MIN_SIZE;size<=CURSOR_MAX_SIZE;size++)check(CursorSizeFromPosition((size-CURSOR_MIN_SIZE)*10,0,(CURSOR_MAX_SIZE-CURSOR_MIN_SIZE)*10)==size,"slider_each_integer_size_reachable");
    for(const std::string& token:{"SCURSOR_V1_SIZE_0","SCURSOR_V1_SIZE_64","SCURSOR_V1_SIZE_32;run=anything","SCURSOR_V2_SIZE_32","SCURSOR_V1_SIZE_32 trailing","SCURSOR_V1_SIZE_32SCURSOR_V1_SIZE_24","SCURSOR_V1_EXEC_cmd","SCURSOR_V1_SIZE_32_INCOMPLETE"})
        check(ParseBridgeLine(prefix+token).kind==BridgeKind::None,"rejected_invalid_or_embedded_request");
    check(ParseBridgeLine("SCURSOR_V1_SIZE_32").kind==BridgeKind::None,"rejected_missing_logger_prefix");
    check(ParseBridgeLine(prefix+"unrelated gameplay log").kind==BridgeKind::None,"ignored_other_mod_log");
    const std::string eventPrefix="[23:01:28][eventcommands.cpp:88]: Event scursor.100 added info about event selection. selectedOption ";
    int eventSizes[]={24,32,40,48};
    for(int option=0;option<5;option++) {
        BridgeCommand cmd=ParseBridgeLine(eventPrefix+std::to_string(option)+", human 1, playerEventId 18");
        check(option==4?cmd.kind==BridgeKind::Restore:cmd.kind==BridgeKind::Size&&cmd.size==eventSizes[option],"mapped_actual_game_event_button");
    }
    for(const std::string& suffix:{"4, human 0, playerEventId 18","5, human 1, playerEventId 18","40, human 1, playerEventId 18","1, human 1, playerEventId 18 trailing","1, human 1, playerEventId ","1, human 1, playerEventId -1"})
        check(ParseBridgeLine(eventPrefix+suffix).kind==BridgeKind::None,"rejected_nonplayer_close_or_invalid_event_record");
    check(ParseBridgeLine("[23:00:00][other_mod.cpp:88]: Event scursor.100 added info about event selection. selectedOption 1, human 1, playerEventId 18").kind==BridgeKind::None,"rejected_wrong_event_record_source");
    check(ParseBridgeLine("[23:00:00][eventcommands.cpp:88]: Event some_other_mod.100 added info about event selection. selectedOption 1, human 1, playerEventId 18").kind==BridgeKind::None,"ignored_unrelated_event_choice");
    std::wstring logs=Parent(Parent(ModulePath()))+L"\\logs";CreateDirectoryW(logs.c_str(),NULL);
    std::wstring file=logs+L"\\bridge_tail_selftest.log";
    auto write=[&](const std::string& value,bool append){std::ofstream out(file,append?std::ios::binary|std::ios::app:std::ios::binary|std::ios::trunc);out<<value;};
    BridgeTail tail;std::vector<BridgeCommand> got;
    auto apply=[&](const BridgeCommand& c){got.push_back(c);};
    write(prefix+"SCURSOR_V1_SIZE_48\n",false);tail.Reset(file);tail.Poll(apply);check(got.empty(),"ignored_history_at_connect");
    write(prefix+"SCURSOR_V1_SIZE_24",true);tail.Poll(apply);check(got.empty(),"waited_for_complete_line");
    write("\r\n",true);tail.Poll(apply);check(got.size()==1&&got.back().size==24,"read_split_line_once");
    tail.Poll(apply);check(got.size()==1,"no_duplicate_poll_delivery");
    write(prefix+"SCURSOR_V1_SIZE_40\n"+prefix+"SCURSOR_V1_RESTORE\n",true);tail.Poll(apply);
    check(got.size()==3&&got[1].size==40&&got[2].kind==BridgeKind::Restore,"preserved_request_order");
    write(prefix+"SCURSOR_V1_SIZE_32\n",false);tail.Poll(apply);check(got.size()==4&&got.back().size==32,"recovered_after_log_truncation");
    write(std::string(5000,'x')+prefix+"SCURSOR_V1_SIZE_48\n"+prefix+"SCURSOR_V1_SIZE_24\n",true);tail.Poll(apply);
    check(got.size()==5&&got.back().size==24,"discarded_oversized_line_and_recovered");
    tail.Reset(file);tail.Poll(apply);check(got.size()==5,"reconnect_does_not_replay_history");
    std::wstring moved=file+L".previous";
    // Replace only this self-test's log and preserve the old file for inspection.
    std::wstring unique=moved+std::to_wstring(GetTickCount64());
    bool renamed=MoveFileW(file.c_str(),unique.c_str())!=FALSE;
    write(prefix+"SCURSOR_V1_SIZE_40\n",false);tail.Poll(apply);
    check(renamed&&got.size()==6&&got.back().size==40,"recovered_after_file_rotation");
    write(eventPrefix+"4, human 1, playerEventId 20\n"+eventPrefix+"4, human 1, playerEventId 21\n",true);tail.Poll(apply);
    check(got.size()==8&&got[6].kind==BridgeKind::Restore&&got[7].kind==BridgeKind::Restore,"delivered_repeated_same_date_event_requests");
    std::string detail=rows.str();if(detail.size()>=2)detail.erase(detail.size()-2,1);
    std::ofstream output(logs+L"\\selftest-bridge.json");output<<"{\"passed\":"<<(failed==0?"true":"false")<<",\"checks\":"<<checks<<",\"failed\":"<<failed<<",\"cases\":["<<detail<<"]}\n";
    return failed?1:0;
}
