#include "window.h"

namespace sv {
namespace {
class EffectsFolders final : public QSortFilterProxyModel {
public:
    QString projectFolder;
protected:
    bool filterAcceptsRow(int row,const QModelIndex& parent) const override {
        auto* fs=static_cast<QFileSystemModel*>(sourceModel());
        if(fs->filePath(parent).compare(projectFolder,Qt::CaseInsensitive)!=0)return true;
        return fs->fileName(fs->index(row,0,parent)).compare("effects",Qt::CaseInsensitive)==0;
    }
};
bool within(const QString& path,const QString& root) {
    return path.compare(root,Qt::CaseInsensitive)==0 || path.startsWith(root+"/",Qt::CaseInsensitive);
}
QString checkedPath(const QString& projectPath,const QString& path) {
    const auto base=QFileInfo(projectPath).absolutePath();
    const auto root=QDir(base).filePath("effects");
    const auto clean=QDir::cleanPath(QDir::fromNativeSeparators(path));
    require(within(clean,root),"Choose a path inside this project's effects folder");
    // Reject links/junctions, including ancestors, rather than moving their targets.
    for(auto p=clean;within(p,root);p=QFileInfo(p).absolutePath()) {
        const QFileInfo info(p);
        require(!info.isSymLink() && !info.isJunction(),"Linked folders and files cannot be managed here");
        if(p.compare(root,Qt::CaseInsensitive)==0)break;
    }
    scriptPath(projectPath,QDir(base).relativeFilePath(clean));
    return clean;
}
QString fileName(QString name,bool script) {
    require(!name.isEmpty() && name==name.trimmed(),"Enter a name without leading or trailing spaces");
    require(!name.contains(QRegularExpression("[<>:\"/\\\\|?*\\x00-\\x1f]")) && !name.endsWith('.') && !name.endsWith(' '),"Invalid file or folder name");
    require(name!="." && name!="..","Invalid name");
    require(!QRegularExpression("^(CON|PRN|AUX|NUL|COM[0-9]|LPT[0-9])(?:\\.|$)",QRegularExpression::CaseInsensitiveOption).match(name).hasMatch(),"Reserved Windows name");
    if(script && !name.endsWith(".lua",Qt::CaseInsensitive))name+=".lua";
    return name;
}
const char* scriptExample=R"lua(-- Lua effect API example. This script produces no vibration until you edit it.
-- Use the Telemetry view and autocomplete for your project's game-specific inputs.
local effect = {}

function effect:init(context)
    context:parameter {
        name="Power", type="haptic_power",
        default={strength=20, pwm=false, on_ms=20, period_ms=80}
    }
    -- Empty defaults leave the output unassigned. Example: {"Stick:1"}
    context:output("Effect", {})
    self.previous = nil -- State belongs to this effect instance.
end

function effect:update(context, telemetry, parameters)
    if not context:input_valid() then
        context:vibrate("Effect", off())
        self.previous = nil
        return
    end

    -- Missing telemetry fields are nil, not measured zero.
    -- if context:sample_is_new() then
    --     Inspect game-specific telemetry and update self here.
    --     Establish a baseline on the first sample; do not fire an event.
    -- end

    -- Continuous effect: renew a short request only while its condition is true.
    -- context:vibrate("Effect", parameters.Power, 100)
    -- Event effect: request once with its own duration (milliseconds).
    -- context:vibrate("Effect", constant(20), 250)
    -- context:vibrate("Effect", pwm{strength=20, on_ms=20, period_ms=80}, 250)
    -- context:vibrate("Effect", off())
    -- context:now() / context:elapsed() return host time / elapsed time in ms.
    -- context:log("Event detected") -- Log transitions, not every update.
    -- A new vibrate request replaces this output's specification and deadline.
end

return effect
)lua";
}

