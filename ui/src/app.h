/*
  Q Light Controller
  app.h

  Copyright (c) Heikki Junnila

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

#ifndef APP_H
#define APP_H

#include <QMainWindow>
#include <QString>
#include <QList>
#include <QFile>

#include "dmxdumpfactoryproperties.h"
#include "qlcfixturedefcache.h"
#include "doc.h"

class QProgressDialog;
class QDomDocument;
class QDomElement;
class QMessageBox;
class QToolButton;
class QFileDialog;
class QTabWidget;
class WebAccess;
class QToolBar;
class QPixmap;
class QAction;
class QLabel;
class App;

#if QT_VERSION >= 0x050000
class VideoProvider;
#endif

/** @addtogroup ui UI
 * @{
 */

#define KXMLQLCWorkspace "Workspace"

class App : public QMainWindow
{
    Q_OBJECT
    Q_DISABLE_COPY(App)

    /*********************************************************************
     * Initialization
     *********************************************************************/
public:
    App();
    ~App();
    void startup();
    void enableOverscan();

private:
    void init();
    void closeEvent(QCloseEvent*);
    void setActiveWindow(const QString& name);

private:
    QTabWidget* m_tab;
    QDir m_workingDirectory;
    bool m_overscan;

    /*********************************************************************
     * Progress dialog
     *********************************************************************/
public:
    void createProgressDialog();
    void destroyProgressDialog();

public slots:
    void slotSetProgressText(const QString& text);

private:
    QProgressDialog* m_progressDialog;

    /*********************************************************************
     * Doc
     *********************************************************************/
public:
    void clearDocument();

    Doc *doc();

private slots:
    void slotDocModified(bool state);

private:
    void initDoc();

private:
    Doc* m_doc;

    /*********************************************************************
     * Main operating mode
     *********************************************************************/
public:
    void enableKioskMode();
    void createKioskCloseButton(const QRect& rect);

public slots:
    void slotModeOperate();
    void slotModeDesign();
    void slotModeToggle();
    void slotModeChanged(Doc::Mode mode);

    /*********************************************************************
     * Actions and toolbar
     *********************************************************************/
private:
    void initActions();
    void initToolBar();
    bool handleFileError(QFile::FileError error);
    bool saveModifiedDoc(const QString & title, const QString & message);

public slots:
    bool slotFileNew();
    QFile::FileError slotFileOpen();
    QFile::FileError slotFileSave();
    QFile::FileError slotFileSaveAs();

    void slotControlMonitor();
    void slotAddressTool();
    void slotControlFullScreen();
    void slotControlFullScreen(bool usingGeometry);
    void slotControlBlackout();
    void slotBlackoutChanged(bool state);
    void slotControlPanic();
    void slotFadeAndStopAll();
    void slotRunningFunctionsChanged();
    void slotDumpDmxIntoFunction();
    void slotFunctionLiveEdit();
    void slotLiveEditVirtualConsole();

    void slotHelpIndex();
    void slotHelpAbout();

    void slotRecentFileClicked(QAction *recent);

private:
    QAction* m_fileNewAction;
    QAction* m_fileOpenAction;
    QAction* m_fileSaveAction;
    QAction* m_fileSaveAsAction;

    QAction* m_modeToggleAction;
    QAction* m_controlMonitorAction;
    QAction* m_addressToolAction;
    QAction* m_controlFullScreenAction;
    QAction* m_controlBlackoutAction;
    QAction* m_controlPanicAction;
    QAction* m_dumpDmxAction;
    QAction* m_liveEditAction;
    QAction* m_liveEditVirtualConsoleAction;

    QAction* m_helpIndexAction;
    QAction* m_helpAboutAction;
    QMenu* m_fileOpenMenu;
    QMenu* m_fadeAndStopMenu;

private:
    QToolBar* m_toolbar;

    /*********************************************************************
     * Utilities
     *********************************************************************/
private:
    DmxDumpFactoryProperties *m_dumpProperties;
#if QT_VERSION >= 0x050000
    VideoProvider *m_videoProvider;
#endif

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /**
     * Set the name of the current workspace file
     */
    void setFileName(const QString& fileName);

    /**
     * Get the name of the current workspace file
     */
    QString fileName() const;

    /**
     * Update the recent file drop down menu
     */
    void updateFileOpenMenu(QString addRecent);

    /**
     * Load workspace contents from a file with the given name.
     *
     * @param fileName The name of the file to load from.
     * @return QFile::NoError if successful.
     */
    QFile::FileError loadXML(const QString& fileName);

    /**
     * Load workspace contents from the given XML document.
     *
     * @param doc The XML document to load from.
     */
    bool loadXML(const QDomDocument& doc, bool goToConsole = false, bool fromMemory = false);

    /**
     * Save workspace contents to a file with the given name. Changes the
     * current workspace file name to the given fileName.
     *
     * @param fileName The name of the file to save to.
     * @return QFile::NoError if successful.
     */
    QFile::FileError saveXML(const QString& fileName);

public slots:
    void slotLoadDocFromMemory(QString xmlData);

    void slotSaveAutostart(QString fileName);

private:
    QString m_fileName;
};

/** @} */

#endif
