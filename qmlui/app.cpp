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

#include <QQuickItemGrabResult>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QtCore/qbuffer.h>
#include <QFontDatabase>
#include <QOpenGLContext>
#include <QPrintDialog>
#include <QApplication>
#include <QTranslator>
#include <QQmlContext>
#include <QQuickItem>
#include <QSettings>
#include <QKeyEvent>
#include <QPrinter>
#include <QPainter>
#include <QScreen>
#include <unistd.h>

#include "app.h"
#include "uimanager.h"
#include "simpledesk.h"
#include "showmanager.h"
#include "fixtureeditor.h"
#include "modelselector.h"
#include "videoprovider.h"
#include "importmanager.h"
#include "contextmanager.h"
#include "virtualconsole.h"
#include "fixturebrowser.h"
#include "fixturemanager.h"
#include "palettemanager.h"
#include "functionmanager.h"
#include "fixturegroupeditor.h"
#include "inputoutputmanager.h"

#include "tardis.h"
#include "networkmanager.h"

#include "qlcfixturedefcache.h"
#include "audioplugincache.h"
#include "rgbscriptscache.h"
#include "qlcconfig.h"
#include "qlcfile.h"

#define SETTINGS_WORKINGPATH "workspace/workingpath"
#define SETTINGS_RECENTFILE "workspace/recent"
#define KXMLQLCWorkspaceWindow "CurrentWindow"

#define MAX_RECENT_FILES    10

App::App()
    : QQuickView()
    , m_translator(nullptr)
    , m_fixtureBrowser(nullptr)
    , m_fixtureManager(nullptr)
    , m_contextManager(nullptr)
    , m_ioManager(nullptr)
    , m_showManager(nullptr)
    , m_simpleDesk(nullptr)
    , m_videoProvider(nullptr)
    , m_networkManager(nullptr)
    , m_uiManager(nullptr)
    , m_doc(nullptr)
    , m_docLoaded(false)
    , m_printItem(nullptr)
    , m_fileName(QString())
    , m_importManager(nullptr)
    , m_fixtureEditor(nullptr)
{
    QSettings settings;

    updateRecentFilesList();

    QVariant dir = settings.value(SETTINGS_WORKINGPATH);
    if (dir.isValid() == true)
        m_workingPath = dir.toString();

    setAccessMask(defaultMask());

    connect(this, &App::screenChanged, this, &App::slotScreenChanged);
    connect(this, SIGNAL(closing(QQuickCloseEvent*)), this, SLOT(slotClosing()));
    connect(this, &App::sceneGraphInitialized, this, &App::slotSceneGraphInitialized);
}

App::~App()
{
}

QString App::appName() const
{
    return QString(APPNAME);
}

QString App::appVersion() const
{
    return QString(APPVERSION);
}

