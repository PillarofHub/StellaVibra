#include "game_catalog.h"
#include "window.h"
#include "telemetry.h"
#include <memory>
namespace sv {
class FlexibleDock final : public QDockWidget {
public:
    using QDockWidget::QDockWidget;
    QSize minimumSizeHint() const override {return {0,0};}
};
static QString readSource(const QString& path) { QFile f(path); require(f.open(QIODevice::ReadOnly),"Cannot read "+path); require(f.size()<=256*1024,"Script too large"); return QString::fromUtf8(f.readAll()); }
class CompactNumber final : public QDoubleSpinBox {
protected:
    QString textFromValue(double value) const override {
        auto text=QDoubleSpinBox::textFromValue(value);const auto point=locale().decimalPoint();
        if(text.contains(point)){while(text.endsWith(locale().zeroDigit()))text.chop(locale().zeroDigit().size());if(text.endsWith(point))text.chop(point.size());}
        return text;
    }
};
static QWidget* numericControl(double minimum,double maximum,double step,double value,bool integer,const std::function<void(double)>& changed) {
    auto* host=new QWidget;auto* row=new QHBoxLayout(host);row->setContentsMargins(0,0,0,0);
    auto* slider=new QSlider(Qt::Horizontal);auto* spin=new CompactNumber;
    int ticks=static_cast<int>(std::ceil((maximum-minimum)/step));slider->setRange(0,ticks);
    spin->setDecimals(integer?0:6);spin->setRange(minimum,maximum);spin->setSingleStep(step);spin->setKeyboardTracking(false);spin->setValue(value);spin->setMaximumWidth(integer?76:120);
    slider->setValue(qRound((value-minimum)/step));row->addWidget(slider,1);row->addWidget(spin);
    QObject::connect(slider,&QSlider::valueChanged,host,[=](int v){spin->setValue(std::min(maximum,minimum+v*step));});
    QObject::connect(spin,&QDoubleSpinBox::valueChanged,host,[=](double v){QSignalBlocker block(slider);slider->setValue(qRound((v-minimum)/step));changed(v);});return host;
}
static QStringList chooseSlots(QWidget* parent,const QStringList& current,const QString& title, bool* accepted=nullptr,const QJsonObject& counts={}) {
    QDialog dialog(parent); dialog.setWindowTitle(title); QVBoxLayout layout(&dialog); QLabel hint("Select destinations. An empty selection means no output."); layout.addWidget(&hint); QListWidget list; layout.addWidget(&list);
    for(auto s:availableSlots(counts)) { auto* item=new QListWidgetItem(s,&list); item->setFlags(item->flags()|Qt::ItemIsUserCheckable); item->setCheckState(current.contains(s)?Qt::Checked:Qt::Unchecked); }
    QDialogButtonBox buttons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel); layout.addWidget(&buttons); QObject::connect(&buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept); QObject::connect(&buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    if(accepted)*accepted=false; if(dialog.exec()!=QDialog::Accepted) return current; if(accepted)*accepted=true; QStringList result; for(int i=0;i<list.count();++i) if(list.item(i)->checkState()==Qt::Checked) result<<list.item(i)->text(); return result;
}
Window::Window() {

    setWindowTitle("Stella Vibra");setDockOptions(dockOptions() & ~QMainWindow::AnimatedDocks); resize(1400,900);
    try { if(QFileInfo::exists(localPath())) local=readJson(localPath()); } catch(const std::exception& e) { QMessageBox::warning(this,"Local settings",e.what()); }
    engine.setTick(std::clamp(local["tick_ms"].toInt(20),5,100));engine.setMaximumUpdatePeriod(std::clamp(local["maximum_update_period_ms"].toInt(100),5,500));local["version"]=1; engine.loadMappings(local["motors"].toObject());
    auto* fileMenu=menuBar()->addMenu("File");
    auto action=[&](const QString& name,const std::function<void()>& fn){auto* a=fileMenu->addAction(name);connect(a,&QAction::triggered,this,[this,fn]{guarded(fn);});return a;};
    action("New Project...",[this]{newProject();})->setShortcut(QKeySequence::New);
    action("Open Project...",[this]{openProject();})->setShortcut(QKeySequence::Open);
    recentMenu=fileMenu->addMenu("Open Recent");updateRecent();
    action("Close Project",[this]{if(confirmDiscard())closeProject();});fileMenu->addSeparator();
    action("Save",[this]{save();})->setShortcut(QKeySequence::Save);
    action("Save All",[this]{saveAll();})->setShortcut(QKeySequence("Ctrl+Shift+S"));
    action("Close File",[this]{closeFile();})->setShortcut(QKeySequence::Close);
    fileMenu->addSeparator();
    action("Exit",[this]{close();});
    auto* left=new FlexibleDock("Project Explorer",this);left->setObjectName("Explorer");
    files=new EffectFiles(this);files->setReadOnly(true);tree=new EffectTree;tree->setModel(files);tree->setItemDelegate(new EffectCheckDelegate(tree));for(int i=1;i<4;++i)tree->hideColumn(i);tree->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);tree->header()->setStretchLastSection(true);tree->setMinimumSize(0,0);connect(files,&QFileSystemModel::directoryLoaded,this,[this]{expandEffectFolders();});tree->setEnabled(false);left->setWidget(tree);addDockWidget(Qt::LeftDockWidgetArea,left);
    files->toggle=[this](QString path,bool enabled){guarded([&]{
        require(!hasRawDrafts(),"Save or discard JSON edits before changing effect settings");
        const bool group=QFileInfo(path).isDir();auto entries=project.data["effects"].toArray();
        for(int i=0;i<entries.size();++i){auto e=entries[i].toObject();auto source=scriptPath(project.path,e["script"].toString());
            if(source.compare(path,Qt::CaseInsensitive)!=0 && !(group && source.startsWith(path+"/",Qt::CaseInsensitive)))continue;
            if(profileKey().isEmpty()){e["enabled"]=enabled;entries[i]=e;}else{auto patch=project.aircraft.value(profileKey())["effects"].toObject()[e["id"].toString()].toObject();patch["enabled"]=enabled;project.setOverride(profileKey(),e["id"].toString(),patch);}
        }
        project.data["effects"]=entries;updateTabs();apply();rebuildParameters();
    });};
    connect(tree,&QTreeView::doubleClicked,this,[this](const QModelIndex& index){guarded([&]{auto path=files->filePath(index);if(!project.path.isEmpty() && (path.endsWith(".lua",Qt::CaseInsensitive)||path.endsWith(".sv",Qt::CaseInsensitive)))showScript(QDir(QFileInfo(project.path).absolutePath()).relativeFilePath(path));});});    auto* split=new QSplitter(Qt::Vertical); paramPane=new QWidget; auto* paramLayout=new QVBoxLayout(paramPane); auto* profileRow=new QHBoxLayout; profile=new QComboBox; profile->addItem("Global",QString()); enabledButton=new QPushButton("Disabled");enabledButton->setCheckable(true);profileRow->addWidget(enabledButton);connect(enabledButton,&QPushButton::clicked,this,[this](bool value){guarded([&]{setValue("","enabled",value);rebuildParameters();});});profileRow->addWidget(new QLabel("Parameters:")); profileRow->addWidget(profile,1); auto* resetAll=new QPushButton("Reset All to Default"); profileRow->addWidget(resetAll); paramLayout->addLayout(profileRow); connect(profile,&QComboBox::currentIndexChanged,this,[this,resetAll]{resetAll->setText(profileKey().isEmpty()?"Reset All to Default":"Reset All to Global");if(!refreshing)guarded([this]{rebuildParameters();});});  connect(resetAll,&QPushButton::clicked,this,[this]{guarded([this]{if(selectedId.isEmpty())return;require(!hasRawDrafts(),"Save or discard JSON edits first");if(profileKey().isEmpty()){auto entries=project.data["effects"].toArray();for(int i=0;i<entries.size();++i){auto entry=entries[i].toObject();if(entry["id"]==selectedId){entry.remove("enabled");entry.remove("parameters");entry.remove("outputs");entries[i]=entry;}}project.data["effects"]=entries;}else project.setOverride(profileKey(),selectedId,{});updateTabs();apply();rebuildParameters();});}); auto* scroll=new QScrollArea; scroll->setWidgetResizable(true);parameterHost=new QWidget;parameters=new QFormLayout(parameterHost);parameters->setFormAlignment(Qt::AlignTop);scroll->setWidget(parameterHost);paramLayout->addWidget(scroll);split->addWidget(paramPane);
    auto* editHost=new QWidget;auto* editLayout=new QVBoxLayout(editHost);auto* findRow=new QHBoxLayout;auto* search=new QLineEdit;search->setPlaceholderText("Find in script (Enter)");findRow->addWidget(new QLabel("Script Editor"));findRow->addWidget(search);editLayout->addLayout(findRow);rawNotice=new QLabel;rawNotice->setWordWrap(true);rawNotice->hide();editLayout->addWidget(rawNotice);fileTabs=new QTabBar;fileTabs->setTabsClosable(true);fileTabs->setMovable(true);fileTabs->setExpanding(false);fileTabs->setUsesScrollButtons(true);openFiles=new QStackedWidget;editLayout->addWidget(openFiles);connect(openFiles,&QStackedWidget::currentChanged,this,[this]{if(!refreshing)guarded([this]{activateTab();});});connect(fileTabs,&QTabBar::tabCloseRequested,this,[this](int index){guarded([&]{openFiles->setCurrentIndex(index);closeFile();});});split->addWidget(editHost);split->setSizes({400,400});split->setStretchFactor(0,1);split->setStretchFactor(1,1);auto* effectDock=new FlexibleDock("Effect Control",this);effectDock->setObjectName("EffectControl");auto* effectHost=new QWidget;auto* effectLayout=new QVBoxLayout(effectHost);effectLayout->setContentsMargins(0,0,0,0);effectLayout->setSpacing(0);effectLayout->addWidget(fileTabs);effectLayout->addWidget(split,1);effectDock->setWidget(effectHost);setDockNestingEnabled(true);addDockWidget(Qt::RightDockWidgetArea,effectDock);
    connect(fileTabs,&QTabBar::currentChanged,this,[this](int index){if(!refreshing)openFiles->setCurrentIndex(index);});
    connect(fileTabs,&QTabBar::tabMoved,this,[this](int from,int to){refreshing=true;auto* document=openFiles->widget(from);openFiles->removeWidget(document);openFiles->insertWidget(to,document);openFiles->setCurrentIndex(fileTabs->currentIndex());refreshing=false;guarded([this]{activateTab();});});
    connect(search,&QLineEdit::returnPressed,this,[this,search]{if(editor && !editor->find(search->text())){editor->moveCursor(QTextCursor::Start);editor->find(search->text());}});
    auto* right=new FlexibleDock("Motor Monitor",this);right->setObjectName("Monitor");auto* rightHost=new QWidget;auto* rightLayout=new QVBoxLayout(rightHost);monitor=new QTableWidget(0,3);monitor->setHorizontalHeaderLabels({"Motor","Slots","HID"});monitor->horizontalHeader()->setStretchLastSection(false);monitor->horizontalHeader()->setSectionResizeMode(0,QHeaderView::Stretch);monitor->horizontalHeader()->setSectionResizeMode(1,QHeaderView::Interactive);monitor->horizontalHeader()->setSectionResizeMode(2,QHeaderView::Fixed);monitor->setColumnWidth(0,260);monitor->setColumnWidth(1,110);monitor->setColumnWidth(2,52);monitor->horizontalHeaderItem(2)->setToolTip("Last successfully sent motor strength (0–255), refreshed every 20 ms. Not joystick axis input or measured feedback.");monitor->setMinimumSize(0,0);monitor->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Expanding);monitor->setSelectionBehavior(QAbstractItemView::SelectRows);monitor->setSelectionMode(QAbstractItemView::SingleSelection);monitor->setEditTriggers(QAbstractItemView::NoEditTriggers);rightLayout->addWidget(monitor);pauseButton=new QPushButton("Stop All");rightLayout->addWidget(pauseButton);connect(pauseButton,&QPushButton::clicked,this,[this]{paused=!paused;if(paused)engine.stop();else engine.run();pauseButton->setText(paused?"Resume All":"Stop All");});
    monitor->horizontalHeaderItem(1)->setToolTip("Double-click or right-click to edit mapping");monitor->horizontalHeaderItem(2)->setToolTip("Last sent HID strength (0–255). Double-click or right-click to test this motor.");
    connect(monitor,&QTableWidget::cellDoubleClicked,this,[this](int row,int column){monitor->setCurrentCell(row,column);guarded([&]{if(column==1)mapping();else if(column==2)motorTest();});});
    monitor->setContextMenuPolicy(Qt::CustomContextMenu);connect(monitor,&QWidget::customContextMenuRequested,this,[this](const QPoint& point){auto index=monitor->indexAt(point);if(!index.isValid() || index.column()==0)return;monitor->setCurrentCell(index.row(),index.column());guarded([&]{if(index.column()==1)mapping();else motorTest();});});
    right->setWidget(rightHost);addDockWidget(Qt::RightDockWidgetArea,right);splitDockWidget(effectDock,right,Qt::Horizontal);
    auto* bottom=new FlexibleDock("Debug Output",this);bottom->setObjectName("Output");auto* bottomHost=new QWidget;auto* bottomLayout=new QVBoxLayout(bottomHost);auto* logRow=new QHBoxLayout;filter=new QLineEdit;filter->setPlaceholderText("Filter by effect or message");logRow->addWidget(filter);auto* clear=new QPushButton("Clear");logRow->addWidget(clear);bottomLayout->addLayout(logRow);output=new QListWidget;output->setMinimumSize(0,0);output->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Ignored);bottomLayout->addWidget(output);bottom->setWidget(bottomHost);addDockWidget(Qt::BottomDockWidgetArea,bottom);
    connect(clear,&QPushButton::clicked,output,&QListWidget::clear);connect(filter,&QLineEdit::textChanged,this,[this]{for(int i=0;i<output->count();++i)output->item(i)->setHidden(!output->item(i)->text().contains(filter->text(),Qt::CaseInsensitive));});
    connect(output,&QListWidget::itemDoubleClicked,this,[this](QListWidgetItem* item){auto match=QRegularExpression("(effects/[^:]+\\.lua):(\\d+):").match(item->text());if(!match.hasMatch())return;guarded([&]{showScript(match.captured(1));auto cursor=editor->textCursor();cursor.setPosition(editor->document()->findBlockByLineNumber(match.captured(2).toInt()-1).position());editor->setTextCursor(cursor);editor->setFocus();});});
    auto* editMenu=menuBar()->addMenu("Edit");
    auto* undo=editMenu->addAction("Undo");auto* redo=editMenu->addAction("Redo");
    undo->setShortcuts({QKeySequence("Ctrl+Z")});redo->setShortcuts({QKeySequence("Ctrl+Y"),QKeySequence("Ctrl+Shift+Z")});
    for(auto* editAction:{undo,redo}){editAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);openFiles->addAction(editAction);}
    connect(undo,&QAction::triggered,this,[this]{if(editor && !editor->isReadOnly())editor->undo();});
    connect(redo,&QAction::triggered,this,[this]{if(editor && !editor->isReadOnly())editor->redo();});
    connect(editMenu,&QMenu::aboutToShow,this,[this,undo,redo]{undo->setEnabled(editor && !editor->isReadOnly() && editor->document()->isUndoAvailable());redo->setEnabled(editor && !editor->isReadOnly() && editor->document()->isRedoAvailable());});
    connect(editMenu,&QMenu::aboutToHide,this,[undo,redo]{undo->setEnabled(true);redo->setEnabled(true);});
    auto* telemetryDock=new FlexibleDock("Telemetry",this);telemetryDock->setObjectName("Telemetry");auto* telemetryHost=new QWidget;auto* telemetryLayout=new QVBoxLayout(telemetryHost);telemetryStatus=new QLabel("No telemetry");telemetryLayout->addWidget(telemetryStatus);telemetryView=new QTreeWidget;telemetryView->setMinimumSize(0,0);telemetryView->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Expanding);telemetryView->setColumnCount(3);telemetryView->setHeaderLabels({"Variable","Value","Unit / source"});telemetryLayout->addWidget(telemetryView);telemetryDock->setWidget(telemetryHost);addDockWidget(Qt::RightDockWidgetArea,telemetryDock);splitDockWidget(right,telemetryDock,Qt::Vertical);
    for(const auto& field:simVariables()){auto* row=new QTreeWidgetItem(telemetryView,{field.key,"Unavailable",field.unit});row->setToolTip(0,field.name);}
    for(auto name:{"sample_id","received_ms","aircraft_key","sim_time_advanced","msfs.paused"})new QTreeWidgetItem(telemetryView,{name,"Unavailable","Host / session"});
    telemetryView->setColumnWidth(0,220);telemetryView->setColumnWidth(1,90);
    setCorner(Qt::BottomLeftCorner,Qt::BottomDockWidgetArea);setCorner(Qt::BottomRightCorner,Qt::BottomDockWidgetArea);
    proportionalDocks={left,effectDock,right,bottom,telemetryDock};
    for(auto* dock:proportionalDocks){
        dock->setMinimumSize(0,0);dock->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
        if(dock->layout())dock->layout()->setSizeConstraint(QLayout::SetNoConstraint);
        auto* content=dock->widget();content->setMinimumSize(0,0);content->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
        if(content->layout())content->layout()->setSizeConstraint(QLayout::SetNoConstraint);
        dock->installEventFilter(this);
        connect(dock,&QDockWidget::dockLocationChanged,this,[this]{if(!resizingLayout)captureLayoutTimer.start();});
        connect(dock,&QDockWidget::topLevelChanged,this,[this]{if(!resizingLayout)captureLayoutTimer.start();});
        connect(dock,&QDockWidget::visibilityChanged,this,[this]{if(!resizingLayout)captureLayoutTimer.start();});
    }
    for(auto* pane:{paramPane,editHost}){pane->setMinimumSize(0,0);pane->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);pane->layout()->setSizeConstraint(QLayout::SetNoConstraint);}
    split->setMinimumSize(0,0);split->setChildrenCollapsible(true);
    captureLayoutTimer.setSingleShot(true);captureLayoutTimer.setInterval(0);
    resizeLayoutTimer.setSingleShot(true);resizeLayoutTimer.setInterval(0);
    connect(&captureLayoutTimer,&QTimer::timeout,this,[this]{captureDockRatios();});
    connect(&resizeLayoutTimer,&QTimer::timeout,this,[this]{applyDockRatios();});
    auto applyDefaultSizes=[this,split,left,effectDock,right,bottom,telemetryDock]{
        resizeLayoutTimer.stop();resizingLayout=false;dockRatios.clear();
        const int unit=std::max(1,(width()-12)/6);
        const int heightUnit=std::max(1,(height()-menuBar()->height()-statusBar()->height()-12)/4);
        resizeDocks({left,effectDock,right},{unit,4*unit,unit},Qt::Horizontal);
        resizeDocks({effectDock,bottom},{3*heightUnit,heightUnit},Qt::Vertical);
        resizeDocks({right,telemetryDock},{3*heightUnit/2,3*heightUnit/2},Qt::Vertical);
        split->setSizes({std::max(1,split->height()/2),std::max(1,split->height()/2)});
        captureLayoutTimer.start();
    };
    auto* view=menuBar()->addMenu("View");for(auto* dock:{left,effectDock,right,bottom,telemetryDock})view->addAction(dock->toggleViewAction());
    view->addSeparator();auto* resetView=view->addAction("Reset Default Layout");connect(resetView,&QAction::triggered,this,[this,left,effectDock,right,bottom,telemetryDock,applyDefaultSizes]{restoreState(defaultLayout);for(auto* dock:{left,effectDock,right,bottom,telemetryDock}){dock->setFloating(false);dock->show();}applyDefaultSizes();QTimer::singleShot(0,this,[this,applyDefaultSizes]{applyDefaultSizes();saveLocal();});});
    auto* settingsMenu=menuBar()->addMenu("Settings");connect(settingsMenu->addAction("Set Timings..."),&QAction::triggered,this,[this]{guarded([this]{setTimingsDialog();});});
    connect(settingsMenu->addAction("Set Channels..."),&QAction::triggered,this,[this]{guarded([this]{setChannelsDialog();});});
    auto* help=menuBar()->addMenu("Help");connect(help->addAction("About..."),&QAction::triggered,this,[this]{aboutDialog();});
    applyDefaultSizes();defaultLayout=saveState();
    state=new QLabel("MSFS: Discovering | HID: Discovering");state->setMinimumWidth(0);state->setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Preferred);statusBar()->addWidget(state,1);
    restoreGeometry(QByteArray::fromBase64(local["geometry"].toString().toLatin1()));
    // A pre-show restore may leave Qt with a pending maximized layout. Restoring
    // another layout immediately can invalidate those items. Restore only once,
    // and only for this version; otherwise keep the freshly constructed layout.
    if(local["layout_version"].toInt()==4 && !local["layout"].toString().isEmpty())
        restoreState(QByteArray::fromBase64(local["layout"].toString().toLatin1()));
    else QTimer::singleShot(0,this,applyDefaultSizes);
    captureLayoutTimer.start();
    connect(&timer,&QTimer::timeout,this,[this]{tick();});timer.start(20);addLog("Automatic MSFS and supported HID discovery enabled. Use Stop All to pause output.");QTimer::singleShot(0,this,[this]{if(QCoreApplication::arguments().contains("--ui-check"))return;auto path=local["last_project"].toString();if(!path.isEmpty() && QFileInfo::exists(path)) {try{loadProject(path);}catch(const std::exception& e){addLog(QString("Last project could not be opened: ")+e.what());closeProject();}}});
}
Window::~Window(){engine.stop();}
QSize Window::dockSpace() const {
    return {std::max(1,contentsRect().width()),std::max(1,contentsRect().height()-menuBar()->height()-statusBar()->height())};
}
void Window::captureDockRatios() {
    if(resizingLayout || !isVisible() || isMinimized())return;
    const auto space=dockSpace();dockRatios.clear();
    for(auto* dock:proportionalDocks){
        // Inactive dock tabs are positioned outside the main window by Qt.
        if(dock->isFloating() || !dock->isVisible() || !dock->geometry().intersects(contentsRect()))continue;
        dockRatios[dock]={double(dock->width())/space.width(),double(dock->height())/space.height()};
    }
}
bool Window::eventFilter(QObject* watched,QEvent* event) {
    if(!resizingLayout && (event->type()==QEvent::Resize || event->type()==QEvent::Move || event->type()==QEvent::Show || event->type()==QEvent::Hide))
        captureLayoutTimer.start();
    return QMainWindow::eventFilter(watched,event);
}
void Window::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    if(!isVisible() || isMinimized() || dockRatios.isEmpty()){captureLayoutTimer.start();return;}
    // Child resize notifications may precede this event. Keep the last settled
    // user proportions instead of sampling Qt's already redistributed geometry.
    resizingLayout=true;captureLayoutTimer.stop();resizeLayoutTimer.start();
}
void Window::applyDockRatios() {
    if(!isMinimized()){
        const auto space=dockSpace();QList<QDockWidget*> docks;QList<int> widths,heights;
        for(auto* dock:proportionalDocks){
            if(dock->isFloating() || !dock->isVisible() || !dockRatios.contains(dock))continue;
            const auto ratio=dockRatios.value(dock);docks<<dock;
            widths<<std::max(1,qRound(ratio.width()*space.width()));heights<<std::max(1,qRound(ratio.height()*space.height()));
        }
        if(!docks.isEmpty()){resizeDocks(docks,widths,Qt::Horizontal);resizeDocks(docks,heights,Qt::Vertical);}
    }
    // Let Qt settle its layout without feeding rounding/constraints back into
    // the stored proportions on every frame of a window resize.
    QTimer::singleShot(0,this,[this]{if(!resizeLayoutTimer.isActive())resizingLayout=false;});
}
void Window::guarded(const std::function<void()>& action){try{action();}catch(const std::exception& e){addLog(e.what());QMessageBox::warning(this,"Stella Vibra",e.what());}}
void Window::addLog(const QString& message){qInfo().noquote()<<message;auto* item=new QListWidgetItem(message,output);item->setHidden(!message.contains(filter->text(),Qt::CaseInsensitive));while(output->count()>1000)delete output->takeItem(0);output->scrollToBottom();}
bool Window::confirmDiscard(){if(!dirty)return true;auto answer=QMessageBox::question(this,"Unsaved changes","Save changes before continuing?",QMessageBox::Save|QMessageBox::Discard|QMessageBox::Cancel);if(answer==QMessageBox::Cancel)return false;if(answer==QMessageBox::Save)saveAll();return true;}
void Window::closeEvent(QCloseEvent* event){try{if(!confirmDiscard()){event->ignore();return;}engine.stop();saveLocal();event->accept();}catch(const std::exception& e){QMessageBox::warning(this,"Save failed",e.what());event->ignore();}}
void Window::openProject(){if(!confirmDiscard())return;auto file=QFileDialog::getOpenFileName(this,"Open Project",{},"Stella Vibra (*.sv)");if(!file.isEmpty())loadProject(file);}
QString Window::profileKey()const{return profile->currentData().toString();}
QJsonObject Window::selectedEffect()const{for(auto v:project.data["effects"].toArray())if(v.toObject()["id"]==selectedId)return v.toObject();return {};}
void Window::refreshExplorer(){files->effectsRoot=project.path.isEmpty()?QString():QDir::cleanPath(QFileInfo(project.path).absolutePath()+"/effects").toLower();tree->setToolTip("Effect checkboxes edit: "+(profileKey().isEmpty()?QString("Global"):profileKey()));auto previous=files->checks;files->checks.clear();for(auto v:project.data["effects"].toArray()){auto e=v.toObject();files->checks[scriptPath(project.path,e["script"].toString()).toLower()]=project.resolved(e,profileKey())["enabled"].toBool();}for(auto path:previous.keys())files->refresh(path);for(auto path:files->checks.keys())files->refresh(path);updateTabs();}
void Window::setValue(const QString& group,const QString& name,const QJsonValue& value,bool reset){
    require(!hasRawDrafts(),"Save or discard JSON edits before changing effect settings");if(!group.isEmpty() && !effectDefaults.contains(selectedId)){auto e=selectedEffect();LuaEffect meta(selectedId,readSource(scriptPath(project.path,e["script"].toString())),e["script"].toString());cacheDefaults(selectedId,meta);}auto key=profileKey();if(key.isEmpty()){auto array=project.data["effects"].toArray();for(int i=0;i<array.size();++i){auto e=array[i].toObject();if(e["id"]!=selectedId)continue;if(group.isEmpty()){if(reset)e.remove(name);else e[name]=value;}else{auto o=e[group].toObject();if(reset)o.remove(name);else o[name]=value;e[group]=o;}array[i]=e;}project.data["effects"]=array;}
    else{auto patch=project.aircraft.value(key)["effects"].toObject()[selectedId].toObject();if(group.isEmpty()){if(reset)patch.remove(name);else patch[name]=value;}else{auto o=patch[group].toObject();if(reset)o.remove(name);else o[name]=value;if(o.isEmpty())patch.remove(group);else patch[group]=o;}project.setOverride(key,selectedId,patch);}
    updateTabs();apply();
}
void Window::rebuildParameters(){refreshExplorer();enabledButton->setEnabled(!selectedId.isEmpty());{QSignalBlocker block(enabledButton);bool enabled=!selectedId.isEmpty() && project.resolved(selectedEffect(),profileKey())["enabled"].toBool();enabledButton->setChecked(enabled);enabledButton->setText(enabled?"Enabled":"Disabled");}paramPane->setVisible(!selectedScript.endsWith(".sv",Qt::CaseInsensitive));paramPane->setEnabled(!hasRawDrafts());while(parameters->rowCount())parameters->removeRow(0);if(selectedId.isEmpty()){parameters->addRow(new QLabel("Select a registered effect to edit parameters."));return;}auto e=selectedEffect();if(e.isEmpty())return;
    try{LuaEffect meta(selectedId,readSource(scriptPath(project.path,selectedScript)),selectedScript);cacheDefaults(selectedId,meta);updateTabs();e=selectedEffect();meta.configure(project.resolved(e,profileKey()),local["channels"].toObject());

        auto decorate=[&](const QString& group,const QString& name,QWidget* widget){auto* row=new QWidget;auto* layout=new QHBoxLayout(row);layout->setContentsMargins(0,0,0,0);layout->setSpacing(5);if(widget)layout->addWidget(widget,widget->sizePolicy().horizontalPolicy()==QSizePolicy::Fixed?0:1);auto* origin=new QLabel;origin->setObjectName("origin");origin->setProperty("group",group);origin->setProperty("key",name);origin->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Preferred);layout->addWidget(origin);auto* reset=new QPushButton(profileKey().isEmpty()?"Reset to Default":"Reset to Global");reset->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Fixed);layout->addWidget(reset);layout->addStretch(widget && widget->sizePolicy().horizontalPolicy()!=QSizePolicy::Fixed?0:1);connect(reset,&QPushButton::clicked,this,[this,group,name]{guarded([&]{setValue(group,name,{},true);rebuildParameters();});});return row;};
        auto addRow=[&](const QString& group,const QString& name,QWidget* widget){auto* row=decorate(group,name,widget);row->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);parameters->addRow(name,row);};
        for(auto p:meta.parameters) {
            auto value=meta.settings["parameters"].toObject()[p.name];
            if(p.boolean) {
                auto* check=new QCheckBox;check->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);check->setChecked(value.toBool());
                connect(check,&QCheckBox::toggled,this,[this,p](bool v){setValue("parameters",p.name,v);});addRow("parameters",p.name,check);
            } else if(p.type=="enum") {
                auto* choice=new QComboBox;choice->addItems(p.choices);choice->setCurrentText(value.toString());
                connect(choice,&QComboBox::currentTextChanged,this,[this,p](const QString& v){setValue("parameters",p.name,v);});addRow("parameters",p.name,choice);
            } else if(p.type=="haptic_power") {
                auto current=std::make_shared<QJsonObject>(value.toObject());auto* host=new QWidget;auto* form=new QHBoxLayout(host);form->setContentsMargins(0,0,0,0);form->setSpacing(5);
                auto publish=[this,p,current]{setValue("parameters",p.name,*current);};
                auto* power=numericControl(p.minimum,p.maximum,1,(*current)["strength"].toDouble(),true,[current,publish](double v){(*current)["strength"]=v;publish();});power->setMinimumWidth(125);form->addWidget(power,1);
                auto* check=new QCheckBox("PWM");check->setChecked((*current)["pwm"].toBool());form->addWidget(check);
                auto* timing=new QWidget;auto* timingRow=new QHBoxLayout(timing);timingRow->setContentsMargins(0,0,0,0);
                auto* on=new QSpinBox;auto* period=new QSpinBox;period->setRange(20,60000);period->setSingleStep(20);period->setValue((*current)["period_ms"].toInt());on->setRange(0,period->value());on->setSingleStep(20);on->setValue((*current)["on_ms"].toInt());on->setKeyboardTracking(false);period->setKeyboardTracking(false);
                timingRow->addWidget(new QLabel("ON"));timingRow->addWidget(on);timingRow->addWidget(new QLabel("Period"));timingRow->addWidget(period);on->setFixedWidth(68);period->setFixedWidth(68);on->setToolTip("ON time in milliseconds");period->setToolTip("Period in milliseconds");timingRow->setSpacing(4);form->addWidget(timing);timing->setVisible(check->isChecked());
                connect(check,&QCheckBox::toggled,host,[current,publish,timing](bool v){(*current)["pwm"]=v;timing->setVisible(v);publish();});
                connect(on,&QSpinBox::valueChanged,host,[current,publish](int v){(*current)["on_ms"]=v;publish();});
                connect(period,&QSpinBox::valueChanged,host,[current,publish,on](int v){QSignalBlocker block(on);on->setMaximum(v);(*current)["period_ms"]=v;(*current)["on_ms"]=on->value();publish();});
                addRow("parameters",p.name,host);
            } else addRow("parameters",p.name,numericControl(p.minimum,p.maximum,p.step,value.toDouble(),p.type=="int",[this,p](double v){setValue("parameters",p.name,v);}));
        }

        if(!meta.outputs.isEmpty()){
            auto* table=new QTableWidget(1,static_cast<int>(meta.outputs.size()));table->setHorizontalHeaderLabels(meta.outputs);table->verticalHeader()->hide();table->setEditTriggers(QAbstractItemView::NoEditTriggers);table->setSelectionMode(QAbstractItemView::NoSelection);table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
            int column=0,rowHeight=0;for(auto name:meta.outputs){QStringList current;for(auto v:meta.settings["outputs"].toObject()[name].toArray())current<<v.toString();auto* button=new QPushButton(current.isEmpty()?"No destinations":current.join("\n"));button->setMinimumHeight(button->fontMetrics().height()*std::max(1,static_cast<int>(current.size()))+14);connect(button,&QPushButton::clicked,this,[this,name]{guarded([&]{QStringList current;auto routes=project.resolved(selectedEffect(),profileKey())["outputs"].toObject();auto destinations=routes.contains(name)?routes[name]:effectDefaults[selectedId]["outputs"].toObject()[name];for(auto v:destinations.toArray())current<<v.toString();bool accepted=false;auto chosen=chooseSlots(this,current,"Output: "+name,&accepted,local["channels"].toObject());if(!accepted)return;QJsonArray values;for(auto s:chosen)values<<s;setValue("outputs",name,values);rebuildParameters();});});auto* cell=new QWidget;auto* layout=new QVBoxLayout(cell);layout->setContentsMargins(4,4,4,4);layout->setSpacing(4);layout->addWidget(button);layout->addWidget(decorate("outputs",name,nullptr));rowHeight=std::max(rowHeight,cell->sizeHint().height());table->setCellWidget(0,column++,cell);}
            table->setRowHeight(0,rowHeight);table->setFixedHeight(table->horizontalHeader()->sizeHint().height()+rowHeight+table->horizontalScrollBar()->sizeHint().height()+6);parameters->addRow("Outputs",table);
        }
        for(auto item:meta.unresolved)parameters->addRow(new QLabel("Unresolved (preserved): "+item));
    }catch(const std::exception& error){parameters->addRow(new QLabel(QString("Script error: ")+error.what()));addLog(error.what());}
    refreshExplorer();
    for(auto* table:parameterHost->findChildren<QTableWidget*>()){
        table->resizeColumnsToContents();int height=0;
        for(int i=0;i<table->columnCount();++i)height=std::max(height,table->cellWidget(0,i)->sizeHint().height());
        table->setRowHeight(0,height);table->setFixedHeight(table->horizontalHeader()->sizeHint().height()+height+table->horizontalScrollBar()->sizeHint().height()+6);
    }
}
void Window::tick(){if(nowMs()>=nextFileScan){nextFileScan=nowMs()+1000;try{syncFiles();}catch(const std::exception& e){addLog(QString("File sync: ")+e.what());}}auto snapshot=engine.snapshot();for(const auto& message:snapshot.logs){addLog(message);if(message.startsWith("ERROR "))for(auto v:project.data["effects"].toArray()){auto e=v.toObject();if(message.contains(e["id"].toString())){auto path=scriptPath(project.path,e["script"].toString()).toLower();files->errors[path]=message;files->refresh(path);}}}local["motors"]=snapshot.mappings;currentAircraft=snapshot.aircraft;if(!currentAircraft.isEmpty() && profile->findData(currentAircraft)<0)profile->addItem("Specific Aircraft: "+currentAircraft,currentAircraft);state->setText(gameLabel(project.data["game"].toString())+": "+snapshot.sim+" | HID: "+QString::number(std::count_if(snapshot.motors.begin(),snapshot.motors.end(),[](const Motor& motor){return motor.connected;}))+" motors connected | Aircraft: "+(currentAircraft.isEmpty()?"Unknown":currentAircraft)+" | "+snapshot.execution);
    if(nowMs()>=nextTelemetryRefresh){
        nextTelemetryRefresh=nowMs()+100;const auto& t=snapshot.telemetry;const auto game=project.data["game"].toString();
        auto values=t.game==game?telemetryObject(t):QJsonObject{};bool valid=t.game==game && snapshot.telemetryConnected && t.valid(nowMs());
        telemetryStatus->setText(t.game!=game || !snapshot.telemetryConnected?"Disconnected":!t.sequence?"Waiting for telemetry":QString("%1 | Age: %2 ms").arg(valid?"Live":t.paused?"Inactive / paused":"Stale").arg(nowMs()-t.received));
        QMap<QString,QPair<QJsonValue,QString>> fields;
        if(game.startsWith("MSFS")){
            for(const auto& field:simVariables())fields[field.key]={values[field.key],field.name+" ["+field.unit+"]"};
            for(const auto& name:{"sample_id","received_ms","aircraft_key","sim_time_advanced"})fields[name]={values[name],"Host / session"};
            fields["msfs.paused"]={values["msfs"].toObject()["paused"],"Pause_EX1"};
        } else if(game=="XPlane12"){
            for(const auto& field:xplaneVariables())fields[field.name]={values[field.name],"Native dataref / "+QString::number(field.frequency)+" Hz"};
        } else if(game=="WarThunder"){
            for(const auto& endpoint:{QString("state"),QString("indicators")}){
                const auto object=values[endpoint].toObject();auto keys=endpoint=="state"?warThunderStateKeys():warThunderIndicatorKeys();keys+=object.keys();keys.removeDuplicates();
                for(const auto& key:keys)fields[endpoint+"[\""+key+"\"]"]={object[key],"/"+endpoint+" / native JSON"};
            }
        }
        QStringList shown;for(int i=0;i<telemetryView->topLevelItemCount();++i)shown<<telemetryView->topLevelItem(i)->text(0);
        if(shown!=fields.keys()){telemetryView->clear();for(auto it=fields.begin();it!=fields.end();++it)new QTreeWidgetItem(telemetryView,{it.key(),"Unavailable",it.value().second});}
        int rowIndex=0;for(auto it=fields.begin();it!=fields.end();++it){auto* row=telemetryView->topLevelItem(rowIndex++);const auto v=it.value().first;
            auto text=v.isNull()||v.isUndefined()?QString("Unavailable"):v.isBool()?(v.toBool()?QString("true"):QString("false")):v.isDouble()?QString::number(v.toDouble(),'g',10):v.isString()?v.toString():QString::fromUtf8(v.isObject()?QJsonDocument(v.toObject()).toJson(QJsonDocument::Compact):QJsonDocument(v.toArray()).toJson(QJsonDocument::Compact));
            row->setText(1,text);row->setToolTip(1,text);row->setForeground(1,valid?palette().text():palette().placeholderText());
        }
        for(int i=0;i<openFiles->count();++i){auto* doc=openFiles->widget(i);doc->setProperty("game",game);doc->setProperty("wtStateKeys",values["state"].toObject().keys());doc->setProperty("wtIndicatorKeys",values["indicators"].toObject().keys());}
    }
    QString selected;
    if(monitor->currentRow()>=0 && monitor->item(monitor->currentRow(),0))selected=monitor->item(monitor->currentRow(),0)->data(Qt::UserRole).toString();
    QList<Motor> connected;for(const auto& m:snapshot.motors)if(m.connected)connected<<m;
    const auto orderedSlots=logicalSlots();auto rank=[&](const Motor& m){int result=static_cast<int>(orderedSlots.size());for(const auto& slot:m.destinations){int i=static_cast<int>(orderedSlots.indexOf(slot));if(i>=0)result=std::min(result,i);}return result;};std::sort(connected.begin(),connected.end(),[&](const Motor& a,const Motor& b){if(rank(a)!=rank(b))return rank(a)<rank(b);int order=QString::compare(a.label,b.label,Qt::CaseInsensitive);return order?order<0:a.id<b.id;});
    monitor->setRowCount(connected.size());monitor->clearSelection();monitor->setCurrentCell(-1,-1);
    for(int i=0;i<connected.size();++i){
        const auto& m=connected[i];QStringList values{m.label,m.destinations.isEmpty()?"Unassigned":m.destinations.join(", "),QString::number(m.sent)};
        for(int j=0;j<3;++j){auto* item=monitor->item(i,j);if(!item){item=new QTableWidgetItem;monitor->setItem(i,j,item);}item->setText(values[j]);
            if(j==0){item->setData(Qt::UserRole,m.id);item->setToolTip(m.label+"\n"+m.id+(m.ambiguous && m.destinations.isEmpty()?"\nManual assignment required":""));}
            if(j==2)item->setTextAlignment(Qt::AlignCenter);
        }
        if(m.id==selected){monitor->setCurrentCell(i,0);monitor->selectRow(i);}
    }
}
void Window::saveLocal(){if(QCoreApplication::arguments().contains("--ui-check"))return;local["layout_version"]=4;local["tick_ms"]=engine.tickMs();local["maximum_update_period_ms"]=engine.maximumUpdatePeriodMs();local.remove("simconnect_dll");local["version"]=1;local["geometry"]=QString::fromLatin1(saveGeometry().toBase64());local["layout"]=QString::fromLatin1(saveState().toBase64());writeJson(localPath(),local);}
void Window::mapping(){int row=monitor->currentRow();require(row>=0,"Select a motor");auto id=monitor->item(row,0)->data(Qt::UserRole).toString();QStringList current;for(auto v:local["motors"].toObject()[id].toObject()["slots"].toArray())current<<v.toString();bool accepted=false;auto chosen=chooseSlots(this,current,"Motor Mapping",&accepted);if(!accepted)return;engine.mapMotor(id,chosen);tick();saveLocal();}
void Window::motorTest(){int row=monitor->currentRow();require(row>=0,"Select a motor");auto id=monitor->item(row,0)->data(Qt::UserRole).toString();QDialog dialog(this);dialog.setWindowTitle("Motor Test — "+monitor->item(row,0)->text());QFormLayout layout(&dialog);QSpinBox strength,on,period,duration;strength.setRange(0,255);strength.setValue(1);on.setRange(0,5000);on.setValue(20);period.setRange(20,5000);period.setValue(80);duration.setRange(1,5000);duration.setValue(3000);QCheckBox pwm("PWM");QLabel effective;layout.addRow("Strength",&strength);layout.addRow(&pwm);layout.addRow("ON ms",&on);layout.addRow("Period ms",&period);layout.addRow("Duration ms (max 5000)",&duration);layout.addRow(&effective);auto update=[&]{effective.setText(QString("Effective PWM: %1 / %2 ms").arg(int(std::floor(double(on.value())/engine.tickMs()+0.5))*engine.tickMs()).arg(int(std::floor(double(period.value())/engine.tickMs()+0.5))*engine.tickMs()));};connect(&on,&QSpinBox::valueChanged,&dialog,update);connect(&period,&QSpinBox::valueChanged,&dialog,update);update();QPushButton start("Start"),stop("Stop");layout.addRow(&start,&stop);connect(&start,&QPushButton::clicked,&dialog,[&]{guarded([&]{engine.test(id,pwm.isChecked()?Mode::Pwm:Mode::Constant,strength.value(),on.value(),period.value(),duration.value());});});connect(&stop,&QPushButton::clicked,&dialog,[this]{engine.stopTest();});dialog.exec();engine.stopTest();}
}
