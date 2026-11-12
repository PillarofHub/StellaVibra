#include "window.h"
#include "diagnostics.h"
int main(int argc,char** argv) {
    QApplication app(argc,argv); QApplication::setOrganizationName("StellaVibra"); QApplication::setApplicationName("StellaVibra"); QApplication::setApplicationVersion(STELLA_VERSION);
    QApplication::setWindowIcon(QIcon(":/assets/stella-vibra.png"));
    sv::initializeDiagnostics();
    sv::Window window;window.show();
    return app.exec();
}
