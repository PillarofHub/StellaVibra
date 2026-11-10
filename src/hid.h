#pragma once
#include "model.h"
#include <windows.h>
#include <memory>
#include <vector>
namespace sv {
class Hid {
public:
    ~Hid();
    QList<Motor> scan();
    bool write(const QString& motor,int strength,QString& error);
    void close();
    QStringList takeLogs() { auto result=logs;logs.clear();return result; }
private:
    struct Device { HANDLE handle=INVALID_HANDLE_VALUE; QString key; QString label; int length=0; bool ambiguous=false; QString path; USHORT pid=0; unsigned char report=0; };
    std::vector<Device> devices;
    QMap<QString,QString> issues;
    QStringList logs;
    void reportIssue(const QString& path,const QString& message);
};
}
