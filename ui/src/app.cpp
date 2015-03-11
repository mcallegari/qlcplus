/*
  Q Light Controller
  app.cpp

  Copyright (c) Heikki Junnila,
                Christopher Staite

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

#include <QToolButton>
#include <QtCore>
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QtGui>
#else
#include <QtWidgets>
#endif
#include <QtXml>

#if defined(WIN32) || defined(Q_OS_WIN)
  #include <windows.h>
#endif

#include "functionliveeditdialog.h"
#include "inputoutputmanager.h"
#include "functionselection.h"
#include "functionmanager.h"
#include "inputoutputmap.h"
#include "virtualconsole.h"
#include "fixturemanager.h"
#include "dmxdumpfactory.h"
#include "showmanager.h"
#include "mastertimer.h"
#include "addresstool.h"
#include "simpledesk.h"
#include "docbrowser.h"
#include "aboutbox.h"
#include "monitor.h"
#include "vcframe.h"
#include "app.h"
#include "doc.h"

#include "rgbscriptscache.h"
#include "qlcfixturedefcache.h"
#include "qlcfixturedef.h"
#include "qlcconfig.h"
#include "qlcfile.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
 #include "videoprovider.h"
#endif

#define SETTINGS_GEOMETRY "workspace/geometry"
#define SETTINGS_WORKINGPATH "workspace/workingpath"
#define SETTINGS_RECENTFILE "workspace/recent"
#define KXMLQLCWorkspaceWindow "CurrentWindow"

#define MAX_RECENT_FILES    10

#define KModeTextOperate QObject::tr("Operate")
#define KModeTextDesign QObject::tr("Design")
#define KUniverseCount 4

/*****************************************************************************
 * Initialization
 *****************************************************************************/

App::App()
    : QMainWindow()
    , m_tab(NULL)
    , m_overscan(false)
    , m_progressDialog(NULL)
    , m_doc(NULL)

    , m_fileNewAction(NULL)
    , m_fileOpenAction(NULL)
    , m_fileSaveAction(NULL)
    , m_fileSaveAsAction(NULL)

    , m_modeToggleAction(NULL)
    , m_controlMonitorAction(NULL)
    , m_addressToolAction(NULL)
    , m_controlFullScreenAction(NULL)
    , m_controlBlackoutAction(NULL)
    , m_controlPanicAction(NULL)
    , m_dumpDmxAction(NULL)
    , m_liveEditAction(NULL)
    , m_liveEditVirtualConsoleAction(NULL)

    , m_helpIndexAction(NULL)
    , m_helpAboutAction(NULL)
    , m_fileOpenMenu(NULL)
    , m_fadeAndStopMenu(NULL)

    , m_toolbar(NULL)

    , m_dumpProperties(NULL)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    , m_videoProvider(NULL)
#endif
{
    QCoreApplication::setOrganizationName("qlcplus");
    QCoreApplication::setOrganizationDomain("sf.net");
    QCoreApplication::setApplicationName(APPNAME);
}

App::~App()
{
    QSettings settings;

    // Don't save kiosk-mode window geometry because that will screw things up
    if (m_doc->isKiosk() == false)
        settings.setValue(SETTINGS_GEOMETRY, saveGeometry());
    else
        settings.setValue(SETTINGS_GEOMETRY, QVariant());

    if (Monitor::instance() != NULL)
        delete Monitor::instance();

    if (FixtureManager::instance() != NULL)
        delete FixtureManager::instance();

    if (FunctionManager::instance() != NULL)
        delete FunctionManager::instance();

    if (ShowManager::instance() != NULL)
        delete ShowManager::instance();

    if (InputOutputManager::instance() != NULL)
        delete InputOutputManager::instance();

    if (VirtualConsole::instance() != NULL)
        delete VirtualConsole::instance();

    if (SimpleDesk::instance() != NULL)
        delete SimpleDesk::instance();

    if (m_dumpProperties != NULL)
        delete m_dumpProperties;

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    if (m_videoProvider != NULL)
        delete m_videoProvider;
#endif

    if (m_doc != NULL)
        delete m_doc;

    m_doc = NULL;
}

void App::startup()
{
#if defined(__APPLE__) || defined(Q_OS_MAC)
    createProgressDialog();
#endif

    init();
    slotModeDesign();
    slotDocModified(false);

#if defined(__APPLE__) || defined(Q_OS_MAC)
    destroyProgressDialog();
#endif

    // Activate FixtureManager
    setActiveWindow(FixtureManager::staticMetaObject.className());
}

void App::enableOverscan()
{
    m_overscan = true;
}

