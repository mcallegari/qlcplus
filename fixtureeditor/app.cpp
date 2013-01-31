/*
  Q Light Controller - Fixture Definition Editor
  app.cpp

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

#include <QDesktopWidget>
#include <QMdiSubWindow>
#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QWidgetList>
#include <QMessageBox>
#include <QSettings>
#include <QMdiArea>
#include <QMenuBar>
#include <QToolBar>
#include <QToolTip>
#include <QAction>
#include <QLabel>
#include <QColor>
#include <QDebug>
#include <QMenu>
#include <QIcon>
#include <QUrl>

#include "qlcfixturedefcache.h"
#include "qlcfixturedef.h"
#include "qlcchannel.h"
#include "qlcconfig.h"
#include "qlcfile.h"
#include "avolitesd4parser.h"

#include "app.h"
#include "aboutbox.h"
#include "docbrowser.h"
#include "fixtureeditor.h"

#define SETTINGS_GEOMETRY "workspace/geometry"
#define SETTINGS_OPENDIALOGSTATE "workspace/opendialog"

App* _app;

App::App(QWidget* parent) : QMainWindow(parent)
{
    _app = this;

    m_fileMenu = NULL;
    m_helpMenu = NULL;
    m_toolBar = NULL;

    m_copyChannel = NULL;

    setWindowTitle(App::longName());
    setWindowIcon(QIcon(":/qlcplus-fixtureeditor.png"));
    setCentralWidget(new QMdiArea(this));

    QCoreApplication::setOrganizationName("qlcplus");
    QCoreApplication::setOrganizationDomain("sf.net");
    QCoreApplication::setApplicationName(FXEDNAME);

    initActions();
    initMenuBar();
    initToolBar();

    QSettings settings;
    QVariant var = settings.value(SETTINGS_GEOMETRY);
    if (var.isValid() == true)
        restoreGeometry(var.toByteArray());

    this->raise();
}

App::~App()
{
    QSettings settings;
    settings.setValue(SETTINGS_GEOMETRY, saveGeometry());

    setCopyChannel(NULL);

    // Remove the reference to the application
    _app = NULL;
}

QString App::longName()
{
    return QString("%1 - %2").arg(APPNAME).arg(FXEDNAME);
}

QString App::version()
{
    return QString("Version %1").arg(APPVERSION);
}

void App::loadFixtureDefinition(const QString& path)
{
    QLCFixtureDef* fixtureDef = NULL;

    /* Attempt to create a fixture definition from the selected file */
    QString error(tr("Unrecognized file extension: %1").arg(path));
    if (path.toLower().endsWith(KExtFixture) == true)
        fixtureDef = loadQXF(path, error);
    else if (path.toLower().endsWith(KExtAvolitesFixture) == true)
        fixtureDef = loadD4(path, error);
    else
        fixtureDef = NULL;

    if (fixtureDef != NULL)
    {
        /* Create a new sub window and put a fixture editor widget
           in that sub window with the newly-created fixture def */
        QMdiSubWindow* sub = new QMdiSubWindow(centralWidget());
        QLCFixtureEditor* editor = new QLCFixtureEditor(sub, fixtureDef, path);

        sub->setWidget(editor);
        sub->setAttribute(Qt::WA_DeleteOnClose);
        qobject_cast<QMdiArea*> (centralWidget())->addSubWindow(sub);

        editor->show();
        sub->show();
    }
    else
    {
        QMessageBox::warning(this, tr("Fixture loading failed"),
                             tr("Unable to load fixture definition: ") + error);
    }
}

QLCFixtureDef* App::loadQXF(const QString& path, QString& errorMsg) const
{
    QLCFixtureDef* fixtureDef = new QLCFixtureDef;
    Q_ASSERT(fixtureDef != NULL);

    QFile::FileError error = fixtureDef->loadXML(path);
    if (error != QFile::NoError)
    {
        delete fixtureDef;
        fixtureDef = NULL;
        errorMsg = QLCFile::errorString(error);
    }

    return fixtureDef;
}

