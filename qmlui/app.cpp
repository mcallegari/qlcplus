/*
  Q Light Controller Plus
  app.cpp

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

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFontDatabase>
#include <QQmlContext>
#include <QQuickItem>
#include <QSettings>
#include <QKeyEvent>

#include "app.h"
#include "mainview2d.h"
#include "showmanager.h"
#include "actionmanager.h"
#include "modelselector.h"
#include "contextmanager.h"
#include "virtualconsole.h"
#include "fixturebrowser.h"
#include "fixturemanager.h"
#include "functionmanager.h"
#include "inputoutputmanager.h"

#include "qlcfixturedefcache.h"
#include "audioplugincache.h"
#include "rgbscriptscache.h"
#include "qlcfixturedef.h"
#include "qlcconfig.h"
#include "qlcfile.h"

#define SETTINGS_WORKINGPATH "workspace/workingpath"
#define SETTINGS_RECENTFILE "workspace/recent"

#define MAX_RECENT_FILES    10

App::App()
    : QQuickView()
    , m_fixtureBrowser(NULL)
    , m_fixtureManager(NULL)
    , m_contextManager(NULL)
    , m_ioManager(NULL)
    , m_doc(NULL)
    , m_docLoaded(false)
{
    updateRecentFilesList();
}

App::~App()
{

}

void App::startup()
{
    qmlRegisterType<Fixture>("com.qlcplus.classes", 1, 0, "Fixture");
    qmlRegisterType<Function>("com.qlcplus.classes", 1, 0, "Function");
    qmlRegisterType<ModelSelector>("com.qlcplus.classes", 1, 0, "ModelSelector");

    setTitle("Q Light Controller Plus");
    setIcon(QIcon(":/qlcplus.png"));

    QFontDatabase::addApplicationFont(":/RobotoCondensed-Regular.ttf");

    rootContext()->setContextProperty("qlcplus", this);

    initDoc();

    //connect(m_doc, SIGNAL(loaded()),
    //        this, SIGNAL(docLoadedChanged()));
    //qmlRegisterType<App>("com.qlcplus.app", 1, 0, "App");

    m_ioManager = new InputOutputManager(m_doc);
    rootContext()->setContextProperty("ioManager", m_ioManager);

    m_fixtureBrowser = new FixtureBrowser(this, m_doc);
    rootContext()->setContextProperty("fixtureBrowser", m_fixtureBrowser);

    m_fixtureManager = new FixtureManager(this, m_doc);
    rootContext()->setContextProperty("fixtureManager", m_fixtureManager);

    m_functionManager = new FunctionManager(this, m_doc);
    rootContext()->setContextProperty("functionManager", m_functionManager);

    m_contextManager = new ContextManager(this, m_doc, m_fixtureManager, m_functionManager);
    rootContext()->setContextProperty("contextManager", m_contextManager);

    m_virtualConsole = new VirtualConsole(this, m_doc);
    rootContext()->setContextProperty("virtualConsole", m_virtualConsole);

    m_showManager = new ShowManager(this, m_doc);
    rootContext()->setContextProperty("showManager", m_showManager);

    m_actionManager = new ActionManager(this, m_functionManager, m_showManager);
    rootContext()->setContextProperty("actionManager", m_actionManager);
    qmlRegisterType<ActionManager>("com.qlcplus.classes", 1, 0, "ActionManager"); // to use the enums in QML

    // Start up in non-modified state
    m_doc->resetModified();

    // and here we go !
    setSource(QUrl("qrc:/MainView.qml"));
    //m_contextManager->activateContext("2D");
}

void App::show()
{
    //setGeometry(0, 0, 800, 600);
    //setGeometry(0, 0, 1272, 689); // youtube recording
    showMaximized();
    //showFullScreen();
}

void App::keyPressEvent(QKeyEvent *e)
{
    if (m_contextManager)
        m_contextManager->handleKeyPress(e);

    QQuickView::keyPressEvent(e);
}

void App::clearDocument()
{
    m_doc->masterTimer()->stop();
    m_doc->clearContents();
    m_virtualConsole->resetContents();
    //SimpleDesk::instance()->clearContents();
    m_showManager->resetContents();
    m_doc->inputOutputMap()->resetUniverses();
    setFileName(QString());
    m_doc->resetModified();
    m_doc->masterTimer()->start();
}

Doc *App::doc()
{
    return m_doc;
}

void App::slotDocModified(bool state)
{
    Q_UNUSED(state)
}

void App::initDoc()
{
    Q_ASSERT(m_doc == NULL);
    m_doc = new Doc(this);

    connect(m_doc, SIGNAL(modified(bool)), this, SLOT(slotDocModified(bool)));
    connect(m_doc, SIGNAL(modeChanged(Doc::Mode)), this, SLOT(slotModeChanged(Doc::Mode)));

    /* Load user fixtures first so that they override system fixtures */
    m_doc->fixtureDefCache()->load(QLCFixtureDefCache::userDefinitionDirectory());
    m_doc->fixtureDefCache()->loadMap(QLCFixtureDefCache::systemDefinitionDirectory());

    /* Load channel modifiers templates */
    m_doc->modifiersCache()->load(QLCModifiersCache::systemTemplateDirectory(), true);
    m_doc->modifiersCache()->load(QLCModifiersCache::userTemplateDirectory());

    /* Load RGB scripts */
    m_doc->rgbScriptsCache()->load(RGBScriptsCache::systemScriptsDirectory());
    m_doc->rgbScriptsCache()->load(RGBScriptsCache::userScriptsDirectory());

    /* Load plugins */
