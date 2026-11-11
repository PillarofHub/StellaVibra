#include "game_catalog.h"
#include "model.h"
#include <cmath>
namespace sv {
QJsonObject readJson(const QString& path) {
    QFile file(path); require(file.open(QIODevice::ReadOnly), "Cannot read "+path);
    require(file.size()<=4*1024*1024,"JSON file too large: "+path);
    QJsonParseError error; auto doc=QJsonDocument::fromJson(file.readAll(),&error);
    require(error.error==QJsonParseError::NoError && doc.isObject(),"Invalid JSON: "+path+": "+error.errorString());
    auto o=doc.object(); require(o.value("version")==QJsonValue(1),"Unsupported file version: "+path); return o;
}
void writeJson(const QString& path,const QJsonObject& object) {
    require(QDir().mkpath(QFileInfo(path).absolutePath()),"Cannot create directory");
    QSaveFile file(path); require(file.open(QIODevice::WriteOnly),"Cannot write "+path);
    auto bytes=QJsonDocument(object).toJson(); require(file.write(bytes)==bytes.size() && file.commit(),"Cannot commit "+path);
}
QStringList logicalSlots() { QStringList result; for(auto c:{"Yoke","Stick","Throttle","Pedal","Wheel"}) for(int i=1;i<=4;++i) result << QString("%1:%2").arg(c).arg(i); return result; }
QStringList availableSlots(const QJsonObject& counts) {
    QStringList result;for(const auto& slot:logicalSlots())if(slot.section(':',1).toInt()<=std::clamp(counts[slot.section(':',0,0)].toInt(4),0,4))result<<slot;return result;
}
QJsonObject fitOutputDefaults(const QJsonObject& outputs,const QJsonObject& counts) {
    QJsonObject result;
    for(auto it=outputs.begin();it!=outputs.end();++it){QJsonArray destinations;
        for(const auto& value:it.value().toArray()){auto slot=value.toString();auto category=slot.section(':',0,0);int count=std::clamp(counts[category].toInt(4),0,4);
            if(count){auto fitted=category+":"+QString::number((slot.section(':',1).toInt()-1)%count+1);if(!destinations.contains(fitted))destinations<<fitted;}}
        result[it.key()]=destinations;
    }return result;
}
QString appRoot() { return QDir(QCoreApplication::applicationDirPath()).dirName().compare("bin",Qt::CaseInsensitive)==0?QDir::cleanPath(QCoreApplication::applicationDirPath()+"/.."):QCoreApplication::applicationDirPath(); }
// Preferences belong to this executable directory, independent of the working
// directory and Windows user profile. Do not fall back to machine-wide storage.
QString localPath() { return appRoot()+"/StellaVibra.pref"; }
QString scriptPath(const QString& project,const QString& relative) {
    require(!relative.isEmpty() && QDir::isRelativePath(relative),"Script path must be relative");
    auto base=QFileInfo(project).absolutePath(); auto clean=QDir::cleanPath(QDir(base).absoluteFilePath(relative));
    require(clean.startsWith(base+"/",Qt::CaseInsensitive),"Script path escapes project");
    auto canonical=QFileInfo(clean).canonicalFilePath(); auto root=QFileInfo(base).canonicalFilePath();
    if(canonical.isEmpty()) {
        auto parent=QFileInfo(clean).absolutePath();
        while(!QFileInfo::exists(parent)) {auto next=QFileInfo(parent).absolutePath();if(next==parent)break;parent=next;}
        auto ancestor=QFileInfo(parent).canonicalFilePath();
        require(ancestor==root || ancestor.startsWith(root+"/",Qt::CaseInsensitive),"Script path escapes project");
        return clean;
    }
    require(canonical.startsWith(root+"/",Qt::CaseInsensitive),"Symlink outside project: "+relative); return canonical;
}
void validateSettings(const QJsonObject& o,bool partial) {
    Q_UNUSED(partial);require(!o.contains("enabled") || o["enabled"].isBool(),"Invalid enabled value");
    for(auto field:{"parameters","outputs"}) require(!o.contains(field) || o[field].isObject(),QString("Invalid ")+field);
    auto params=o["parameters"].toObject(); for(auto it=params.begin();it!=params.end();++it) {
        auto v=it.value();
        if(v.isObject()) {
            auto h=v.toObject(); require(h.size()==4 && h["pwm"].isBool(),"Invalid haptic_power value");
            for(auto k:{"strength","on_ms","period_ms"}) require(h[k].isDouble() && std::isfinite(h[k].toDouble()) && floor(h[k].toDouble())==h[k].toDouble(),"Invalid haptic_power number");
            require(h["strength"].toDouble()>=0 && h["strength"].toDouble()<=255 && h["on_ms"].toDouble()>=0 && h["period_ms"].toDouble()>=20 && h["period_ms"].toDouble()<=60000 && h["on_ms"].toDouble()<=h["period_ms"].toDouble(),"Invalid haptic_power range");
        } else require(v.isBool() || (v.isString() && !v.toString().isEmpty() && v.toString().size()<=512) || (v.isDouble() && std::isfinite(v.toDouble())),"Invalid parameter value");
    }
    auto outputs=o["outputs"].toObject(); for(auto it=outputs.begin();it!=outputs.end();++it) {
        require(it.value().isArray(),"Output destinations must be an array"); QSet<QString> used;
        for(auto v:it.value().toArray()) { require(v.isString() && logicalSlots().contains(v.toString()) && !used.contains(v.toString()),"Invalid or duplicate slot"); used.insert(v.toString()); }
    }
}
void Project::load(const QString& file,const QMap<QString,QJsonObject>& replacements) {
    auto read=[&](const QString& path){auto absolute=QFileInfo(path).absoluteFilePath();QJsonObject o;bool replaced=false;for(auto it=replacements.begin();it!=replacements.end();++it)if(it.key().compare(absolute,Qt::CaseInsensitive)==0){o=it.value();replaced=true;break;}if(!replaced)o=readJson(path);require(o["version"]==1,"Unsupported file version: "+path);return o;};
    Project next; next.path=QFileInfo(file).absoluteFilePath(); next.data=read(file);
    require(next.data["id"].isString() && !next.data["id"].toString().isEmpty() && next.data["name"].isString() && targetGames().contains(next.data["game"].toString()) && next.data["effects"].isArray(),"Invalid project schema");
    require(next.data["effects"].toArray().size()<=64,"At most 64 effects are supported");
    QSet<QString> ids, paths;
    for(auto value:next.data["effects"].toArray()) {
        require(value.isObject(),"Invalid effect"); auto e=value.toObject(); auto id=e["id"].toString();
        require(!id.isEmpty() && !ids.contains(id),"Duplicate or missing effect ID"); ids.insert(id);
        auto source=scriptPath(next.path,e["script"].toString()); require(!paths.contains(source.toLower()),"Duplicate script instance"); paths.insert(source.toLower()); validateSettings(e,false);
    }
    QDir folder(QFileInfo(file).absolutePath()+"/aircrafts"); for(auto name:folder.entryList({"*.sv"},QDir::Files)) {
        auto o=read(folder.filePath(name)); auto key=o["aircraft_key"].toString(); require(!key.isEmpty() && !next.aircraft.contains(key) && o["effects"].isObject(),"Invalid or duplicate aircraft key");
        auto effects=o["effects"].toObject(); for(auto it=effects.begin();it!=effects.end();++it) { require(it.value().isObject(),"Invalid override"); validateSettings(it.value().toObject(),true); }
        next.aircraft[key]=o; next.aircraftPaths[key]=folder.filePath(name);
    }
    *this=std::move(next);
}
void Project::save() { require(!path.isEmpty(),"No project"); writeJson(path,data); for(auto it=aircraft.begin();it!=aircraft.end();++it) writeJson(aircraftPaths[it.key()],it.value()); }
QJsonObject Project::resolved(const QJsonObject& effect,const QString& key) const {
    auto result=effect; auto patch=aircraft.value(key)["effects"].toObject()[effect["id"].toString()].toObject();
    if(!result.contains("enabled"))result["enabled"]=true;
    if(patch.contains("enabled")) result["enabled"]=patch["enabled"];
    for(auto field:{"parameters","outputs"}) { auto merged=result[field].toObject(); auto values=patch[field].toObject(); for(auto it=values.begin();it!=values.end();++it) merged[it.key()]=it.value(); result[field]=merged; } return result;
}
void Project::compact(const QString& id,const QJsonObject& defaults) {
    auto trim=[](QJsonObject o,const QJsonObject& base){
        if(o.contains("enabled") && o["enabled"]==base["enabled"])o.remove("enabled");
        for(auto group:{"parameters","outputs"}){auto values=o[group].toObject();auto inherited=base[group].toObject();for(auto key:values.keys())if(inherited.contains(key)&&values[key]==inherited[key])values.remove(key);if(values.isEmpty())o.remove(group);else o[group]=values;}return o;
    };
    auto entries=data["effects"].toArray();QJsonObject global;
    for(int i=0;i<entries.size();++i)if(entries[i].toObject()["id"]==id){global=trim(entries[i].toObject(),defaults);entries[i]=global;break;}
    if(global.isEmpty())return;data["effects"]=entries;
    auto base=defaults;if(global.contains("enabled"))base["enabled"]=global["enabled"];
    for(auto group:{"parameters","outputs"}){auto values=base[group].toObject();auto stored=global[group].toObject();for(auto it=stored.begin();it!=stored.end();++it)values[it.key()]=it.value();base[group]=values;}
    for(auto key:aircraft.keys()){auto patch=aircraft[key]["effects"].toObject()[id].toObject();if(!patch.isEmpty())setOverride(key,id,trim(patch,base));}
}
void Project::setOverride(const QString& key,const QString& id,const QJsonObject& value) {
    if(!aircraft.contains(key)) { aircraft[key]=QJsonObject{{"version",1},{"aircraft_key",key},{"effects",QJsonObject{}}}; if(!aircraftPaths.contains(key))aircraftPaths[key]=QFileInfo(path).absolutePath()+"/aircrafts/"+QUuid::createUuid().toString(QUuid::WithoutBraces)+".sv"; }
    auto o=aircraft[key]; auto effects=o["effects"].toObject(); if(value.isEmpty()) effects.remove(id); else effects[id]=value; o["effects"]=effects; aircraft[key]=o;
}
}
