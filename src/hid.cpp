#include "hid.h"
#include <hidsdi.h>
#include <hidpi.h>
#include <setupapi.h>
#include <bit>
namespace sv {
static char byte(unsigned char value) { return std::bit_cast<char>(value); }
// PID/channel/frame definitions: Aurora src/hid/protocol.rs. No unknown-PID fallback.
struct Hardware { USHORT pid; const char* name; bool stick; int channels; unsigned char identifier; };
static const Hardware hardware[]={
    {0xBC27,"URSA MINOR Airbus L",true,1,0x07},{0xBC28,"URSA MINOR Airbus R",true,1,0x08},
    {0xBC29,"URSA MINOR Combat/Fighter L",true,1,0x09},{0xBC2A,"URSA MINOR Combat/Fighter R",true,1,0x0A},
    {0xBC2B,"URSA MINOR Space L",true,1,0x0B},{0xBC2C,"URSA MINOR Space R",true,1,0x0C},
    {0xB920,"URSA MINOR Throttle",false,2,0x10},{0xB930,"URSA MINOR Throttle R",false,2,0x10},
    {0xB970,"Combat Throttle Metal.EX",false,2,0},
    {0xBEA8,"Orion Joystick Base II",true,1,0x01},{0xBD64,"Orion Throttle Base II",false,1,0x01}
};
static const Hardware* hardwareFor(USHORT pid) {for(const auto& h:hardware)if(h.pid==pid)return &h;return nullptr;}
void Hid::reportIssue(const QString& path,const QString& message) {
    if(issues.value(path)!=message){issues[path]=message;logs<<message+" ("+path+")";}
}
Hid::~Hid(){close();}
void Hid::close(){ for(auto& d:devices) if(d.handle!=INVALID_HANDLE_VALUE) CloseHandle(d.handle); devices.clear(); }
QList<Motor> Hid::scan() {
    GUID guid; HidD_GetHidGuid(&guid); auto set=SetupDiGetClassDevsW(&guid,nullptr,nullptr,DIGCF_PRESENT|DIGCF_DEVICEINTERFACE); require(set!=INVALID_HANDLE_VALUE,"Cannot enumerate HID interfaces");
    auto previous=std::move(devices); devices.clear();
    SP_DEVICE_INTERFACE_DATA info{}; info.cbSize=sizeof info;
    for(DWORD index=0;SetupDiEnumDeviceInterfaces(set,nullptr,&guid,index,&info);++index) {
        DWORD size=0; SetupDiGetDeviceInterfaceDetailW(set,&info,nullptr,0,&size,nullptr); if(!size) continue;
        std::vector<char> storage(size); auto* detail=reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(storage.data()); detail->cbSize=sizeof(*detail);
        if(!SetupDiGetDeviceInterfaceDetailW(set,&info,detail,size,nullptr,nullptr)) continue;
        HANDLE probe=CreateFileW(detail->DevicePath,0,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr); if(probe==INVALID_HANDLE_VALUE) continue;
        HIDD_ATTRIBUTES attrs{}; attrs.Size=sizeof attrs; PHIDP_PREPARSED_DATA preparsed=nullptr; HIDP_CAPS caps{};
        bool supported=HidD_GetAttributes(probe,&attrs) && attrs.VendorID==0x4098 && hardwareFor(attrs.ProductID);
        bool descriptor=false;QSet<int> reports;
        if(supported && HidD_GetPreparsedData(probe,&preparsed)) {
            if(HidP_GetCaps(preparsed,&caps)==HIDP_STATUS_SUCCESS && caps.OutputReportByteLength>=14 && caps.UsagePage==1 && caps.Usage==4) {
                USHORT count=caps.NumberOutputValueCaps; std::vector<HIDP_VALUE_CAPS> values(count);
                if(count && HidP_GetValueCaps(HidP_Output,values.data(),&count,preparsed)==HIDP_STATUS_SUCCESS) for(USHORT i=0;i<count;++i)reports.insert(values[i].ReportID);
                count=caps.NumberOutputButtonCaps; std::vector<HIDP_BUTTON_CAPS> buttons(count);
                if(count && HidP_GetButtonCaps(HidP_Output,buttons.data(),&count,preparsed)==HIDP_STATUS_SUCCESS) for(USHORT i=0;i<count;++i)reports.insert(buttons[i].ReportID);
                descriptor=attrs.ProductID==0xB970?reports.contains(2):!reports.isEmpty();
            }
            HidD_FreePreparsedData(preparsed);
        }
        wchar_t serial[256]{}; bool serialOk=supported && HidD_GetSerialNumberString(probe,serial,sizeof serial) && serial[0]; CloseHandle(probe);
        auto path=QString::fromWCharArray(detail->DevicePath); auto serialText=serialOk?QString::fromWCharArray(serial):QString();
        if(!supported){if(attrs.VendorID==0x4098)reportIssue(path,QString("Unsupported WINCTRL HID PID %1; no output opened").arg(attrs.ProductID,4,16,QChar('0')));continue;}
        if(!descriptor){reportIssue(path,QString("HID %1: compatible joystick output descriptor unavailable").arg(attrs.ProductID,4,16,QChar('0')));continue;}
        int report=0;for(int id:reports)if(id && (!report || id<report))report=id;if(attrs.ProductID==0xB970)report=2;
        QString key=serialOk?QString("4098:%1:").arg(attrs.ProductID,4,16,QChar('0')).toUpper()+serialText:"session:"+QString(QCryptographicHash::hash(path.toUtf8(),QCryptographicHash::Sha256).toHex());
        HANDLE handle=INVALID_HANDLE_VALUE;
        for(auto& old:previous) if(old.path==path && old.handle!=INVALID_HANDLE_VALUE) {handle=old.handle;old.handle=INVALID_HANDLE_VALUE;break;}
        if(handle==INVALID_HANDLE_VALUE) handle=CreateFileW(detail->DevicePath,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,nullptr);
        if(handle==INVALID_HANDLE_VALUE) {reportIssue(path,"HID write access unavailable: Windows error "+QString::number(GetLastError()));continue;}
        issues.remove(path);
        devices.push_back({handle,key,hardwareFor(attrs.ProductID)->name,caps.OutputReportByteLength,!serialOk,path,attrs.ProductID,static_cast<unsigned char>(report)});
    }
    SetupDiDestroyDeviceInfoList(set);
    QMap<QString,int> counts; for(const auto& d:devices) ++counts[d.key];
    for(auto& old:previous) if(old.handle!=INVALID_HANDLE_VALUE)CloseHandle(old.handle);
    QList<Motor> motors;
    for(auto& d:devices) { if(counts[d.key]>1) { d.ambiguous=true; d.key="session:duplicate:"+QString(QCryptographicHash::hash(d.path.toUtf8(),QCryptographicHash::Sha256).toHex()); }
        const auto* h=hardwareFor(d.pid);
        const QStringList channels=h->channels==2?QStringList{"Left","Right"}:QStringList{h->stick?"Stick":"Combined"};
        for(const auto& channel:channels)motors<<Motor{d.key+"/"+channel,d.label+" "+channel,{},true,d.ambiguous,0,{},h->stick?"Stick":"Throttle"};
    }
    return motors;
}
bool Hid::write(const QString& motor,int strength,QString& error) {
    for(auto& d:devices) {
        const auto* h=hardwareFor(d.pid);
        bool left=motor==d.key+"/Left",right=motor==d.key+"/Right";
        if(!(h->channels==2?(left||right):motor==d.key+(h->stick?"/Stick":"/Combined")))continue;
        if(d.handle==INVALID_HANDLE_VALUE) {error="Device disconnected";return false;}
        QByteArray base(d.length,'\0');base[0]=char(d.report);base[1]=char(h->identifier);base[5]=3;base[6]=0x49;base[8]=char(strength);
        if(d.pid==0xB970){base[1]=left?4:8;base[2]=byte(0xB9);base[7]=left?0x0E:0x10;}
        else if(d.pid==0xB920 || d.pid==0xB930){base[2]=byte(0xB9);base[7]=left?0x0E:0x10;}
        else base[2]=d.pid==0xBEA8?0:byte(0xBF);
        QList<QByteArray> frames{base};if(d.pid==0xBD64){base[2]=byte(0xCF);frames<<base;}
        for(const auto& frame:frames) {
        OVERLAPPED overlap{}; overlap.hEvent=CreateEventW(nullptr,TRUE,FALSE,nullptr); if(!overlap.hEvent) {error="Cannot create HID event";return false;}
        DWORD written=0; bool ok=WriteFile(d.handle,frame.constData(),DWORD(frame.size()),&written,&overlap)!=FALSE;
        if(!ok && GetLastError()==ERROR_IO_PENDING) { if(WaitForSingleObject(overlap.hEvent,10)==WAIT_OBJECT_0) ok=GetOverlappedResult(d.handle,&overlap,&written,FALSE)!=FALSE; else { CancelIoEx(d.handle,&overlap); GetOverlappedResult(d.handle,&overlap,&written,TRUE); } }
        auto code=GetLastError(); CloseHandle(overlap.hEvent);
        if(!ok || written!=DWORD(frame.size())) { error="HID write failed: "+QString::number(code); CloseHandle(d.handle); d.handle=INVALID_HANDLE_VALUE; return false; }
        }
        return true;
    }
    error="Device unavailable"; return false;
}
}
