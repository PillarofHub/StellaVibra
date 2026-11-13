#include "window.h"
#include "dcs_install.h"
#include <ShlObj.h>

namespace sv {
namespace {
QStringList discoverProfiles(QString& error) {
    PWSTR path=nullptr;
    const auto result=SHGetKnownFolderPath(FOLDERID_SavedGames,0,nullptr,&path);
    if(FAILED(result)){if(path)CoTaskMemFree(path);error="Could not locate Windows Saved Games.";return {};}
    const QDir saved(QString::fromWCharArray(path));CoTaskMemFree(path);
    if(!saved.exists() || !QFileInfo(saved.absolutePath()).isReadable()){
        error="Windows Saved Games is missing or inaccessible.";return {};
    }
    QStringList profiles;
    for(const auto& entry:saved.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot,QDir::Name|QDir::IgnoreCase)) {
        const auto name=entry.fileName();const QDir profile(entry.absoluteFilePath());
        // Standard profiles plus custom -w profiles with DCS-specific artifacts.
        if(name.compare("DCS",Qt::CaseInsensitive)==0 || name.startsWith("DCS.",Qt::CaseInsensitive) ||
            (QFileInfo::exists(profile.filePath("Config/options.lua")) && QFileInfo::exists(profile.filePath("Logs/dcs.log"))))
            profiles<<profile.absolutePath();
    }
    if(profiles.isEmpty())error="No DCS profile found in Windows Saved Games. Start DCS once, then reopen this dialog.";
    return profiles;
}
}
void Window::checkDcsTelemetry() {
    QString error;const auto profiles=discoverProfiles(error);
    if(profiles.isEmpty()){addLog("DCS telemetry: "+error);return;}
    for(const auto& dcsProfile:profiles)addLog("DCS telemetry ["+QFileInfo(dcsProfile).fileName()+"]: "+inspectDcsInstallation(QDir(dcsProfile).filePath("Scripts")).text);
}
void Window::setDcsTelemetryDialog() {
    QDialog dialog(this);dialog.setWindowTitle("Set DCS Telemetry");dialog.resize(560,220);
    QVBoxLayout layout(&dialog);QString error;const auto profiles=discoverProfiles(error);
    QComboBox dcsProfile;
    for(const auto& path:profiles)dcsProfile.addItem(QDir::toNativeSeparators(QDir(path).filePath("Scripts/StellaVibra Telemetry")),path);
    layout.addWidget(&dcsProfile);dcsProfile.setVisible(profiles.size()>1);
    QLabel target;target.setWordWrap(true);target.setTextFormat(Qt::PlainText);layout.addWidget(&target);target.setVisible(profiles.size()<=1);
    QFormLayout timing;QSpinBox interval;interval.setRange(10,100);interval.setValue(50);interval.setSuffix(" ms");
    interval.setToolTip("Restart DCS after adding, restoring, updating or removing scripts. Timing changes apply after restart.");
    timing.addRow("Update interval",&interval);layout.addLayout(&timing);
    QPushButton install("Add Scripts"),remove("Remove Scripts"),done("Done");
    install.setEnabled(false);remove.setEnabled(false);layout.addWidget(&install);layout.addWidget(&remove);layout.addWidget(&done);
    DcsInstallation installed;
    auto inspect=[&]{
        if(dcsProfile.currentIndex()<0){target.setText(error);interval.setEnabled(false);install.setEnabled(false);remove.setEnabled(false);return;}
        const auto scripts=QDir(dcsProfile.currentData().toString()).filePath("Scripts");
        installed=inspectDcsInstallation(scripts);
        target.setVisible(profiles.size()<=1);
        target.setText(QDir::toNativeSeparators(QDir(scripts).filePath("StellaVibra Telemetry")));
        target.setToolTip(installed.text);dcsProfile.setToolTip(installed.text);install.setToolTip(installed.text);
        install.setText(installed.canRestore?"Restore Scripts":installed.canUpdate?"Update Scripts":"Add Scripts");
        {QSignalBlocker block(interval);interval.setValue(installed.interval);}
        const bool available=installed.canAdd || installed.canUpdate || installed.canRestore;
        interval.setEnabled(available);install.setEnabled(available);remove.setEnabled(installed.canRemove);
        if(!available){target.setText(target.text()+"\n"+installed.text);target.show();}
    };
    connect(&dcsProfile,&QComboBox::currentIndexChanged,&dialog,inspect);
    auto confirm=[&](bool restoring,const QString& details){
        QMessageBox box(QMessageBox::Question,restoring?"Restore DCS Scripts":"Remove DCS Scripts",
            restoring?"The Stella Vibra loader or scripts are missing or differ from the installed originals. Restore loader.lua, telemetry.lua, config.lua and manifest.json using the bundled scripts and selected interval?\n\nEach listed Export.lua line will be replaced in full, including any other code on that same line. Other lines will be preserved.":
            "Remove the listed Stella Vibra lines from Export.lua and permanently delete the entire StellaVibra Telemetry folder, including modified or additional files?\n\nEach listed line is removed in full, including any other code on that same line. Other Export.lua lines will be preserved.",
            QMessageBox::Yes|QMessageBox::No,&dialog);
        box.setDefaultButton(QMessageBox::No);box.setDetailedText(details);return box.exec()==QMessageBox::Yes;
    };
    connect(&install,&QPushButton::clicked,&dialog,[&]{
        guarded([&]{installDcsTelemetry(QDir(dcsProfile.currentData().toString()).filePath("Scripts"),interval.value(),
            [&](const QString& details){return confirm(true,details);});});
        inspect();
    });
    connect(&remove,&QPushButton::clicked,&dialog,[&]{
        guarded([&]{removeDcsTelemetry(QDir(dcsProfile.currentData().toString()).filePath("Scripts"),
            [&](const QString& details){return confirm(false,details);});});
        inspect();
    });
    connect(&done,&QPushButton::clicked,&dialog,&QDialog::accept);
    inspect();dialog.exec();
}
}
