/*
  Q Light Controller
  main.cpp

  Copyright (C) Heikki Junnila

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
#include <QTextStream>
#include <QTranslator>
#include <QMetaType>
#include <QtGlobal>
#include <QVariant>
#include <QLocale>
#include <QString>
#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QHash>
#include <QDir>

#include "qlcconfig.h"
#include "qlci18n.h"

#if defined(WIN32) || defined(__APPLE__)
  #include "debugbox.h"
#endif

#include "virtualconsole.h"
#include "simpledesk.h"
#include "webaccess.h"
#include "app.h"
#include "doc.h"

/* Use this namespace for command-line arguments so that we don't pollute
   the global namespace. */
namespace QLCArgs
{
    /**
     * If true, switch to operate mode after ALL initialization is done.
     */
    bool operate = false;

    /**
     * Specifies a workspace file name to load after all initialization
     * has been done, but before switching to operate mode (if applicable)
     */
    QString workspace;

    /** If true, enables kiosk-mode (Operate mode locked, only virtual console) */
    bool kioskMode = false;

    /** If true, opens the application in full screen mode */
    bool fullScreen = false;

    /** If true, adjusts the main window geometry instead of instructing the windowing system to "maximize" */
    bool fullScreenResize = false;

    /** If true, create and run a class to enable a web server for remote controlling */
    bool enableWebAccess = false;

    /** If true, enable a 5% of overscan when in fullscreen mode (Raspberry Only) */
    bool enableOverscan = false;

    /** If not null, defines the place for a close button that in virtual console */
    QRect closeButtonRect = QRect();

    /** Debug output level */
    QtMsgType debugLevel = QtSystemMsg;

    /** Log to file flag */
    bool logToFile = false;

    QFile logFile;

#if defined(WIN32) || defined(__APPLE__)
    /** The debug windows for Windows and OSX */
    DebugBox *dbgBox = NULL;
#endif
}

/**
 * Suppresses debug messages
 */
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
void qlcMessageHandler(QtMsgType type, const char* msg)
{
    if (type >= QLCArgs::debugLevel)
    {
        if (QLCArgs::logToFile == true && QLCArgs::logFile.isOpen())
        {
            QLCArgs::logFile.write(msg);
            QLCArgs::logFile.write((char *)"\n");
            QLCArgs::logFile.flush();
            return;
        }
#if defined(WIN32) || defined(__APPLE__)
        if (QLCArgs::dbgBox != NULL)
            QLCArgs::dbgBox->addText(msg);
#else
        fprintf(stderr, "%s\n", msg);
        fflush(stderr);
#endif
    }
}
#else
void qlcMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context)

    QByteArray localMsg = msg.toLocal8Bit();
    if (type >= QLCArgs::debugLevel)
    {
        if (QLCArgs::logToFile == true && QLCArgs::logFile.isOpen())
        {
            QLCArgs::logFile.write(localMsg);
            QLCArgs::logFile.write((char *)"\n");
            QLCArgs::logFile.flush();
        }
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
        if (QLCArgs::dbgBox != NULL)
            QLCArgs::dbgBox->addText(msg);
#else
        fprintf(stderr, "%s\n", localMsg.constData());
        fflush(stderr);
#endif
    }
}
#endif

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

/**
 * Prints possible command-line options
 */