QLCFixtureDef* App::loadD4(const QString& path, QString& errorMsg) const
{
    QLCFixtureDef* fixtureDef = NULL;

    AvolitesD4Parser parser;
    if (parser.loadXML(path) == false)
    {
        errorMsg = parser.lastError();
    }
    else
    {
        fixtureDef = new QLCFixtureDef;
        Q_ASSERT(fixtureDef != NULL);
        if (parser.fillFixtureDef(fixtureDef) == false)
        {
            errorMsg = parser.lastError();
            delete fixtureDef;
            fixtureDef = NULL;
        }
    }

    return fixtureDef;
}

void App::closeEvent(QCloseEvent* e)
{
    /* Accept the close event by default */
    e->accept();

    QListIterator <QMdiSubWindow*> it(
        qobject_cast<QMdiArea*> (centralWidget())->subWindowList());
    while (it.hasNext() == true)
    {
        QLCFixtureEditor* editor;
        QMdiSubWindow* sub;

        sub = it.next();
        Q_ASSERT(sub != NULL);

        editor = static_cast<QLCFixtureEditor*> (sub->widget());
        Q_ASSERT(editor != NULL);

        editor->show();
        editor->setFocus();

        if (editor->close() == false)
        {
            /* Ignore the close event if just one editor refuses */
            e->ignore();
            break;
        }
    }
}

/*****************************************************************************
 * Copy channel
 *****************************************************************************/

void App::setCopyChannel(QLCChannel* ch)
{
    if (m_copyChannel != NULL)
        delete m_copyChannel;
    m_copyChannel = NULL;

    if (ch != NULL)
        m_copyChannel = new QLCChannel(ch);

    emit clipboardChanged();
}

QLCChannel* App::copyChannel() const
{
    return m_copyChannel;
}

/*****************************************************************************
 * Actions, toolbar & menubar
 *****************************************************************************/

void App::initActions()
{
    /* File actions */
    m_fileNewAction = new QAction(QIcon(":/filenew.png"),
                                  tr("&New"), this);
    m_fileNewAction->setShortcut(QKeySequence(tr("CTRL+N", "File|New")));
    connect(m_fileNewAction, SIGNAL(triggered(bool)),
            this, SLOT(slotFileNew()));

    m_fileOpenAction = new QAction(QIcon(":/fileopen.png"),
                                   tr("&Open"), this);
    m_fileOpenAction->setShortcut(QKeySequence(tr("CTRL+O", "File|Open")));
    connect(m_fileOpenAction, SIGNAL(triggered(bool)),
            this, SLOT(slotFileOpen()));

    m_fileSaveAction = new QAction(QIcon(":/filesave.png"),
                                   tr("&Save"), this);
    m_fileSaveAction->setShortcut(QKeySequence(tr("CTRL+S", "File|Save")));
    connect(m_fileSaveAction, SIGNAL(triggered(bool)),
            this, SLOT(slotFileSave()));

    m_fileSaveAsAction = new QAction(tr("Save &As..."), this);
    m_fileSaveAsAction->setShortcut(QKeySequence(tr("CTRL+SHIFT+S", "File|Save As...")));
    connect(m_fileSaveAsAction, SIGNAL(triggered(bool)),
            this, SLOT(slotFileSaveAs()));

    m_fileQuitAction = new QAction(QIcon(":/exit.png"),
                                   tr("&Quit"), this);
    m_fileQuitAction->setShortcut(QKeySequence(tr("CTRL+Q", "File|Quit")));
    connect(m_fileQuitAction, SIGNAL(triggered(bool)),
            this, SLOT(slotFileQuit()));

    /* Help actions */
    m_helpIndexAction = new QAction(QIcon(":/help.png"),
                                    tr("Index"), this);
    m_helpIndexAction->setShortcut(QKeySequence(tr("SHIFT+F1", "Help|Index")));
    connect(m_helpIndexAction, SIGNAL(triggered(bool)),
            this, SLOT(slotHelpIndex()));

    m_helpAboutAction = new QAction(QIcon(":/qlcplus.png"),
                                    tr("About Fixture Definition Editor..."), this);
    connect(m_helpAboutAction, SIGNAL(triggered(bool)),
            this, SLOT(slotHelpAbout()));

    m_helpAboutQtAction = new QAction(QIcon(":/qt.png"),
                                      tr("About Qt..."), this);
    connect(m_helpAboutQtAction, SIGNAL(triggered(bool)),
            this, SLOT(slotHelpAboutQt()));
}