#if defined(__APPLE__) || defined(Q_OS_MAC)
    connect(m_doc->ioPluginCache(), SIGNAL(pluginLoaded(const QString&)),
            this, SLOT(slotSetProgressText(const QString&)));
#endif
#if defined Q_OS_ANDROID
    QString pluginsPath = QString("%1/../lib").arg(QDir::currentPath());
    m_doc->ioPluginCache()->load(QDir(pluginsPath));
#else
    m_doc->ioPluginCache()->load(IOPluginCache::systemPluginDirectory());
#endif

    /* Load audio decoder plugins
     * This doesn't use a AudioPluginCache::systemPluginDirectory() cause
     * otherwise the qlcconfig.h creation should have been moved into the
     * audio folder, which doesn't make much sense */
    m_doc->audioPluginCache()->load(QLCFile::systemDirectory(AUDIOPLUGINDIR, KExtPlugin));

    /* Restore outputmap settings */
    Q_ASSERT(m_doc->inputOutputMap() != NULL);

    /* Load input plugins & profiles */
    m_doc->inputOutputMap()->loadProfiles(InputOutputMap::userProfileDirectory());
    m_doc->inputOutputMap()->loadProfiles(InputOutputMap::systemProfileDirectory());
    m_doc->inputOutputMap()->loadDefaults();

    m_doc->masterTimer()->start();
}

void App::enableKioskMode()
{

}

void App::createKioskCloseButton(const QRect &rect)
{
    Q_UNUSED(rect)
}

void App::slotModeOperate()
{

}

void App::slotModeDesign()
{

}

void App::slotModeToggle()
{
    if (m_doc->mode() == Doc::Design)
        slotModeOperate();
    else
        slotModeDesign();
}

void App::slotModeChanged(Doc::Mode mode)
{
    Q_UNUSED(mode)
}

/*********************************************************************
 * Load & Save
 *********************************************************************/

void App::setFileName(const QString &fileName)
{
    m_fileName = fileName;
}

QString App::fileName() const
{
    return m_fileName;
}

bool App::loadWorkspace(const QString &fileName)
{
    /* Clear existing document data */
    clearDocument();
    m_docLoaded = false;
    emit docLoadedChanged();

    QString localFilename =  fileName;
    if (localFilename.startsWith("file:"))
        localFilename = QUrl(fileName).toLocalFile();

    if (loadXML(localFilename) == QFile::NoError)
    {
        setTitle(QString("Q Light Controller Plus - %1").arg(localFilename));
        setFileName(localFilename);
        m_docLoaded = true;
        updateRecentFilesList(localFilename);
        emit docLoadedChanged();
        m_contextManager->resetContexts();
        m_doc->resetModified();
        return true;
    }
    return false;
}

