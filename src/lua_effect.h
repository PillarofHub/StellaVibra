#pragma once
#include "model.h"
struct lua_State;
struct lua_Debug;
namespace sv {
class LuaEffect {
public:
    LuaEffect(QString id,QString source,QString filename);
    ~LuaEffect();
    LuaEffect(const LuaEffect&)=delete;
    QList<Parameter> parameters;
    QStringList outputs;
    QString id;
    QJsonObject settings;
    QStringList unresolved;
    void configure(const QJsonObject& values);
    QList<Request> update(const Telemetry& telemetry,bool fresh,qint64 now);
    QStringList takeLogs();
private:
    lua_State* state=nullptr;
    size_t allocated=0;
    int reference=-1, budget=0;
    qint64 started=0, current=0, previous=0;
    bool registering=false;
    QList<Request> pending;
    QStringList logs;
    QByteArray callbackError;
    QString sourceText,sourceName;
    Telemetry updateTelemetry;
    bool updateFresh=false;
    void initialize();
    void performUpdate();
    static void* allocate(void*,void*,size_t,size_t);
    static void hook(lua_State*,lua_Debug*);
    static int dispatch(lua_State*);
    int api(lua_State*,int operation);
    void pushApi();
    void protectedCall(int args,int results);
    void pushJson(const QJsonValue& value);
};
}