void App::initToolBar()
{
    m_toolBar = new QToolBar(App::longName(), this);
    addToolBar(m_toolBar);
    m_toolBar->setMovable(false);

    m_toolBar->addAction(m_fileNewAction);
    m_toolBar->addAction(m_fileOpenAction);
    m_toolBar->addAction(m_fileSaveAction);
}

void App::initMenuBar()
{
    /* File Menu */
    m_fileMenu = new QMenu(menuBar());
    m_fileMenu->setTitle(tr("&File"));
    m_fileMenu->addAction(m_fileNewAction);
    m_fileMenu->addAction(m_fileOpenAction);
    m_fileMenu->addAction(m_fileSaveAction);
    m_fileMenu->addAction(m_fileSaveAsAction);
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_fileQuitAction);

    /* Help menu */
    m_helpMenu = new QMenu(menuBar());
    m_helpMenu->setTitle(tr("&Help"));
    m_helpMenu->addAction(m_helpIndexAction);
    m_helpMenu->addSeparator();
    m_helpMenu->addAction(m_helpAboutAction);
    m_helpMenu->addAction(m_helpAboutQtAction);

    menuBar()->addMenu(m_fileMenu);
    menuBar()->addMenu(m_helpMenu);
}

/*****************************************************************************
 * File action slots
 *****************************************************************************/

void App::slotFileNew()
{
    QLCFixtureEditor* editor;
    QMdiSubWindow* sub;

    sub = new QMdiSubWindow(centralWidget());
    editor = new QLCFixtureEditor(sub, new QLCFixtureDef());

    sub->setWidget(editor);
    sub->setAttribute(Qt::WA_DeleteOnClose);
    sub->setWindowIcon(QIcon(":/fixture.png"));

    qobject_cast<QMdiArea*> (centralWidget())->addSubWindow(sub);

    editor->show();
    sub->show();
}

void App::slotFileOpen()
{
    QSettings settings;

    /* Create a file open dialog */
    QFileDialog dialog(this);
    dialog.setWindowTitle(tr("Open a fixture definition"));
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setAcceptMode(QFileDialog::AcceptOpen);

    QStringList filters;
    filters << KAllFilter;
    filters << KQXFFilter;
    filters << KD4Filter;
    dialog.setNameFilters(filters);

    QVariant var = settings.value(SETTINGS_OPENDIALOGSTATE);
    if (var.isValid() == true)
    {
        dialog.restoreState(var.toByteArray());
    }
    else
    {
        QDir dir = QLCFixtureDefCache::userDefinitionDirectory();
        dialog.setDirectory(dir);
    }

    /* Execute the dialog */
    if (dialog.exec() != QDialog::Accepted)
        return;

    /* Get a file name */
    QStringListIterator it(dialog.selectedFiles());
    while (it.hasNext() == true)
        loadFixtureDefinition(it.next());

    settings.setValue(SETTINGS_OPENDIALOGSTATE, dialog.saveState());
}

void App::slotFileSave()
{
    QLCFixtureEditor* editor;
    QMdiSubWindow* sub;

    sub = (qobject_cast<QMdiArea*> (centralWidget()))->activeSubWindow();
    if (sub == NULL)
        return;

    editor = static_cast<QLCFixtureEditor*> (sub->widget());
    if (editor == NULL)
        return;

    editor->save();
}

void App::slotFileSaveAs()
{
    QLCFixtureEditor* editor;
    QMdiSubWindow* sub;

    sub = (qobject_cast<QMdiArea*> (centralWidget()))->activeSubWindow();
    if (sub == NULL)
        return;

    editor = static_cast<QLCFixtureEditor*> (sub->widget());
    if (editor == NULL)
        return;

    editor->saveAs();
}

void App::slotFileQuit()
{
    close();
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
    AboutBox aboutbox(this);
    aboutbox.exec();
}

void App::slotHelpAboutQt()
{
    QMessageBox::aboutQt(this, App::longName());
}