bool App::newWorkspace()
{
    /*
    QString msg(tr("Do you wish to save the current workspace?\n" \
                   "Changes will be lost if you don't save them."));
    if (saveModifiedDoc(tr("New Workspace"), msg) == false)
    {
        return false;
    }
    */

    clearDocument();
    m_fixtureManager->slotDocLoaded();
    m_functionManager->slotDocLoaded();
    m_contextManager->resetContexts();
    return true;
}

void App::updateRecentFilesList(QString filename)
{
    QSettings settings;
    if (filename.isEmpty() == false)
    {
        m_recentFiles.removeAll(filename); // in case the string is already present, remove it...
        m_recentFiles.prepend(filename); // and add it to the top
        for (int i = 0; i < m_recentFiles.count(); i++)
        {
            settings.setValue(QString("%1%2").arg(SETTINGS_RECENTFILE).arg(i), m_recentFiles.at(i));
            emit recentFilesChanged();
        }
    }
    else
    {
        for (int i = 0; i < MAX_RECENT_FILES; i++)
        {
            QVariant recent = settings.value(QString("%1%2").arg(SETTINGS_RECENTFILE).arg(i));
            if (recent.isValid() == true)
                m_recentFiles.append(recent.toString());
        }
    }
}

QStringList App::recentFiles() const
{
    return m_recentFiles;
}

QFileDevice::FileError App::loadXML(const QString &fileName)
{
    QFile::FileError retval = QFile::NoError;

    if (fileName.isEmpty() == true)
        return QFile::OpenError;

    QXmlStreamReader *doc = QLCFile::getXMLReader(fileName);
    if (doc == NULL || doc->device() == NULL || doc->hasError())
    {
        qWarning() << Q_FUNC_INFO << "Unable to read from" << fileName;
        return QFile::ReadError;
    }

    while (!doc->atEnd())
    {
        if (doc->readNext() == QXmlStreamReader::DTD)
            break;
    }
    if (doc->hasError())
    {
        QLCFile::releaseXMLReader(doc);
        return QFile::ResourceError;
    }

    if (doc->dtdName() == KXMLQLCWorkspace)
    {
        if (loadXML(*doc) == false)
        {
            retval = QFile::ReadError;
        }
        else
        {
            setFileName(fileName);
            m_doc->resetModified();
            retval = QFile::NoError;
        }
    }
    else
    {
        retval = QFile::ReadError;
        qWarning() << Q_FUNC_INFO << fileName
                   << "is not a workspace file";
    }

    QLCFile::releaseXMLReader(doc);

    return retval;
}

bool App::loadXML(QXmlStreamReader &doc, bool goToConsole, bool fromMemory)
{
    Q_UNUSED(goToConsole) // TODO
    if (doc.readNextStartElement() == false)
        return false;

    if (doc.name() != KXMLQLCWorkspace)
    {
        qWarning() << Q_FUNC_INFO << "Workspace node not found";
        return false;
    }

    //QString activeWindowName = doc.attributes().value(KXMLQLCWorkspaceWindow).toString();

    while (doc.readNextStartElement())
    {
        if (doc.name() == KXMLQLCEngine)
        {
            m_doc->loadXML(doc);
        }
        else if (doc.name() == KXMLQLCVirtualConsole)
        {
            m_virtualConsole->loadXML(doc);
        }
#if 0
        else if (doc.name() == KXMLQLCSimpleDesk)
        {
            SimpleDesk::instance()->loadXML(doc);
        }
#endif
        else if (doc.name() == KXMLQLCCreator)
        {
            /* Ignore creator information */
            doc.skipCurrentElement();
        }
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Workspace tag:" << doc.name().toString();
            doc.skipCurrentElement();
        }
    }

/*
    if (goToConsole == true)
        // Force the active window to be Virtual Console
        setActiveWindow(VirtualConsole::staticMetaObject.className());
    else
        // Set the active window to what was saved in the workspace file
        setActiveWindow(activeWindowName);
*/
    // Perform post-load operations
    m_virtualConsole->postLoad();

    if (m_doc->errorLog().isEmpty() == false &&
        fromMemory == false)
    {
        // emit a signal to inform the QML UI to display an error message
        /*
        QMessageBox msg(QMessageBox::Warning, tr("Warning"),
                        tr("Some errors occurred while loading the project:") + "\n\n" + m_doc->errorLog(),
                        QMessageBox::Ok);
        msg.exec();
        */
    }

    return true;
}


