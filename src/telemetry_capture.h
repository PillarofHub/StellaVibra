#pragma once
#include "telemetry.h"
#include <mutex>

namespace sv {
// Temporary, opt-in capture. Disk writes belong to the UI, never the HID loop.
class TelemetryCapture {
    std::mutex mutex;
    bool enabled=false;
    qint64 started=0;
    quint64 samples=0,dropped=0;
    QString context;
    QJsonObject previous;
    bool previousValid=false;
    QByteArray pending;
    void append(const QByteArray& line) {
        if(pending.size()+line.size()>256*1024){++dropped;return;}
        pending+=line;
    }
    static void changes(const QJsonObject& before,const QJsonObject& after,const QString& prefix,QJsonObject& set,QJsonArray& removed) {
        auto pointer=[](QString key){return key.replace("~","~0").replace("/","~1");};
        for(auto it=before.begin();it!=before.end();++it)if(!after.contains(it.key()))removed.append(prefix+"/"+pointer(it.key()));
        for(auto it=after.begin();it!=after.end();++it){
            const auto path=prefix+"/"+pointer(it.key());const auto old=before.value(it.key());
            if(before.contains(it.key()) && old==it.value())continue;
            if(old.isObject() && it.value().isObject())changes(old.toObject(),it.value().toObject(),path,set,removed);
            else set[path]=it.value();
        }
    }
public:
    void start() {
        std::lock_guard lock(mutex);enabled=true;started=nowMs();samples=dropped=0;context.clear();previous={};previousValid=false;pending.clear();
    }
    void stop() {
        std::lock_guard lock(mutex);if(!enabled)return;enabled=false;
        append("#end ms="+QByteArray::number(nowMs()-started)+" samples="+QByteArray::number(samples)+"\n");
    }
    QByteArray take() {
        std::lock_guard lock(mutex);auto result=std::move(pending);pending.clear();
        if(dropped){result+="#dropped="+QByteArray::number(dropped)+"\n";dropped=0;context.clear();}return result;
    }
    void record(const Telemetry& t,bool fresh,bool valid) {
        std::lock_guard lock(mutex);if(!enabled)return;if(fresh)++samples;
        const auto meta=QJsonDocument(QJsonObject{{"game",t.game},{"aircraft",t.aircraft},{"session",t.session}}).toJson(QJsonDocument::Compact);
        const auto values=telemetryObject(t);
        QJsonObject row{{"t",double(nowMs()-started)},{"v",valid}};
        if(context!=QString::fromUtf8(meta)){
            context=QString::fromUtf8(meta);append("#"+meta+"\n");row["f"]=values;
        } else {
            QJsonObject set;QJsonArray removed;changes(previous,values,QString(),set,removed);
            if(set.isEmpty() && removed.isEmpty() && valid==previousValid)return;
            if(!set.isEmpty())row["d"]=set;
            if(!removed.isEmpty())row["x"]=removed;
        }
        append(QJsonDocument(row).toJson(QJsonDocument::Compact)+"\n");previous=values;previousValid=valid;
    }
};
}
