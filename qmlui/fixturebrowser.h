/*
  Q Light Controller Plus
  fixturebrowser.h

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

#ifndef FIXTUREBROWSER_H
#define FIXTUREBROWSER_H

#include <QQuickView>
#include <QDebug>

class QLCFixtureMode;
class QLCFixtureDef;
class Doc;

class FixtureBrowser : public QObject
{
    Q_OBJECT

public:
    FixtureBrowser(QQuickView *view, Doc *doc, QObject *parent = 0);

    Q_INVOKABLE QStringList manufacturers();
    Q_INVOKABLE int genericIndex();
    Q_INVOKABLE QStringList models(QString manufacturer);
    Q_INVOKABLE QStringList modes(QString manufacturer, QString model);
    Q_INVOKABLE int modeChannels(QString modeName);
    Q_INVOKABLE int availableChannel(int uniIdx, int channels, int quantity, int gap, int requested);

signals:
    void modeChanged();
    void modeChannelsChanged();

protected slots:

private:
    Doc *m_doc;
    QQuickView *m_view;
    QLCFixtureDef *m_definition;
};

#endif // FIXTUREBROWSER_H