void App::init()
{
    QSettings settings;

    setWindowIcon(QIcon(":/qlcplus.png"));

    m_tab = new QTabWidget(this);
    m_tab->setTabPosition(QTabWidget::South);
    setCentralWidget(m_tab);

    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
    {
        this->restoreGeometry(var.toByteArray());
    }
    else
    {
        /* Application geometry and window state */
        QSize size = settings.value("/workspace/size").toSize();
        if (size.isValid() == true)
            resize(size);
        else
        {
            if (QLCFile::isRaspberry())
            {
                QRect geometry = qApp->desktop()->availableGeometry();
                int w = geometry.width();
                int h = geometry.height();
                if (m_overscan == true)
                {
                    // if we're on a Raspberry Pi, introduce a 5% margin
                    w = (float)geometry.width() * 0.95;
                    h = (float)geometry.height() * 0.95;
                }
                setGeometry((geometry.width() - w) / 2, (geometry.height() - h) / 2,
                            w, h);
            }
            else
                resize(800, 600);
        }

        QVariant state = settings.value("/workspace/state", Qt::WindowNoState);
        if (state.isValid() == true)
            setWindowState(Qt::WindowState(state.toInt()));
    }

    QVariant dir = settings.value(SETTINGS_WORKINGPATH);
    if (dir.isValid() == true)
        m_workingDirectory = QDir(dir.toString());

    // The engine object
    initDoc();
    // Main view actions
    initActions();
    // Main tool bar
    initToolBar();

    m_dumpProperties = new DmxDumpFactoryProperties(KUniverseCount);

    // Create primary views.
    m_tab->setIconSize(QSize(24, 24));
    QWidget* w = new FixtureManager(m_tab, m_doc);
    m_tab->addTab(w, QIcon(":/fixture.png"), tr("Fixtures"));
    w = new FunctionManager(m_tab, m_doc);
    m_tab->addTab(w, QIcon(":/function.png"), tr("Functions"));
    w = new ShowManager(m_tab, m_doc);
    m_tab->addTab(w, QIcon(":/show.png"), tr("Shows"));
    w = new VirtualConsole(m_tab, m_doc);
    m_tab->addTab(w, QIcon(":/virtualconsole.png"), tr("Virtual Console"));
    w = new SimpleDesk(m_tab, m_doc);
    m_tab->addTab(w, QIcon(":/slidermatrix.png"), tr("Simple Desk"));
    w = new InputOutputManager(m_tab, m_doc);
    m_tab->addTab(w, QIcon(":/input_output.png"), tr("Inputs/Outputs"));

    // Listen to blackout changes and toggle m_controlBlackoutAction
    connect(m_doc->inputOutputMap(), SIGNAL(blackoutChanged(bool)), this, SLOT(slotBlackoutChanged(bool)));

    // Enable/Disable panic button
    connect(m_doc->masterTimer(), SIGNAL(functionListChanged()), this, SLOT(slotRunningFunctionsChanged()));
    slotRunningFunctionsChanged();

    // Start up in non-modified state
    m_doc->resetModified();

    QString ssDir;

#if defined(WIN32) || defined(Q_OS_WIN)
    /* User's input profile directory on Windows */
    LPTSTR home = (LPTSTR) malloc(256 * sizeof(TCHAR));
    GetEnvironmentVariable(TEXT("UserProfile"), home, 256);
    ssDir = QString("%1/%2").arg(QString::fromUtf16(reinterpret_cast<ushort*> (home)))
                            .arg(USERQLCPLUSDIR);
    free(home);
#else
    /* User's input profile directory on *NIX systems */
    ssDir = QString("%1/%2").arg(getenv("HOME")).arg(USERQLCPLUSDIR);
#endif

    QFile ssFile(ssDir + QDir::separator() + "qlcplusStyle.qss");
    if (ssFile.exists() == true)
    {
        ssFile.open(QFile::ReadOnly);
        QString styleSheet = QLatin1String(ssFile.readAll());
        this->setStyleSheet(styleSheet);
    }

#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    m_videoProvider = new VideoProvider(m_doc, this);
#endif
}

void App::setActiveWindow(const QString& name)
{
    if (name.isEmpty() == true)
        return;

    for (int i = 0; i < m_tab->count(); i++)
    {
        QWidget* widget = m_tab->widget(i);
        if (widget != NULL && widget->metaObject()->className() == name)
        {
            m_tab->setCurrentIndex(i);
            break;
        }
    }
}

void App::closeEvent(QCloseEvent* e)
{
    int result = 0;

    if (m_doc->mode() == Doc::Operate && m_doc->isKiosk() == false)
    {
        QMessageBox::warning(this,
                             tr("Cannot exit in Operate mode"),
                             tr("You must switch back to Design mode " \
                                "to close the application."));
        e->ignore();
        return;
    }

    if (m_doc->isKiosk() == false)
    {
        if( saveModifiedDoc(tr("Close"), tr("Do you wish to save the current workspace " \
                                            "before closing the application?")) == true)
        {
            e->accept();
        }
        else
        {
            e->ignore();
        }
    }
    else
    {
        if (m_doc->isKiosk() == true)
        {
            result = QMessageBox::warning(this, tr("Close the application?"),
                                          tr("Do you wish to close the application?"),
                                          QMessageBox::Yes, QMessageBox::No);
            if (result == QMessageBox::No)
            {
                e->ignore();
                return;
            }
        }

        e->accept();
    }
}

/*****************************************************************************
 * Progress dialog
 *****************************************************************************/

