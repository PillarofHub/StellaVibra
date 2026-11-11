#include "window.h"
#include "diagnostics.h"
int main(int argc,char** argv) {
    QApplication app(argc,argv); QApplication::setOrganizationName("StellaVibra"); QApplication::setApplicationName("StellaVibra"); QApplication::setApplicationVersion(STELLA_VERSION);
    const bool check=app.arguments().contains("--ui-check");
    if(!check)sv::initializeDiagnostics();
    sv::Window window;window.show();
    if(check){
        qInfo("UI check: window shown; hardware, project restore and settings writes disabled");
        QTimer::singleShot(500,&window,[&window]{
            for(auto* action:window.findChildren<QAction*>())if(action->text()=="Reset Default Layout"){action->trigger();qInfo("UI check: default layout restored");break;}
        });
        QTimer::singleShot(1200,&window,[&window]{
            qInfo("UI check: %d docks, %d splitters",int(window.findChildren<QDockWidget*>().size()),int(window.findChildren<QSplitter*>().size()));
            for(auto* dock:window.findChildren<QDockWidget*>())qInfo("UI check: %s %d x %d",qPrintable(dock->objectName()),dock->width(),dock->height());
            for(auto* split:window.findChildren<QSplitter*>()){auto sizes=split->sizes();if(sizes.size()==2)qInfo("UI check: editor split %d / %d",sizes[0],sizes[1]);}
        });
        QTimer::singleShot(2000,&app,[]{qInfo("UI check: event loop responsive");QCoreApplication::quit();});
    }
    return app.exec();
}
