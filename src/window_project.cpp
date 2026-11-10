#include "window.h"

namespace sv {
static QString sourceText(const QString& path) {
    QFile f(path); require(f.open(QIODevice::ReadOnly),"Cannot read "+path);
    require(f.size()<=(path.endsWith(".sv",Qt::CaseInsensitive)?4*1024*1024:256*1024),"Script too large: "+path);return QString::fromUtf8(f.readAll());
}
static void saveSource(const QString& path,const QString& text) {
    require(QDir().mkpath(QFileInfo(path).absolutePath()),"Cannot create script directory");
    QSaveFile f(path); require(f.open(QIODevice::WriteOnly),"Cannot save "+path);
    auto b=text.toUtf8();require(f.write(b)==b.size() && f.commit(),"Cannot commit script");
}
void Window::loadProject(const QString& path) {
    Project next;next.load(path);engine.clearProject();project=std::move(next);savedProject=project;
    drafts.clear();dirtyEffects.clear();diskHashes.clear();effectDefaults.clear();selectedId.clear();selectedScript.clear();dirty=false;
    refreshing=true;clearTabs();profile->clear();profile->addItem("Global",QString());
    for(auto key:project.aircraft.keys())profile->addItem("Specific Aircraft: "+key,key);refreshing=false;
    files->setRootPath(QFileInfo(path).absolutePath());tree->setRootIndex(files->index(QFileInfo(path).absolutePath()));tree->setEnabled(true);
    setWindowTitle(project.data["name"].toString()+" — Stella Vibra[*]");setWindowModified(false);
    local["last_project"]=project.path;QJsonArray recent{project.path};
    for(auto v:local["recent_projects"].toArray())if(v.toString()!=project.path && recent.size()<10)recent<<v;
    local["recent_projects"]=recent;updateRecent();saveLocal();syncFiles();refreshExplorer();apply();rebuildParameters();
}
void Window::save() {
    require(!project.path.isEmpty() && !selectedScript.isEmpty(),"Select a file to save");if(selectedScript.endsWith(".sv",Qt::CaseInsensitive)){if(drafts.contains(selectedScript))saveRaw({selectedScript});return;}
    bool sourceSaved=drafts.contains(selectedScript);
    if(sourceSaved){effectDefaults.remove(selectedId);saveSource(scriptPath(project.path,selectedScript),drafts[selectedScript]);drafts.remove(selectedScript);if(editor)editor->setProperty("savedText",editor->toPlainText());}
    refreshDefaults();
    Project next=savedProject;auto array=next.data["effects"].toArray();
    for(int i=0;i<array.size();++i)if(array[i].toObject()["id"]==selectedId)array[i]=selectedEffect();next.data["effects"]=array;
    for(auto it=project.aircraft.begin();it!=project.aircraft.end();++it)if(!selectedId.isEmpty()) {
        if(project.aircraftPaths.contains(it.key()))next.aircraftPaths[it.key()]=project.aircraftPaths[it.key()];
        next.setOverride(it.key(),selectedId,it.value()["effects"].toObject()[selectedId].toObject());
    }
    if(!hasRawDrafts()){next.save();savedProject=next;}dirtyEffects.remove(selectedId);dirty=!drafts.isEmpty() || !dirtyEffects.isEmpty();setWindowModified(dirty);
    apply(sourceSaved?selectedId:QString());refreshRawTabs();updateTabs();rebuildParameters();addLog("Saved and queued script update: "+selectedScript);
}
void Window::saveAll() {
    if(project.path.isEmpty())return;
    QStringList raw;for(auto path:drafts.keys())if(path.endsWith(".sv",Qt::CaseInsensitive))raw<<path;if(!raw.isEmpty())saveRaw(raw);
    for(auto it=drafts.begin();it!=drafts.end();++it)saveSource(scriptPath(project.path,it.key()),it.value());
    refreshDefaults();
    project.save();savedProject=project;for(int i=0;i<openFiles->count();++i){auto* doc=static_cast<Editor*>(openFiles->widget(i));doc->setProperty("savedText",doc->toPlainText());}drafts.clear();dirtyEffects.clear();dirty=false;setWindowModified(false);apply();refreshRawTabs();updateTabs();rebuildParameters();addLog("Saved all scripts and parameters");
}
void Window::apply(const QString& reloadId) {
    if(project.path.isEmpty())return;Bundle bundle;bundle.project=project;
    for(auto v:project.data["effects"].toArray()) {
        auto e=v.toObject();auto relative=e["script"].toString();auto path=scriptPath(project.path,relative);
        try {if(QFileInfo::exists(path))bundle.sources[relative]=sourceText(path);} catch(const std::exception& error){addLog(error.what());}
        if(e["id"]==reloadId)bundle.reloadIds.insert(reloadId);
    }
    engine.apply(std::move(bundle));
}
int Window::tabIndex(const QString& path) const {for(int i=0;i<openFiles->count();++i)if(tabPath(i)==path)return i;return -1;}
QString Window::tabPath(int index) const {auto* w=openFiles->widget(index);return w?w->property("path").toString():QString();}
void Window::removeTab(int index){if(index<0)return;auto path=tabPath(index);auto* w=openFiles->widget(index);openFiles->removeWidget(w);fileTabs->removeTab(index);delete w;rawBases.remove(path);}
void Window::clearTabs(){while(openFiles->count())removeTab(0);editor=nullptr;rawBases.clear();}
bool Window::hasRawDrafts() const {for(auto path:drafts.keys())if(path.endsWith(".sv",Qt::CaseInsensitive))return true;return false;}
void Window::cacheDefaults(const QString& id,const LuaEffect& meta){QJsonObject values,ports;for(const auto& p:meta.parameters)values[p.name]=p.initial;for(auto name:meta.outputs)ports[name]=QJsonArray{};effectDefaults[id]=QJsonObject{{"enabled",true},{"parameters",values},{"outputs",ports}};}
void Window::refreshDefaults(){
    effectDefaults.clear();
    for(auto v:project.data["effects"].toArray()){auto e=v.toObject();auto relative=e["script"].toString();try{LuaEffect meta(e["id"].toString(),sourceText(scriptPath(project.path,relative)),relative);cacheDefaults(e["id"].toString(),meta);}catch(const std::exception& error){addLog(QString("Default metadata unavailable; settings preserved: ")+error.what());}}
    updateTabs();
}
void Window::updateTabs(){
    for(auto it=effectDefaults.begin();it!=effectDefaults.end();++it){project.compact(it.key(),it.value());savedProject.compact(it.key(),it.value());}
    dirtyEffects.clear();
    for(auto v:project.data["effects"].toArray()){auto e=v.toObject();auto id=e["id"].toString();QJsonObject saved;for(auto old:savedProject.data["effects"].toArray())if(old.toObject()["id"]==id)saved=old.toObject();bool changed=e!=saved;auto keys=project.aircraft.keys();for(auto k:savedProject.aircraft.keys())if(!keys.contains(k))keys<<k;for(auto k:keys)if(project.aircraft.value(k)["effects"].toObject()[id]!=savedProject.aircraft.value(k)["effects"].toObject()[id])changed=true;if(changed)dirtyEffects.insert(id);}
    auto e=selectedEffect();auto patch=project.aircraft.value(profileKey())["effects"].toObject()[selectedId].toObject();
    for(auto* label:parameterHost->findChildren<QLabel*>("origin")){auto group=label->property("group").toString(),name=label->property("key").toString();auto has=[&](QJsonObject o){return group.isEmpty()?o.contains(name):o[group].toObject().contains(name);};label->setText(!profileKey().isEmpty()&&has(patch)?"Specific":has(e)?"Global":"Default");}
    for(int i=0;i<openFiles->count();++i){auto path=tabPath(i);bool modified=drafts.contains(path);for(auto v:project.data["effects"].toArray())if(v.toObject()["script"]==path && dirtyEffects.contains(v.toObject()["id"].toString()))modified=true;
        fileTabs->setTabText(i,QFileInfo(path).fileName()+(modified?" *":""));fileTabs->setTabToolTip(i,path);
        if(path.endsWith(".sv",Qt::CaseInsensitive))static_cast<Editor*>(openFiles->widget(i))->setReadOnly(!dirtyEffects.isEmpty());
    }
    rawNotice->setVisible(selectedScript.endsWith(".sv",Qt::CaseInsensitive));
    rawNotice->setText(!dirtyEffects.isEmpty()?"Auto-generated JSON. Save or discard parameter changes before editing this file.":"Auto-generated JSON — manual edits are discouraged. Save validates and reloads settings. Parameter edits and file tracking wait while JSON has unsaved changes.");
    paramPane->setEnabled(!hasRawDrafts());dirty=!drafts.isEmpty()||!dirtyEffects.isEmpty();setWindowModified(dirty);
}
void Window::activateTab(){
    {QSignalBlocker block(fileTabs);fileTabs->setCurrentIndex(openFiles->currentIndex());}
    editor=static_cast<Editor*>(openFiles->currentWidget());selectedScript=tabPath(openFiles->currentIndex());selectedId.clear();
    for(auto v:project.data["effects"].toArray())if(v.toObject()["script"]==selectedScript)selectedId=v.toObject()["id"].toString();
    updateTabs();rebuildParameters();
}
void Window::showScript(const QString& relative){
    auto path=scriptPath(project.path,relative);int index=tabIndex(relative);
    if(index<0){
        auto text=drafts.contains(relative)?drafts[relative]:sourceText(path);auto* document=new Editor;
        document->setProperty("path",relative);document->document()->setProperty("json",relative.endsWith(".sv",Qt::CaseInsensitive));document->setPlainText(text);
        document->setProperty("savedText",document->toPlainText());
        if(relative.endsWith(".sv",Qt::CaseInsensitive))rawBases[relative]=sourceText(path);
        connect(document,&QPlainTextEdit::textChanged,this,[this,document]{if(refreshing)return;auto path=document->property("path").toString();if(document->toPlainText()==document->property("savedText").toString())drafts.remove(path);else drafts[path]=document->toPlainText();updateTabs();});
        refreshing=true;index=openFiles->addWidget(document);fileTabs->addTab(QFileInfo(relative).fileName());refreshing=false;
    }
    refreshing=true;openFiles->setCurrentIndex(index);refreshing=false;activateTab();
}
void Window::refreshRawTabs(){
    for(int i=0;i<openFiles->count();++i){auto path=tabPath(i);if(!path.endsWith(".sv",Qt::CaseInsensitive)||drafts.contains(path))continue;
        auto absolute=scriptPath(project.path,path);if(!QFileInfo::exists(absolute))continue;auto text=sourceText(absolute);
        auto normalized=text;normalized.replace("\r\n","\n");normalized.replace('\r','\n');
        auto* document=static_cast<Editor*>(openFiles->widget(i));if(document->toPlainText()!=normalized){QSignalBlocker block(document);document->setPlainText(text);}document->setProperty("savedText",document->toPlainText());rawBases[path]=text;
    }
}
void Window::saveRaw(const QStringList& paths){
    require(dirtyEffects.isEmpty(),"Save or discard parameter changes before saving raw JSON");
    QMap<QString,QJsonObject> replacements;
    for(auto path:paths){
        auto absolute=scriptPath(project.path,path);bool managed=absolute.compare(project.path,Qt::CaseInsensitive)==0;
        for(auto file:project.aircraftPaths)if(absolute.compare(file,Qt::CaseInsensitive)==0)managed=true;
        require(managed,"Only the active project and its aircraft .sv files can be saved here");
        require(sourceText(absolute)==rawBases.value(path),"File changed on disk. Close/discard and reopen before saving: "+path);
        auto bytes=drafts.value(path).toUtf8();require(bytes.size()<=4*1024*1024,"JSON exceeds 4 MiB");QJsonParseError error;auto json=QJsonDocument::fromJson(bytes,&error);
        require(error.error==QJsonParseError::NoError && json.isObject(),"Invalid JSON: "+path+": "+error.errorString());replacements[QFileInfo(absolute).absoluteFilePath()]=json.object();
    }
    Project next;next.load(project.path,replacements); // Validate the whole proposed project before writing anything.
    for(auto path:paths){saveSource(scriptPath(project.path,path),drafts.value(path));drafts.remove(path);}
    project=next;savedProject=next;effectDefaults.clear();
    auto key=profileKey();refreshing=true;profile->clear();profile->addItem("Global",QString());for(auto aircraft:project.aircraft.keys())profile->addItem("Specific Aircraft: "+aircraft,aircraft);int index=profile->findData(key);profile->setCurrentIndex(index<0?0:index);refreshing=false;
    setWindowTitle(project.data["name"].toString()+" — Stella Vibra[*]");refreshRawTabs();activateTab();refreshExplorer();apply();addLog("JSON saved and project settings reloaded");
}

void Window::closeFile() {
    if(selectedScript.isEmpty())return;
    if(drafts.contains(selectedScript)||dirtyEffects.contains(selectedId)) {
        auto answer=QMessageBox::question(this,"Unsaved file","Save this file and its settings?",QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel);
        if(answer==QMessageBox::Cancel)return;if(answer==QMessageBox::Save)save();
        else {
            drafts.remove(selectedScript);auto a=project.data["effects"].toArray();
            for(int i=0;i<a.size();++i)if(a[i].toObject()["id"]==selectedId)for(auto v:savedProject.data["effects"].toArray())if(v.toObject()["id"]==selectedId)a[i]=v;
            project.data["effects"]=a;for(auto key:project.aircraft.keys())project.setOverride(key,selectedId,savedProject.aircraft.value(key)["effects"].toObject()[selectedId].toObject());
            dirtyEffects.remove(selectedId);apply();
        }
    }
    refreshing=true;removeTab(tabIndex(selectedScript));selectedScript.clear();selectedId.clear();editor=nullptr;refreshing=false;
    dirty=!drafts.isEmpty()||!dirtyEffects.isEmpty();setWindowModified(dirty);activateTab();updateTabs();
}
void Window::closeProject() {
    engine.clearProject();project={};savedProject={};drafts.clear();dirtyEffects.clear();diskHashes.clear();effectDefaults.clear();selectedId.clear();selectedScript.clear();dirty=false;
    refreshing=true;clearTabs();files->checks.clear();files->errors.clear();profile->clear();profile->addItem("Global",QString());
    tree->setRootIndex(QModelIndex());tree->setEnabled(false);refreshing=false;local.remove("last_project");saveLocal();setWindowTitle("Stella Vibra");setWindowModified(false);updateTabs();rebuildParameters();
}
void Window::updateRecent() {
    recentMenu->clear();for(auto v:local["recent_projects"].toArray()) {
        auto path=v.toString();auto* a=recentMenu->addAction(path);connect(a,&QAction::triggered,this,[this,path]{guarded([this,path]{if(confirmDiscard())loadProject(path);});});
    }
    recentMenu->setEnabled(!recentMenu->actions().isEmpty());
}
void Window::syncFiles() {
    if(project.path.isEmpty() || hasRawDrafts())return;
    if(!QFileInfo::exists(project.path)){addLog("Project file removed; project closed");closeProject();return;}
    QDir root(QFileInfo(project.path).absolutePath());QStringList found;
    QDirIterator it(root.filePath("effects"),{"*.lua"},QDir::Files|QDir::NoSymLinks,QDirIterator::Subdirectories);
    while(it.hasNext()) { auto relative=root.relativeFilePath(it.next());scriptPath(project.path,relative);found<<relative; }found.sort();
    auto entries=project.data["effects"].toArray();auto persisted=savedProject.data["effects"].toArray();QSet<QString> registered;
    for(auto v:entries)registered.insert(v.toObject()["script"].toString().toLower());
    QStringList added;for(const auto& path:found)if(!registered.contains(path.toLower()))added<<path;
    bool structural=false;
    for(int i=0;i<entries.size();++i) {
        auto e=entries[i].toObject();auto old=e["script"].toString();if(QFileInfo::exists(root.filePath(old)))continue;
        QStringList matches;for(const auto& path:added)if(QFileInfo(path).fileName().compare(QFileInfo(old).fileName(),Qt::CaseInsensitive)==0)matches<<path;
        int missingCount=0;for(auto v:entries){auto p=v.toObject()["script"].toString();if(!QFileInfo::exists(root.filePath(p)) && QFileInfo(p).fileName().compare(QFileInfo(old).fileName(),Qt::CaseInsensitive)==0)++missingCount;}
        if(matches.size()!=1 || missingCount!=1)continue;
        auto moved=matches.first();added.removeAll(moved);e["script"]=moved;entries[i]=e;
        for(int j=0;j<persisted.size();++j){auto p=persisted[j].toObject();if(p["id"]==e["id"]){p["script"]=moved;persisted[j]=p;}}
        if(drafts.contains(old))drafts[moved]=drafts.take(old);
        refreshing=true;auto index=tabIndex(old);if(index>=0){openFiles->widget(index)->setProperty("path",moved);fileTabs->setTabText(index,QFileInfo(moved).fileName());fileTabs->setTabToolTip(index,moved);}refreshing=false;
        if(selectedScript==old)selectedScript=moved;
        structural=true;addLog("Moved effect; settings preserved: "+old+" -> "+moved);
    }
    for(int i=static_cast<int>(entries.size())-1;i>=0;--i) {
        auto e=entries[i].toObject();auto path=e["script"].toString();
        if(QFileInfo::exists(root.filePath(path)))continue;
        auto id=e["id"].toString();entries.removeAt(i);
        for(int j=static_cast<int>(persisted.size())-1;j>=0;--j)if(persisted[j].toObject()["id"]==id)persisted.removeAt(j);
        for(auto key:project.aircraft.keys())project.setOverride(key,id,{});
        for(auto key:savedProject.aircraft.keys())savedProject.setOverride(key,id,{});
        dirtyEffects.remove(id);
        refreshing=true;auto index=tabIndex(path);
        // Deleting settings must not silently discard unsaved code in an open editor.
        if(!drafts.contains(path)) {
            if(index>=0)removeTab(index);
            if(selectedScript==path){selectedScript.clear();selectedId.clear();editor=nullptr;}
        } else if(selectedScript==path)selectedId.clear();
        refreshing=false;
        structural=true;addLog("Removed missing effect and its settings: "+path);
    }
    for(const auto& path:added) {
        if(entries.size()>=64){addLog("Effect limit reached; not registered: "+path);break;}
        QJsonObject e{{"id",QUuid::createUuid().toString(QUuid::WithoutBraces)},{"script",path},{"enabled",true},{"parameters",QJsonObject{}},{"outputs",QJsonObject{}}};
        entries<<e;persisted<<e;structural=true;addLog("Discovered effect with script defaults: "+path);
    }
    if(structural){project.data["effects"]=entries;savedProject.data["effects"]=persisted;savedProject.save();dirty=!drafts.isEmpty()||!dirtyEffects.isEmpty();setWindowModified(dirty);}
    QMap<QString,QByteArray> hashes;
    for(auto v:entries) {auto relative=v.toObject()["script"].toString();auto path=scriptPath(project.path,relative);if(!QFileInfo::exists(path))continue;
        QString text;
        try {text=sourceText(path);}catch(const std::exception& error){hashes[relative]="unreadable";if(diskHashes.value(relative)!="unreadable")addLog(error.what());continue;}
        auto hash=QCryptographicHash::hash(text.toUtf8(),QCryptographicHash::Sha256);hashes[relative]=hash;
        if(diskHashes.contains(relative) && diskHashes[relative]!=hash) {
            effectDefaults.remove(v.toObject()["id"].toString());
            if(drafts.contains(relative))addLog("External edit detected; unsaved editor retained: "+relative);
            else if(tabIndex(relative)>=0){refreshing=true;auto* document=static_cast<Editor*>(openFiles->widget(tabIndex(relative)));document->setPlainText(text);document->setProperty("savedText",document->toPlainText());refreshing=false;}
        }
    }
    bool changed=hashes!=diskHashes;diskHashes=hashes;
    if(structural||changed){activateTab();refreshExplorer();apply();refreshRawTabs();rebuildParameters();}
}
}
