#include "network_telemetry.h"
#include <cmath>
namespace sv {
QList<Telemetry> NetworkTelemetry::pollDcs() {
    QList<Telemetry> result;
    for(int n=0;n<64 && udp->hasPendingDatagrams();++n) {
        const auto packet=udp->receiveDatagram(16385);
        if(packet.senderAddress()!=QHostAddress::LocalHost || packet.data().size()>16384)continue;
        QJsonParseError error;const auto document=QJsonDocument::fromJson(packet.data(),&error);
        if(error.error!=QJsonParseError::NoError || !document.isObject())continue;
        const auto object=document.object();
        auto number=[&](const char* name){return object[name].isDouble() && std::isfinite(object[name].toDouble());};
        if(object["protocol"]!=QJsonValue(1) || !object["session"].isString() || !object["dcs"].isObject() || !object["stopped"].isBool()
            || !number("sequence") || !number("sim_time") || !number("sent_ms") || !number("interval_ms"))continue;
        const auto session=object["session"].toString();const double seq=object["sequence"].toDouble(),sent=object["sent_ms"].toDouble();
        const double interval=object["interval_ms"].toDouble();
        if(session.isEmpty() || session.size()>160 || seq<1 || seq>9007199254740991.0 || std::floor(seq)!=seq
            || interval<10 || interval>100 || std::floor(interval)!=interval || sent<0 || sent>9007199254740991.0)continue;
        const auto age=double(QDateTime::currentMSecsSinceEpoch())-sent;
        // Local wall clock is only a transport-age check. Lifetime uses host monotonic time.
        if(age< -100 || age>=500 || sent<dcsSent || retiredDcsSessions.contains(session))continue;
        const auto now=nowMs();
        if(dcsPort && packet.senderPort()!=dcsPort && now-lastResponse<1500)continue;
        const bool newSession=session!=dcsSession;
        if(!newSession && quint64(seq)<=dcsSequence)continue;
        if(newSession){
            if(!dcsSession.isEmpty())retiredDcsSessions<<dcsSession;
            if(retiredDcsSessions.size()>64)retiredDcsSessions.removeFirst();
            dcsSession=session;
        }
        const auto data=object["dcs"].toObject();
        const auto identity=data["LoGetPlayerPlaneId"];
        const auto self=data["LoGetSelfData"].toObject();
        const bool baseline=newSession || !lastResponse || now-lastResponse>=500 || object["sim_time"].toDouble()<previous.simTime
            || identity!=previous.values["dcs"].toObject()["LoGetPlayerPlaneId"];
        if(baseline)++dcsEpoch;
        Telemetry t;t.game="DCS";t.sequence=previous.sequence+1;t.received=now;t.simTime=object["sim_time"].toDouble();
        t.sourceDeadline=now+500-qint64(std::max(0.0,age));
        t.session=session+"/"+QString::number(dcsEpoch);t.aircraft=self["Name"].toString();
        t.paused=object["stopped"].toBool() || data["LoIsOwnshipExportAllowed"]!=QJsonValue(true)
            || data["LoSimulationOnPause"]!=QJsonValue(false) || data["LoSimulationOnActivePause"]!=QJsonValue(false)
            || !identity.isDouble() || t.aircraft.isEmpty();
        t.advanced=!baseline && !t.paused && t.simTime>previous.simTime;
        t.advancedAt=baseline?0:t.advanced?now:previous.advancedAt;
        t.values={{"dcs",data},{"transport",QJsonObject{{"protocol",1},{"session",session},{"sequence",seq},
            {"sim_time",t.simTime},{"sent_ms",sent},{"interval_ms",interval},{"stopped",object["stopped"]}}},
            {"sample_id",double(t.sequence)},{"received_ms",double(t.received)},{"aircraft_key",t.aircraft},{"sim_time_advanced",t.advanced}};
        dcsSequence=quint64(seq);dcsSent=qint64(sent);dcsPort=packet.senderPort();lastResponse=now;previous=t;result<<t;
    }
    return result;
}
}
