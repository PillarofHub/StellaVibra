#include "network_telemetry.h"
#include "game_catalog.h"
#include <cstring>
namespace sv {
NetworkTelemetry::NetworkTelemetry(QString target):game(std::move(target)) {
    if(game=="WarThunder") {http=std::make_unique<QNetworkAccessManager>();http->setProxy(QNetworkProxy::NoProxy);}
    else {udp=std::make_unique<QUdpSocket>();require(udp->bind(QHostAddress::LocalHost,game=="DCS"?43891:0,QUdpSocket::DontShareAddress),game=="DCS"?"Cannot bind DCS UDP port 43891 (already in use?)":"Cannot bind X-Plane UDP socket");}
}
NetworkTelemetry::~NetworkTelemetry() {
    for(auto*& reply:replies)if(reply){reply->abort();delete reply;reply=nullptr;}
    if(udp && game=="XPlane12")for(int i=0;i<xplaneVariables().size();++i)sendSubscription(i,0);
}
bool NetworkTelemetry::connected() const {return lastResponse && nowMs()-lastResponse<1500;}
QList<Telemetry> NetworkTelemetry::poll() {
    events.processEvents(QEventLoop::AllEvents);
    auto result=game=="WarThunder"?pollWarThunder():game=="DCS"?pollDcs():pollXPlane();
    status=!connected()?"Disconnected / retrying":previous.paused?"Connected / inactive or paused":previous.valid(nowMs())?"Connected":"Connected / waiting for fresh telemetry";
    return result;
}
QList<Telemetry> NetworkTelemetry::pollWarThunder() {
    const auto now=nowMs();QList<Telemetry> result;
    if(replies[0] && ((replies[0]->isFinished() && replies[1]->isFinished()) || now-cycleStarted>=350)) {
        QJsonObject bodies[2];bool ok=true;
        for(int i=0;i<2;++i){auto* reply=replies[i];QJsonParseError error;
            if(!reply->isFinished() || reply->error()!=QNetworkReply::NoError || reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt()!=200 || reply->bytesAvailable()>1024*1024)ok=false;
            else {auto doc=QJsonDocument::fromJson(reply->readAll(),&error);if(error.error!=QJsonParseError::NoError || !doc.isObject())ok=false;else bodies[i]=doc.object();}
            reply->abort();delete reply;replies[i]=nullptr;
        }
        if(ok){
            lastResponse=now;Telemetry t;t.game=game;t.sequence=previous.sequence+1;t.received=now;
            t.values={{"state",bodies[0]},{"indicators",bodies[1]}};
            t.aircraft=bodies[1]["type"].toString();
            t.paused=!bodies[0]["valid"].toBool() || (bodies[1].contains("valid") && !bodies[1]["valid"].toBool());
            // WT exposes no reliable simulation clock/pause flag. Conservatively expire
            // unchanged payloads; polling or replay alone must not manufacture progress.
            t.advanced=!t.paused && t.values!=previous.values;
            t.advancedAt=t.advanced?now:previous.advancedAt;
            previous=t;result<<t;nextPoll=cycleStarted+50;
        } else nextPoll=now+1000;
    }
    if(!replies[0] && now>=nextPoll) {
        cycleStarted=now;
        for(int i=0;i<2;++i){QNetworkRequest request(QUrl(i?"http://127.0.0.1:8111/indicators":"http://127.0.0.1:8111/state"));
            request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,QNetworkRequest::ManualRedirectPolicy);request.setTransferTimeout(350);request.setRawHeader("Cache-Control","no-cache");
            replies[i]=http->get(request);replies[i]->setReadBufferSize(1024*1024+1);
        }
    }
    return result;
}
void NetworkTelemetry::sendSubscription(int index,int frequency) {
    QByteArray packet(413,'\0');memcpy(packet.data(),"RREF",4);
    const qint32 hz=frequency,id=index;memcpy(packet.data()+5,&hz,4);memcpy(packet.data()+9,&id,4);
    auto name=xplaneVariables()[index].name.toUtf8();memcpy(packet.data()+13,name.constData(),size_t(name.size()));
    udp->writeDatagram(packet,QHostAddress::LocalHost,49000);
}
QList<Telemetry> NetworkTelemetry::pollXPlane() {
    const auto now=nowMs();const auto& fields=xplaneVariables();bool clockReceived=false;
    // Retry the known catalog in small batches, including after the simulator restarts.
    if(now>=nextSubscribe){for(int n=0;n<10 && subscribeIndex<fields.size();++n,++subscribeIndex)sendSubscription(subscribeIndex,fields[subscribeIndex].frequency);
        if(subscribeIndex>=fields.size()){subscribeIndex=0;nextSubscribe=now+2000;}else nextSubscribe=now+5;
    }
    for(int n=0;n<64 && udp->hasPendingDatagrams();++n){auto datagram=udp->receiveDatagram(8192);auto bytes=datagram.data();
        if(datagram.senderAddress()!=QHostAddress::LocalHost || datagram.senderPort()!=49000 || bytes.size()<5 || bytes.left(4)!="RREF" || (bytes.size()-5)%8)continue;
        for(int offset=5;offset+8<=bytes.size();offset+=8){qint32 index;float value;memcpy(&index,bytes.constData()+offset,4);memcpy(&value,bytes.constData()+offset+4,4);
            if(index<0 || index>=fields.size() || !std::isfinite(value))continue;
            const auto& name=fields[index].name;datarefs[name]=double(value);updated[name]=now;lastResponse=now;
            if(name=="sim/network/misc/network_time_sec")clockReceived=true;
        }
    }
    if(!clockReceived)return {};
    Telemetry t;t.game=game;t.sequence=previous.sequence+1;t.received=now;
    for(const auto& field:fields)if(updated.contains(field.name) && now-updated[field.name]<(field.frequency==1?2500:500))t.values[field.name]=datarefs[field.name];
    const auto clock=QString("sim/network/misc/network_time_sec"),pause=QString("sim/time/paused");
    t.simTime=t.values[clock].toDouble();t.paused=!t.values.contains(pause) || t.values[pause].toDouble()!=0;
    QByteArray title;
    for(int i=0;i<40;++i){auto name=QString("sim/aircraft/view/acf_descrip[%1]").arg(i);if(!t.values.contains(name)){title.clear();break;}auto v=t.values[name].toInt();if(v<0 || v>255){title.clear();break;}title.append(char(v));}
    auto end=title.indexOf('\0');if(end>=0)title.truncate(end);t.aircraft=QString::fromUtf8(title).trimmed();
    t.advanced=t.simTime>previous.simTime;t.advancedAt=t.advanced?now:previous.advancedAt;
    previous=t;return {t};
}
}