void App::startup()
{
    qmlRegisterUncreatableType<App>("org.qlcplus.classes", 1, 0, "App", "Can't create an App!");
    qmlRegisterUncreatableType<Fixture>("org.qlcplus.classes", 1, 0, "Fixture", "Can't create a Fixture!");
    qmlRegisterUncreatableType<Function>("org.qlcplus.classes", 1, 0, "QLCFunction", "Can't create a Function!");
    qmlRegisterType<ModelSelector>("org.qlcplus.classes", 1, 0, "ModelSelector");

    setTitle(APPNAME);
    setIcon(QIcon(":/qlcplus.svg"));

    if (QFontDatabase::addApplicationFont(":/RobotoCondensed-Regular.ttf") < 0)
        qWarning() << "Roboto condensed cannot be loaded!";

    if (QFontDatabase::addApplicationFont(":/RobotoMono-Regular.ttf") < 0)
        qWarning() << "Roboto mono cannot be loaded!";

    rootContext()->setContextProperty("qlcplus", this);

    slotScreenChanged(screen());

    initDoc();

    m_uiManager = new UiManager(this, m_doc);
    rootContext()->setContextProperty("uiManager", m_uiManager);
    m_ioManager = new InputOutputManager(this, m_doc);
    m_fixtureBrowser = new FixtureBrowser(this, m_doc);
    m_fixtureManager = new FixtureManager(this, m_doc);
    m_fixtureGroupEditor = new FixtureGroupEditor(this, m_doc, m_fixtureManager);
    m_functionManager = new FunctionManager(this, m_doc);
    m_simpleDesk = new SimpleDesk(this, m_doc, m_functionManager);
    m_contextManager = new ContextManager(this, m_doc, m_fixtureManager, m_functionManager);
    m_paletteManager = new PaletteManager(this, m_doc, m_contextManager);

    m_virtualConsole = new VirtualConsole(this, m_doc, m_contextManager);
    m_showManager = new ShowManager(this, m_doc);
    m_networkManager = new NetworkManager(this, m_doc);
    rootContext()->setContextProperty("networkManager", m_networkManager);

    connect(m_networkManager, &NetworkManager::clientAccessRequest, this, &App::slotClientAccessRequest);
    connect(m_networkManager, &NetworkManager::accessMaskChanged, this, &App::setAccessMask);
    connect(m_networkManager, &NetworkManager::requestProjectLoad, this, &App::slotLoadDocFromMemory);

    m_tardis = new Tardis(this, m_doc, m_networkManager, m_fixtureManager, m_functionManager,
                          m_contextManager, m_simpleDesk, m_showManager, m_virtualConsole);
    rootContext()->setContextProperty("tardis", m_tardis);

    m_contextManager->registerContext(m_virtualConsole);
    m_contextManager->registerContext(m_simpleDesk);
    m_contextManager->registerContext(m_showManager);
    m_contextManager->registerContext(m_ioManager);

    // register an uncreatable type just to use the enums in QML
    qmlRegisterUncreatableType<ContextManager>("org.qlcplus.classes", 1, 0, "ContextManager", "Can't create a ContextManager!");
    qmlRegisterUncreatableType<ShowManager>("org.qlcplus.classes", 1, 0, "ShowManager", "Can't create a ShowManager!");
    qmlRegisterUncreatableType<NetworkManager>("org.qlcplus.classes", 1, 0, "NetworkManager", "Can't create a NetworkManager!");
    qmlRegisterUncreatableType<SimpleDesk>("org.qlcplus.classes", 1, 0, "SimpleDesk", "Can't create a SimpleDesk!");

    // Start up in non-modified state
    m_doc->resetModified();

    m_uiManager->initialize();

    // and here we go !
    setSource(QUrl("qrc:/MainView.qml"));
}

void App::toggleFullscreen()
{
    static int wstate = windowState();

    if (windowState() & Qt::WindowFullScreen)
    {
        if (wstate & Qt::WindowMaximized)
            showMaximized();
        else
            showNormal();
        wstate = windowState();
    }
    else
    {
        wstate = windowState();
        showFullScreen();
    }
}

void App::setLanguage(QString locale)
{
    if (m_translator != nullptr)
    {
        QCoreApplication::removeTranslator(m_translator);
        delete m_translator;
    }

    QString translationPath = QLCFile::systemDirectory(TRANSLATIONDIR).absolutePath();

    if (locale.isEmpty() == true)
        locale = QLocale::system().name();

    QString file(QString("%1_%2").arg("qlcplus").arg(locale));
    m_translator = new QTranslator(QCoreApplication::instance());
    if (m_translator->load(file, translationPath) == true)
        QCoreApplication::installTranslator(m_translator);

    engine()->retranslate();
}

QString App::goboSystemPath() const
{
    return QLCFile::systemDirectory(GOBODIR).absolutePath();
}

void App::show()
{
    QScreen *currScreen = screen();
    QRect rect(0, 0, 800, 600);
    rect.moveTopLeft(currScreen->geometry().topLeft());
    setGeometry(rect);
    showMaximized();
}

qreal App::pixelDensity() const
{
    return m_pixelDensity;
}

int App::accessMask() const
{
    return m_accessMask;
}

bool App::is3DSupported() const
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (openglContext() == nullptr)
        return false;

    int glVersion = (openglContext()->format().majorVersion() * 10) + openglContext()->format().minorVersion();
    return glVersion < 33 ? false : true;
#else
    // TODO: Qt6

    return true;
#endif
}

void App::exit()
{
    //destroy();
    QApplication::quit();
}

void App::setAccessMask(int mask)
{
    if (mask == m_accessMask)
        return;

    m_accessMask = mask;
    emit accessMaskChanged(mask);
}

