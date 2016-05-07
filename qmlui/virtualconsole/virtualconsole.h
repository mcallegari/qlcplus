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
#include <QHash>

#include "previewcontext.h"

class QXmlStreamReader;
class QXmlStreamWriter;
class VCWidget;
class VCFrame;
class Doc;

#define KXMLQLCVirtualConsole "VirtualConsole"

class VirtualConsole : public PreviewContext
{
    Q_OBJECT

    Q_PROPERTY(int selectedPage READ selectedPage WRITE setSelectedPage NOTIFY selectedPageChanged)
    Q_PROPERTY(bool editMode READ editMode WRITE setEditMode NOTIFY editModeChanged)
    Q_PROPERTY(VCWidget *selectedWidget READ selectedWidget NOTIFY selectedWidgetChanged)

public:
    VirtualConsole(QQuickView *view, Doc *doc, QObject *parent = 0);

    Q_INVOKABLE void renderPage(QQuickItem *parent, QQuickItem *contentItem, int page);

    Q_INVOKABLE void setWidgetSelection(quint32 wID, QQuickItem *item, bool enable);

    Q_INVOKABLE void resetWidgetSelection();

    /*********************************************************************
     * Contents
     *********************************************************************/

public:
    /** Get the Virtual Console's frame representing the given $page,
     *  where all the widgets are placed */
    VCFrame* page(int page) const;

    /** Reset the Virtual Console contents to an initial state */
    void resetContents();

    void addWidgetToMap(VCWidget* widget);

    VCWidget *widget(quint32 id);

    //QList<VCWidget *> getChildren(VCWidget *obj);

    int selectedPage() const;

    void setSelectedPage(int selectedPage);

    /** Get resize mode flag */
    bool editMode() const;

    /** Set the VC in resize mode */
    void setEditMode(bool editMode);

    VCWidget *selectedWidget() const;

signals:
    void editModeChanged(bool editMode);

    void selectedWidgetChanged(VCWidget * selectedWidget);

    void selectedPageChanged(int selectedPage);

protected:
    /** Create a new widget ID */
    quint32 newWidgetId();

protected:
    /** A list of VCFrames representing the main VC pages */
    QVector<VCFrame*> m_pages;

    /** A map of all the VC widgets references with their IDs */
    QHash <quint32, VCWidget *> m_widgetsMap;

    /** Latest assigned widget ID */
    quint32 m_latestWidgetId;

    bool m_resizeMode;

    int m_selectedPage;

    VCWidget *m_selectedWidget;

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
