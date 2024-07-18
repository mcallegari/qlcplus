/*
  Q Light Controller Plus
  main.cpp

  Copyright (c) Massimo Callegari

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0.txt

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#include <QApplication>
#include <QSurfaceFormat>
#include <QCommandLineParser>
#include <QQmlApplicationEngine>

#include "app.h"
#include "qlcfile.h"
#include "qlcconfig.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
#define endl Qt::endl
#endif

void debugMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)
    Q_UNUSED(type)

    QByteArray localMsg = msg.toLocal8Bit();
    //if (type >= QtSystemMsg)
    {
        fprintf(stderr, "%s\n", localMsg.constData());
        fflush(stderr);
    }
}

/**
 * Prints the application version
 */
void printVersion()
{
    QTextStream cout(stdout, QIODevice::WriteOnly);

    cout << endl;
    cout << APPNAME << " " << "version " << APPVERSION << endl;
    cout << "This program is licensed under the terms of the ";
    cout << "Apache 2.0 license." << endl;
    cout << "Copyright (c) Heikki Junnila (hjunnila@users.sf.net)" << endl;
    cout << "Copyright (c) Massimo Callegari (massimocallegari@yahoo.it)" << endl;
    cout << endl;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("qlcplus");
    QApplication::setOrganizationDomain("org");
    QApplication::setApplicationName(APPNAME);
    QApplication::setApplicationVersion(QString(APPVERSION));

    printVersion();

    QCommandLineParser parser;
    parser.setApplicationDescription("Q Light Controller Plus");

    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption debugOption(QStringList() << "d" << "debug",
                                      "Enable debug messages.");
    parser.addOption(debugOption);

    QCommandLineOption openFileOption(QStringList() << "o" << "open",
                                      "Specify a file to open.",
                                      "filename", "");
    parser.addOption(openFileOption);

    QCommandLineOption kioskOption(QStringList() << "k" << "kiosk",
                                      "Enable kiosk mode (only Virtual Console)");
    parser.addOption(kioskOption);

    QCommandLineOption localeOption(QStringList() << "l" << "locale",
                                      "Specify a language to use.",
                                      "locale", "");
    parser.addOption(localeOption);

    QCommandLineOption threedSupportOption(QStringList() << "3" << "no3d",
                                      "Disable the 3D preview.");
    parser.addOption(threedSupportOption);

    parser.process(app);

#if !defined Q_OS_ANDROID
    if (!parser.isSet(threedSupportOption))
    {
        QSurfaceFormat format;
        format.setMajorVersion(3);
        format.setMinorVersion(3);
        format.setProfile(QSurfaceFormat::CoreProfile);
        QSurfaceFormat::setDefaultFormat(format);
    }
#endif
    if (parser.isSet(debugOption))
        qInstallMessageHandler(debugMessageHandler);

    QString locale = parser.value(localeOption);

    App qlcplusApp;
    qlcplusApp.setLanguage(locale);

    if (parser.isSet(kioskOption))
        qlcplusApp.enableKioskMode();

    qlcplusApp.startup();
    qlcplusApp.show();

    QString filename = parser.value(openFileOption);
    if (filename.isEmpty() == false)
    {
        if (filename.endsWith(KExtFixture))
            qlcplusApp.loadFixture(filename);
        else
            qlcplusApp.loadWorkspace(filename);
    }

    return app.exec();
}
