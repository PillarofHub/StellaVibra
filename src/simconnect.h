#pragma once
#include "model.h"
#include <windows.h>
namespace sv {
// Public SimConnect ABI subset; see CONTRACTS.md. No SDK binary is bundled.
class SimConnect {
public:
    ~SimConnect();
    void connect(const QString& dll);
    void disconnect();
    QList<Telemetry> poll();
    QString status="Disconnected";
    bool connected() const { return handle!=nullptr; }
private:
    HMODULE module=nullptr;
    HANDLE handle=nullptr;
    using Open=HRESULT(WINAPI*)(HANDLE*,LPCSTR,HWND,DWORD,HANDLE,DWORD);
    using Close=HRESULT(WINAPI*)(HANDLE);
    using Add=HRESULT(WINAPI*)(HANDLE,DWORD,LPCSTR,LPCSTR,DWORD,float,DWORD);
    using RequestData=HRESULT(WINAPI*)(HANDLE,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD,DWORD);
    using Next=HRESULT(WINAPI*)(HANDLE,void**,DWORD*);
    using Subscribe=HRESULT(WINAPI*)(HANDLE,DWORD,LPCSTR);
    Close closeFunction=nullptr;
    Next nextFunction=nullptr;
    bool paused=true;
    Telemetry previous;
    QByteArray lastPacket;
};
}
