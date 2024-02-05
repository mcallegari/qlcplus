/*
  Q Light Controller Plus
  previewcontext.h

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

#ifndef PREVIEWCONTEXT_H
#define PREVIEWCONTEXT_H

#include <QObject>
#include <QScreen>
#include <QQuickView>

class Doc;

class ContextQuickView : public QQuickView
{
    Q_OBJECT

public:
    ContextQuickView() { }
    ~ContextQuickView() { }

protected:
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

public slots:
    void slotScreenChanged(QScreen *screen);

signals:
    void keyPressed(QKeyEvent *e);
    void keyReleased(QKeyEvent *e);
};

class PreviewContext : public QObject
{
    Q_OBJECT

    Q_PROPERTY(quint32 universeFilter READ universeFilter WRITE setUniverseFilter NOTIFY universeFilterChanged)

public:
    explicit PreviewContext(QQuickView *view, Doc *doc, QString name, QObject *parent = 0);
    ~PreviewContext();

    /** Get/Set the QML resource URL for this context */
    virtual QString contextResource() const;
    virtual void setContextResource(QString res);

    virtual void enableContext(bool enable);

    virtual bool isEnabled();

    /** Get/Set the current universe filter */
    virtual quint32 universeFilter() const;
    virtual void setUniverseFilter(quint32 universeFilter);

    /** Get a handler for the associated QuickView */
    QQuickView *view();

    /** Get/Set the Quick item to access the context properties/objects */
    QQuickItem *contextItem();
    void setContextItem(QQuickItem *item);

    /** Get the context display name */
    QString name() const;

    /** Get/Set the title that will be displayed on a detached window title bar */
    QString contextTitle() const;
    void setContextTitle(QString title);

    /** Get/Set the context page. Used only by VC pages */
    int contextPage() const;
    void setContextPage(int page);

    /** Get/Set the detach state of this context */
    bool detached() const;
    void setDetached(bool detached);

    /** Virtual method to handle a key press event.
     *  Subclasses should reimplement this if interested in key events */
    virtual void handleKeyEvent(QKeyEvent *e, bool pressed);

public slots:
    virtual void slotRefreshView();

protected slots:
    void slotWindowClosing();

signals:
    void universeFilterChanged(quint32 universeFilter);
    void keyPressed(QKeyEvent *e);
    void keyReleased(QKeyEvent *e);

protected:
    /** Reference to the current view window.
     *  If the context is not detached, this is equal to $m_mainView,
     *  otherwise this is an indipendent view */
    QQuickView *m_view;

    /** Reference to the root QML view */
    QQuickView *m_mainView;

    /** Reference to the project workspace */
    Doc *m_doc;

    QQuickItem *m_contextItem;

    /** The context unique identifier string */
    QString m_name;

    /** The context title to be displayed on a detached window title bar */
    QString m_title;

    /** Optional page of the context. Used only by VC pages */
    int m_page;

    /** A string with the QML resource URL representing the context */
    QString m_resource;

    /** Flag that holds the enable status of the view.
     *  Enabled means visible on the screen */
    bool m_enabled;

    /** Flag that holds the detach status of the context */
    bool m_detached;

    /** The currently displayed universe
      * The value Universe::invalid() means "All universes" */
    quint32 m_universeFilter;

    /** Map of QLC+ objects with ID and QML items */
    QMap<quint32, QQuickItem*> m_itemsMap;
};

#endif // PREVIEWCONTEXT_H
