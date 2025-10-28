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

#include <QSettings>
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

QFile logFile;

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
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
// Since Qt6, the default rendering backend is Rhi. QLC doesn't support it yet so OpenGL have to be forced.
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGLRhi);
    qputenv("QT3D_RENDERER", "opengl");
#endif

    QApplication::setOrganizationName("qlcplus");
    QApplication::setOrganizationDomain("org");
    QApplication::setApplicationName(APPNAME);
    QApplication::setApplicationVersion(QString(APPVERSION));

    printVersion();

    QCommandLineParser parser;
    parser.setApplicationDescription("Q Light Controller Plus");

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption openFileOption(QStringList() << "o" << "open",
                                      "Specify a file to open.",
                                      "filename", "");
    parser.addOption(openFileOption);

    QCommandLineOption openLastOption(QStringList() << "9" << "openlast",
                                      "Open the file from last session.");
    parser.addOption(openLastOption);

    QCommandLineOption kioskOption(QStringList() << "k" << "kiosk",
                                      "Enable kiosk mode (only Virtual Console)");
    parser.addOption(kioskOption);

    QCommandLineOption localeOption(QStringList() << "l" << "locale",
                                      "Specify a language to use.",
                                      "locale", "");
    parser.addOption(localeOption);

    QCommandLineOption debugOption(QStringList() << "d" << "debug",
                                   "Enable debug messages.");
    parser.addOption(debugOption);

    QCommandLineOption logOption(QStringList() << "g" << "log",
                                   "Log debug messages to a file.");
    parser.addOption(logOption);

    QCommandLineOption threedSupportOption(QStringList() << "3" << "no3d",
                                      "Disable the 3D preview.");
    parser.addOption(threedSupportOption);

    parser.process(app);

    // 3D enablement
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

    if (parser.isSet(logOption))
    {
        QString logFilename = QDir::homePath() + QDir::separator() + "QLC+.log";
        logFile.setFileName(logFilename);
        if (!logFile.open(QIODevice::Append))
            qWarning("Warning: Unable to open log file.");
    }

    // logging option
    if (parser.isSet(debugOption))
        qInstallMessageHandler(
            [](QtMsgType, const QMessageLogContext &, const QString &msg) {
                QByteArray localMsg = msg.toLocal8Bit();
                //if (type >= QtSystemMsg)
                {
                    if (logFile.isOpen())
                    {
                        logFile.write(localMsg);
                        logFile.write((char *)"\n");
                        logFile.flush();
                    }

                    fprintf(stderr, "%s\n", localMsg.constData());
                    fflush(stderr);
                }
        });

    // language settings
    QString locale = parser.value(localeOption);

    App qlcplusApp;
    if (locale.isEmpty())
    {
        QSettings settings;
        QVariant language = settings.value(SETTINGS_LANGUAGE);
        if (language.isValid())
            locale = language.toString();
    }
    qlcplusApp.setLanguage(locale);

    // kiosk mode
    if (parser.isSet(kioskOption))
        qlcplusApp.enableKioskMode();

    qlcplusApp.startup();

    // open file
    QString filename = parser.value(openFileOption);
    if (filename.isEmpty() == false)
    {
        if (filename.endsWith(KExtFixture))
            qlcplusApp.loadFixture(filename);
        else
            qlcplusApp.loadWorkspace(filename);
    }

    // open last file
    if (parser.isSet(openLastOption))
        qlcplusApp.loadLastWorkspace();

    return app.exec();
}
