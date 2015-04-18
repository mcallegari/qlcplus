#include <QApplication>
#include <QQmlApplicationEngine>

#include "app.h"
#include "qlcconfig.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("qlcplus");
    QApplication::setOrganizationDomain("sf.net");
    QApplication::setApplicationName(APPNAME);

    App qlcplusApp;
    qlcplusApp.startup();
    qlcplusApp.show();

    return app.exec();
}