void App::createProgressDialog()
{
    m_progressDialog = new QProgressDialog;
    m_progressDialog->setCancelButton(NULL);
    m_progressDialog->show();
    m_progressDialog->raise();
    m_progressDialog->setRange(0, 10);
    slotSetProgressText(QString());
    QApplication::processEvents();
}

void App::destroyProgressDialog()
{
    delete m_progressDialog;
    m_progressDialog = NULL;
}

void App::slotSetProgressText(const QString& text)
{
    if (m_progressDialog == NULL)
        return;

    static int progress = 0;
    m_progressDialog->setValue(progress++);
    m_progressDialog->setLabelText(QString("<B>%1</B><BR/>%2")
                                   .arg(tr("Starting Q Light Controller Plus"))
                                   .arg(text));
    QApplication::processEvents();
}

/*****************************************************************************
 * Doc
 *****************************************************************************/

void App::clearDocument()
{
    VirtualConsole::instance()->resetContents();
    m_doc->clearContents();
    SimpleDesk::instance()->clearContents();
    ShowManager::instance()->clearContents();
    m_doc->inputOutputMap()->resetUniverses();
    setFileName(QString());
    m_doc->resetModified();
}

Doc *App::doc()
{
    return m_doc;
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
    connect(m_doc->ioPluginCache(), SIGNAL(pluginLoaded(const QString&)),
            this, SLOT(slotSetProgressText(const QString&)));
    m_doc->ioPluginCache()->load(IOPluginCache::systemPluginDirectory());

    /* Restore outputmap settings */
    Q_ASSERT(m_doc->inputOutputMap() != NULL);

    /* Load input plugins & profiles */
    m_doc->inputOutputMap()->loadProfiles(InputOutputMap::userProfileDirectory());
    m_doc->inputOutputMap()->loadProfiles(InputOutputMap::systemProfileDirectory());
    m_doc->inputOutputMap()->loadDefaults();

    m_doc->masterTimer()->start();
}

void App::slotDocModified(bool state)
{
    QString caption(APPNAME);

    if (fileName().isEmpty() == false)
        caption += QString(" - ") + QDir::toNativeSeparators(fileName());
    else
        caption += tr(" - New Workspace");

    if (state == true)
        setWindowTitle(caption + QString(" *"));
    else
        setWindowTitle(caption);
}

/*****************************************************************************
 * Main application Mode
 *****************************************************************************/

void App::enableKioskMode()
{
    // Turn on operate mode
    m_doc->setKiosk(true);
    m_doc->setMode(Doc::Operate);
    if (VirtualConsole::instance()->checkStartupFunction(m_doc->startupFunction()) == false)
        m_doc->checkStartupFunction();

    // No need for these
    m_tab->removeTab(m_tab->indexOf(FixtureManager::instance()));
    m_tab->removeTab(m_tab->indexOf(FunctionManager::instance()));
    m_tab->removeTab(m_tab->indexOf(ShowManager::instance()));
    m_tab->removeTab(m_tab->indexOf(SimpleDesk::instance()));
    m_tab->removeTab(m_tab->indexOf(InputOutputManager::instance()));

    // Hide the tab bar to save some pixels
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    // tabBar() in QT4 is protected.
    m_tab->tabBar()->hide();
#endif

    // No need for the toolbar
    delete m_toolbar;
    m_toolbar = NULL;
}

void App::createKioskCloseButton(const QRect& rect)
{
    QPushButton* btn = new QPushButton(VirtualConsole::instance()->contents());
    btn->setIcon(QIcon(":/exit.png"));
    btn->setToolTip(tr("Exit"));
    btn->setGeometry(rect);
    connect(btn, SIGNAL(clicked()), this, SLOT(close()));
    btn->show();
}

void App::slotModeOperate()
{
    m_doc->setMode(Doc::Operate);
    if (VirtualConsole::instance()->checkStartupFunction(m_doc->startupFunction()) == false)
        m_doc->checkStartupFunction();
}

void App::slotModeDesign()
{
    if (m_doc->masterTimer()->runningFunctions() > 0)
    {
        int result = QMessageBox::warning(
                         this,
                         tr("Switch to Design Mode"),
                         tr("There are still running functions.\n"
                            "Really stop them and switch back to "
                            "Design mode?"),
                         QMessageBox::Yes,
                         QMessageBox::No);

        if (result == QMessageBox::No)
            return;
        else
            m_doc->masterTimer()->stopAllFunctions();
    }

    m_liveEditVirtualConsoleAction->setChecked(false);
    m_doc->setMode(Doc::Design);
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
    if (mode == Doc::Operate)
    {
        /* Disable editing features */
        m_fileNewAction->setEnabled(false);
        m_fileOpenAction->setEnabled(false);
        m_liveEditAction->setEnabled(true);
        m_liveEditVirtualConsoleAction->setEnabled(true);

        m_modeToggleAction->setIcon(QIcon(":/design.png"));
        m_modeToggleAction->setText(tr("Design"));
        m_modeToggleAction->setToolTip(tr("Switch to design mode"));
    }
    else if (mode == Doc::Design)
    {
        /* Enable editing features */
        m_fileNewAction->setEnabled(true);
        m_fileOpenAction->setEnabled(true);
        m_liveEditAction->setEnabled(false);
        m_liveEditVirtualConsoleAction->setEnabled(false);

        m_modeToggleAction->setIcon(QIcon(":/operate.png"));
        m_modeToggleAction->setText(tr("Operate"));
        m_modeToggleAction->setToolTip(tr("Switch to operate mode"));
    }
}

