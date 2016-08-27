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
#include <QCommandLineParser>
#include <QQmlApplicationEngine>

#include "app.h"
#include "qlcconfig.h"

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

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setOrganizationName("qlcplus");
    QApplication::setOrganizationDomain("org");
    QApplication::setApplicationName(APPNAME);
    QApplication::setApplicationVersion(QString(APPVERSION));

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

    parser.process(app);

    if (parser.isSet(debugOption))
        qInstallMessageHandler(debugMessageHandler);

    App qlcplusApp;
    qlcplusApp.startup();
    qlcplusApp.show();

    QString filename = parser.value(openFileOption);
    if (filename.isEmpty() == false)
        qlcplusApp.loadWorkspace(filename);

    return app.exec();
}
