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
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QEffect>

#include "previewcontext.h"

class Doc;
class Fixture;
class MonitorProperties;

using namespace Qt3DCore;
using namespace Qt3DRender;

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
    /** The attached light index */
    unsigned int m_lightIndex;
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

    Q_INVOKABLE void sceneReady(QEntity *sceneEntity);
    Q_INVOKABLE void quadReady(QMaterial *quadMaterial);

    void createFixtureItem(quint32 fxID, qreal x, qreal y, qreal z, bool mmCoords = true);

    Q_INVOKABLE void initializeFixture(quint32 fxID, QEntity *fxEntity, QComponent *picker,
                                       QSceneLoader *loader, QLayer *layer, QEffect *effect);

    void updateFixture(Fixture *fixture);

    void updateFixtureSelection(QList<quint32>fixtures);

    void updateFixtureSelection(quint32 fxID, bool enable);

    void updateFixturePosition(quint32 fxID, QVector3D pos);

    void updateFixtureRotation(quint32 fxID, QVector3D degrees);

protected:
    /** First time 3D view variables initializations */
    void initialize3DProperties();

private:
    Qt3DCore::QTransform *getTransform(QEntity *entity);
    QMaterial *getMaterial(QEntity *entity);
    unsigned int getNewLightIndex();

protected slots:
    /** @reimp */
    void slotRefreshView();

private:
    MonitorProperties *m_monProps;

    /** Pre-cached QML component for quick item creation */
    QQmlComponent *m_fixtureComponent;

    /** Reference to the Scene3D component */
    QQuickItem *m_scene3D;

    /** Reference to the scene root entity for items creation */
    QEntity *m_rootEntity;

    /** Reference to the light pass entity and material for uniform updates */
    QEntity *m_quadEntity;
    QMaterial *m_quadMaterial;

    /** Map of QLC+ fixture IDs and QML Entity items */
    QMap<quint32, FixtureMesh*> m_entitiesMap;
};

#endif // MAINVIEW3D_H