/*****************************************************************************
 * Actions and toolbar
 *****************************************************************************/

void App::initActions()
{
    /* File actions */
    m_fileNewAction = new QAction(QIcon(":/filenew.png"), tr("&New"), this);
    m_fileNewAction->setShortcut(QKeySequence(tr("CTRL+N", "File|New")));
    connect(m_fileNewAction, SIGNAL(triggered(bool)), this, SLOT(slotFileNew()));

    m_fileOpenAction = new QAction(QIcon(":/fileopen.png"), tr("&Open"), this);
    m_fileOpenAction->setShortcut(QKeySequence(tr("CTRL+O", "File|Open")));
    connect(m_fileOpenAction, SIGNAL(triggered(bool)), this, SLOT(slotFileOpen()));

    m_fileSaveAction = new QAction(QIcon(":/filesave.png"), tr("&Save"), this);
    m_fileSaveAction->setShortcut(QKeySequence(tr("CTRL+S", "File|Save")));
    connect(m_fileSaveAction, SIGNAL(triggered(bool)), this, SLOT(slotFileSave()));

    m_fileSaveAsAction = new QAction(QIcon(":/filesaveas.png"), tr("Save &As..."), this);
    connect(m_fileSaveAsAction, SIGNAL(triggered(bool)), this, SLOT(slotFileSaveAs()));

    /* Control actions */
    m_modeToggleAction = new QAction(QIcon(":/operate.png"), tr("&Operate"), this);
    m_modeToggleAction->setToolTip(tr("Switch to operate mode"));
    m_modeToggleAction->setShortcut(QKeySequence(tr("CTRL+F12", "Control|Toggle operate/design mode")));
    connect(m_modeToggleAction, SIGNAL(triggered(bool)), this, SLOT(slotModeToggle()));

    m_controlMonitorAction = new QAction(QIcon(":/monitor.png"), tr("&Monitor"), this);
    m_controlMonitorAction->setShortcut(QKeySequence(tr("CTRL+M", "Control|Monitor")));
    connect(m_controlMonitorAction, SIGNAL(triggered(bool)), this, SLOT(slotControlMonitor()));

    m_addressToolAction = new QAction(QIcon(":/diptool.png"), tr("Address Tool"), this);
    connect(m_addressToolAction, SIGNAL(triggered()), this, SLOT(slotAddressTool()));

    m_controlBlackoutAction = new QAction(QIcon(":/blackout.png"), tr("Toggle &Blackout"), this);
    m_controlBlackoutAction->setCheckable(true);
    connect(m_controlBlackoutAction, SIGNAL(triggered(bool)), this, SLOT(slotControlBlackout()));
    m_controlBlackoutAction->setChecked(m_doc->inputOutputMap()->blackout());

    m_liveEditAction = new QAction(QIcon(":/liveedit.png"), tr("Live edit a function"), this);
    connect(m_liveEditAction, SIGNAL(triggered()), this, SLOT(slotFunctionLiveEdit()));
    m_liveEditAction->setEnabled(false);

    m_liveEditVirtualConsoleAction = new QAction(QIcon(":/liveedit_vc.png"), tr("Toggle Virtual Console Live edit"), this);
    connect(m_liveEditVirtualConsoleAction, SIGNAL(triggered()), this, SLOT(slotLiveEditVirtualConsole()));
    m_liveEditVirtualConsoleAction->setCheckable(true);
    m_liveEditVirtualConsoleAction->setEnabled(false);

    m_dumpDmxAction = new QAction(QIcon(":/add_dump.png"), tr("Dump DMX values to a function"), this);
    m_dumpDmxAction->setShortcut(QKeySequence(tr("CTRL+D", "Control|Dump DMX")));
    connect(m_dumpDmxAction, SIGNAL(triggered()), this, SLOT(slotDumpDmxIntoFunction()));

    m_controlPanicAction = new QAction(QIcon(":/panic.png"), tr("Stop ALL functions!"), this);
    m_controlPanicAction->setShortcut(QKeySequence("CTRL+SHIFT+ESC"));
    connect(m_controlPanicAction, SIGNAL(triggered(bool)), this, SLOT(slotControlPanic()));

    m_fadeAndStopMenu = new QMenu();
    QAction *fade1 = new QAction(tr("Fade 1 second and stop"), this);
    fade1->setData(QVariant(1000));
    connect(fade1, SIGNAL(triggered()), this, SLOT(slotFadeAndStopAll()));
    m_fadeAndStopMenu->addAction(fade1);

    QAction *fade5 = new QAction(tr("Fade 5 seconds and stop"), this);
    fade5->setData(QVariant(5000));
    connect(fade5, SIGNAL(triggered()), this, SLOT(slotFadeAndStopAll()));
    m_fadeAndStopMenu->addAction(fade5);

    QAction *fade10 = new QAction(tr("Fade 10 second and stop"), this);
    fade10->setData(QVariant(10000));
    connect(fade10, SIGNAL(triggered()), this, SLOT(slotFadeAndStopAll()));
    m_fadeAndStopMenu->addAction(fade10);

    QAction *fade30 = new QAction(tr("Fade 30 second and stop"), this);
    fade30->setData(QVariant(30000));
    connect(fade30, SIGNAL(triggered()), this, SLOT(slotFadeAndStopAll()));
    m_fadeAndStopMenu->addAction(fade30);

    m_controlPanicAction->setMenu(m_fadeAndStopMenu);

    m_controlFullScreenAction = new QAction(QIcon(":/fullscreen.png"), tr("Toggle Full Screen"), this);
    m_controlFullScreenAction->setCheckable(true);
    m_controlFullScreenAction->setShortcut(QKeySequence(tr("CTRL+F11", "Control|Toggle Full Screen")));
    connect(m_controlFullScreenAction, SIGNAL(triggered(bool)), this, SLOT(slotControlFullScreen()));

    /* Help actions */
    m_helpIndexAction = new QAction(QIcon(":/help.png"), tr("&Index"), this);
    m_helpIndexAction->setShortcut(QKeySequence(tr("SHIFT+F1", "Help|Index")));
    connect(m_helpIndexAction, SIGNAL(triggered(bool)), this, SLOT(slotHelpIndex()));

    m_helpAboutAction = new QAction(QIcon(":/qlcplus.png"), tr("&About QLC+"), this);
    connect(m_helpAboutAction, SIGNAL(triggered(bool)), this, SLOT(slotHelpAbout()));
}

