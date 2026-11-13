#pragma once
#include "model.h"
#include <QtNetwork>
#include <memory>
namespace sv {
// Concrete local transports. Only the selected project's transport is active.
// Values retain the game's names/units; Telemetry carries host lifetime metadata only.
class NetworkTelemetry {
public:
    explicit NetworkTelemetry(QString target);
    ~NetworkTelemetry();
    QList<Telemetry> poll();
    bool connected() const;
    QString status="Disconnected / retrying";
private:
    QString game;
    QEventLoop events;
    std::unique_ptr<QNetworkAccessManager> http;
    std::unique_ptr<QUdpSocket> udp;
    QNetworkReply* replies[2]{};
    qint64 cycleStarted=0,nextPoll=0,lastResponse=0,nextSubscribe=0;
    int subscribeIndex=0;
    QJsonObject datarefs;
    QMap<QString,qint64> updated;
    Telemetry previous;
    QString dcsSession;
    QStringList retiredDcsSessions;
    quint64 dcsSequence=0,dcsEpoch=0;
    qint64 dcsSent=0;
    quint16 dcsPort=0;
    void sendSubscription(int index,int frequency);
    QList<Telemetry> pollWarThunder();
    QList<Telemetry> pollXPlane();
    QList<Telemetry> pollDcs();
};
}
