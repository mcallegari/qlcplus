/*
  Q Light Controller Plus
  fixturemanager.h

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

#ifndef FIXTUREMANAGER_H
#define FIXTUREMANAGER_H

#include <QQuickView>
#include <QObject>
#include <QList>

class Doc;

class FixtureManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int fixturesCount READ fixturesCount NOTIFY fixturesCountChanged)

public:
    explicit FixtureManager(QQuickView *view, Doc *doc, QObject *parent = 0);

    Q_INVOKABLE quint32 invalidFixture();
    Q_INVOKABLE quint32 fixtureForAddress(quint32 index);
    Q_INVOKABLE bool addFixture(QString manuf, QString model, QString mode, QString name,
                                int uniIdx, int address, int channels, int quantity, quint32 gap,
                                qreal xPos, qreal yPos);

    int fixturesCount();

signals:
    void docLoaded();
    void fixturesCountChanged();

public slots:


private:
    void createQMLFixture(quint32 fxID, qreal x, qreal y);

private:
    QQuickView *m_view;
    Doc *m_doc;

};

#endif // FIXTUREMANAGER_H
