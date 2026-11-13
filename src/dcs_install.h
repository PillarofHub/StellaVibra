#pragma once
#include "model.h"
#include <functional>
namespace sv {
struct DcsInstallation {
    bool canAdd=false,canUpdate=false,canRestore=false,canRemove=false,needsRestore=false;
    int interval=50;
    QString text,details,fingerprint;
};
DcsInstallation inspectDcsInstallation(const QString& scripts);
void installDcsTelemetry(const QString& scripts,int interval,const std::function<bool(const QString&)>& confirmRestore);
void removeDcsTelemetry(const QString& scripts,const std::function<bool(const QString&)>& confirmRemove);
}
