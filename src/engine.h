#pragma once
#include "model.h"
#include "lua_effect.h"
#include <atomic>
#include <mutex>
#include <thread>
#include <memory>
namespace sv {
struct Bundle { Project project; QMap<QString,QString> sources; QSet<QString> reloadIds; QJsonObject channelCounts; };
struct Snapshot { QString sim="Disconnected",aircraft,execution="Stopped"; QList<Motor> motors; QStringList logs; QJsonObject mappings; Telemetry telemetry; bool telemetryConnected=false; };
class Engine {
public:
    Engine(); ~Engine();
    void apply(Bundle bundle);
    void clearProject();
    void run(); void stop(); void disable(const QString& id);
    void mapMotor(QString id,QStringList destinations);
    void loadMappings(QJsonObject mappings);
    void test(QString motor,Mode mode,int strength,int on,int period,int duration);
    void stopTest();
    Snapshot snapshot();
    void setTick(int milliseconds);
    int tickMs() const {return tickMilliseconds.load();}
    void setMaximumUpdatePeriod(int milliseconds);
    int maximumUpdatePeriodMs() const {return maximumUpdateMilliseconds.load();}
private:
    std::mutex mutex;
    std::thread runtimeThread,hidThread;
    std::atomic<bool> quitting=false, running=true;
    std::atomic<bool> ready=false;
    std::atomic<quint64> projectRevision=0;
    std::atomic<quint64> generation=1;
    std::atomic<qint64> heartbeat=0, inputDeadline=0;
    std::unique_ptr<Bundle> pending;
    bool clearRequested=false;
    QSet<QString> disabled;
    QMap<QString,Request> requests;
    QJsonObject mappings;
    QList<Motor> motors;
    QStringList logs;
    QString simStatus="Disconnected",aircraftKey,execution="Stopped";
    Request testRequest;
    std::atomic<int> tickMilliseconds=20;
    std::atomic<int> maximumUpdateMilliseconds=100;
    Telemetry latestTelemetry;
    bool telemetryConnected=false;
    QString testMotor;
    void log(const QString& text);
    void runtimeLoop(); void hidLoop();
};
}
