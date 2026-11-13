#pragma once
#include <QtCore>
#include <chrono>
#include <array>
#include <cmath>
#include <stdexcept>

namespace sv {
inline qint64 nowMs() { return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count(); }
inline void require(bool ok, const QString& message) { if (!ok) throw std::runtime_error(message.toStdString()); }
QJsonObject readJson(const QString& path);
void writeJson(const QString& path, const QJsonObject& object);
QStringList logicalSlots();
QStringList availableSlots(const QJsonObject& counts);
QJsonObject fitOutputDefaults(const QJsonObject& outputs,const QJsonObject& counts);
QString appRoot();
QString localPath();
QString scriptPath(const QString& project, const QString& relative);
void validateSettings(const QJsonObject& object, bool partial);
struct Project {
    QString path;
    QJsonObject data;
    QMap<QString,QJsonObject> aircraft;
    QMap<QString,QString> aircraftPaths;
    void load(const QString& file,const QMap<QString,QJsonObject>& replacements={});
    void save();
    void compact(const QString& id,const QJsonObject& defaults);
    QJsonObject resolved(const QJsonObject& effect, const QString& key) const;
    void setOverride(const QString& key, const QString& id, const QJsonObject& value);
};
struct Parameter { QString name,type; QStringList choices; bool boolean=false; QJsonValue initial; double minimum=0,maximum=255,step=1; };
struct Telemetry {
    quint64 sequence=0;
    qint64 received=0, advancedAt=0;
    qint64 sourceDeadline=0;
    double simTime=0, speed=0;
    bool ground=false, paused=false, advanced=false;
    QString aircraft,game,session;
    QJsonObject values;
    std::array<bool,3> contact{};
    std::array<double,3> animation{};
    bool valid(qint64 now) const { return sequence && !paused && now-received<500 && now-advancedAt<500 && (!sourceDeadline || now<sourceDeadline) && !aircraft.isEmpty(); }
};
enum class Mode { Off, Constant, Pwm };
struct Request { QString key; QStringList destinations; Mode mode=Mode::Off; int strength=0,on=0,period=20; qint64 deadline=0,phase=0; };
struct Motor { QString id,label; QStringList destinations; bool connected=false,ambiguous=false; int sent=0; QString error; QString category="Throttle"; };
}