void printUsage()
{
    QTextStream cout(stdout, QIODevice::WriteOnly);

    cout << "Usage:";
    cout << "  qlcplus [options]" << endl;
    cout << "Options:" << endl;
    cout << "  -c or --closebutton <x,y,w,h>\tPlace a close button in virtual console (only when -k is specified)" << endl;
    cout << "  -d or --debug <level>\t\tSet debug output level (0-3, see QtMsgType)" << endl;
    cout << "  -f or --fullscreen <method>\tStart the application in fullscreen mode (method is either 'normal' or 'resize')" << endl;
    cout << "  -g or --log\t\t\tLog debug messages to a file" << endl;
    cout << "  -h or --help\t\t\tPrint this help" << endl;
    cout << "  -k or --kiosk\t\t\tEnable kiosk mode (only virtual console in forced operate mode)" << endl;
    cout << "  -l or --locale <locale>\tForce a locale for translation" << endl;
    cout << "  -o or --open <file>\t\tOpen the specified workspace file" << endl;
    cout << "  -p or --operate\t\tStart in operate mode" << endl;
    cout << "  -v or --version\t\tPrint version information" << endl;
    cout << "  -w or --web\t\t\tEnable remote web access" << endl;
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
bool parseArgs()
{
    QStringListIterator it(QCoreApplication::arguments());
    while (it.hasNext() == true)
    {
        QString arg(it.next());

        if ((arg == "-c" || arg == "--closebutton") && it.hasNext() == true)
        {
            QString str(it.next());
            QStringList parts = str.split(",");
            if (parts.size() == 4)
            {
                QRect rect(parts[0].toInt(), parts[1].toInt(),
                           parts[2].toInt(), parts[3].toInt());
                if (rect.isValid() == true)
                    QLCArgs::closeButtonRect = rect;
            }
        }
        else if (arg == "-d" || arg == "--debug")
        {
            if (it.hasNext() == true)
                QLCArgs::debugLevel = QtMsgType(it.peekNext().toInt());
            else
                QLCArgs::debugLevel = QtMsgType(0);
        }
        else if (arg == "-g" || arg == "--log")
        {
            QLCArgs::logToFile = true;
            QString logFilename = QDir::homePath() + QDir::separator() + "QLC+.log";
            QLCArgs::logFile.setFileName(logFilename);
            QLCArgs::logFile.open(QIODevice::Append);
        }
        else if (arg == "-f" || arg == "--fullscreen")
        {
            QLCArgs::fullScreen = true;
            if (it.hasNext() == true && it.peekNext() == "resize")
                QLCArgs::fullScreenResize = true;
        }
        else if (arg == "-r" || arg == "--overscan")
        {
            QLCArgs::enableOverscan = true;
        }
        else if (arg == "-h" || arg == "--help")
        {
            printUsage();
            return false;
        }
        else if (arg == "-k" || arg == "--kiosk")
        {
            QLCArgs::kioskMode = true;
        }
        else if (arg == "-l" || arg == "--locale")
        {
            if (it.hasNext() == true)
                QLCi18n::setDefaultLocale(it.next());
        }
        else if (arg == "-o" || arg == "--open")
        {
            if (it.hasNext() == true)
                QLCArgs::workspace = it.next();
        }
        else if (arg == "-p" || arg == "--operate")
        {
            QLCArgs::operate = true;
        }
        else if (arg == "-w" || arg == "--web")
        {
            QLCArgs::enableWebAccess = true;
        }
        else if (arg == "-v" || arg == "--version")
        {
            /* Don't print anything, since version is always
               printed before anything else. Just make the app
               exit by returning false. */
            return false;
        }
    }

    return true;
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

    /* At least MIDI plugin requires this so best to declare it here for everyone */
    qRegisterMetaType<QVariant>("QVariant");

#if defined(__APPLE__) || defined(Q_OS_MAC)
    /* Load plugins from within the bundle ONLY */
    QDir dir(QApplication::applicationDirPath());
    dir.cdUp();
    dir.cd("plugins");
    QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

    QLCi18n::init();

    /* Let the world know... */
    printVersion();

    /* Parse command-line arguments */
    if (parseArgs() == false)
        return 0;

    /* Load translation for main application */
    QLCi18n::loadTranslation("qlcplus");

    /* Handle debug messages */
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    qInstallMsgHandler(qlcMessageHandler);
#else
    qInstallMessageHandler(qlcMessageHandler);
#endif

    /* Create and initialize the QLC application object */
    App app;

#if defined(WIN32) || defined(__APPLE__)
    if (QLCArgs::logToFile == false && QLCArgs::debugLevel < QtSystemMsg)
    {
        QLCArgs::dbgBox = new DebugBox(&app);
        QLCArgs::dbgBox->show();
    }
#endif

    if (QLCArgs::enableOverscan == true)
        app.enableOverscan();

    app.startup();
    app.show();

    if (QLCArgs::workspace.isEmpty() == false)
    {
        if (app.loadXML(QLCArgs::workspace) == QFile::NoError)
            app.updateFileOpenMenu(QLCArgs::workspace);
    }
    if (QLCArgs::operate == true)
        app.slotModeOperate();
    if (QLCArgs::kioskMode == true)
        app.enableKioskMode();
    if (QLCArgs::fullScreen == true)
        app.slotControlFullScreen(QLCArgs::fullScreenResize);
    if (QLCArgs::kioskMode == true && QLCArgs::closeButtonRect.isValid() == true)
        app.createKioskCloseButton(QLCArgs::closeButtonRect);

    if (QLCArgs::enableWebAccess == true)
    {
        WebAccess *webAccess = new WebAccess(app.doc(), VirtualConsole::instance(),
                                               SimpleDesk::instance());

        QObject::connect(webAccess, SIGNAL(toggleDocMode()),
                &app, SLOT(slotModeToggle()));
        QObject::connect(webAccess, SIGNAL(loadProject(QString)),
                &app, SLOT(slotLoadDocFromMemory(QString)));
        QObject::connect(webAccess, SIGNAL(storeAutostartProject(QString)),
                &app, SLOT(slotSaveAutostart(QString)));
    }

    return qapp.exec();
}
