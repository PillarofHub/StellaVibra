#pragma once
#include "engine.h"
#include "editor.h"
namespace sv {
class Window : public QMainWindow {
public:
    Window();
    ~Window() override;
protected:
    void closeEvent(QCloseEvent* event) override;
private:
    Engine engine;
    Project project;
    QJsonObject local;
    QMap<QString,QString> drafts;
    QMap<QString,QString> looseDrafts;
    QString selectedId,selectedScript,currentAircraft;
    bool refreshing=false,dirty=false;
    QListWidget* effectList;
    QFileSystemModel* files;
    QTreeView* tree;
    Editor* editor;
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
    void save(); void apply(); void addEffect();
    void refreshExplorer(); void selectEffect(); void rebuildParameters();
    QJsonObject selectedEffect() const;
    QString profileKey() const;
    void setValue(const QString& group,const QString& name,const QJsonValue& value,bool reset=false);
    void tick(); void settings(); void mapping(); void motorTest();
    void saveLocal();
};
}