int App::defaultMask() const
{
    return AC_FixtureEditing | AC_FunctionEditing | AC_InputOutput |
            AC_ShowManager | AC_SimpleDesk | AC_VCControl | AC_VCEditing;
}

void App::keyPressEvent(QKeyEvent *e)
{
    if (m_contextManager)
        m_contextManager->handleKeyPress(e);

    QQuickView::keyPressEvent(e);
}

void App::keyReleaseEvent(QKeyEvent *e)
{
    if (m_contextManager)
        m_contextManager->handleKeyRelease(e);

    QQuickView::keyReleaseEvent(e);
}

bool App::event(QEvent *event)
{
    if (event->type() == QEvent::Close)
    {
        if (m_doc->isModified())
        {
            QMetaObject::invokeMethod(rootObject(), "saveBeforeExit");
            event->ignore();
            return false;
        }
    }
    return QQuickView::event(event);
}

void App::slotSceneGraphInitialized()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    if (openglContext() == nullptr)
        return;

    qDebug() << "OpenGL version: " << openglContext()->format().majorVersion() << openglContext()->format().minorVersion();
#else
    // TODO: Qt6
#endif
}

void App::slotScreenChanged(QScreen *screen)
{
    bool isLandscape = (screen->orientation() == Qt::LandscapeOrientation ||
                     screen->orientation() == Qt::InvertedLandscapeOrientation) ? true : false;
    qreal sSize = isLandscape ? screen->size().height() : screen->size().width();
    m_pixelDensity = qMax(screen->physicalDotsPerInch() *  0.039370, sSize / 220.0);
    qDebug() << "Screen changed to" << screen->name() << ", pixel density:" << m_pixelDensity
             << ", physical size:" << screen->physicalSize();
    rootContext()->setContextProperty("screenPixelDensity", m_pixelDensity);
}

void App::slotClosing()
{
    if (m_contextManager)
    {
        delete m_contextManager;
        m_contextManager = nullptr;
    }
}

void App::slotClientAccessRequest(QString name)
{
    QMetaObject::invokeMethod(rootObject(), "openAccessRequest",
                              Q_ARG(QVariant, name));
}

void App::slotAccessMaskChanged(int mask)
{
    setAccessMask(mask);
}

/*********************************************************************
 * Doc
 *********************************************************************/
Doc *App::doc()
{
    return m_doc;
}

bool App::docLoaded()
{
    return m_docLoaded;
}

bool App::docModified() const
{
    return m_doc->isModified();
}

void App::initDoc()
{
    Q_ASSERT(m_doc == nullptr);
    m_doc = new Doc(this);

    connect(m_doc, SIGNAL(modified(bool)), this, SIGNAL(docModifiedChanged()));
    connect(m_doc->masterTimer(), SIGNAL(functionListChanged()),
            this, SIGNAL(runningFunctionsCountChanged()));

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
    m_videoProvider = new VideoProvider(this, m_doc);

    Q_ASSERT(m_doc->inputOutputMap() != nullptr);

    /* Load input plugins & profiles */
    m_doc->inputOutputMap()->loadProfiles(InputOutputMap::userProfileDirectory());
    m_doc->inputOutputMap()->loadProfiles(InputOutputMap::systemProfileDirectory());
    m_doc->inputOutputMap()->loadDefaults();

    m_doc->inputOutputMap()->setBeatGeneratorType(InputOutputMap::Internal);
    m_doc->inputOutputMap()->startUniverses();
    m_doc->masterTimer()->start();
}

void App::clearDocument()
{
    if (m_videoProvider)
    {
        delete m_videoProvider;
        m_videoProvider = nullptr;
    }

    m_contextManager->resetFixtureSelection();
    //m_simpleDesk->resetContents(); // TODO
    m_showManager->resetContents();
    m_virtualConsole->resetContents();

    m_doc->masterTimer()->stop();
    m_doc->clearContents();

    m_tardis->resetHistory();
    m_doc->inputOutputMap()->resetUniverses();
    setFileName(QString());
    m_doc->resetModified();
    m_doc->inputOutputMap()->startUniverses();
    m_doc->masterTimer()->start();
}

int App::runningFunctionsCount() const
{
    return m_doc->masterTimer()->runningFunctions();
}

