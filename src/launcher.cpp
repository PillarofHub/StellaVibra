#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

// Keep the public entry point dependency-free. Qt and its app-local runtimes
// live with the real application in bin; no PATH or registry changes are needed.
int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR arguments,int show) {
    wchar_t path[32768]{};
    const auto length=GetModuleFileNameW(nullptr,path,32768);
    if(!length || length>=32768)return 1;
    std::wstring executable(path,length);
    executable.erase(executable.find_last_of(L"\\/"));
    executable+=L"\\bin\\StellaVibra.exe";
    std::wstring command=L"\""+executable+L"\" "+arguments;
    STARTUPINFOW startup{};startup.cb=sizeof(startup);startup.dwFlags=STARTF_USESHOWWINDOW;startup.wShowWindow=static_cast<WORD>(show);
    PROCESS_INFORMATION process{};
    if(!CreateProcessW(executable.c_str(),command.data(),nullptr,nullptr,FALSE,0,nullptr,nullptr,&startup,&process)) {
        const auto message=L"Cannot start bin\\StellaVibra.exe. Keep the complete application folder together.\nWindows error: "+std::to_wstring(GetLastError());
        MessageBoxW(nullptr,message.c_str(),L"Stella Vibra",MB_OK|MB_ICONERROR);return 1;
    }
    CloseHandle(process.hThread);CloseHandle(process.hProcess);return 0;
}
