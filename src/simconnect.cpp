#include "simconnect.h"
#include <cmath>
#include <cstring>
#ifdef STELLA_CHECK_SIMCONNECT_SDK
#include <SimConnect.h>
#include <cstddef>
static_assert(SIMCONNECT_DATATYPE_FLOAT64==4 && SIMCONNECT_DATATYPE_STRING256==9);
static_assert(SIMCONNECT_RECV_ID_EXCEPTION==1 && SIMCONNECT_RECV_ID_EVENT==4 && SIMCONNECT_RECV_ID_SIMOBJECT_DATA==8);
static_assert(SIMCONNECT_PERIOD_SIM_FRAME==3 && SIMCONNECT_OBJECT_ID_USER_AIRCRAFT==0);
static_assert(sizeof(SIMCONNECT_RECV)==12 && offsetof(SIMCONNECT_RECV_SIMOBJECT_DATA,dwData)==40);
static_assert(SIMCONNECT_RECV_ID_SYSTEM_STATE==15 && offsetof(SIMCONNECT_RECV_SYSTEM_STATE,dwRequestID)==12);
#endif
namespace sv {
SimConnect::~SimConnect(){disconnect();}
void SimConnect::disconnect() { if(handle && closeFunction) closeFunction(handle); handle=nullptr; if(module) FreeLibrary(module); module=nullptr; previous={}; lastPacket.clear(); paused=true; status="Disconnected"; }
void SimConnect::connect() {
    disconnect();
    QString selected=QCoreApplication::applicationDirPath()+"/SimConnect.dll";
    require(QFileInfo(selected).isFile(),"Bundled SimConnect.dll is missing beside StellaVibra.exe");
    module=LoadLibraryExW(reinterpret_cast<LPCWSTR>(selected.utf16()),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    require(module!=nullptr,"Cannot load x64 SimConnect.dll (Windows error "+QString::number(GetLastError())+")");
    try {
        auto open=reinterpret_cast<Open>(GetProcAddress(module,"SimConnect_Open")); closeFunction=reinterpret_cast<Close>(GetProcAddress(module,"SimConnect_Close"));
        auto add=reinterpret_cast<Add>(GetProcAddress(module,"SimConnect_AddToDataDefinition")); auto request=reinterpret_cast<RequestData>(GetProcAddress(module,"SimConnect_RequestDataOnSimObject"));
        nextFunction=reinterpret_cast<Next>(GetProcAddress(module,"SimConnect_GetNextDispatch")); auto subscribe=reinterpret_cast<Subscribe>(GetProcAddress(module,"SimConnect_SubscribeToSystemEvent"));
        systemStateFunction=reinterpret_cast<SystemState>(GetProcAddress(module,"SimConnect_RequestSystemState"));
        require(open && closeFunction && add && request && nextFunction && subscribe && systemStateFunction,"Missing SimConnect exports");
        require(SUCCEEDED(open(&handle,"Stella Vibra",nullptr,0,nullptr,0)),"MSFS 2024 connection unavailable");
        // Official SIMCONNECT_DATATYPE: FLOAT64=4, STRING256=9.
        const std::pair<const char*,const char*> variables[]={ {"ABSOLUTE TIME","Seconds"},{"GROUND VELOCITY","Knots"},{"SIM ON GROUND","Bool"},{"GEAR IS ON GROUND:0","Bool"},{"GEAR IS ON GROUND:1","Bool"},{"GEAR IS ON GROUND:2","Bool"},{"GEAR ANIMATION POSITION:0","Percent"},{"GEAR ANIMATION POSITION:1","Percent"},{"GEAR ANIMATION POSITION:2","Percent"} };
        for(const auto& [name,unit]:variables) require(SUCCEEDED(add(handle,1,name,unit,4,0,0xffffffff)),QString("SimConnect definition failed: ")+name);
        require(SUCCEEDED(add(handle,1,"TITLE",nullptr,9,0,0xffffffff)),"TITLE definition failed");
        require(SUCCEEDED(subscribe(handle,10,"Pause_EX1")),"Pause_EX1 subscription failed");
        require(SUCCEEDED(request(handle,1,1,0,3,0,0,0,0)),"Telemetry request failed");nextCheck=checkDeadline=0; status="Connected / waiting for telemetry";
    } catch(...) { disconnect(); throw; }
}
QList<Telemetry> SimConnect::poll() {
    QList<Telemetry> result; if(!handle) return result;
    for(int packet=0;packet<=64;++packet) {
        void* raw=nullptr; DWORD length=0; HRESULT hr=nextFunction(handle,&raw,&length);
        if(hr==E_FAIL) break; require(SUCCEEDED(hr),"SimConnect dispatch failed"); if(!raw || length<12) break;
        require(packet<64,"SimConnect backlog exceeded; reconnect required");
        auto* bytes=static_cast<const char*>(raw); auto u32=[&](int offset){ DWORD v=0; require(offset+4<=int(length),"Truncated SimConnect packet"); memcpy(&v,bytes+offset,4); return v; };
        auto kind=u32(8); if(kind==0) break;
        if(kind==1) throw std::runtime_error(QString("SimConnect exception %1 (send %2, index %3)").arg(u32(12)).arg(u32(16)).arg(u32(20)).toStdString());
        if(kind==3) throw std::runtime_error("MSFS disconnected");
        if(kind==15 && length>=20 && u32(12)==20) { checkDeadline=0;nextCheck=nowMs()+2000;continue; }
        if(kind==4 && length>=24 && u32(16)==10) { paused=u32(20)!=0; if(paused) { auto t=previous; t.paused=true; result<<t; } }
        if(kind!=8) continue;
        require(length>=40+9*8+256 && u32(12)==1 && u32(20)==1 && u32(24)==0,"Unexpected telemetry packet layout");
        QByteArray payload(bytes+40,9*8+256); if(payload==lastPacket) continue; lastPacket=payload;
        double values[9]; memcpy(values,bytes+40,sizeof values); for(double v:values) require(std::isfinite(v),"Invalid telemetry number");
        Telemetry t; t.sequence=previous.sequence+1; t.received=nowMs(); t.simTime=values[0]; t.speed=values[1]; t.ground=values[2]!=0; t.paused=paused;
        auto title=QByteArray(bytes+40+9*8,256); int end=title.indexOf('\0'); require(end>0,"Missing aircraft TITLE"); t.aircraft=QString::fromUtf8(title.constData(),end);
        t.advanced=t.simTime>previous.simTime; t.advancedAt=t.advanced?t.received:previous.advancedAt;
        for(int i=0;i<3;++i) { t.contact[i]=values[3+i]!=0; t.animation[i]=values[6+i]; }
        previous=t; result<<t; status=t.paused?"Connected / paused":"Connected";
    }
    auto now=nowMs();require(!checkDeadline || now<checkDeadline,"MSFS connection check timed out");
    if(!checkDeadline && now>=nextCheck){require(SUCCEEDED(systemStateFunction(handle,20,"Sim")),"MSFS connection check failed");checkDeadline=now+5000;}
    return result;
}
}