void App::stopAllFunctions()
{
    // first, gracefully stop via Function Manager (if that's the case)
    m_functionManager->setPreviewEnabled(false);

    // then, brutally kill the rest (could be started from VC, etc)
    m_doc->masterTimer()->stopAllFunctions();
}

void App::enableKioskMode()
{
    // enable Virtual console only
    setAccessMask(AC_VCControl);
}

void App::createKioskCloseButton(const QRect &rect)
{
    Q_UNUSED(rect)
    // TODO
}

/*********************************************************************
 * Printer
 *********************************************************************/

void App::printItem(QQuickItem *item)
{
    if (item == nullptr)
        return;

    m_printItem = item;
    m_printerImage = item->grabToImage();
    connect(m_printerImage.data(), &QQuickItemGrabResult::ready, this, &App::slotItemReadyForPrinting);
}

void App::slotItemReadyForPrinting()
{
    QPrinter printer;
    QPrintDialog *dlg = new QPrintDialog(&printer);
    if (dlg->exec() == QDialog::Accepted)
    {
        QRectF pageRect = printer.pageLayout().paintRect();
        QSize imgSize = m_printerImage->image().size();
        int totalHeight = imgSize.height();
        int yOffset = 0;

        qDebug() << "Page size:" << pageRect << ", image size:" << imgSize;
        QPainter painter(&printer);
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setRenderHint(QPainter::SmoothPixmapTransform);

        QImage img = m_printerImage->image();
        int actualWidth = imgSize.width();

        // if the grabbed image is larger than the page, fit it to the page width
        if (pageRect.width() < imgSize.width())
        {
            img = m_printerImage->image().scaledToWidth(pageRect.width(), Qt::SmoothTransformation);
            actualWidth = pageRect.width();
        }

        // handle multi-page printing
        while (totalHeight > 0)
        {
            painter.drawImage(QPoint(0, 0), img, QRectF(0, yOffset, actualWidth, pageRect.height()));
            yOffset += pageRect.height();
            totalHeight -= pageRect.height();
            if (totalHeight > 0)
                printer.newPage();
        }

        painter.end();
    }

    m_printerImage.clear();
    m_printItem->setProperty("isPrinting", false);
    m_printItem = nullptr;
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

QString App::workingPath() const
{
    return m_workingPath;
}

void App::setWorkingPath(QString workingPath)
{
    QString strippedPath = workingPath.replace("file://", "");

    if (m_workingPath == strippedPath)
        return;

    m_workingPath = strippedPath;

    QSettings settings;
    settings.setValue(SETTINGS_WORKINGPATH, m_workingPath);

    emit workingPathChanged(strippedPath);
}

bool App::newWorkspace()
{
    clearDocument();
    m_fixtureManager->slotDocLoaded();
    m_functionManager->slotDocLoaded();
    m_contextManager->resetContexts();
    return true;
}

bool App::loadWorkspace(const QString &fileName)
{
    m_contextManager->resetContexts();

    /* Clear existing document data */
    clearDocument();
    m_docLoaded = false;
    emit docLoadedChanged();

    QString localFilename =  fileName;
    if (localFilename.startsWith("file:"))
        localFilename = QUrl(fileName).toLocalFile();

    if (loadXML(localFilename) == QFile::NoError)
    {
        setTitle(QString("%1 - %2").arg(APPNAME).arg(localFilename));
        setFileName(localFilename);
        m_docLoaded = true;
        updateRecentFilesList(localFilename);
        emit docLoadedChanged();
        m_doc->resetModified();
        m_videoProvider = new VideoProvider(this, m_doc);
        m_contextManager->resetContexts();

        // autostart Function if set
        if (m_doc->startupFunction() != Function::invalidId())
        {
            Function *func = m_doc->function(m_doc->startupFunction());
            if (func != nullptr)
            {
                qDebug() << Q_FUNC_INFO << "Starting startup function. (" << m_doc->startupFunction() << ")";
                func->start(m_doc->masterTimer(), FunctionParent::master());
            }
            else
            {
                qWarning() << Q_FUNC_INFO << "Startup function does not exist, erasing. (" << m_doc->startupFunction() << ")";
                m_doc->setStartupFunction(Function::invalidId());
            }
        }

        m_doc->inputOutputMap()->startUniverses();

        return true;
    }
    return false;
}

void App::slotLoadDocFromMemory(QByteArray &xmlData)
{
    if (xmlData.isEmpty())
        return;

    /* Clear existing document data */
    clearDocument();

    QBuffer databuf;
    databuf.setData(xmlData);
    databuf.open(QIODevice::ReadOnly | QIODevice::Text);

    //qDebug() << "Buffer data:" << databuf.data();
    QXmlStreamReader doc(&databuf);

    if (doc.hasError())
    {
        qWarning() << Q_FUNC_INFO << "Unable to read from XML in memory";
        return;
    }

    while (!doc.atEnd())
    {
        if (doc.readNext() == QXmlStreamReader::DTD)
            break;
    }
    if (doc.hasError())
    {
        qDebug() << "XML has errors:" << doc.errorString();
        return;
    }

    if (doc.dtdName() == KXMLQLCWorkspace)
        loadXML(doc, true, true);
    else
        qDebug() << "XML doesn't have a Workspace tag";
}

bool App::saveWorkspace(const QString &fileName)
{
    QString localFilename = fileName;
    if (localFilename.startsWith("file:"))
        localFilename = QUrl(fileName).toLocalFile();

    /* Always use the workspace suffix */
    if (localFilename.right(4) != KExtWorkspace)
        localFilename += KExtWorkspace;

    /* Set the workspace path before saving the new XML. In this way local files
       can be loaded even if the workspace file will be moved */
    m_doc->setWorkspacePath(QFileInfo(localFilename).absolutePath());

    if (saveXML(localFilename) == QFile::NoError)
    {
        setTitle(QString("%1 - %2").arg(APPNAME).arg(localFilename));
        updateRecentFilesList(localFilename);
        return true;
    }

    return false;
}

QFileDevice::FileError App::loadXML(const QString &fileName)
{
    QFile::FileError retval = QFile::NoError;

    if (fileName.isEmpty() == true)
        return QFile::OpenError;

    QXmlStreamReader *doc = QLCFile::getXMLReader(fileName);
    if (doc == nullptr || doc->device() == nullptr || doc->hasError())
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

    /* Set the workspace path before loading the new XML. In this way local files
       can be loaded even if the workspace file has been moved */
    m_doc->setWorkspacePath(QFileInfo(fileName).absolutePath());

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
        qWarning() << Q_FUNC_INFO << fileName << "is not a workspace file";
    }

    QLCFile::releaseXMLReader(doc);

    return retval;
}

