/*
  Q Light Controller Plus
  app.h

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

#ifndef APP_H
#define APP_H

#include <QQmlEngine>
#include <QQuickView>
#include <QQuickItem>
#include <QObject>
#include "doc.h"

class MainView2D;
class ShowManager;
class ActionManager;
class FixtureBrowser;
class FixtureManager;
class ContextManager;
class VirtualConsole;
class FunctionManager;
class QXmlStreamReader;
class FixtureGroupEditor;
class InputOutputManager;
class ImportManager;
class NetworkManager;
class VideoProvider;
class Tardis;

#define KXMLQLCWorkspace "Workspace"

class App : public QQuickView
{
    Q_OBJECT
    Q_DISABLE_COPY(App)
    Q_PROPERTY(bool docLoaded READ docLoaded NOTIFY docLoadedChanged)
    Q_PROPERTY(bool docModified READ docModified NOTIFY docModifiedChanged)
    Q_PROPERTY(QStringList recentFiles READ recentFiles NOTIFY recentFilesChanged)
    Q_PROPERTY(QString workingPath READ workingPath WRITE setWorkingPath NOTIFY workingPathChanged)
    Q_PROPERTY(int accessMask READ accessMask WRITE setAccessMask NOTIFY accessMaskChanged)

    Q_PROPERTY(QString appName READ appName CONSTANT)
    Q_PROPERTY(QString appVersion READ appVersion CONSTANT)

public:
    App();
    ~App();

    QString appName() const;
    QString appVersion() const;

    enum MouseEvents
    {
        Pressed = 0,
        Released,
        Clicked,
        DoubleClicked,
        DragStarted,
        DragFinished,
        Checked
    };
    Q_ENUM(MouseEvents)

    enum DragItemTypes
    {
        GenericDragItem,
        FolderDragItem,
        FunctionDragItem,
        UniverseDragItem,
        FixtureGroupDragItem,
        FixtureDragItem,
        ChannelDragItem,
        HeadDragItem,
        WidgetDragItem
    };
    Q_ENUM(DragItemTypes)

    enum ChannelColors
    {
        Red     = (1 << 0),
        Green   = (1 << 1),
        Blue    = (1 << 2),
        Cyan    = (1 << 3),
        Magenta = (1 << 4),
        Yellow  = (1 << 5),
        White   = (1 << 6),
        Amber   = (1 << 7),
        UV      = (1 << 8),
        Lime    = (1 << 9),
        Indigo  = (1 << 10),
    };
    Q_ENUM(ChannelColors)

    enum AccessControl
    {
        AC_FixtureEditing  = (1 << 0),
        AC_FunctionEditing = (1 << 1),
        AC_VCControl       = (1 << 2),
        AC_VCEditing       = (1 << 3),
        AC_SimpleDesk      = (1 << 4),
        AC_ShowManager     = (1 << 5),
        AC_InputOutput     = (1 << 6)
    };
    Q_ENUM(AccessControl)

    /** Method to turn the key and start the engine */
    void startup();

    /** Toggle between windowed and fullscreeen mode */
    Q_INVOKABLE void toggleFullscreen();

    void enableKioskMode();
    void createKioskCloseButton(const QRect& rect);

    void show();

    /** Return the number of pixels in 1mm */
    qreal pixelDensity() const;

    /** Get/Set the UI access mask */
    int defaultMask() const;
    int accessMask() const;

    Q_INVOKABLE void exit();

public slots:
    void setAccessMask(int mask);

protected:
    void keyPressEvent(QKeyEvent * e) override;
    void keyReleaseEvent(QKeyEvent * e) override;
    bool event(QEvent *event) override;

protected slots:
    void slotScreenChanged(QScreen *screen);
    void slotClosing();
    void slotClientAccessRequest(QString name);
    void slotAccessMaskChanged(int mask);

signals:
    void accessMaskChanged(int mask);

private:
    /** The number of pixels in one millimiter */
    qreal m_pixelDensity;

    /** Bitmask to enable/disable UI functionalities */
    int m_accessMask;

    FixtureBrowser *m_fixtureBrowser;
    FixtureManager *m_fixtureManager;
    FixtureGroupEditor *m_fixtureGroupEditor;
    ContextManager *m_contextManager;
    FunctionManager *m_functionManager;
    InputOutputManager *m_ioManager;
    VirtualConsole *m_virtualConsole;
    ShowManager *m_showManager;
    ActionManager *m_actionManager;
    VideoProvider *m_videoProvider;
    NetworkManager *m_networkManager;
    Tardis *m_tardis;

    /*********************************************************************
     * Doc
     *********************************************************************/
public:
    void clearDocument();

    Doc *doc();

    bool docLoaded() { return m_docLoaded; }

    bool docModified() const;

private:
    void initDoc();

signals:
    void docLoadedChanged();
    void docModifiedChanged();

private:
    Doc *m_doc;
    bool m_docLoaded;

    /*********************************************************************
     * Printer
     *********************************************************************/
public:
    /** Send $item content to a printer */
    Q_INVOKABLE void printItem(QQuickItem *item);

protected slots:
    void slotItemReadyForPrinting();

private:
    QSharedPointer<QQuickItemGrabResult> m_printerImage;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /** Get/Set the name of the current workspace file */
    Q_INVOKABLE QString fileName() const;
    void setFileName(const QString& fileName);

    /** Return the list of the recently opened files */
    QStringList recentFiles() const;

    /** Get/Set the path currently used by QLC+ to access projects and resources */
    QString workingPath() const;
    void setWorkingPath(QString workingPath);

    /** Reset everything and start a new workspace */
    Q_INVOKABLE bool newWorkspace();

    /** Load the workspace with the given $fileName */
    Q_INVOKABLE bool loadWorkspace(const QString& fileName);

    /** Save the current workspace with the given $fileName */
    Q_INVOKABLE bool saveWorkspace(const QString& fileName);

    /** Start the import process for the workspace with the given $fileName */
    Q_INVOKABLE bool loadImportWorkspace(const QString& fileName);

    /** Cancel an ongoing import process started with loadImportWorkspace */
    Q_INVOKABLE void cancelImport();

    /** Perform the actual import of the selected items */
    Q_INVOKABLE void importFromWorkspace();

    /**
     * Load workspace contents from a XML file with the given name.
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
    bool loadXML(QXmlStreamReader &doc, bool goToConsole = false, bool fromMemory = false);

    /**
     * Save workspace contents to a file with the given name. Changes the
     * current workspace file name to the given fileName.
     *
     * @param fileName The name of the file to save to.
     * @return QFile::NoError if successful.
     */
    QFile::FileError saveXML(const QString& fileName);

private:
    /**
     * Update the list of the recently open files.
     * If filename is specified, it will be removed from the list
     * if present and added to the beginning of the list
     */
    void updateRecentFilesList(QString filename = QString());

signals:
    void recentFilesChanged();
    void workingPathChanged(QString workingPath);

public slots:
    void slotLoadDocFromMemory(QByteArray &xmlData);

private:
    QString m_fileName;
    QStringList m_recentFiles;
    QString m_workingPath;
    ImportManager *m_importManager;
};
#endif // APP_H