void App::initToolBar()
{
    m_toolbar = new QToolBar(tr("Workspace"), this);
    m_toolbar->setFloatable(false);
    m_toolbar->setMovable(false);
    m_toolbar->setAllowedAreas(Qt::TopToolBarArea);
    m_toolbar->setContextMenuPolicy(Qt::CustomContextMenu);
    addToolBar(m_toolbar);
    m_toolbar->addAction(m_fileNewAction);
    m_toolbar->addAction(m_fileOpenAction);
    m_toolbar->addAction(m_fileSaveAction);
    m_toolbar->addAction(m_fileSaveAsAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_controlMonitorAction);
    m_toolbar->addAction(m_addressToolAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_controlFullScreenAction);
    m_toolbar->addAction(m_helpIndexAction);
    m_toolbar->addAction(m_helpAboutAction);

    /* Create an empty widget between help items to flush them to the right */
    QWidget* widget = new QWidget(this);
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_toolbar->addWidget(widget);
    m_toolbar->addAction(m_dumpDmxAction);
    m_toolbar->addAction(m_liveEditAction);
    m_toolbar->addAction(m_liveEditVirtualConsoleAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_controlPanicAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_controlBlackoutAction);
    m_toolbar->addSeparator();
    m_toolbar->addAction(m_modeToggleAction);

    QToolButton* btn = qobject_cast<QToolButton*> (m_toolbar->widgetForAction(m_fileOpenAction));
    Q_ASSERT(btn != NULL);
    btn->setPopupMode(QToolButton::DelayedPopup);
    updateFileOpenMenu("");

    btn = qobject_cast<QToolButton*> (m_toolbar->widgetForAction(m_controlPanicAction));
    Q_ASSERT(btn != NULL);
    btn->setPopupMode(QToolButton::DelayedPopup);
}

/*****************************************************************************
 * File action slots
 *****************************************************************************/

bool App::handleFileError(QFile::FileError error)
{
    QString msg;

    switch (error)
    {
    case QFile::NoError:
        return true;
        break;
    case QFile::ReadError:
        msg = tr("Unable to read from file");
        break;
    case QFile::WriteError:
        msg = tr("Unable to write to file");
        break;
    case QFile::FatalError:
        msg = tr("A fatal error occurred");
        break;
    case QFile::ResourceError:
        msg = tr("Unable to access resource");
        break;
    case QFile::OpenError:
        msg = tr("Unable to open file for reading or writing");
        break;
    case QFile::AbortError:
        msg = tr("Operation was aborted");
        break;
    case QFile::TimeOutError:
        msg = tr("Operation timed out");
        break;
    default:
    case QFile::UnspecifiedError:
        msg = tr("An unspecified error has occurred. Nice.");
        break;
    }

    QMessageBox::warning(this, tr("File error"), msg);

    return false;
}