bool App::loadXML(QXmlStreamReader &doc, bool goToConsole, bool fromMemory)
{
    if (doc.readNextStartElement() == false)
        return false;

    if (doc.name() != KXMLQLCWorkspace)
    {
        qWarning() << Q_FUNC_INFO << "Workspace node not found";
        return false;
    }

    QString contextName = doc.attributes().value(KXMLQLCWorkspaceWindow).toString();

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

    if (goToConsole == true || accessMask() == AC_VCControl)
        // Force the active window to be Virtual Console
        m_contextManager->switchToContext("VirtualConsole");
    else
        // Set the active window to what was saved in the workspace file
        m_contextManager->switchToContext(contextName);

    // Perform post-load operations
    m_virtualConsole->postLoad();

    if (m_doc->errorLog().isEmpty() == false &&
        fromMemory == false)
    {
        // TODO: emit a signal to inform the QML UI to display an error message
        /*
        QMessageBox msg(QMessageBox::Warning, tr("Warning"),
                        tr("Some errors occurred while loading the project:") + "\n\n" + m_doc->errorLog(),
                        QMessageBox::Ok);
        msg.exec();
        */
    }

    return true;
}

QFile::FileError App::saveXML(const QString& fileName)
{
    QString tempFileName(fileName);
    tempFileName += ".temp";
    QFile file(tempFileName);
    if (file.open(QIODevice::WriteOnly) == false)
        return file.error();

    QXmlStreamWriter doc(&file);
    doc.setAutoFormatting(true);
    doc.setAutoFormattingIndent(1);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    doc.setCodec("UTF-8");
#endif
    doc.writeStartDocument();
    doc.writeDTD(QString("<!DOCTYPE %1>").arg(KXMLQLCWorkspace));

    doc.writeStartElement(KXMLQLCWorkspace);
    doc.writeAttribute("xmlns", QString("%1%2").arg(KXMLQLCplusNamespace).arg(KXMLQLCWorkspace));

    /* Currently active context */
    doc.writeAttribute(KXMLQLCWorkspaceWindow, m_contextManager->currentContext());

    /* Creator information */
    doc.writeStartElement(KXMLQLCCreator);
    doc.writeTextElement(KXMLQLCCreatorName, APPNAME);
    doc.writeTextElement(KXMLQLCCreatorVersion, APPVERSION);
    doc.writeTextElement(KXMLQLCCreatorAuthor, QLCFile::currentUserName());
    doc.writeEndElement();

    /* Write engine components to the XML document */
    m_doc->saveXML(&doc);

    /* Write virtual console to the XML document */
    m_virtualConsole->saveXML(&doc);

    /* Write Simple Desk to the XML document */
    //SimpleDesk::instance()->saveXML(&doc);

    doc.writeEndElement(); // close KXMLQLCWorkspace

    /* End the document and close all the open elements */
    doc.writeEndDocument();
    file.close();

    // Save to actual requested file name
    QFile currFile(fileName);
    if (currFile.exists() && !currFile.remove())
    {
        qWarning() << "Could not erase" << fileName;
        return currFile.error();
    }
    if (!file.rename(fileName))
    {
        qWarning() << "Could not rename" << tempFileName << "to" << fileName;
        return file.error();
    }

    /* Set the file name for the current Doc instance and
       set it also in an unmodified state. */
    setFileName(fileName);
    m_doc->resetModified();

    return QFile::NoError;
}

