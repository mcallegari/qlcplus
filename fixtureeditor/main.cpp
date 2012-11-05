/*
  Q Light Controller - Fixture Definition Editor
  main.cpp

  Copyright (C) Heikki Junnila

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  Version 2 as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details. The license is
  in the file "COPYING".

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <QApplication>
#include <QTextStream>
#include <QTranslator>
#include <QLocale>
#include <QString>
#include <QDebug>
#include <QDir>

#include "qlcconfig.h"

#include "app.h"

/* Use this namespace for command-line arguments so that we don't pollute
   the global namespace. */
namespace FXEDArgs
{
/**
 * Specifies a fixture file name to load after all initialization
 * has been done
 */
QString fixture;

/**
 * Specifies a locale for forced translation
 */
QString locale;
}

/**
 * Prints the application version
 */
void printVersion()
{
    QTextStream cout(stdout, QIODevice::WriteOnly);

    cout << endl;
    cout << App::longName() << " " << App::version() << endl;
    cout << "This program is licensed under the terms of the GNU ";
    cout << "General Public License v2." << endl;
    cout << "Copyright (c) Heikki Junnila (hjunnila@users.sf.net)." << endl;
    cout << endl;
}

/**
 * Prints possible command-line options
 */
void printUsage()
{
    QTextStream cout(stdout, QIODevice::WriteOnly);

    cout << "Usage:";
    cout << "  qlc-fixtureeditor [options]" << endl;
    cout << "Options:" << endl;
    cout << "  -o or --open <file>\t\tOpen the specified fixture definition file" << endl;
    cout << "  -l or --locale <locale>\tForce a locale for translation" << endl;
    cout << "  -h or --help\t\t\tPrint this help" << endl;
    cout << "  -v or --version\t\tPrint version information" << endl;
    cout << endl;
}

/**
 * Parse command line arguments
 *
 * @param argc Number of arguments in array argv
 * @param argv Arguments array
 *
 * @return true to continue with application launch; otherwise false
 */
bool parseArgs(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
    {
        if (::strcmp(argv[i], "-v") == 0 ||
                ::strcmp(argv[i], "--version") == 0)
        {
            /* Don't print anything, since version is always
               printed before anything else. Just make the app
               exit by returning false. */
            return false;
        }
        else if (::strcmp(argv[i], "-h") == 0 ||
                 ::strcmp(argv[i], "--help") == 0)
        {
            printUsage();
            return false;
        }
        else if (::strcmp(argv[i], "-o") == 0 ||
                 ::strcmp(argv[i], "--open") == 0)
        {
            FXEDArgs::fixture = QString(argv[++i]);
        }
        else if (::strcmp(argv[i], "-l") == 0 ||
                 ::strcmp(argv[i], "--locale") == 0)
        {
            FXEDArgs::locale = QString(argv[++i]);
        }
    }

    return true;
}

void loadTranslation(const QString& locale, QApplication& app)
{
    QString lc;
    if (FXEDArgs::locale.isEmpty() == true)
        lc = locale;
    else
        lc = FXEDArgs::locale;
    QString file(QString("fixtureeditor_%1").arg(lc));

#ifdef __APPLE__
    QString path(QString("%1/../%2").arg(QApplication::applicationDirPath())
                 .arg(TRANSLATIONDIR));
#else
    QString path(TRANSLATIONDIR);
#endif
    QTranslator* translator = new QTranslator(&app);
    if (translator->load(file, path) == true)
    {
        qDebug() << "Using translation for" << lc;
        QCoreApplication::installTranslator(translator);
    }
    else
    {
        qDebug() << "Unable to find translation for" << lc;
    }
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
    QApplication qapp(argc, argv);

#ifdef __APPLE__
    /* Load plugins from within the bundle ONLY */
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("plugins");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

    /* Let te world know... */
    printVersion();

    /* Parse command-line arguments */
    if (parseArgs(argc, argv) == false)
        return 0;

    /* Load translation for current locale */
    loadTranslation(QLocale::system().name(), qapp);

    /* Create and initialize the Fixture Editor application object */
    App app;
    if (FXEDArgs::fixture.isEmpty() == false)
        app.loadFixtureDefinition(FXEDArgs::fixture);

    /* Show and execute the application */
    app.show();
    return qapp.exec();
}
