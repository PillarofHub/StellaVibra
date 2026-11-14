#include "dcs_install.h"
namespace sv {
namespace {
const QString folder="StellaVibra Telemetry";
const QByteArray loaderLine="pcall(dofile, lfs.writedir() .. [[Scripts/StellaVibra Telemetry/loader.lua]]) -- StellaVibra Telemetry";
const QStringList owned={"loader.lua","telemetry.lua","config.lua","manifest.json"};
QString managed(const QString& scripts){return QDir(scripts).filePath(folder);}
QByteArray read(const QString& path) {
    QFile f(path);require(f.open(QIODevice::ReadOnly),"Cannot read "+path);
    require(f.size()<=2*1024*1024,"File exceeds 2 MiB: "+path);
    auto bytes=f.readAll();require(f.error()==QFileDevice::NoError,"Cannot read "+path);return bytes;
}
QString hash(const QByteArray& bytes){return QString::fromLatin1(QCryptographicHash::hash(bytes,QCryptographicHash::Sha256).toHex());}
void write(const QString& path,const QByteArray& bytes) {
    QSaveFile f(path);require(f.open(QIODevice::WriteOnly),"Cannot write "+path);
    require(f.write(bytes)==bytes.size() && f.commit(),"Cannot save "+path);
}
void safePaths(const QString& scripts) {
    require(QFileInfo(scripts).isAbsolute() && QFileInfo(scripts).fileName().compare("Scripts",Qt::CaseInsensitive)==0,"Invalid DCS Scripts path");
    require(QFileInfo(QFileInfo(scripts).absolutePath()).isDir(),"DCS profile no longer exists");
    for(const auto& path:QStringList{scripts,managed(scripts)}){
        QFileInfo info(path);require(!info.isSymLink() && !info.isJunction() && (!info.exists() || info.isDir()),"Invalid or linked directory: "+path);
    }
    QStringList files{QDir(scripts).filePath("Export.lua")};
    for(const auto& name:owned)files<<managed(scripts)+"/"+name;
    for(const auto& path:files){QFileInfo info(path);require(!info.isSymLink() && !info.isJunction() && (!info.exists() || info.isFile()),"Invalid or linked file: "+path);}
}
QMap<QString,QByteArray> bundle(int interval) {
    require(interval>=10 && interval<=100,"Update interval must be 10..100 ms");
    return {{"loader.lua",read(appRoot()+"/dcs/loader.lua")},{"telemetry.lua",read(appRoot()+"/dcs/telemetry.lua")},
        {"config.lua",QByteArray("-- Managed by Stella Vibra. Apply changes after restarting DCS.\nreturn { interval_ms = ")+QByteArray::number(interval)+" }\n"}};
}
int configInterval(QByteArray bytes) {
    // Parse the complete, small generated format without executing Lua.
    // Editors may change LF to CRLF; only the decimal interval is otherwise editable.
    bytes.replace("\r\n","\n");
    static const QRegularExpression format(QStringLiteral(
        "\\A-- Managed by Stella Vibra\\. Apply changes after restarting DCS\\.\\n"
        "return \\{ interval_ms = ([0-9]{1,3}) \\}\\n\\z"));
    const auto match=format.match(QString::fromLatin1(bytes));
    if(!match.hasMatch())return 0;
    const int interval=match.captured(1).toInt();return interval>=10 && interval<=100?interval:0;
}
struct Line {qsizetype start=0,length=0;QByteArray text,ending;bool exact=false;};
struct ExportFile {bool exists=false;QByteArray bytes,newline="\n";QList<Line> lines;};
ExportFile exportFile(const QString& scripts) {
    const auto path=QDir(scripts).filePath("Export.lua");ExportFile f;f.exists=QFileInfo::exists(path);
    if(!f.exists)return f;
    f.bytes=read(path);require(f.bytes.size()<=1024*1024,"Export.lua exceeds 1 MiB");
    require(!f.bytes.contains('\0') && !f.bytes.startsWith(QByteArray::fromHex("fffe")) && !f.bytes.startsWith(QByteArray::fromHex("feff")),
        "Export.lua encoding is not ASCII-compatible; automatic editing is unavailable");
    bool foundEnding=false;
    for(qsizetype pos=0;pos<f.bytes.size();) {
        auto end=pos;while(end<f.bytes.size() && f.bytes[end]!='\r' && f.bytes[end]!='\n')++end;
        auto next=end;if(next<f.bytes.size()){if(f.bytes[next++]=='\r' && next<f.bytes.size() && f.bytes[next]=='\n')++next;}
        const auto ending=f.bytes.mid(end,next-end);
        if(!foundEnding && !ending.isEmpty()){f.newline=ending;foundEnding=true;}
        const qsizetype bom=pos==0 && f.bytes.startsWith(QByteArray::fromHex("efbbbf"))?3:0;
        const auto text=f.bytes.mid(pos+bom,end-pos-bom);
        if(text.toLower().contains("stellavibra") || text.toLower().contains("stella vibra"))
            f.lines<<Line{pos+bom,next-pos-bom,text,ending,text==loaderLine};
        pos=next;
    }
    return f;
}
QByteArray editExport(const ExportFile& f,bool remove) {
    auto result=f.bytes;
    // Work backwards so offsets remain valid. All other bytes stay untouched.
    for(qsizetype i=f.lines.size();i>0;--i){
        const auto& line=f.lines[i-1];
        result.replace(line.start,line.length,!remove && i==1?loaderLine+line.ending:QByteArray());
    }
    if(!remove && f.lines.isEmpty()){
        if(!result.isEmpty() && !result.endsWith('\n') && !result.endsWith('\r'))result+=f.newline;
        result+=loaderLine+f.newline;
    }
    return result;
}
void sameExport(const QString& scripts,const ExportFile& before) {
    const auto latest=exportFile(scripts);
    require(latest.exists==before.exists && latest.bytes==before.bytes,"Export.lua changed during this operation. Reopen the dialog and retry.");
}
}
DcsInstallation inspectDcsInstallation(const QString& scripts) {
    DcsInstallation state;
    try {
        safePaths(scripts);const auto f=exportFile(scripts);
        state.canRemove=!f.lines.isEmpty() || QFileInfo::exists(managed(scripts));
        QStringList details;QByteArray fingerprint=f.exists?"exists:":"absent:";fingerprint+=hash(f.bytes).toUtf8();
        details<<"Export.lua lines containing Stella Vibra:";
        for(const auto& line:f.lines)details<<QString::fromUtf8(line.text);
        if(f.lines.isEmpty())details<<"(none)";
        const bool exact=f.lines.size()==1 && f.lines[0].exact;
        QMap<QString,QByteArray> contents;bool filesPresent=false;
        for(const auto& name:owned){
            const auto path=managed(scripts)+"/"+name;
            if(QFileInfo::exists(path)){contents[name]=read(path);filesPresent=true;fingerprint+=name.toUtf8()+hash(contents[name]).toUtf8();}
            else fingerprint+=name.toUtf8()+":missing";
        }
        state.fingerprint=hash(fingerprint);
        const int configuredInterval=configInterval(contents.value("config.lua"));
        if(configuredInterval)state.interval=configuredInterval;
        bool intact=false;
        if(filesPresent){
            QJsonParseError error;const auto doc=QJsonDocument::fromJson(contents.value("manifest.json"),&error);const auto m=doc.object();
            if(error.error==QJsonParseError::NoError && doc.isObject() && m["owner"]==QJsonValue("StellaVibra Telemetry")
                && (m["schema"]==QJsonValue(1) || m["schema"]==QJsonValue(2))){
                intact=configuredInterval!=0;
                for(const auto& name:QStringList{"loader.lua","telemetry.lua"})
                    if(!contents.contains(name) || m["hashes"].toObject()[name].toString()!=hash(contents[name]))intact=false;
            }
        }
        state.needsRestore=(!f.lines.isEmpty() && !exact) || (filesPresent && !intact) || (exact && !intact);
        if(f.lines.isEmpty()){state.canAdd=true;state.text="Loader not found";}
        else if(exact && intact){state.canUpdate=true;state.text="Installed — files verified";}
        else {state.canRestore=true;state.text="Stella Vibra scripts need restoration";}
        if(state.canUpdate){
            const auto current=bundle(state.interval);
            for(auto it=current.begin();it!=current.end();++it)if(it.key()!="config.lua" && contents.value(it.key())!=it.value()){state.text="Installed — script update available";break;}
        }
        details<<"\nManaged folder: "+QDir::toNativeSeparators(managed(scripts));
        state.details=details.join('\n');
    }catch(const std::exception& e){
        state.canAdd=state.canUpdate=state.canRestore=state.canRemove=false;
        state.text=QString("Cannot manage installation: ")+e.what();
    }
    return state;
}
void installDcsTelemetry(const QString& scripts,int interval,const std::function<bool(const QString&)>& confirmRestore) {
    const auto state=inspectDcsInstallation(scripts);require(state.canAdd || state.canUpdate || state.canRestore,state.text);
    const auto files=bundle(interval);
    if(state.needsRestore && !confirmRestore(state.details))return;
    require(inspectDcsInstallation(scripts).fingerprint==state.fingerprint,"Installation changed while confirming. Reopen the dialog and retry.");
    safePaths(scripts);const auto before=exportFile(scripts);const auto after=editExport(before,false);
    QMap<QString,QByteArray> previous;QSet<QString> absent;
    for(const auto& name:owned){const auto path=managed(scripts)+"/"+name;if(QFileInfo::exists(path))previous[path]=read(path);else absent.insert(path);}
    const bool folderExisted=QFileInfo::exists(managed(scripts));
    require(QDir().mkpath(managed(scripts)),"Cannot create Stella Vibra Telemetry folder");
    try {
        QJsonObject hashes;for(auto it=files.begin();it!=files.end();++it){
            const auto path=managed(scripts)+"/"+it.key();
            if(!previous.contains(path) || previous[path]!=it.value())write(path,it.value());
            if(it.key()!="config.lua")hashes[it.key()]=hash(it.value());
        }
        const QJsonObject m{{"owner","StellaVibra Telemetry"},{"schema",2},{"hashes",hashes},{"interval_ms",interval},{"relay_version",2}};
        write(managed(scripts)+"/manifest.json",QJsonDocument(m).toJson());
        sameExport(scripts,before);
        if(after!=before.bytes || !before.exists)write(QDir(scripts).filePath("Export.lua"),after);
    }catch(const std::exception& e){
        const auto reason=QString::fromUtf8(e.what());bool restored=true;
        // Roll back only our files. Never restore an earlier shared Export.lua.
        for(auto it=previous.begin();it!=previous.end();++it)try{write(it.key(),it.value());}catch(...){restored=false;}
        for(const auto& path:absent)if(QFileInfo::exists(path) && !QFile::remove(path))restored=false;
        if(!folderExisted)QDir().rmdir(managed(scripts));
        require(false,reason+(restored?"":"\nManaged-file rollback incomplete. Use Restore Scripts."));
    }
}
void removeDcsTelemetry(const QString& scripts,const std::function<bool(const QString&)>& confirmRemove) {
    const auto state=inspectDcsInstallation(scripts);require(state.canRemove,state.text);
    if(!confirmRemove(state.details))return;
    require(inspectDcsInstallation(scripts).fingerprint==state.fingerprint,"Installation changed while confirming. Reopen the dialog and retry.");
    safePaths(scripts);
    const auto directory=managed(scripts);const auto root=QFileInfo(directory).canonicalFilePath();
    if(QFileInfo::exists(directory)){
        require(!root.isEmpty() && QFileInfo(root).absolutePath().compare(QFileInfo(scripts).canonicalFilePath(),Qt::CaseInsensitive)==0,"Unexpected telemetry directory");
        QDirIterator entries(directory,QDir::AllEntries|QDir::Hidden|QDir::System|QDir::NoDotAndDotDot,QDirIterator::Subdirectories);
        while(entries.hasNext()){const QFileInfo info(entries.next());const auto path=info.canonicalFilePath();
            require(!info.isSymLink() && !info.isJunction() && !path.isEmpty() && path.startsWith(root+"/",Qt::CaseInsensitive),"Linked or invalid content prevents removal: "+info.filePath());
        }
    }
    const auto before=exportFile(scripts);const auto after=editExport(before,true);sameExport(scripts,before);
    const auto exportPath=QDir(scripts).filePath("Export.lua");
    if(before.exists && after.isEmpty())require(QFile::remove(exportPath),"Cannot remove empty Export.lua");
    else if(after!=before.bytes)write(exportPath,after);
    if(QFileInfo::exists(directory))require(QDir(directory).removeRecursively(),"Loader removed, but some telemetry files could not be removed. Retry Remove Scripts.");
}
}