/*********************************************************************
 * Import project
 *********************************************************************/

bool App::loadImportWorkspace(const QString &fileName)
{
    if (m_importManager != nullptr)
        delete m_importManager;

    m_importManager = new ImportManager(this, m_doc);
    return m_importManager->loadWorkspace(fileName);
}

void App::cancelImport()
{
    if (m_importManager != nullptr)
        delete m_importManager;

    m_importManager = nullptr;
}

void App::importFromWorkspace()
{
    if (m_importManager == nullptr)
        return;

    m_importManager->apply();
    m_paletteManager->updatePaletteList();

    delete m_importManager;
    m_importManager = nullptr;
}

/*********************************************************************
 * Fixture editor
 *********************************************************************/

void App::createFixture()
{
    if (m_fixtureEditor == nullptr)
    {
        m_fixtureEditor = new FixtureEditor(this, m_doc);
        QMetaObject::invokeMethod(rootObject(), "switchToContext",
                                  Q_ARG(QVariant, "FXEDITOR"),
                                  Q_ARG(QVariant, "qrc:/FixtureEditor.qml"));
    }

    m_fixtureEditor->createDefinition();
}

void App::loadFixture(QString fileName)
{
    if (m_fixtureEditor == nullptr)
    {
        m_fixtureEditor = new FixtureEditor(this, m_doc);
        QMetaObject::invokeMethod(rootObject(), "switchToContext",
                                  Q_ARG(QVariant, "FXEDITOR"),
                                  Q_ARG(QVariant, "qrc:/FixtureEditor.qml"));
    }
    m_fixtureEditor->loadDefinition(fileName);
}

void App::editFixture(QString manufacturer, QString model)
{
    bool switchToEditor = false;

    if (m_fixtureEditor == nullptr)
    {
        m_fixtureEditor = new FixtureEditor(this, m_doc);
        switchToEditor = true;
    }

    if (m_fixtureEditor->editDefinition(manufacturer, model) == false)
    {
        delete m_fixtureEditor;
        m_fixtureEditor = nullptr;
        return;
    }

    if (switchToEditor)
    {
        QMetaObject::invokeMethod(rootObject(), "switchToContext",
                                  Q_ARG(QVariant, "FXEDITOR"),
                                  Q_ARG(QVariant, "qrc:/FixtureEditor.qml"));
    }
}

void App::closeFixtureEditor()
{
    if (m_fixtureEditor)
    {
        delete m_fixtureEditor;
        m_fixtureEditor = nullptr;
    }

    // reload the QLC+ main view
    //setSource(QUrl("qrc:/MainView.qml"));
    QMetaObject::invokeMethod(rootObject(), "switchToContext",
                              Q_ARG(QVariant, "FIXANDFUNC"),
                              Q_ARG(QVariant, "qrc:/FixturesAndFunctions.qml"));
}

