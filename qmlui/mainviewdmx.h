/*
  Q Light Controller Plus
  mainviewdmx.h

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

#ifndef MAINVIEWDMX_H
#define MAINVIEWDMX_H

#include <QObject>
#include <QQuickView>

#include "previewcontext.h"

class Doc;
class Fixture;

class MainViewDMX : public PreviewContext
{
    Q_OBJECT
public:
    explicit MainViewDMX(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~MainViewDMX();

    /** @reimp */
    void enableContext(bool enable);

    /** @reimp */
    void setUniverseFilter(quint32 universeFilter);

    void reset();

    void createFixtureItem(quint32 fxID);

    void updateFixture(Fixture *fixture);

    void updateFixtureSelection(QList<quint32>fixtures);

    void updateFixtureSelection(quint32 fxID, bool enable);

protected:

signals:

protected slots:
    /** @reimp */
    void slotRefreshView();

private:
    /** Pre-cached QML component for quick item creation */
    QQmlComponent *fixtureComponent;
};

#endif // MAINVIEWDMX_H
