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
private:
    struct Device { HANDLE handle=INVALID_HANDLE_VALUE; QString key; QString label; int length=0; bool ambiguous=false; };
    std::vector<Device> devices;
};
}
