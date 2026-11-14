#include "simconnect.h"
#include "telemetry.h"
#include <cmath>
#include <cstring>
#include <tlhelp32.h>
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
static void checkLocalSimulator(const QString& game) {
    // Both versions can publish the default SimConnect pipe. Do not attach to
    // the wrong game just because its client SDK is backward compatible.
    auto processes=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    require(processes!=INVALID_HANDLE_VALUE,"Cannot inspect the local MSFS process");
    PROCESSENTRY32W entry{};entry.dwSize=sizeof(entry);bool msfs2020=false,msfs2024=false;
    if(Process32FirstW(processes,&entry))do{const auto name=QString::fromWCharArray(entry.szExeFile);
        msfs2020|=name.compare("FlightSimulator.exe",Qt::CaseInsensitive)==0;
        msfs2024|=name.compare("FlightSimulator2024.exe",Qt::CaseInsensitive)==0;
    }while(Process32NextW(processes,&entry));CloseHandle(processes);
    require(!(msfs2020 && msfs2024),"Both MSFS versions are running; close the other version to avoid an ambiguous SimConnect server");
    require(game=="MSFS2020"?msfs2020:msfs2024,game+" is not running locally");
}
SimConnect::~SimConnect(){disconnect();}
void SimConnect::disconnect() { if(handle && closeFunction) closeFunction(handle); handle=nullptr; if(module) FreeLibrary(module); module=nullptr; previous={}; lastPacket.clear(); paused=true; status="Disconnected"; }
void SimConnect::connect(const QString& target) {
    disconnect();
    game=target;
    checkLocalSimulator(game);
    QString selected=appRoot()+"/bin/simconnect/"+game+"/SimConnect.dll";
    require(QFileInfo(selected).isFile(),"Bundled SimConnect.dll is missing: "+selected);
    module=LoadLibraryExW(reinterpret_cast<LPCWSTR>(selected.utf16()),nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    require(module!=nullptr,"Cannot load x64 SimConnect.dll (Windows error "+QString::number(GetLastError())+")");
    try {
        // Close connection handles on retry, but retain code for possible SDK
        // background callbacks until process exit. This also covers failed Open.
        HMODULE pinned=nullptr;
        require(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
            reinterpret_cast<LPCWSTR>(module),&pinned)!=FALSE,"Cannot retain SimConnect client module");
        auto open=reinterpret_cast<Open>(GetProcAddress(module,"SimConnect_Open")); closeFunction=reinterpret_cast<Close>(GetProcAddress(module,"SimConnect_Close"));
        auto add=reinterpret_cast<Add>(GetProcAddress(module,"SimConnect_AddToDataDefinition")); auto request=reinterpret_cast<RequestData>(GetProcAddress(module,"SimConnect_RequestDataOnSimObject"));
        nextFunction=reinterpret_cast<Next>(GetProcAddress(module,"SimConnect_GetNextDispatch")); auto subscribe=reinterpret_cast<Subscribe>(GetProcAddress(module,"SimConnect_SubscribeToSystemEvent"));
        systemStateFunction=reinterpret_cast<SystemState>(GetProcAddress(module,"SimConnect_RequestSystemState"));
        require(open && closeFunction && add && request && nextFunction && subscribe && systemStateFunction,"Missing SimConnect exports");
        qInfo().noquote()<<"SimConnect Open:"<<game;
        const auto opened=open(&handle,"Stella Vibra",nullptr,0,nullptr,0);
        qInfo().noquote()<<"SimConnect Open result:"<<QString::number(quint32(opened),16);
        require(SUCCEEDED(opened),game+" connection unavailable");
        // Official SIMCONNECT_DATATYPE: FLOAT64=4, STRING256=9.
        for(const auto& field:simVariables()) require(SUCCEEDED(add(handle,1,field.name.toUtf8().constData(),field.unit.toUtf8().constData(),4,0,0xffffffff)),"SimConnect definition failed: "+field.name);
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
        const int count=static_cast<int>(simVariables().size());
        require(length>=DWORD(40+count*8+256) && u32(12)==1 && u32(20)==1 && u32(24)==0,"Unexpected telemetry packet layout");
        QByteArray payload(bytes+40,count*8+256); if(payload==lastPacket) continue; lastPacket=payload;
        QList<double> values(count); memcpy(values.data(),bytes+40,count*8);
        Telemetry t; t.game=game; t.sequence=previous.sequence+1; t.received=nowMs(); t.paused=paused;
        for(int i=0;i<count;++i){const auto& field=simVariables()[i];t.values[field.key]=!std::isfinite(values[i])?QJsonValue():field.boolean?QJsonValue(values[i]!=0):QJsonValue(values[i]);}
        for(const auto& key:QStringList{"sim_time_s","ground_speed_knots","on_ground"})require(!t.values[key].isNull(),"Invalid required telemetry: "+key);
        t.simTime=t.values["sim_time_s"].toDouble();t.speed=t.values["ground_speed_knots"].toDouble();t.ground=t.values["on_ground"].toBool();
        auto title=QByteArray(bytes+40+count*8,256); int end=title.indexOf('\0'); require(end>0,"Missing aircraft TITLE"); t.aircraft=QString::fromUtf8(title.constData(),end);
        t.advanced=t.simTime>previous.simTime; t.advancedAt=t.advanced?t.received:previous.advancedAt;
        // Catalog additions must not shift the derived Lua gear view.
        for(int i=0;i<3;++i) {
            const auto contact=t.values["gear_contact_"+QString::number(i)];
            const auto animation=t.values["gear_animation_"+QString::number(i)];
            require(contact.isBool() && animation.isDouble(),"Invalid required gear telemetry");
            t.contact[i]=contact.toBool();t.animation[i]=animation.toDouble();
        }
        if(!previous.sequence)qInfo().noquote()<<"First SimConnect telemetry:"<<t.aircraft<<"paused"<<t.paused;
        previous=t; result<<t; status=t.paused?"Connected / paused":"Connected";
    }
    auto now=nowMs();require(!checkDeadline || now<checkDeadline,"MSFS connection check timed out");
    if(!checkDeadline && now>=nextCheck){require(SUCCEEDED(systemStateFunction(handle,20,"Sim")),"MSFS connection check failed");checkDeadline=now+5000;}
    return result;
}
}
