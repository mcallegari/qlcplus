#include <QApplication>
#include <QQmlApplicationEngine>

#include "app.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    App qlcplusApp;
    qlcplusApp.startup();
    qlcplusApp.show();

    return app.exec();
}
