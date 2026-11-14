#include "window.h"

namespace sv {
void Window::toggleLogging(bool enabled) {
    if(enabled) {
        require(!project.path.isEmpty(),"Open a project before starting Logging");
        const auto folder=appRoot()+"/diagnostics/telemetry";
        require(QDir().mkpath(folder),"Cannot create telemetry log directory");
        const auto timestamp=QDateTime::currentDateTimeUtc();
        const auto game=project.data["game"].toString();
        const auto path=folder+"/"+timestamp.toString("yyyyMMddTHHmmsszzzZ")+"_"+game+"_"+QUuid::createUuid().toString(QUuid::WithoutBraces).left(8)+".jsonl";
        telemetryLogFile.setFileName(path);
        require(telemetryLogFile.open(QIODevice::WriteOnly|QIODevice::NewOnly),"Cannot create telemetry log");
        const auto header="#start "+QJsonDocument(QJsonObject{{"utc",timestamp.toString(Qt::ISODateWithMs)},{"game",game},{"format",1}}).toJson(QJsonDocument::Compact)+"\n#t=elapsed_ms v=valid f=full d=set(JSON Pointer) x=remove; arrays replaced whole\n";
        if(telemetryLogFile.write(header)!=header.size() || !telemetryLogFile.flush()){telemetryLogFile.close();throw std::runtime_error("Cannot write telemetry log header");}
        engine.capture.start();nextLogFlush=nowMs()+1000;
        addLog("Logging started: "+path);
    } else if(telemetryLogFile.isOpen()) {
        engine.capture.stop();drainTelemetryLog();
        if(telemetryLogFile.isOpen()){
            const auto end="#stop "+QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs).toUtf8()+"\n";
            const bool ok=telemetryLogFile.write(end)==end.size() && telemetryLogFile.flush();telemetryLogFile.close();
            addLog(ok?"Logging stopped":"Logging stopped with a file write error");
        }
    }
}
void Window::drainTelemetryLog() {
    if(!telemetryLogFile.isOpen())return;
    const auto bytes=engine.capture.take();
    bool ok=bytes.isEmpty() || telemetryLogFile.write(bytes)==bytes.size();
    if(nowMs()>=nextLogFlush){ok=telemetryLogFile.flush() && ok;nextLogFlush=nowMs()+1000;}
    if(!ok){engine.capture.stop();telemetryLogFile.close();loggingAction->setChecked(false);addLog("Logging stopped: file write failed");}
}
}
