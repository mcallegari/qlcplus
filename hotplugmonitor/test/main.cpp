#include <QApplication>
#include <QDebug>
#include <QTimer>

#include "hotplugmonitor.h"
#include "hpmtest.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    HPMTest* test = new HPMTest;
    test->show();

    return app.exec();
}