void Window::newFile() {
    require(!project.path.isEmpty(),"Open a project before creating a script");
    require(!hasRawDrafts(),"Save or discard JSON edits before changing files");
    const auto openingProject=project.path;
    const auto root=QDir(QFileInfo(project.path).absolutePath()).filePath("effects");
    auto path=tree->currentIndex().data(QFileSystemModel::FilePathRole).toString();
    if(!within(path,root))path=root;
    if(QFileInfo(path).isFile())path=QFileInfo(path).absolutePath();
    QDialog dialog(this);dialog.setWindowTitle("New File");dialog.resize(560,430);
    QVBoxLayout layout(&dialog);
    QLabel label("Script path (effects/ is optional):");layout.addWidget(&label);
    auto relative=QDir(root).relativeFilePath(path);if(relative==".")relative.clear();
    QLineEdit input("effects/"+(relative.isEmpty()?QString():relative+"/"));input.setPlaceholderText("effects/folder/new_script.lua");layout.addWidget(&input);
    QLabel rootLabel("Root: "+QDir::toNativeSeparators(root));rootLabel.setWordWrap(true);layout.addWidget(&rootLabel);
    QFileSystemModel folders;folders.setFilter(QDir::Dirs|QDir::NoDotAndDotDot|QDir::NoSymLinks);folders.setReadOnly(true);
    EffectsFolders folderProxy;folderProxy.projectFolder=QFileInfo(root).absolutePath();folderProxy.setSourceModel(&folders);
    QTreeView view;view.setModel(&folderProxy);view.setRootIndex(folderProxy.mapFromSource(folders.setRootPath(folderProxy.projectFolder)));
    folders.sort(0,Qt::AscendingOrder);
    connect(&folders,&QFileSystemModel::directoryLoaded,&dialog,[&](const QString& loaded){
        if(loaded.compare(folderProxy.projectFolder,Qt::CaseInsensitive)==0)view.expand(folderProxy.mapFromSource(folders.index(root)));
    });
    for(int i=1;i<4;++i)view.hideColumn(i);view.setHeaderHidden(true);layout.addWidget(&view);
    auto selectFolder=[&](const QString& selected){auto prefix=QDir(root).relativeFilePath(selected);if(prefix==".")prefix.clear();
        const auto text=QDir::fromNativeSeparators(input.text());const auto leaf=text.section('/',-1);
        input.setText("effects/"+(prefix.isEmpty()?leaf:prefix+"/"+leaf));};
    connect(&view,&QTreeView::clicked,&dialog,[&](const QModelIndex& index){selectFolder(folders.filePath(folderProxy.mapToSource(index)));});
    QDialogButtonBox buttons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);layout.addWidget(&buttons);
    connect(&buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    connect(&buttons,&QDialogButtonBox::accepted,&dialog,[&]{guarded([&]{
        require(project.path==openingProject,"The active project changed. Reopen New File.");
        auto text=QDir::fromNativeSeparators(input.text());
        require(QDir::isRelativePath(text),"Enter a path relative to effects");
        if(text.startsWith("effects/",Qt::CaseInsensitive))text.remove(0,8);
        auto parts=text.split('/',Qt::KeepEmptyParts);
        for(int i=0;i<parts.size();++i)parts[i]=fileName(parts[i],i==parts.size()-1);
        const auto target=checkedPath(project.path,QDir(root).filePath(parts.join('/')));
        editEffectFile("New Script",root,target);dialog.accept();
    });});
    input.setFocus();dialog.exec();
}

bool Window::canMoveEffect(const QString& source,const QString& destination) const {
    if(project.path.isEmpty() || hasRawDrafts() || fileOperation)return false;
    try {
        const auto root=QDir(QFileInfo(project.path).absolutePath()).filePath("effects");
        checkedPath(project.path,source);const QFileInfo info(source);
        if(source.compare(root,Qt::CaseInsensitive)==0 || !info.exists() || (!info.isDir() && !source.endsWith(".lua",Qt::CaseInsensitive)))return false;
        if(destination.isEmpty())return true;
        checkedPath(project.path,destination);
        if(!QFileInfo(destination).isDir() || within(destination,source))return false;
        const auto target=QDir(destination).filePath(info.fileName());
        return !QFileInfo::exists(target);
    }catch(const std::exception&){return false;}
}