bool App::saveModifiedDoc(const QString & title, const QString & message)
{
    // if it's not modified, there's nothing to save
    if (m_doc->isModified() == false)
        return true;

    int result = QMessageBox::warning(this, title,
                                          message,
                                          QMessageBox::Yes,
                                          QMessageBox::No,
                                          QMessageBox::Cancel);
    if (result == QMessageBox::Yes)
    {
        slotFileSave();
        // we check whether m_doc is not modified anymore, rather than 
        // result of slotFileSave() since the latter returns NoError
        // in cases like when the user pressed cancel in the save dialog
        if (m_doc->isModified() == false) 
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (result == QMessageBox::No)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void App::updateFileOpenMenu(QString addRecent)
{
    QSettings settings;
    QStringList menuRecentList;

    if (m_fileOpenMenu == NULL)
    {
        m_fileOpenMenu = new QMenu(this);
        QString style = "QMenu { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #B9D9E8, stop:1 #A4C0CE);"
                        "border: 1px solid black; font:bold; }"
                        "QMenu::item { background-color: transparent; padding: 5px 10px 5px 10px; border: 1px solid black; }"
                        "QMenu::item:selected { background-color: #2D8CFF; }";
        m_fileOpenMenu->setStyleSheet(style);
        connect(m_fileOpenMenu, SIGNAL(triggered(QAction*)),
                this, SLOT(slotRecentFileClicked(QAction*)));
    }

    foreach (QAction* a, m_fileOpenMenu->actions())
    {
        menuRecentList.append(a->text());
        m_fileOpenMenu->removeAction(a);
    }

    if (addRecent.isEmpty() == false)
    {
        menuRecentList.removeAll(addRecent); // in case the string is already present, remove it...
        menuRecentList.prepend(addRecent); // and add it to the top
        for (int i = 0; i < menuRecentList.count(); i++)
        {
            settings.setValue(QString("%1%2").arg(SETTINGS_RECENTFILE).arg(i), menuRecentList.at(i));
            m_fileOpenMenu->addAction(menuRecentList.at(i));
        }
    }
    else
    {
        for (int i = 0; i < MAX_RECENT_FILES; i++)
        {
            QVariant recent = settings.value(QString("%1%2").arg(SETTINGS_RECENTFILE).arg(i));
            if (recent.isValid() == true)
            {
                menuRecentList.append(recent.toString());
                m_fileOpenMenu->addAction(menuRecentList.at(i));
            }
        }
    }

    // Set the recent files menu to the file open action
    if (menuRecentList.isEmpty() == false)
        m_fileOpenAction->setMenu(m_fileOpenMenu);
}

bool App::slotFileNew()
{
    QString msg(tr("Do you wish to save the current workspace?\n" \
                   "Changes will be lost if you don't save them."));
    if (saveModifiedDoc(tr("New Workspace"), msg) == false)
    {
        return false;
    }

    clearDocument();
    return true;
}

QFile::FileError App::slotFileOpen()
{
    QString fn;

    /* Check that the user is aware of losing previous changes */
    QString msg(tr("Do you wish to save the current workspace?\n" \
                   "Changes will be lost if you don't save them."));
    if (saveModifiedDoc(tr("Open Workspace"), msg) == false)
    {
        /* Second thoughts... Cancel loading. */
        return QFile::NoError;
    }

    /* Create a file open dialog */
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Open Workspace"));
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.selectFile(fileName());
    if (m_workingDirectory.exists() == true)
        dialog.setDirectory(m_workingDirectory);

    /* Append file filters to the dialog */
    QStringList filters;
    filters << tr("Workspaces (*%1)").arg(KExtWorkspace);
#if defined(WIN32) || defined(Q_OS_WIN)
    filters << tr("All Files (*.*)");
#else
    filters << tr("All Files (*)");
#endif
    dialog.setNameFilters(filters);

    /* Append useful URLs to the dialog */
    QList <QUrl> sidebar;
    sidebar.append(QUrl::fromLocalFile(QDir::homePath()));
    sidebar.append(QUrl::fromLocalFile(QDir::rootPath()));
    dialog.setSidebarUrls(sidebar);

    /* Get file name */
    if (dialog.exec() != QDialog::Accepted)
        return QFile::NoError;
    QSettings settings;
    m_workingDirectory = dialog.directory();
    settings.setValue(SETTINGS_WORKINGPATH, m_workingDirectory.absolutePath());

    fn = dialog.selectedFiles().first();
    if (fn.isEmpty() == true)
        return QFile::NoError;

    /* Clear existing document data */
    clearDocument();

    /* Set the workspace path before loading the new XML. In this way local files
       can be loaded even if the workspace file has been moved */
    m_doc->setWorkspacePath(QFileInfo(fn).absolutePath());

    /* Load the file */
    QFile::FileError error = loadXML(fn);
    if (handleFileError(error) == true)
        m_doc->resetModified();

    /* Update these in any case, since they are at least emptied now as
       a result of calling clearDocument() a few lines ago. */
    //if (FunctionManager::instance() != NULL)
    //    FunctionManager::instance()->updateTree();
    if (FixtureManager::instance() != NULL)
        FixtureManager::instance()->updateView();
    if (InputOutputManager::instance() != NULL)
        InputOutputManager::instance()->updateList();

    updateFileOpenMenu(fn);

    return error;
}

QFile::FileError App::slotFileSave()
{
    QFile::FileError error;

    /* Attempt to save with the existing name. Fall back to Save As. */
    if (fileName().isEmpty() == true)
        error = slotFileSaveAs();
    else
        error = saveXML(fileName());

    handleFileError(error);
    return error;
}

QFile::FileError App::slotFileSaveAs()
{
    QString fn;

    /* Create a file save dialog */
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Save Workspace As"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.selectFile(fileName());

    /* Append file filters to the dialog */
    QStringList filters;
    filters << tr("Workspaces (*%1)").arg(KExtWorkspace);
#if defined(WIN32) || defined(Q_OS_WIN)
    filters << tr("All Files (*.*)");
#else
    filters << tr("All Files (*)");
#endif
    dialog.setNameFilters(filters);

    /* Append useful URLs to the dialog */
    QList <QUrl> sidebar;
    sidebar.append(QUrl::fromLocalFile(QDir::homePath()));
    sidebar.append(QUrl::fromLocalFile(QDir::rootPath()));
    dialog.setSidebarUrls(sidebar);

    /* Get file name */
    if (dialog.exec() != QDialog::Accepted)
        return QFile::NoError;

    fn = dialog.selectedFiles().first();
    if (fn.isEmpty() == true)
        return QFile::NoError;

    /* Always use the workspace suffix */
    if (fn.right(4) != KExtWorkspace)
        fn += KExtWorkspace;

    /* Set the workspace path before saving the new XML. In this way local files
       can be loaded even if the workspace file will be moved */
    m_doc->setWorkspacePath(QFileInfo(fn).absolutePath());

    /* Save the document and set workspace name */
    QFile::FileError error = saveXML(fn);
    handleFileError(error);

    updateFileOpenMenu(fn);
    return error;
}

/*****************************************************************************
 * Control action slots
 *****************************************************************************/

void App::slotControlMonitor()
{
    Monitor::createAndShow(this, m_doc);
}

void App::slotAddressTool()
{
    AddressTool at(this);
    at.exec();
}

void App::slotControlBlackout()
{
    m_doc->inputOutputMap()->setBlackout(!m_doc->inputOutputMap()->blackout());
}

void App::slotBlackoutChanged(bool state)
{
    m_controlBlackoutAction->setChecked(state);
}

void App::slotControlPanic()
{
    m_doc->masterTimer()->stopAllFunctions();
}

void App::slotFadeAndStopAll()
{
    QAction *action = (QAction *)sender();
    int timeout = action->data().toInt();

    m_doc->masterTimer()->fadeAndStopAll(timeout);
}

void App::slotRunningFunctionsChanged()
{
    if (m_doc->masterTimer()->runningFunctions() > 0)
        m_controlPanicAction->setEnabled(true);
    else
        m_controlPanicAction->setEnabled(false);
}

void App::slotDumpDmxIntoFunction()
{
    DmxDumpFactory ddf(m_doc, m_dumpProperties, this);
    if (ddf.exec() != QDialog::Accepted)
        return;
}

void App::slotFunctionLiveEdit()
{
    FunctionSelection fs(this, m_doc);
    fs.setMultiSelection(false);
    fs.setFilter(Function::Scene | Function::Chaser | Function::EFX | Function::RGBMatrix);
    fs.disableFilters(Function::Show | Function::Script | Function::Collection | Function::Audio);

    if (fs.exec() == QDialog::Accepted)
    {
        if (fs.selection().count() > 0)
        {
            FunctionLiveEditDialog fle(m_doc, fs.selection().first(), this);
            fle.exec();
        }
    }
}

void App::slotLiveEditVirtualConsole()
{
    VirtualConsole::instance()->toggleLiveEdit();
}

void App::slotControlFullScreen()
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

        // In case slotControlFullScreen() is called programmatically (from main.cpp)
        if (m_controlFullScreenAction->isChecked() == false)
            m_controlFullScreenAction->setChecked(true);
    }
}

