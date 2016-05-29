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
#include <QQuickView>

class Doc;

class PreviewContext : public QObject
{
    Q_OBJECT
public:
    explicit PreviewContext(QQuickView *view, Doc *doc, QObject *parent = 0);

    virtual void enableContext(bool enable);

    virtual bool isEnabled();

    virtual void setUniverseFilter(quint32 universeFilter);

    QQuickView *view();

signals:

public slots:

protected:
    /** Reference to the root QML view */
    QQuickView *m_view;

    /** Reference to the project workspace */
    Doc *m_doc;

    /** Flag that holds the enable status of the view.
     *  Enabled means visible on the screen */
    bool m_enabled;

    /** The currently displayed universe
      * The value Universe::invalid() means "All universes" */
    quint32 m_universeFilter;

    /** Map of QLC+ objects with ID and QML items */
    QMap<quint32, QQuickItem*> m_itemsMap;
};

#endif // PREVIEWCONTEXT_H
