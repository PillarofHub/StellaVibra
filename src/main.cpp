#include "window.h"
int main(int argc,char** argv) {
    QApplication app(argc,argv); QApplication::setOrganizationName("StellaVibra"); QApplication::setApplicationName("StellaVibra"); QApplication::setApplicationVersion("0.1.0");
    sv::Window window; window.show(); return app.exec();
}
