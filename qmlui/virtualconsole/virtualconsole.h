/*
  Q Light Controller Plus
  virtualconsole.h

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

#ifndef VIRTUALCONSOLE_H
#define VIRTUALCONSOLE_H

#include <QQuickView>
#include <QObject>
#include <QFont>
#include <QHash>

#include "previewcontext.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class QLCInputSource;
class ContextManager;
class VCWidget;
class VCFrame;
class VCPage;
class Doc;

#define KXMLQLCVirtualConsole "VirtualConsole"

class VirtualConsole : public PreviewContext
{
    Q_OBJECT

    Q_PROPERTY(int pagesCount READ pagesCount NOTIFY pagesCountChanged)
    Q_PROPERTY(int selectedPage READ selectedPage WRITE setSelectedPage NOTIFY selectedPageChanged)
    Q_PROPERTY(bool editMode READ editMode WRITE setEditMode NOTIFY editModeChanged)
    Q_PROPERTY(VCWidget *selectedWidget READ selectedWidget NOTIFY selectedWidgetChanged)
    Q_PROPERTY(int selectedWidgetsCount READ selectedWidgetsCount NOTIFY selectedWidgetsCountChanged)

public:
    VirtualConsole(QQuickView *view, Doc *doc, ContextManager *ctxManager, QObject *parent = 0);

    /** Return the number of pixels in 1mm */
    qreal pixelDensity() const;

    /** Reset the Virtual Console contents to an initial state */
    void resetContents();

    /** Set/Get the VC edit mode flag */
    bool editMode() const;
    void setEditMode(bool editMode);

signals:
    void editModeChanged(bool editMode);

protected:
    bool m_editMode;

    /** Reference to the Context Manager. Used to track VC pages as
     *  regular contexts */
    ContextManager *m_contextManager;

    /*********************************************************************
     * Pages
     *********************************************************************/
public:
    Q_INVOKABLE void renderPage(QQuickItem *parent, QQuickItem *contentItem, int page);

    /** Get the Virtual Console's frame representing the given $page,
     *  where all the widgets are placed */
    Q_INVOKABLE VCPage* page(int page) const;

    /** Return the reference of the currently selected VC page */
    Q_INVOKABLE QQuickItem *currentPageItem() const;

    /** Return a list with the VC page names */
    int pagesCount() const;

    /** Add a new VC page at $index */
    Q_INVOKABLE void addPage(int index);

    /** Delete a VC page at $index */
    void deletePage(int index);

    /** Set a protection PIN for the page at $index */
    Q_INVOKABLE bool setPagePIN(int index, QString currentPIN, QString newPIN);

    /** Validate a PIN for a VC Page. Returns true if the user entered the
     *  correct PIN, otherwise false is returned.
     *  The $remember flag is used to avoid requesting the PIN again
     *  for the entire session (on PIN check success) */
    Q_INVOKABLE bool validatePagePIN(int index, QString PIN, bool remember);

    /** Set/Get the currently selected VC page index */
    int selectedPage() const;
    void setSelectedPage(int selectedPage);

    /** Enable/disable the current page scroll interaction */
    Q_INVOKABLE void setPageInteraction(bool enable);

signals:
    /** Notify the listeners that the currenly selected VC page has changed */
    void selectedPageChanged(int selectedPage);

    /** Notify the listener that some page names have changed */
    void pagesCountChanged();

protected:
    /** A list of VCPage representing the main VC pages */
    QVector<VCPage*> m_pages;

    /** The index of the currently selected VC page */
    int m_selectedPage;

    /*********************************************************************
     * Widgets
     *********************************************************************/
public:
    /** Add $widget to the global VC widgets map */
    void addWidgetToMap(VCWidget* widget);

    /** Remove $widget from the global VC widgets map */
    void removeWidgetFromMap(VCWidget* widget);

    /** Return a reference to the VC widget with the specified $id.
     *  On invalid $id, NULL is returned */
    VCWidget *widget(quint32 id);

    Q_INVOKABLE void setWidgetSelection(quint32 wID, QQuickItem *item, bool enable, bool multi);

    /** Resets the currently selected widgets selection list */
    Q_INVOKABLE void resetWidgetSelection();

    /** Return a list of strings with the currently selected VC widget names */
    Q_INVOKABLE QStringList selectedWidgetNames();

    /** Return the number of currently selected VC widgets */
    int selectedWidgetsCount() const;

    /** Return a list of the currently selected VC widget IDs */
    Q_INVOKABLE QVariantList selectedWidgetIDs();

    Q_INVOKABLE void moveWidget(VCWidget *widget, VCFrame *targetFrame, QPoint pos);

    /** Helper methods to handle alignment, label, background/foreground colors,
     *  background image and font when multiple widgets are selected */
    Q_INVOKABLE void setWidgetsAlignment(VCWidget *refWidget, int alignment);
    Q_INVOKABLE void setWidgetsCaption(QString caption);
    Q_INVOKABLE void setWidgetsForegroundColor(QColor color);
    Q_INVOKABLE void setWidgetsBackgroundColor(QColor color);
    Q_INVOKABLE void setWidgetsBackgroundImage(QString path);
    Q_INVOKABLE void setWidgetsFont(QFont font);

    /** Delete the VC widgets with the IDs specified in $IDList */
    void deleteVCWidgets(QVariantList IDList);

    /** Return a reference to the currently selected VC widget */
    VCWidget *selectedWidget() const;

