/*
  Q Light Controller Plus
  mainview3d.h

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

#ifndef MAINVIEW3D_H
#define MAINVIEW3D_H

#include <QObject>
#include <QQuickView>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DRender/QSceneLoader>

#include "previewcontext.h"

class Doc;
class Fixture;
class MonitorProperties;

using namespace Qt3DCore;

typedef struct
{
    /** Reference to the fixture root item, for hierarchy walk and function calls */
    QEntity *m_rootItem;
    /** Reference to the root item tranform component, to perform translations/rotations */
    Qt3DCore::QTransform *m_rootTransform;
    /** Reference to the arm entity used by moving heads */
    QEntity *m_armItem;
    /** Reference to the head entity used by moving heads */
    QEntity *m_headItem;

} FixtureMesh;

class MainView3D : public PreviewContext
{
    Q_OBJECT

public:
    explicit MainView3D(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~MainView3D();

    /** @reimp */
    void enableContext(bool enable);

    /** @reimp */
    void setUniverseFilter(quint32 universeFilter);

    void resetItems();

    void createFixtureItem(quint32 fxID, qreal x, qreal y, qreal z, bool mmCoords = true);

    void updateFixture(Fixture *fixture);

    void updateFixtureSelection(QList<quint32>fixtures);

    void updateFixtureSelection(quint32 fxID, bool enable);

    void updateFixturePosition(quint32 fxID, QVector3D pos);

    void updateFixtureRotation(quint32 fxID, QVector3D degrees);

    Q_INVOKABLE void createView();
    Q_INVOKABLE void initializeFixture(quint32 fxID, QComponent *picker, Qt3DRender::QSceneLoader *loader);

protected:
    /** First time 3D view variables initializations */
    void initialize3DProperties();

private:
    Qt3DCore::QTransform *getTransform(QEntity *entity);

protected slots:
    /** @reimp */
    void slotRefreshView();

private:
    MonitorProperties *m_monProps;

    /** Pre-cached QML component for quick item creation */
    QQmlComponent *fixtureComponent;

    /** Reference to the Scene3D component */
    QQuickItem *m_scene3D;

    /** Reference to the scene3D root entity for items creation */
    QEntity *m_rootEntity;

    /** Map of QLC+ fixture IDs and QML Entity items */
    QMap<quint32, FixtureMesh*> m_entitiesMap;
};

#endif // MAINVIEW3D_H
