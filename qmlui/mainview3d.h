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
#include <Qt3DRender/QGeometryRenderer>

#include "previewcontext.h"

class Doc;
class Fixture;
class MonitorProperties;

using namespace Qt3DCore;
using namespace Qt3DRender;

typedef struct
{
    QVector3D m_extents;
    QVector3D m_center;
} BoundingVolume;

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
    /** The bounding volume information */
    BoundingVolume m_volume;
    /** The selection box entity */
    QEntity *m_selectionBox;

} FixtureMesh;

class MainView3D : public PreviewContext
{
    Q_OBJECT

    Q_PROPERTY(QString meshDirectory READ meshDirectory CONSTANT)
    Q_PROPERTY(QStringList stagesList READ stagesList CONSTANT)
    Q_PROPERTY(int stageIndex READ stageIndex WRITE setStageIndex NOTIFY stageIndexChanged)
    Q_PROPERTY(float ambientIntensity READ ambientIntensity WRITE setAmbientIntensity NOTIFY ambientIntensityChanged)

public:
    explicit MainView3D(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~MainView3D();

    /** @reimp */
    void enableContext(bool enable);

    /** @reimp */
    void setUniverseFilter(quint32 universeFilter);

    void resetItems();

protected:
    /** Returns a string with the mesh location, suitable to be used by QML */
    QString meshDirectory() const;

protected slots:
    /** @reimp */
    void slotRefreshView();

private:
    /** Reference to the Doc Monitor properties */
    MonitorProperties *m_monProps;

    /** Pre-cached QML components for quick item creation */
    QQmlComponent *m_fixtureComponent;
    QQmlComponent *m_selectionComponent;

    /*********************************************************************
     * Fixtures
     *********************************************************************/
public:
    Q_INVOKABLE void sceneReady();
    Q_INVOKABLE void quadReady();
    Q_INVOKABLE void resetStage(QEntity *entity);

    void createFixtureItem(quint32 fxID, QVector3D pos, bool mmCoords = true);

    Q_INVOKABLE void initializeFixture(quint32 fxID, QEntity *fxEntity, QComponent *picker, QSceneLoader *loader);

    void updateFixture(Fixture *fixture);

    void updateFixtureSelection(QList<quint32>fixtures);

    void updateFixtureSelection(quint32 fxID, bool enable);

    void updateFixturePosition(quint32 fxID, QVector3D pos);

    void updateFixtureRotation(quint32 fxID, QVector3D degrees);

    void removeFixtureItem(quint32 fxID);

    QVector3D lightPosition(quint32 fixtureID);

protected:
    /** First time 3D view variables initializations */
    void initialize3DProperties();

    /** Bounding box volume calculation methods */
    void getMeshCorners(QGeometryRenderer *mesh, QVector3D &minCorner, QVector3D &maxCorner);
    void addVolumes(FixtureMesh *meshRef, QVector3D minCorner, QVector3D maxCorner);

    /** Recursive method to get/set all the information of a scene */
    QEntity *inspectEntity(QEntity *entity, FixtureMesh *meshRef,
                           QLayer *layer, QEffect *effect,
                           bool calculateVolume, QVector3D translation);

private:
    Qt3DCore::QTransform *getTransform(QEntity *entity);
    QMaterial *getMaterial(QEntity *entity);
    unsigned int getNewLightIndex();
    void updateLightPosition(FixtureMesh *meshRef);

protected slots:
    /** Helper method to create fixtures in the event loop thread */
    void slotCreateFixture(quint32 fxID);

private:
    /** Reference to the Scene3D component */
    QQuickItem *m_scene3D;

    /** Reference to the scene root entity for items creation */
    QEntity *m_sceneRootEntity;

    /** Reference to the light pass entity and material for uniform updates */
    QEntity *m_quadEntity;
    QMaterial *m_quadMaterial;

    /** Map of QLC+ fixture IDs and QML Entity items */
    QMap<quint32, FixtureMesh*> m_entitiesMap;

    /** Cache of the loaded models against bounding volumes */
    QMap<QUrl, BoundingVolume> m_boundingVolumesMap;

    /*********************************************************************
     * Environment
     *********************************************************************/
public:
    /** The list of currently supported stage types */
    QStringList stagesList() const;

    /** Get/Set the stage QML resource index to be loaded at runtime */
    int stageIndex() const;
    void setStageIndex(int stageIndex);

    /** Get/Set the ambient light intensity */
    float ambientIntensity() const;
    void setAmbientIntensity(float ambientIntensity);

signals:
    void stageIndexChanged(int stageIndex);
    void ambientIntensityChanged(qreal ambientIntensity);

private:
    QStringList m_stagesList;
    QStringList m_stageResourceList;
    int m_stageIndex;

    /** Reference to the selected stage Entity */
    QEntity *m_stageEntity;

    /** Ambient light amount (0.0 - 1.0) */
    float m_ambientIntensity;
};

#endif // MAINVIEW3D_H