signals:
    /** Notify the listeners that the currenly selected VC widget has changed */
    void selectedWidgetChanged();

    void selectedWidgetsCountChanged();

protected:
    /** Create a new widget ID */
    quint32 newWidgetId();

protected:

    /** A map of all the VC widgets references with their IDs */
    QHash <quint32, VCWidget *> m_widgetsMap;

    /** Latest assigned widget ID */
    quint32 m_latestWidgetId;

    /*********************************************************************
     * Drag & Drop
     *********************************************************************/
public:
    /** Add or remove a target to the dropTargets list.
     *  This is used to handle the stacking order of highlight areas
     *  of frames when dragging/dropping a new widget on the VC */
    Q_INVOKABLE void setDropTarget(QQuickItem *target, bool enable);

    /** Reset the drop targets list.
     *  deleteTargets is true when a new widget is dropped, so
     *  drop areas highlight is no more needed */
    void resetDropTargets(bool deleteTargets);

protected:
    /** A list of the QML targets used for drag & drop of new widgets.
     *  Items are stacked in a precise order to handle the enter/exit events
     *  of a drag item and highlight only the last item entered */
    QList<QQuickItem *>m_dropTargets;

    /*********************************************************************
     * External input
     *********************************************************************/
public:
    /** Enable the autodetection process for an external controller.
     *  This method creates an empty QLCInputSource in the specified
     *  $widget and fills it later once the first input signal is received */
    Q_INVOKABLE bool createAndDetectInputSource(VCWidget *widget);

    /** Enable the autodetection process for a key sequence.
     *  This method creates an empty QKeySequence in the specified
     *  $widget and updates it later once the first key press is received */
    Q_INVOKABLE bool createAndDetectInputKey(VCWidget *widget);

    /** Enable the autodetection process for a specific input source
     *  bound to an external controller. */
    Q_INVOKABLE bool enableInputSourceAutoDetection(VCWidget *widget, quint32 id, quint32 universe, quint32 channel);

    /** Update the control ID of the specified $widget for a source coming
     *  from $universe and $channel */
    Q_INVOKABLE void updateInputSourceControlID(VCWidget *widget, quint32 id, quint32 universe, quint32 channel);

    /** Enable the autodetection process for a specific key sequence */
    Q_INVOKABLE bool enableKeyAutoDetection(VCWidget *widget, quint32 id, QString keyText);

    /** Update the control ID of a key sequence with $keyText for the specified $widget */
    Q_INVOKABLE void updateKeySequenceControlID(VCWidget *widget, quint32 id, QString keyText);

    /** Disable a previously started autodetection process */
    Q_INVOKABLE void disableAutoDetection();

    /** Delete an existing input source from the specified $widget.
     *  $type, $universe and $channel are also needed to remove the
     *  source from a VC Page multi hash map */
    Q_INVOKABLE void deleteInputSource(VCWidget *widget, quint32 id, quint32 universe, quint32 channel);

    /** Delete an existing key sequence from the specified $widget */
    Q_INVOKABLE void deleteKeySequence(VCWidget *widget, quint32 id, QString keyText);

    /** @reimp */
    void handleKeyEvent(QKeyEvent *e, bool pressed);

protected slots:
    /**
     * Slot that receives external input data from the InputOutputMap class.
     *
     * @param universe Input universe
     * @param channel Input channel
     * @param value New value for universe & value
     */
    void slotInputValueChanged(quint32 universe, quint32 channel, uchar value);

protected:
    /** Flag that indicates that an input source autodetection is running
     *  to properly behave when an input signal is received */
    bool m_inputDetectionEnabled;

    /** Temporary reference to a VC widget which is in the process
     *  of auto detecting an input source */
    VCWidget *m_autoDetectionWidget;

    QSharedPointer<QLCInputSource> m_autoDetectionSource;
    QKeySequence m_autoDetectionKey;
    quint32 m_autoDetectionKeyId;

    /*********************************************************************
     * Load & Save
     *********************************************************************/
public:
    /** Load properties and contents from an XML tree */
    bool loadXML(QXmlStreamReader &root);

    /** Load the Virtual Console global properties XML tree */
    bool loadPropertiesXML(QXmlStreamReader &root);

    /** Save properties and contents to an XML document */
    bool saveXML(QXmlStreamWriter *doc);

    /** Do post-load cleanup & checks */
    void postLoad();
};

#endif // VIRTUALCONSOLE_H
