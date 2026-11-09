#include "hid.h"
#include <hidsdi.h>
#include <hidpi.h>
#include <setupapi.h>
namespace sv {
Hid::~Hid(){close();}
void Hid::close(){ for(auto& d:devices) if(d.handle!=INVALID_HANDLE_VALUE) CloseHandle(d.handle); devices.clear(); }
QList<Motor> Hid::scan() {
    close(); GUID guid; HidD_GetHidGuid(&guid); auto set=SetupDiGetClassDevsW(&guid,nullptr,nullptr,DIGCF_PRESENT|DIGCF_DEVICEINTERFACE); require(set!=INVALID_HANDLE_VALUE,"Cannot enumerate HID interfaces");
    SP_DEVICE_INTERFACE_DATA info{}; info.cbSize=sizeof info;
    for(DWORD index=0;SetupDiEnumDeviceInterfaces(set,nullptr,&guid,index,&info);++index) {
        DWORD size=0; SetupDiGetDeviceInterfaceDetailW(set,&info,nullptr,0,&size,nullptr); if(!size) continue;
        std::vector<char> storage(size); auto* detail=reinterpret_cast<SP_DEVICE_INTERFACE_DETAIL_DATA_W*>(storage.data()); detail->cbSize=sizeof(*detail);
        if(!SetupDiGetDeviceInterfaceDetailW(set,&info,detail,size,nullptr,nullptr)) continue;
        HANDLE probe=CreateFileW(detail->DevicePath,0,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr); if(probe==INVALID_HANDLE_VALUE) continue;
        HIDD_ATTRIBUTES attrs{}; attrs.Size=sizeof attrs; PHIDP_PREPARSED_DATA preparsed=nullptr; HIDP_CAPS caps{};
        bool supported=HidD_GetAttributes(probe,&attrs) && attrs.VendorID==0x4098 && attrs.ProductID==0xB970;
        bool descriptor=false;
        if(supported && HidD_GetPreparsedData(probe,&preparsed)) {
            if(HidP_GetCaps(preparsed,&caps)==HIDP_STATUS_SUCCESS && caps.OutputReportByteLength>=14) {
                USHORT count=caps.NumberOutputValueCaps; std::vector<HIDP_VALUE_CAPS> values(count);
                if(count && HidP_GetValueCaps(HidP_Output,values.data(),&count,preparsed)==HIDP_STATUS_SUCCESS) for(const auto& value:values) if(value.ReportID==2) descriptor=true;
                count=caps.NumberOutputButtonCaps; std::vector<HIDP_BUTTON_CAPS> buttons(count);
                if(count && HidP_GetButtonCaps(HidP_Output,buttons.data(),&count,preparsed)==HIDP_STATUS_SUCCESS) for(const auto& button:buttons) if(button.ReportID==2) descriptor=true;
            }
            HidD_FreePreparsedData(preparsed);
        }
        wchar_t serial[256]{}; bool serialOk=supported && HidD_GetSerialNumberString(probe,serial,sizeof serial) && serial[0]; CloseHandle(probe);
        if(!supported || !descriptor) continue;
        auto path=QString::fromWCharArray(detail->DevicePath); auto serialText=serialOk?QString::fromWCharArray(serial):QString();
        QString key=serialOk?"4098:B970:"+serialText:"session:"+QString(QCryptographicHash::hash(path.toUtf8(),QCryptographicHash::Sha256).toHex());
        auto handle=CreateFileW(detail->DevicePath,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,nullptr); if(handle==INVALID_HANDLE_VALUE) continue;
        devices.push_back({handle,key,"Combat Throttle Metal.EX",caps.OutputReportByteLength,!serialOk});
    }
    SetupDiDestroyDeviceInfoList(set);
    QMap<QString,int> counts; for(const auto& d:devices) ++counts[d.key];
    QList<Motor> motors; int duplicate=0;
    for(auto& d:devices) { if(counts[d.key]>1) { d.ambiguous=true; d.key="session:duplicate:"+QString::number(duplicate++); }
        for(auto side:{"Left","Right"}) motors<<Motor{d.key+"/"+side,d.label+" "+side,{},true,d.ambiguous,0,{}};
    }
    return motors;
}
bool Hid::write(const QString& motor,int strength,QString& error) {
    for(auto& d:devices) if(motor==d.key+"/Left" || motor==d.key+"/Right") {
        if(d.handle==INVALID_HANDLE_VALUE) {error="Device disconnected";return false;}
        bool left=motor.endsWith("/Left"); QByteArray frame(d.length,'\0'); frame[0]=2; frame[1]=left?4:8; frame[2]=-71; frame[5]=3; frame[6]=0x49; frame[7]=left?0x0E:0x10; frame[8]=char(strength);
        OVERLAPPED overlap{}; overlap.hEvent=CreateEventW(nullptr,TRUE,FALSE,nullptr); if(!overlap.hEvent) {error="Cannot create HID event";return false;}
        DWORD written=0; bool ok=WriteFile(d.handle,frame.constData(),DWORD(frame.size()),&written,&overlap)!=FALSE;
        if(!ok && GetLastError()==ERROR_IO_PENDING) { if(WaitForSingleObject(overlap.hEvent,10)==WAIT_OBJECT_0) ok=GetOverlappedResult(d.handle,&overlap,&written,FALSE)!=FALSE; else { CancelIoEx(d.handle,&overlap); GetOverlappedResult(d.handle,&overlap,&written,TRUE); } }
        auto code=GetLastError(); CloseHandle(overlap.hEvent);
        if(!ok || written!=DWORD(frame.size())) { error="HID write failed: "+QString::number(code); CloseHandle(d.handle); d.handle=INVALID_HANDLE_VALUE; return false; } return true;
    }
    error="Device unavailable"; return false;
}
}
