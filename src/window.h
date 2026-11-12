#pragma once
#include "engine.h"
#include "editor.h"
#include "effect_files.h"
namespace sv {
class Window : public QMainWindow {
public:
    Window();
    ~Window() override;
protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    bool eventFilter(QObject* watched,QEvent* event) override;
private:
    Engine engine;
    Project project;
    Project savedProject;
    QJsonObject local;
    QMap<QString,QString> drafts;
    QSet<QString> dirtyEffects;
    QMap<QString,QByteArray> diskHashes;
    QMenu* recentMenu;
    QTabBar* fileTabs;
    QStackedWidget* openFiles;
    QMap<QString,QString> rawBases;
    QMap<QString,QJsonObject> effectDefaults;
    QWidget* paramPane;
    QLabel* rawNotice;
    QPushButton* pauseButton;
    QPushButton* enabledButton;
    QTreeWidget* telemetryView;
    QLabel* telemetryStatus;
    qint64 nextTelemetryRefresh=0;
    QByteArray defaultLayout;
    QList<QDockWidget*> proportionalDocks;
    QMap<QDockWidget*,QSizeF> dockRatios;
    QTimer captureLayoutTimer,resizeLayoutTimer;
    bool resizingLayout=false;
    QSize dockSpace() const;
    void captureDockRatios();
    void applyDockRatios();
    QSet<QString> effectDirectoriesToExpand;
    void expandEffectFolders();
    void setTimingsDialog(); void setChannelsDialog(); void aboutDialog();
    void showDocumentation(const QString& relative,const QString& title);
    bool paused=false;
    qint64 nextFileScan=0;
    QString selectedId,selectedScript,currentAircraft;
    bool refreshing=false,dirty=false;
    EffectFiles* files;
    QTreeView* tree;
    Editor* editor=nullptr;
    QFormLayout* parameters;
    QWidget* parameterHost;
    QComboBox* profile;
    QLabel* state;
    QTableWidget* monitor;
    QListWidget* output;
    QLineEdit* filter;
    QTimer timer;
    void guarded(const std::function<void()>& action);
    void addLog(const QString& message);
    bool confirmDiscard();
    void newProject(); void openProject(); void loadProject(const QString& path);
    void save(); void apply(const QString& reloadId=QString());
    void saveAll(); void closeProject(); void closeFile(); void updateRecent(); void syncFiles();
    void showScript(const QString& relative);
    void refreshExplorer(); void rebuildParameters();
    int tabIndex(const QString& path) const;
    QString tabPath(int index) const;
    void removeTab(int index); void clearTabs(); void activateTab(); void updateTabs();
    bool hasRawDrafts() const;
    void saveRaw(const QStringList& paths);
    void refreshRawTabs();
    void cacheDefaults(const QString& id,const LuaEffect& meta);
    void refreshDefaults();
    QJsonObject selectedEffect() const;
    QString profileKey() const;
    void setValue(const QString& group,const QString& name,const QJsonValue& value,bool reset=false);
    void tick(); void mapping(); void motorTest();
    void saveLocal();
};
}
