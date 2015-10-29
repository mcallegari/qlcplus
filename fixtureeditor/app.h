/*
  Q Light Controller - Fixture Definition Editor
  app.h

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

#ifndef APP_H
#define APP_H

#include <QMainWindow>
#include <QDir>

class QLCFixtureDef;
class QLCChannel;
class QToolBar;
class QAction;
class QMenu;

/** @addtogroup fixtureeditor Fixture Editor
 * @{
 */

#define KAllFilter  "Fixture Definitions (*.qxf *.d4)"
#define KQXFFilter  "QLC fixtures (*.qxf)"
#define KD4Filter   "Avolites fixtures (*.d4)"

class App : public QMainWindow
{
    Q_OBJECT

public:
    App(QWidget* parent = 0);
    ~App();

    static QString longName();
    static QString version();

    /**
     * Load a fixture definition from the given path and open it in
     * a new editor window.
     *
     * @param path The file path to open
     */
    void loadFixtureDefinition(const QString& path);

private:
    /**
     * Load a native QLC fixture definition from $path.
     *
     * @param path The path to a .qxf file to load
     * @param errorMsg An optional error message
     * @return A newly-created QLCFixtureDef if successful, otherwise NULL.
     *         Ownership of the pointer is transferred to the caller.
     */
    QLCFixtureDef* loadQXF(const QString& path, QString& errorMsg) const;

    /**
     * Load an Avolites Diamond 4 fixture definition from $path.
     *
     * @param path The path to a .d4 file to load
     * @param errorMsg An optional error message
     * @return A newly-created QLCFixtureDef if successful, otherwise NULL.
     *         Ownership of the pointer is transferred to the caller.
     */
    QLCFixtureDef* loadD4(const QString& path, QString& errorMsg) const;

protected:
    void closeEvent(QCloseEvent*);

    /*********************************************************************
     * Copy channel
     *********************************************************************/
public:
    void setCopyChannel(QLCChannel* ch);
    QLCChannel* copyChannel() const;

signals:
    /** Signal telling that the contents of the clipboard have changed */
    void clipboardChanged();

protected:
    QLCChannel* m_copyChannel;
    QDir m_workingDirectory;

    /*********************************************************************
     * Actions, toolbar & menubar
     *********************************************************************/
protected:
    void initActions();
    void initMenuBar();
    void initToolBar();

protected:
    QAction* m_fileNewAction;
    QAction* m_fileOpenAction;
    QAction* m_fileSaveAction;
    QAction* m_fileSaveAsAction;
    QAction* m_fileQuitAction;

    QAction* m_helpIndexAction;
    QAction* m_helpAboutAction;
    QAction* m_helpAboutQtAction;

protected:
    QMenu* m_fileMenu;
    QMenu* m_helpMenu;
    QToolBar* m_toolBar;

protected slots:
    void slotFileNew();
    void slotFileOpen();
    void slotFileSave();
    void slotFileSaveAs();
    void slotFileQuit();

    void slotHelpIndex();
    void slotHelpAbout();
    void slotHelpAboutQt();
};

/** @} */

#endif