void App::slotControlFullScreen(bool usingGeometry)
{
    if (usingGeometry == true)
    {
        QDesktopWidget dw;
        setGeometry(dw.availableGeometry());
    }
    else
    {
        slotControlFullScreen();
    }
}

/*****************************************************************************
 * Help action slots
 *****************************************************************************/

void App::slotHelpIndex()
{
    DocBrowser::createAndShow(this);
}

void App::slotHelpAbout()
{
    AboutBox ab(this);
    ab.exec();
}

void App::slotRecentFileClicked(QAction *recent)
{
    if (recent == NULL)
        return;

    QString recentAbsPath = recent->text();
    QFile testFile(recentAbsPath);
    if (testFile.exists() == false)
    {
        QMessageBox::critical(this, tr("Error"),
                              tr("File not found !\nThe selected file has been moved or deleted."),
                              QMessageBox::Close);
        return;
    }

    /* Check that the user is aware of losing previous changes */
    QString msg(tr("Do you wish to save the current workspace?\n" \
                   "Changes will be lost if you don't save them."));
    if (saveModifiedDoc(tr("Open Workspace"), msg) == false)
    {
        /* Second thoughts... Cancel loading. */
        return;
    }

    m_workingDirectory = QFileInfo(recentAbsPath).absoluteDir();
    QSettings settings;
    settings.setValue(SETTINGS_WORKINGPATH, m_workingDirectory.absolutePath());

    /* Clear existing document data */
    clearDocument();

    /* Set the workspace path before loading the new XML. In this way local files
       can be loaded even if the workspace file has been moved */
    m_doc->setWorkspacePath(QFileInfo(recentAbsPath).absolutePath());

    /* Load the file */
    QFile::FileError error = loadXML(recentAbsPath);
    if (handleFileError(error) == true)
        m_doc->resetModified();

    /* Update these in any case, since they are at least emptied now as
       a result of calling clearDocument() a few lines ago. */
    //if (FunctionManager::instance() != NULL)
    //    FunctionManager::instance()->updateTree();
    if (FixtureManager::instance() != NULL)
        FixtureManager::instance()->updateView();
    if (InputOutputManager::instance() != NULL)
        InputOutputManager::instance()->updateList();

}

