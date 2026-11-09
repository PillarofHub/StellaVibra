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
QString localPath() { return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)+"/local.json"; }
QString scriptPath(const QString& project,const QString& relative) {
    require(!relative.isEmpty() && QDir::isRelativePath(relative),"Script path must be relative");
    auto base=QFileInfo(project).absolutePath(); auto clean=QDir::cleanPath(QDir(base).absoluteFilePath(relative));
    require(clean.startsWith(base+"/",Qt::CaseInsensitive),"Script path escapes project");
    auto canonical=QFileInfo(clean).canonicalFilePath(); auto root=QFileInfo(base).canonicalFilePath();
    require(!canonical.isEmpty() && canonical.startsWith(root+"/",Qt::CaseInsensitive),"Missing script or symlink outside project: "+relative); return canonical;
}
void validateSettings(const QJsonObject& o,bool partial) {
    require((partial && !o.contains("enabled")) || o["enabled"].isBool(),"Invalid enabled value");
    for(auto field:{"parameters","outputs"}) require(!o.contains(field) || o[field].isObject(),QString("Invalid ")+field);
    auto params=o["parameters"].toObject(); for(auto it=params.begin();it!=params.end();++it) require(it.value().isBool() || (it.value().isDouble() && std::isfinite(it.value().toDouble())),"Invalid parameter value");
    auto outputs=o["outputs"].toObject(); for(auto it=outputs.begin();it!=outputs.end();++it) {
        require(it.value().isArray(),"Output destinations must be an array"); QSet<QString> used;
        for(auto v:it.value().toArray()) { require(v.isString() && logicalSlots().contains(v.toString()) && !used.contains(v.toString()),"Invalid or duplicate slot"); used.insert(v.toString()); }
    }
}
void Project::load(const QString& file) {
    Project next; next.path=QFileInfo(file).absoluteFilePath(); next.data=readJson(file);
    require(next.data["id"].isString() && !next.data["id"].toString().isEmpty() && next.data["name"].isString() && next.data["game"]=="MSFS2024" && next.data["effects"].isArray(),"Invalid project schema");
    require(next.data["effects"].toArray().size()<=64,"At most 64 effects are supported");
    QSet<QString> ids, paths;
    for(auto value:next.data["effects"].toArray()) {
        require(value.isObject(),"Invalid effect"); auto e=value.toObject(); auto id=e["id"].toString();
        require(!id.isEmpty() && !ids.contains(id),"Duplicate or missing effect ID"); ids.insert(id);
        auto source=scriptPath(next.path,e["script"].toString()); require(!paths.contains(source.toLower()),"Duplicate script instance"); paths.insert(source.toLower()); validateSettings(e,false);
    }
    QDir folder(QFileInfo(file).absolutePath()+"/aircrafts"); for(auto name:folder.entryList({"*.sv"},QDir::Files)) {
        auto o=readJson(folder.filePath(name)); auto key=o["aircraft_key"].toString(); require(!key.isEmpty() && !next.aircraft.contains(key) && o["effects"].isObject(),"Invalid or duplicate aircraft key");
        auto effects=o["effects"].toObject(); for(auto it=effects.begin();it!=effects.end();++it) { require(it.value().isObject(),"Invalid override"); validateSettings(it.value().toObject(),true); }
        next.aircraft[key]=o; next.aircraftPaths[key]=folder.filePath(name);
    }
    *this=std::move(next);
}
void Project::save() { require(!path.isEmpty(),"No project"); writeJson(path,data); for(auto it=aircraft.begin();it!=aircraft.end();++it) writeJson(aircraftPaths[it.key()],it.value()); }
QJsonObject Project::resolved(const QJsonObject& effect,const QString& key) const {
    auto result=effect; auto patch=aircraft.value(key)["effects"].toObject()[effect["id"].toString()].toObject();
    if(patch.contains("enabled")) result["enabled"]=patch["enabled"];
    for(auto field:{"parameters","outputs"}) { auto merged=result[field].toObject(); auto values=patch[field].toObject(); for(auto it=values.begin();it!=values.end();++it) merged[it.key()]=it.value(); result[field]=merged; } return result;
}
void Project::setOverride(const QString& key,const QString& id,const QJsonObject& value) {
    if(!aircraft.contains(key)) { aircraft[key]=QJsonObject{{"version",1},{"aircraft_key",key},{"effects",QJsonObject{}}}; aircraftPaths[key]=QFileInfo(path).absolutePath()+"/aircrafts/"+QUuid::createUuid().toString(QUuid::WithoutBraces)+".sv"; }
    auto o=aircraft[key]; auto effects=o["effects"].toObject(); if(value.isEmpty()) effects.remove(id); else effects[id]=value; o["effects"]=effects; aircraft[key]=o;
}
}
