#include "window.h"
namespace sv {
void Window::newProject() {
    if(!confirmDiscard())return;
    QDialog dialog(this);dialog.setWindowTitle("New Project");dialog.resize(620,180);QFormLayout form(&dialog);
    QWidget location;QHBoxLayout row(&location);row.setContentsMargins(0,0,0,0);QLineEdit directory;QPushButton browse("Browse...");row.addWidget(&directory,1);row.addWidget(&browse);form.addRow("Project Directory",&location);
    QRadioButton game("MSFS 2024");game.setChecked(true);form.addRow("Target Game",&game);QCheckBox generate("Generate default Lua scripts");generate.setChecked(true);form.addRow(&generate);
    QDialogButtonBox buttons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);form.addRow(&buttons);
    connect(&browse,&QPushButton::clicked,&dialog,[&]{auto path=QFileDialog::getExistingDirectory(&dialog,"Project Directory",directory.text());if(!path.isEmpty())directory.setText(path);});
    connect(&buttons,&QDialogButtonBox::accepted,&dialog,[&]{auto path=directory.text().trimmed();if(path.isEmpty() || !QFileInfo(path).isAbsolute() || (QFileInfo::exists(path) && (!QFileInfo(path).isDir() || !QDir(path).entryList(QDir::AllEntries|QDir::NoDotAndDotDot|QDir::Hidden|QDir::System).isEmpty()))){QMessageBox::warning(&dialog,"Project Directory","Choose an empty directory or enter a new absolute directory path.");return;}dialog.accept();});
    connect(&buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);if(dialog.exec()!=QDialog::Accepted)return;
    auto folder=QDir::cleanPath(directory.text().trimmed());QJsonArray effects;
    QMap<QString,QString> sources;
    if(generate.isChecked()) {
        auto base=QCoreApplication::applicationDirPath()+"/effects";QDirIterator it(base,{"*.lua"},QDir::Files,QDirIterator::Subdirectories);
        while(it.hasNext()){auto file=it.next();sources["effects/"+QDir(base).relativeFilePath(file)]=file;}
        require(!sources.isEmpty(),"Bundled default scripts are missing");
    }
    require(QDir().mkpath(folder+"/effects") && QDir().mkpath(folder+"/aircrafts"),"Cannot create project directory");
    for(auto it=sources.begin();it!=sources.end();++it){QFile f(it.value());require(f.open(QIODevice::ReadOnly),"Cannot read bundled script");LuaEffect meta("new",QString::fromUtf8(f.readAll()),it.key());QJsonObject ports;
        for(auto name:meta.outputs){QJsonArray targets;if(name=="Left Gear")targets<<"Throttle:1";else if(name=="Right Gear")targets<<"Throttle:2";else if(name=="Airframe")targets<<"Stick:1";else if(name.startsWith("Engine "))targets<<"Throttle:"+name.section(' ',1,1);if(!targets.isEmpty())ports[name]=targets;}
        auto target=folder+"/"+it.key();require(QDir().mkpath(QFileInfo(target).absolutePath()) && QFile::copy(it.value(),target),"Cannot copy bundled script: "+it.key());
        effects<<QJsonObject{{"id",QUuid::createUuid().toString(QUuid::WithoutBraces)},{"script",it.key()},{"outputs",ports}};
    }
    auto path=folder+"/"+QFileInfo(folder).fileName()+".sv";writeJson(path,{{"version",1},{"id",QUuid::createUuid().toString(QUuid::WithoutBraces)},{"name",QFileInfo(folder).fileName()},{"game","MSFS2024"},{"effects",effects}});loadProject(path);
}
void Window::setTickDialog() {
    QDialog dialog(this);dialog.setWindowTitle("Set Tick");QFormLayout layout(&dialog);QSpinBox tick;tick.setRange(5,100);tick.setSuffix(" ms");tick.setValue(engine.tickMs());layout.addRow("HID / PWM tick",&tick);
    QLabel hint("Changing the tick clears active output requests and resets effect state. Telemetry sampling is unchanged.");hint.setWordWrap(true);layout.addRow(&hint);QPushButton reset("Reset to Default (20 ms)");layout.addRow(&reset);connect(&reset,&QPushButton::clicked,&dialog,[&]{tick.setValue(20);});
    QDialogButtonBox buttons(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);layout.addRow(&buttons);connect(&buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);connect(&buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    if(dialog.exec()==QDialog::Accepted && tick.value()!=engine.tickMs()){engine.setTick(tick.value());saveLocal();addLog(QString("HID/PWM tick set to %1 ms").arg(tick.value()));}
}
void Window::aboutDialog() {
    QDialog dialog(this);dialog.setWindowTitle("About Stella Vibra");dialog.resize(780,560);QVBoxLayout layout(&dialog);QLabel title("Stella Vibra 0.1.0 — Lua haptic effects for MSFS 2024");layout.addWidget(&title);QTabWidget tabs;layout.addWidget(&tabs);
    const auto root=QCoreApplication::applicationDirPath();
        auto* notices=new QPlainTextEdit;notices->setReadOnly(true);notices->setPlainText("Stella Vibra: MIT.\n\nQt 6.8.3: dynamically linked under LGPL-3.0. You may replace or modify Qt DLLs and debug/reverse-engineer for that purpose. Corresponding source is in sources/. Full notices and licenses are in licenses/.\n\nLua 5.4.8: MIT, Copyright 1994-2025 Lua.org, PUC-Rio.\n\nAurora Vibra reference: MIT, Copyright 2025 Rodrigo Troncoso; 2026 chegevarko1982. Device protocols and effect conditions informed this implementation; its waveform mixer is not used.\n\nMicrosoft Visual C++ runtime and SimConnect remain subject to Microsoft terms. The official SimConnect client is staged for local use; public redistribution rights have not been confirmed. See SIMCONNECT-LICENSING.md and licenses/MSFS SDK EULA.pdf.");tabs.addTab(notices,"Notices");
    for(const auto& pair:QList<QPair<QString,QString>>{{"MIT License","LICENSE"},{"Qt LGPL","licenses/LGPL-3.0.txt"},{"Aurora MIT","licenses/Aurora-MIT.txt"}}){auto* text=new QPlainTextEdit;text->setReadOnly(true);QFile file(root+"/"+pair.second);text->setPlainText(file.open(QIODevice::ReadOnly)?QString::fromUtf8(file.readAll()):"License file missing: "+pair.second);tabs.addTab(text,pair.first);}
    QDialogButtonBox buttons(QDialogButtonBox::Close);layout.addWidget(&buttons);connect(&buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);dialog.exec();
}
}