/*****************************************************************************
 * Load & Save
 *****************************************************************************/

void App::setFileName(const QString& fileName)
{
    m_fileName = fileName;
}

QString App::fileName() const
{
    return m_fileName;
}

QFile::FileError App::loadXML(const QString& fileName)
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

bool App::loadXML(const QDomDocument& doc, bool goToConsole, bool fromMemory)
{
    Q_ASSERT(m_doc != NULL);

    QDomElement root = doc.documentElement();
    if (root.tagName() != KXMLQLCWorkspace)
    {
        qWarning() << Q_FUNC_INFO << "Workspace node not found";
        return false;
    }

    QString activeWindowName = root.attribute(KXMLQLCWorkspaceWindow);

    QDomNode node = root.firstChild();
    while (node.isNull() == false)
    {
        QDomElement tag = node.toElement();

        if (tag.tagName() == KXMLQLCEngine)
        {
            m_doc->loadXML(tag);
        }
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
        else
        {
            qWarning() << Q_FUNC_INFO << "Unknown Workspace tag:" << tag.tagName();
        }

        node = node.nextSibling();
    }

    if (goToConsole == true)
        // Force the active window to be Virtual Console
        setActiveWindow(VirtualConsole::staticMetaObject.className());
    else
        // Set the active window to what was saved in the workspace file
        setActiveWindow(activeWindowName);

    // Perform post-load operations
    VirtualConsole::instance()->postLoad();

    if (m_doc->errorLog().isEmpty() == false &&
        fromMemory == false)
    {
        QMessageBox msg(QMessageBox::Warning, tr("Warning"),
                        tr("Some errors occurred while loading the project:") + "\n\n" + m_doc->errorLog(),
                        QMessageBox::Ok);
        msg.exec();
    }

    return true;
}

QFile::FileError App::saveXML(const QString& fileName)
{
    QFile::FileError retval;

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly) == false)
        return file.error();

    QDomDocument doc(QLCFile::getXMLHeader(KXMLQLCWorkspace));
    if (doc.isNull() == false)
    {
        QDomElement root;
        QDomElement tag;
        QDomText text;

        /* Create a text stream for the file */
        QTextStream stream(&file);
        stream.setAutoDetectUnicode(true);
        stream.setCodec("UTF-8");

        /* THE MASTER XML ROOT NODE */
        root = doc.documentElement();

        /* Currently active window */
        QWidget* widget = m_tab->currentWidget();
        if (widget != NULL)
            root.setAttribute(KXMLQLCWorkspaceWindow, widget->metaObject()->className());

        /* Write engine components to the XML document */
        m_doc->saveXML(&doc, &root);

        /* Write virtual console to the XML document */
        VirtualConsole::instance()->saveXML(&doc, &root);

        /* Write Simple Desk to the XML document */
        SimpleDesk::instance()->saveXML(&doc, &root);

        /* Write the XML document to the stream (=file) */
        stream << doc.toString() << "\n";

        /* Set the file name for the current Doc instance and
           set it also in an unmodified state. */
        setFileName(fileName);
        m_doc->resetModified();

        retval = QFile::NoError;
    }
    else
    {
        retval = QFile::ReadError;
    }

    return retval;
}

void App::slotLoadDocFromMemory(QString xmlData)
{
    if (xmlData.isEmpty())
        return;

    /* Clear existing document data */
    clearDocument();

    QDomDocument doc;
    doc.setContent(xmlData);
    loadXML(doc, true, true);
}

void App::slotSaveAutostart(QString fileName)
{
    /* Set the workspace path before saving the new XML. In this way local files
       can be loaded even if the workspace file will be moved */
    m_doc->setWorkspacePath(QFileInfo(fileName).absolutePath());

    /* Save the document and set workspace name */
    QFile::FileError error = saveXML(fileName);
    handleFileError(error);
}
