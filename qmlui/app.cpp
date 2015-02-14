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

#include <QQuickItem>
#include <QQmlContext>
#include <QDomDocument>

#include "app.h"
#include "mainview2d.h"
#include "fixturebrowser.h"
#include "fixturemanager.h"
#include "functionmanager.h"
#include "inputoutputmanager.h"

#include "rgbscriptscache.h"
#include "qlcfixturedefcache.h"
#include "qlcfixturedef.h"
#include "qlcconfig.h"
#include "qlcfile.h"

App::App()
    : QObject()
    , m_view(NULL)
    , m_fixtureBrowser(NULL)
    , m_fixtureManager(NULL)
    , m_ioManager(NULL)
    , m_doc(NULL)
    , m_docLoaded(false)
{
}

App::~App()
{

}

void App::startup()
{
    qmlRegisterType<Fixture>("com.qlcplus.classes", 1, 0, "Fixture");

    m_view = new QQuickView();

    m_view->setTitle("Q Light Controller Plus");
    m_view->setIcon(QIcon(":/qlcplus.png"));

    m_view->rootContext()->setContextProperty("qlcplus", this);

    initDoc();

    //connect(m_doc, SIGNAL(loaded()),
    //        this, SIGNAL(docLoadedChanged()));
    //qmlRegisterType<App>("com.qlcplus.app", 1, 0, "App");

    m_ioManager = new InputOutputManager(m_doc);
    m_view->rootContext()->setContextProperty("ioManager", m_ioManager);

    m_fixtureBrowser = new FixtureBrowser(m_view, m_doc);
    m_view->rootContext()->setContextProperty("fixtureBrowser", m_fixtureBrowser);

    m_fixtureManager = new FixtureManager(m_doc);
    m_view->rootContext()->setContextProperty("fixtureManager", m_fixtureManager);

    m_functionManager = new FunctionManager(m_view, m_doc);
    m_view->rootContext()->setContextProperty("functionManager", m_functionManager);

    m_2DView = new MainView2D(m_view, m_doc);
    m_view->rootContext()->setContextProperty("View2D", m_2DView);

    connect(m_fixtureManager, SIGNAL(newFixtureCreated(quint32,qreal,qreal)),
            this, SLOT(slotNewFixtureCreated(quint32,qreal,qreal)));

    // and here we go !
    m_view->setSource(QUrl("qrc:/MainView.qml"));
}

void App::show()
{
    m_view->showMaximized();
    //m_view->showFullScreen();
}

void App::clearDocument()
{
    m_doc->clearContents();
    //VirtualConsole::instance()->resetContents();
    //SimpleDesk::instance()->clearContents();
    //ShowManager::instance()->clearContents();
    m_doc->inputOutputMap()->resetUniverses();
    setFileName(QString());
    m_doc->resetModified();
}

Doc *App::doc()
{
    return m_doc;
}

void App::slotDocModified(bool state)
{
    Q_UNUSED(state)
}

void App::slotNewFixtureCreated(quint32 fxID, qreal x, qreal y, qreal z)
{
    Q_UNUSED(z)

    QObject *viewObj = m_view->rootObject()->findChild<QObject *>("fixturesAndFunctions");
    if (viewObj == NULL)
        return;

    QString currentView = viewObj->property("currentView").toString();
    qDebug() << "Current view:" << currentView;

    if (currentView == "2D")
    {
        m_2DView->createFixtureItem(fxID, x, y, false);
    }
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
    m_doc->ioPluginCache()->load(IOPluginCache::systemPluginDirectory());

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

    QString localFilename =  QUrl(fileName).toLocalFile();

    if (loadXML(localFilename) == QFile::NoError)
    {
        m_view->setTitle(QString("Q Light Controller Plus - %1").arg(localFilename));
        setFileName(localFilename);
        m_docLoaded = true;
        emit docLoadedChanged();
        return true;
    }
    return false;
}

QFileDevice::FileError App::loadXML(const QString &fileName)
{
    QFile::FileError retval = QFile::NoError;

    QDomDocument doc(QLCFile::readXML(fileName));
    if (doc.isNull() == false)
    {
        if (doc.doctype().name() == KXMLQLCWorkspace)
        {
            if (loadXML(doc) == false)
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
        }
    }

    return retval;
}

bool App::loadXML(const QDomDocument &doc, bool goToConsole, bool fromMemory)
{
    Q_UNUSED(goToConsole) // TODO
    Q_ASSERT(m_doc != NULL);

    QDomElement root = doc.documentElement();
    if (root.tagName() != KXMLQLCWorkspace)
    {
        qWarning() << Q_FUNC_INFO << "Workspace node not found";
        return false;
    }

    //QString activeWindowName = root.attribute(KXMLQLCWorkspaceWindow);

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();

        if (tag.tagName() == KXMLQLCEngine)
        {
            m_doc->loadXML(tag);
        }
#if 0
        else if (tag.tagName() == KXMLQLCVirtualConsole)
        {
            VirtualConsole::instance()->loadXML(tag);
        }
        else if (tag.tagName() == KXMLQLCSimpleDesk)
        {
            SimpleDesk::instance()->loadXML(tag);
        }
        else if (tag.tagName() == KXMLFixture)
        {
            /* Legacy support code, nowadays in Doc */
            Fixture::loader(tag, m_doc);
        }
        else if (tag.tagName() == KXMLQLCFunction)
        {
            /* Legacy support code, nowadays in Doc */
            Function::loader(tag, m_doc);
        }
        else if (tag.tagName() == KXMLQLCCreator)
        {
            /* Ignore creator information */
        }
#endif
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Workspace tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

/*
    if (goToConsole == true)
        // Force the active window to be Virtual Console
        setActiveWindow(VirtualConsole::staticMetaObject.className());
    else
        // Set the active window to what was saved in the workspace file
        setActiveWindow(activeWindowName);

    // Perform post-load operations
    VirtualConsole::instance()->postLoad();
*/
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


