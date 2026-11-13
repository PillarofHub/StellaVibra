#include "window.h"
#include "game_catalog.h"
namespace sv {
void Window::newProject() {
    if(!confirmDiscard())return;
    QDialog dialog(this);dialog.setWindowTitle("New Project");dialog.resize(620,180);QFormLayout form(&dialog);
    QWidget location;QHBoxLayout row(&location);row.setContentsMargins(0,0,0,0);QLineEdit directory;QPushButton browse("Browse...");row.addWidget(&directory,1);row.addWidget(&browse);form.addRow("Project Directory",&location);
    QWidget games;QVBoxLayout gameLayout(&games);gameLayout.setContentsMargins(0,0,0,0);QButtonGroup gameGroup;const auto choices=targetGames();for(int i=0;i<choices.size();++i){auto* button=new QRadioButton(gameLabel(choices[i]));gameLayout.addWidget(button);gameGroup.addButton(button,i);if(!i)button->setChecked(true);}form.addRow("Target Game",&games);QCheckBox generate("Generate default Lua scripts");generate.setChecked(true);form.addRow(&generate);
    connect(&gameGroup,&QButtonGroup::idClicked,&dialog,[&](int id){const bool available=choices[id]!="DCS";generate.setEnabled(available);generate.setChecked(available);});
    QDialogButtonBox buttons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);form.addRow(&buttons);
    connect(&browse,&QPushButton::clicked,&dialog,[&]{
        // The native existing-directory picker rejects new paths. This picker
        // deliberately accepts and creates them without persisting Qt dialog history.
        QDialog picker(&dialog);picker.setWindowTitle("Project Directory");picker.resize(640,480);QVBoxLayout layout(&picker);
        auto initial=QDir::fromNativeSeparators(directory.text().trimmed());
        if(initial.isEmpty())initial=appRoot();
        auto base=QFileInfo(initial).absoluteFilePath();
        while(!QFileInfo(base).isDir()){auto parent=QFileInfo(base).absolutePath();if(parent==base){base=appRoot();break;}base=parent;}
        QLineEdit path(initial);layout.addWidget(new QLabel("Directory path",&picker));layout.addWidget(&path);
        QFileSystemModel folders;folders.setFilter(QDir::AllDirs|QDir::NoDotAndDotDot|QDir::Hidden|QDir::System);folders.setRootPath(QString());
        QTreeView tree;tree.setModel(&folders);for(int column=1;column<folders.columnCount();++column)tree.hideColumn(column);tree.setHeaderHidden(true);layout.addWidget(&tree,1);
        auto index=folders.index(base);tree.setCurrentIndex(index);for(auto ancestor=index;ancestor.isValid();ancestor=ancestor.parent())tree.expand(ancestor);tree.scrollTo(index);
        connect(&tree,&QTreeView::clicked,&picker,[&](const QModelIndex& selected){auto folder=folders.filePath(selected);if(!folder.isEmpty()){base=folder;path.setText(QDir::toNativeSeparators(folder));}});
        QDialogButtonBox pickerButtons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);pickerButtons.button(QDialogButtonBox::Ok)->setText("Select Folder");layout.addWidget(&pickerButtons);
        connect(&pickerButtons,&QDialogButtonBox::rejected,&picker,&QDialog::reject);
        connect(&pickerButtons,&QDialogButtonBox::accepted,&picker,[&]{
            auto entered=QDir::fromNativeSeparators(path.text().trimmed());if(entered.isEmpty())return;
            auto selected=QDir::cleanPath(QDir(base).absoluteFilePath(entered));
            if((QFileInfo::exists(selected) && !QFileInfo(selected).isDir()) || !QDir().mkpath(selected)){
                QMessageBox::warning(&picker,"Project Directory","Cannot create or access directory:\n"+QDir::toNativeSeparators(selected));return;
            }
            directory.setText(QDir::toNativeSeparators(selected));picker.accept();
        });
        picker.exec();
    });
    connect(&buttons,&QDialogButtonBox::accepted,&dialog,[&]{
        auto path=QDir::cleanPath(QDir::fromNativeSeparators(directory.text().trimmed()));
        if(path.isEmpty() || !QFileInfo(path).isAbsolute() || QFileInfo(path).fileName().isEmpty() || (QFileInfo::exists(path) && !QFileInfo(path).isDir())){QMessageBox::warning(&dialog,"Project Directory","Enter an absolute directory path. A missing directory will be created.");return;}
        const bool nonEmpty=QFileInfo::exists(path) && !QDir(path).entryList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::Hidden|QDir::System).isEmpty();
        if(nonEmpty && QMessageBox::question(&dialog,"Directory Is Not Empty","The directory contains files or folders. Replace the project file and any matching default scripts?\n\nOther files are retained. Existing Lua files in effects will also be discovered.\n\n"+path,QMessageBox::Yes|QMessageBox::No,QMessageBox::No)!=QMessageBox::Yes)return;
        dialog.accept();
    });
    connect(&buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);if(dialog.exec()!=QDialog::Accepted)return;
    const auto game=choices[gameGroup.checkedId()];auto folder=QDir::cleanPath(QDir::fromNativeSeparators(directory.text().trimmed()));QJsonArray effects;
    QMap<QString,QString> sources;
    if(generate.isChecked() && game!="DCS") {
        auto base=appRoot()+"/effects/"+game;QDirIterator it(base,{"*.lua"},QDir::Files,QDirIterator::Subdirectories);
        while(it.hasNext()){auto file=it.next();sources["effects/"+QDir(base).relativeFilePath(file)]=file;}
        require(!sources.isEmpty(),"Bundled default scripts are missing");
    }
    engine.clearProject();
    require(QDir().mkpath(folder+"/effects") && QDir().mkpath(folder+"/aircrafts"),"Cannot create project directory");
    for(auto it=sources.begin();it!=sources.end();++it){QFile f(it.value());require(f.open(QIODevice::ReadOnly),"Cannot read bundled script");LuaEffect meta("new",QString::fromUtf8(f.readAll()),it.key());
        auto target=scriptPath(folder+"/"+QFileInfo(folder).fileName()+".sv",it.key());require(QDir().mkpath(QFileInfo(target).absolutePath()),"Cannot create script directory");QSaveFile scriptFile(target);require(scriptFile.open(QIODevice::WriteOnly),"Cannot write bundled script: "+it.key());f.seek(0);auto content=f.readAll();require(scriptFile.write(content)==content.size() && scriptFile.commit(),"Cannot save bundled script: "+it.key());
        effects<<QJsonObject{{"id",QUuid::createUuid().toString(QUuid::WithoutBraces)},{"script",it.key()}};
    }
    auto path=folder+"/"+QFileInfo(folder).fileName()+".sv";writeJson(path,{{"version",1},{"id",QUuid::createUuid().toString(QUuid::WithoutBraces)},{"name",QFileInfo(folder).fileName()},{"game",game},{"effects",effects}});loadProject(path);
}
void Window::setTimingsDialog() {
    QDialog dialog(this);dialog.setWindowTitle("Set Timings");QFormLayout layout(&dialog);
    QSpinBox tick;tick.setRange(5,100);tick.setSuffix(" ms");tick.setValue(engine.tickMs());layout.addRow("HID tick",&tick);
    QSpinBox updatePeriod;updatePeriod.setRange(5,500);updatePeriod.setSuffix(" ms");updatePeriod.setValue(engine.maximumUpdatePeriodMs());layout.addRow("Maximum update period",&updatePeriod);
    QPushButton reset("Reset to Defaults");layout.addRow(&reset);connect(&reset,&QPushButton::clicked,&dialog,[&]{tick.setValue(20);updatePeriod.setValue(100);});
    QDialogButtonBox buttons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);layout.addRow(&buttons);connect(&buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);connect(&buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    if(dialog.exec()!=QDialog::Accepted)return;
    if(tick.value()!=engine.tickMs() || updatePeriod.value()!=engine.maximumUpdatePeriodMs()){
        if(tick.value()!=engine.tickMs())engine.setTick(tick.value());
        engine.setMaximumUpdatePeriod(updatePeriod.value());saveLocal();
        addLog(QString("Timings set: HID tick %1 ms, maximum update period %2 ms").arg(tick.value()).arg(updatePeriod.value()));
    }
}
void Window::setChannelsDialog() {
    require(!hasRawDrafts(),"Save or discard JSON edits before changing channels");
    QDialog dialog(this);dialog.setWindowTitle("Set Channels");QFormLayout form(&dialog);
    const auto counts=local["channels"].toObject();QMap<QString,QSpinBox*> controls;
    for(const auto& key:QStringList{"Yoke","Stick","Throttle","Pedal"}){
        auto* spin=new QSpinBox(&dialog);spin->setRange(0,4);spin->setValue(std::clamp(counts[key].toInt(4),0,4));controls[key]=spin;form.addRow(key=="Pedal"?"Rudder":key,spin);
    }

    QDialogButtonBox buttons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);form.addRow(&buttons);
    connect(&buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);connect(&buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    if(dialog.exec()!=QDialog::Accepted)return;
    auto updated=counts;for(auto it=controls.begin();it!=controls.end();++it)updated[it.key()]=it.value()->value();
    if(updated==counts)return;
    local["channels"]=updated;saveLocal();refreshDefaults();apply();rebuildParameters();
    addLog("Channel counts updated. Script default destinations were fitted; explicit destinations are retained.");
}
void Window::aboutDialog() {
    QDialog dialog(this);dialog.setWindowTitle("About Stella Vibra");dialog.resize(780,560);QVBoxLayout layout(&dialog);QLabel title("Stella Vibra "+QCoreApplication::applicationVersion()+" — Lua haptic effects for flight simulators");layout.addWidget(&title);QTabWidget tabs;layout.addWidget(&tabs);
    const auto root=appRoot();
        auto* notices=new QPlainTextEdit;notices->setReadOnly(true);notices->setPlainText("Stella Vibra: MIT.\n\nQt 6.8.3: dynamically linked under LGPL-3.0. You may replace or modify Qt DLLs and debug/reverse-engineer for that purpose. Corresponding source is in sources/. Full notices and licenses are in licenses/.\n\nLua 5.4.8: MIT, Copyright 1994-2025 Lua.org, PUC-Rio.\n\nAurora Vibra reference: MIT, Copyright 2025 Rodrigo Troncoso; 2026 chegevarko1982. Device protocols and effect conditions informed this implementation; its waveform mixer is not used.\n\nMicrosoft Visual C++ runtime and SimConnect remain subject to Microsoft terms. Unmodified SimConnect SDK clients are included by the maintainer. All rights remain with Microsoft Corporation and its licensors; Stella Vibra MIT does not apply to these DLLs. Redistribution permission has not been explicitly confirmed. This project is not endorsed or supported by Microsoft. See docs/THIRD-PARTY-NOTICES.md and licenses/MSFS SDK EULA.pdf.");tabs.addTab(notices,"Notices");
    for(const auto& pair:QList<QPair<QString,QString>>{{"MIT License","LICENSE"},{"Qt LGPL","licenses/LGPL-3.0.txt"},{"Aurora MIT","licenses/Aurora-MIT.txt"}}){auto* text=new QPlainTextEdit;text->setReadOnly(true);QFile file(root+"/"+pair.second);text->setPlainText(file.open(QIODevice::ReadOnly)?QString::fromUtf8(file.readAll()):"License file missing: "+pair.second);tabs.addTab(text,pair.first);}
    QDialogButtonBox buttons(QDialogButtonBox::Close);layout.addWidget(&buttons);connect(&buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);dialog.exec();
}
void Window::showDocumentation(const QString& relative,const QString& title) {
    const auto path=appRoot()+"/"+relative;QFile file(path);
    require(file.open(QIODevice::ReadOnly),"Documentation is missing or unreadable: "+path);file.close();
    auto* dialog=new QDialog(this);dialog->setAttribute(Qt::WA_DeleteOnClose);dialog->setWindowTitle(title);dialog->resize(940,720);
    auto* layout=new QVBoxLayout(dialog);auto* navigation=new QHBoxLayout;
    auto* back=new QPushButton("Back");auto* forward=new QPushButton("Forward");auto* home=new QPushButton("Home");
    navigation->addWidget(back);navigation->addWidget(forward);navigation->addWidget(home);navigation->addStretch();layout->addLayout(navigation);
    auto* browser=new QTextBrowser;browser->setOpenExternalLinks(true);layout->addWidget(browser);
    connect(back,&QPushButton::clicked,browser,&QTextBrowser::backward);connect(forward,&QPushButton::clicked,browser,&QTextBrowser::forward);connect(home,&QPushButton::clicked,browser,&QTextBrowser::home);
    connect(browser,&QTextBrowser::backwardAvailable,back,&QPushButton::setEnabled);connect(browser,&QTextBrowser::forwardAvailable,forward,&QPushButton::setEnabled);back->setEnabled(false);forward->setEnabled(false);
    browser->setSource(QUrl::fromLocalFile(path),QTextDocument::MarkdownResource);
    auto* buttons=new QDialogButtonBox(QDialogButtonBox::Close);layout->addWidget(buttons);connect(buttons,&QDialogButtonBox::rejected,dialog,&QDialog::close);dialog->show();
}
}