void Window::effectContextMenu(const QPoint& point) {
    if(project.path.isEmpty())return;
    const auto index=tree->indexAt(point);if(!index.isValid())return;
    const auto path=index.data(QFileSystemModel::FilePathRole).toString();
    const auto root=QDir(QFileInfo(project.path).absolutePath()).filePath("effects");
    if(!within(path,root))return;
    const QFileInfo info(path);const bool folder=info.isDir();
    if(!folder && !path.endsWith(".lua",Qt::CaseInsensitive))return;
    tree->setCurrentIndex(index);
    QMenu menu(this);
    if(folder){menu.addAction("New Folder...")->setData("New Folder");menu.addAction("New Script...")->setData("New Script");}
    if(path.compare(root,Qt::CaseInsensitive)!=0){
        if(folder)menu.addSeparator();
        for(const auto& op:{"Rename","Delete"})menu.addAction(QString(op)+"...")->setData(op);
    }
    auto* chosen=menu.exec(tree->viewport()->mapToGlobal(point));
    if(chosen)editEffectFile(chosen->data().toString(),path);
}

void Window::editEffectFile(const QString& operation,const QString& requested,const QString& destination) {
    require(!hasRawDrafts(),"Save or discard JSON edits before changing files");
    syncFiles();
    QScopedValueRollback<bool> busy(fileOperation,true);
    const auto projectPath=project.path;
    const auto source=checkedPath(projectPath,requested);
    const auto root=QDir(QFileInfo(projectPath).absolutePath()).filePath("effects");
    const QDir base(QFileInfo(projectPath).absolutePath());
    const bool create=operation.startsWith("New ");
    const bool folder=QFileInfo(source).isDir();
    QString target;
    if(operation=="New Script" && !destination.isEmpty()) {
        target=destination;
    } else if(create || operation=="Rename") {
        require(create || source.compare(root,Qt::CaseInsensitive)!=0,"The effects root cannot be renamed");
        bool ok=false;
        auto name=QInputDialog::getText(this,operation,"Name:",QLineEdit::Normal,create?QString():QFileInfo(source).fileName(),&ok);
        if(!ok)return;
        name=fileName(name,operation=="New Script" || (!create && !folder));
        target=QDir(create?source:QFileInfo(source).absolutePath()).filePath(name);
    } else if(operation=="Move") {
        require(source.compare(root,Qt::CaseInsensitive)!=0,"The effects root cannot be moved");
        checkedPath(projectPath,destination);
        require(QFileInfo(destination).isDir(),"Choose an existing destination folder");
        target=QDir(destination).filePath(QFileInfo(source).fileName());
    } else if(operation=="Delete") {
        require(source.compare(root,Qt::CaseInsensitive)!=0,"The effects root cannot be deleted");
        auto answer=QMessageBox::warning(this,"Delete", "Move to the Recycle Bin?\n"+source+
            "\n\nThis includes all folder contents. Effect settings will be removed. Unsaved edits in these files will be discarded.",
            QMessageBox::Yes|QMessageBox::Cancel,QMessageBox::Cancel);
        if(answer!=QMessageBox::Yes)return;
        checkedPath(projectPath,source);
        if(!QFile::moveToTrash(source)) {
            auto permanent=QMessageBox::warning(this,"Permanently Delete", "Could not move this item to the Recycle Bin.\n"+source+
                "\n\nPermanently delete it and all its contents? This cannot be undone.",QMessageBox::Yes|QMessageBox::No,QMessageBox::No);
            if(permanent!=QMessageBox::Yes)return;
            checkedPath(projectPath,source);
            // Refuse nested links/junctions before recursive deletion.
            if(folder){QDirIterator children(source,QDir::AllEntries|QDir::Hidden|QDir::System|QDir::NoDotAndDotDot,QDirIterator::Subdirectories);
                while(children.hasNext())checkedPath(projectPath,children.next());}
            const bool removed=folder?QDir(source).removeRecursively():QFile::remove(source);
            if(!removed){fileOperation=false;syncFiles();require(false,"Could not delete all contents. Some items may have been removed; inspect the folder before retrying.");}
        }
        const auto relative=base.relativeFilePath(source);
        refreshing=true;
        for(auto path:drafts.keys())if(within(path,relative))drafts.remove(path);
        for(int i=openFiles->count()-1;i>=0;--i)if(within(tabPath(i),relative))removeTab(i);
        refreshing=false;fileOperation=false;syncFiles();activateTab();updateTabs();return;
    } else return;
    require(project.path==projectPath,"The active project changed");
    checkedPath(projectPath,source);target=checkedPath(projectPath,target);
    if(!create && source==target)return;
    const bool caseOnly=!create && source.compare(target,Qt::CaseInsensitive)==0;
    require(caseOnly || !QFileInfo::exists(target),"A file or folder already exists at the destination");
    require(create || caseOnly || !within(target,source),"Cannot move a folder into itself");
    if(create) {
        require(folder || (operation=="New Script" && !destination.isEmpty()),"Choose an existing folder");
        if(operation=="New Folder")require(QDir().mkdir(target),"Cannot create folder");
        else {
            require(project.data["effects"].toArray().size()<64,"Effect limit reached (64)");
            require(!drafts.contains(base.relativeFilePath(target)),"An unsaved editor already uses this path. Save or close it first.");
            require(QDir().mkpath(QFileInfo(target).absolutePath()),"Cannot create script folders");
            checkedPath(projectPath,target);
            QFile file(target);require(file.open(QIODevice::WriteOnly|QIODevice::NewOnly),"Cannot create script");
            const QByteArray text(scriptExample);
            if(file.write(text)!=text.size() || !file.flush()){file.close();QFile::remove(target);require(false,"Cannot write new script");}
        }
    } else {
        const auto old=base.relativeFilePath(source),next=base.relativeFilePath(target);
        auto remap=[&](const QString& path){return within(path,old)?next+path.mid(old.size()):path;};
        auto moved=project,persisted=savedProject;
        for(auto* projectState:{&moved,&persisted}) {
            auto entries=projectState->data["effects"].toArray();
            for(int i=0;i<entries.size();++i){auto entry=entries[i].toObject();entry["script"]=remap(entry["script"].toString());entries[i]=entry;}
            projectState->data["effects"]=entries;
        }
        auto rename=[&](const QString& from,const QString& to){
            if(!caseOnly)return QDir().rename(from,to);
            const auto temporary=checkedPath(projectPath,QDir(QFileInfo(from).absolutePath()).filePath(".stella-rename-"+QUuid::createUuid().toString(QUuid::WithoutBraces)));
            if(!QDir().rename(from,temporary))return false;
            if(QDir().rename(temporary,to))return true;
            if(!QDir().rename(temporary,from))addLog("Could not restore original name; file remains at: "+temporary);
            return false;
        };
        require(rename(source,target),"Cannot rename or move this item");
        try {persisted.save();}catch(...) {
            if(!rename(target,source))addLog("Could not roll back file move: "+target);
            throw;
        }
        project=moved;savedProject=persisted;
        QMap<QString,QString> renamedDrafts;for(auto it=drafts.begin();it!=drafts.end();++it)renamedDrafts[remap(it.key())]=it.value();drafts=renamedDrafts;
        QMap<QString,QByteArray> renamedHashes;for(auto it=diskHashes.begin();it!=diskHashes.end();++it)renamedHashes[remap(it.key())]=it.value();diskHashes=renamedHashes;
        QMap<QString,QString> renamedBases;for(auto it=rawBases.begin();it!=rawBases.end();++it)renamedBases[remap(it.key())]=it.value();rawBases=renamedBases;
        for(int i=0;i<openFiles->count();++i)openFiles->widget(i)->setProperty("path",remap(tabPath(i)));
        selectedScript=remap(selectedScript);
        apply();refreshRawTabs();
    }
    fileOperation=false;syncFiles();activateTab();updateTabs();refreshExplorer();
    tree->expand(sortedFiles->mapFromSource(files->index(QFileInfo(target).absolutePath())));tree->setCurrentIndex(sortedFiles->mapFromSource(files->index(target)));
    if(operation=="New Script")showScript(base.relativeFilePath(target));
    addLog(operation+": "+base.relativeFilePath(target));
}
}
