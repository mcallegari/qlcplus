/*
  Q Light Controller Plus
  videoprovider.h

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

#ifndef VIDEOPROVIDER_H
#define VIDEOPROVIDER_H

#include <QQuickView>
#include <QQuickItem>
#include <QMediaPlayer>

class Doc;

class VideoProvider: public QObject
{
    Q_OBJECT
public:
    VideoProvider(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~VideoProvider();

protected slots:
    void slotFunctionAdded(quint32 id);
    void slotFunctionRemoved(quint32 id);

private:
    /** Reference of the QML view */
    QQuickView *m_view;
    /** Reference of the project workspace */
    Doc *m_doc;

    QMap<quint32, QQuickItem *> m_videoMap;
};

#endif // VIDEOPROVIDER_H
