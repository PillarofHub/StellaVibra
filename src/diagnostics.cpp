#include "diagnostics.h"
#include "model.h"
#include <QtCore>
#include <windows.h>
#include <dbghelp.h>
#include <mutex>
namespace sv {
namespace {
QString logPath;
wchar_t dumpPath[32768]{};
std::mutex logMutex;
LONG dumping=0;
QtMessageHandler previousHandler=nullptr;
void message(QtMsgType type,const QMessageLogContext& context,const QString& text) {
    {
        std::lock_guard lock(logMutex);
        if(QFileInfo(logPath).size()>2*1024*1024){QFile::remove(logPath+".previous");QFile::rename(logPath,logPath+".previous");}
        QFile file(logPath);
        if(file.open(QIODevice::WriteOnly|QIODevice::Append)){
            const auto line=QString("%1 [%2] [thread %3] %4\n").arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs)).arg(int(type)).arg(GetCurrentThreadId()).arg(text).toUtf8();
            file.write(line);file.flush();
        }
    }
    if(previousHandler)previousHandler(type,context,text);
}
LONG WINAPI crash(EXCEPTION_POINTERS* exception) {
    // Best effort only: no Qt, logger locks or recovery inside a damaged process.
    if(InterlockedCompareExchange(&dumping,1,0)==0 && dumpPath[0]) {
        HANDLE file=CreateFileW(dumpPath,GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
        if(file!=INVALID_HANDLE_VALUE){
            MINIDUMP_EXCEPTION_INFORMATION info{GetCurrentThreadId(),exception,FALSE};
            MiniDumpWriteDump(GetCurrentProcess(),GetCurrentProcessId(),file,
                static_cast<MINIDUMP_TYPE>(MiniDumpNormal|MiniDumpWithThreadInfo|MiniDumpWithUnloadedModules),&info,nullptr,nullptr);
            CloseHandle(file);
        }
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
}
void initializeDiagnostics() {
    const auto folder=appRoot()+"/diagnostics";
    if(!QDir().mkpath(folder)){qWarning("Cannot create app-local diagnostics directory");return;}
    const auto session=QDateTime::currentDateTimeUtc().toString("yyyyMMdd-HHmmss-zzz")+"-"+QString::number(GetCurrentProcessId());
    logPath=folder+"/"+session+".log";
    const auto dump=QDir::toNativeSeparators(folder+"/"+session+".dmp");
    if(dump.size()<32768)wcscpy_s(dumpPath,32768,reinterpret_cast<const wchar_t*>(dump.utf16()));
    previousHandler=qInstallMessageHandler(message);
    SetUnhandledExceptionFilter(crash);
    qInfo().noquote()<<"Stella Vibra started; build"<<__DATE__<<__TIME__<<"; executable"<<QCoreApplication::applicationFilePath();
}
}
