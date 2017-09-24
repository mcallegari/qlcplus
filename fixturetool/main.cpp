/*
  Q Light Controller Plus - Fixture tool
  main.cpp

  Copyright (C) Jano Svitok

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

#include <QCoreApplication>
#include <QString>
#include <QDebug>
#include <QCommandLineParser>

#include "qlcconfig.h"
#include "qlcfile.h"
#include "qlcfixturedef.h"

/**
 * Prints the application version
 */
void printVersion()
{
    QTextStream cout(stdout, QIODevice::WriteOnly);

    cout << endl;
    cout << QCoreApplication::applicationName() << " " << QCoreApplication::applicationVersion() << endl;
    cout << "This program is licensed under the terms of the ";
    cout << "Apache 2.0 license." << endl;
    cout << "Copyright (c) Heikki Junnila (hjunnila@users.sf.net)." << endl;
    cout << "Copyright (c) Massimo Callegari (massimocallegari@yahoo.it)." << endl;
    cout << endl;
}

/**
 * THE entry point for the application
 *
 * @param argc Number of arguments in array argv
 * @param argv Arguments array
 */
int main(int argc, char** argv)
{
    /* Create the Qt core application object */
    QCoreApplication qapp(argc, argv);

    QCoreApplication::setApplicationName("qlcplus-fixturetool");
    QCoreApplication::setApplicationVersion(APPVERSION);

    QCommandLineParser parser;

    parser.setApplicationDescription(FXTOOLNAME);
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption updateFixtureOption(QStringList() << "u" << "update-fixture", "Update fixture definition", "fixture.qxf" );
    QCommandLineOption keepVersionOption(QStringList() << "k" << "keep-version", "Keep original QLC+ version in the updated fixture definition" );

    parser.addOption(updateFixtureOption);
    parser.addOption(keepVersionOption);

    parser.process(qapp);

    if (parser.isSet(updateFixtureOption))
    {
        QString fixtureFile = parser.value(updateFixtureOption);

        QLCFixtureDef fixtureDef;
        fixtureDef.loadXML(fixtureFile);
        fixtureDef.saveXML(fixtureFile, parser.isSet(keepVersionOption));

        return 0;
    }

    parser.showHelp(2);
    return 2;
}
