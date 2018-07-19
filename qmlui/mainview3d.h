/*
  Q Light Controller Plus
  mainview3d.h

  Copyright (c) Massimo Callegari
  Copyright (c) Eric Arnebäck

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
#include <Qt3DRender/QLayer>
#include <Qt3DRender/QEffect>
#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QSceneLoader>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QGeometryRenderer>
#include <Qt3DRender/QPaintedTextureImage>

#include "previewcontext.h"

class Doc;
class Fixture;
class MonitorProperties;

using namespace Qt3DCore;
using namespace Qt3DRender;

class GoboTextureImage : public Qt3DRender::QPaintedTextureImage
{
public:
    GoboTextureImage(int w, int h, QString filename);

    QString source() const;

    void setSource(QString filename);

protected:
    void paint(QPainter *painter);

    QString m_source;
};

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
    /** Reference to the layers used for scattering */
    QLayer *m_spotlightShadingLayer;
    QLayer *m_spotlightScatteringLayer;
    QLayer *m_outputDepthLayer;

    GoboTextureImage *m_goboTexture;
} SceneItem;

class MainView3D : public PreviewContext
{
    Q_OBJECT

    Q_PROPERTY(RenderQuality renderQuality READ renderQuality WRITE setRenderQuality NOTIFY renderQualityChanged)
    Q_PROPERTY(QString meshDirectory READ meshDirectory CONSTANT)
    Q_PROPERTY(QStringList stagesList READ stagesList CONSTANT)
    Q_PROPERTY(int stageIndex READ stageIndex WRITE setStageIndex NOTIFY stageIndexChanged)
    Q_PROPERTY(float ambientIntensity READ ambientIntensity WRITE setAmbientIntensity NOTIFY ambientIntensityChanged)
    Q_PROPERTY(float smokeAmount READ smokeAmount WRITE setSmokeAmount NOTIFY smokeAmountChanged)

    Q_PROPERTY(int genericSelectedCount READ genericSelectedCount NOTIFY genericSelectedCountChanged)
    Q_PROPERTY(QVector3D genericItemsPosition READ genericItemsPosition WRITE setGenericItemsPosition NOTIFY genericItemsPositionChanged)
    Q_PROPERTY(QVector3D genericItemsRotation READ genericItemsRotation WRITE setGenericItemsRotation NOTIFY genericItemsRotationChanged)
    Q_PROPERTY(QVector3D genericItemsScale READ genericItemsScale WRITE setGenericItemsScale NOTIFY genericItemsScaleChanged)

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
    /** Returns a string with the gobo location, cross platform */
    QString goboDirectory() const;

public slots:
    /** @reimp */
    void slotRefreshView();

private:
    /** Reference to the Doc Monitor properties */
    MonitorProperties *m_monProps;

    /** Pre-cached QML components for quick item creation */
    QQmlComponent *m_fixtureComponent;
    QQmlComponent *m_genericComponent;
    QQmlComponent *m_selectionComponent;
    QQmlComponent *m_spotlightConeComponent;
    QQmlComponent *m_fillGBufferLayer;

    /*********************************************************************
     * Fixtures
     *********************************************************************/
public:
    Q_INVOKABLE void sceneReady();
    Q_INVOKABLE void quadReady();
    Q_INVOKABLE void resetStage(QEntity *entity);

    void createFixtureItems(quint32 fxID, QVector3D pos, bool mmCoords = true);

    void createFixtureItem(quint32 fxID, quint16 headIndex, quint16 linkedIndex, QVector3D pos, bool mmCoords = true);

    /** Set/update the flags of a fixture item */
    void setFixtureFlags(quint32 itemID, quint32 flags);

    Q_INVOKABLE void initializeFixture(quint32 itemID, QEntity *fxEntity, QSceneLoader *loader);

    Q_INVOKABLE QString makeShader(QString str);

    /** Update the fixture preview items when some channels have changed */
    void updateFixture(Fixture *fixture);

    /** Update a single fixture item for a specific Fixture ID, head index and linked index */
    void updateFixtureItem(Fixture *fixture, quint16 headIndex, quint16 linkedIndex);

    /** Update the selection status of a list of Fixture item IDs */
    void updateFixtureSelection(QList<quint32>fixtures);

    /** Update the selection status of a Fixture with the provided $itemID */
    void updateFixtureSelection(quint32 itemID, bool enable);

    /** Update the position of a Fixture with the provided $itemID */
    void updateFixturePosition(quint32 itemID, QVector3D pos);

    /** Update the rotation of a Fixture with the provided $itemID */
    void updateFixtureRotation(quint32 itemID, QVector3D degrees);

    /** Update the scale of a Fixture with the provided $itemID */
    void updateFixtureScale(quint32 itemID, QVector3D origSize);

    /** Remove a Fixture item with the provided $itemID from the preview */
    void removeFixtureItem(quint32 itemID);

    /** Get the Fixture light 3D position for the provided $itemID */
    QVector3D lightPosition(quint32 itemID);

protected:
    /** First time 3D view variables initializations */
    void initialize3DProperties();

    /** Bounding box volume calculation methods */
    void getMeshCorners(QGeometryRenderer *mesh, QVector3D &minCorner, QVector3D &maxCorner);
    void addVolumes(SceneItem *meshRef, QVector3D minCorner, QVector3D maxCorner);

    /** Recursive method to get/set all the information of a scene */
    QEntity *inspectEntity(QEntity *entity, SceneItem *meshRef,
                           QLayer *layer, QEffect *effect,
                           bool calculateVolume, QVector3D translation);

private:
    Qt3DCore::QTransform *getTransform(QEntity *entity);
    QMaterial *getMaterial(QEntity *entity);
    unsigned int getNewLightIndex();
    void updateLightMatrix(SceneItem *mesh);

private:
    /** Reference to the Scene3D component */
    QQuickItem *m_scene3D;

    /** Reference to the scene root entity for items creation */
    QEntity *m_sceneRootEntity;

    /** Reference to the light pass entity and material for uniform updates */
    QEntity *m_quadEntity;

    /** Reference to the render targets used for scattering */
    QRenderTarget *m_gBuffer;
    QRenderTarget *m_frontDepthTarget;

    /** Map of QLC+ fixture IDs and QML Entity items */
    QMap<quint32, SceneItem*> m_entitiesMap;

    /** Cache of the loaded models against bounding volumes */
    QMap<QUrl, BoundingVolume> m_boundingVolumesMap;

    /*********************************************************************
     * Generic items
     *********************************************************************/
public:
    Q_INVOKABLE void createGenericItem(QString filename, int itemID);

    Q_INVOKABLE void initializeItem(int itemID, QEntity *fxEntity, QSceneLoader *loader);

    Q_INVOKABLE void setItemSelection(int itemID, bool enable, int keyModifiers);

    int genericSelectedCount() const;

    Q_INVOKABLE void removeSelectedGenericItems();

    void updateGenericItemPosition(quint32 itemID, QVector3D pos);
    QVector3D genericItemsPosition();
    void setGenericItemsPosition(QVector3D pos);

    void updateGenericItemRotation(quint32 itemID, QVector3D rot);
    QVector3D genericItemsRotation();
    void setGenericItemsRotation(QVector3D rot);

    void updateGenericItemScale(quint32 itemID, QVector3D scale);
    QVector3D genericItemsScale();
    void setGenericItemsScale(QVector3D scale);

signals:
    void genericSelectedCountChanged();
    void genericItemsPositionChanged();
    void genericItemsRotationChanged();
    void genericItemsScaleChanged();

private:
    /** Counter used to give unique IDs to generic items */
    int m_latestGenericID;

    QList<int> m_genericSelectedItems;

    /** Map of the generic items in the scene */
    QMap<int, SceneItem*> m_genericMap;

    /*********************************************************************
     * Environment
     *********************************************************************/
public:
    enum RenderQuality
    {
        LowQuality = 0,
        MediumQuality,
        HighQuality,
        UltraQuality
    };
    Q_ENUM(RenderQuality)

    enum FixtureMeshType
    {
        ParMeshType = 0,
        MovingHeadMeshType,
        DefaultMeshType
    };
    Q_ENUM(FixtureMeshType)

    /** Get/Set the 3D render quality. This affects shadows and
     *  scattering ray marching steps */
    RenderQuality renderQuality() const;
    void setRenderQuality(RenderQuality renderQuality);

    /** The list of currently supported stage types */
    QStringList stagesList() const;

    /** Get/Set the stage QML resource index to be loaded at runtime */
    int stageIndex() const;
    void setStageIndex(int stageIndex);

    /** Get/Set the ambient light intensity */
    float ambientIntensity() const;
    void setAmbientIntensity(float ambientIntensity);

    float smokeAmount() const;
    void setSmokeAmount(float smokeAmount);

protected:
    void createStage();

signals:
    void renderQualityChanged(RenderQuality renderQuality);
    void stageIndexChanged(int stageIndex);
    void ambientIntensityChanged(qreal ambientIntensity);
    void smokeAmountChanged(float smokeAmount);

private:
    RenderQuality m_renderQuality;

    QStringList m_stagesList;
    QStringList m_stageResourceList;

    /** Reference to the selected stage Entity */
    QEntity *m_stageEntity;

    /** Ambient light amount (0.0 - 1.0) */
    float m_ambientIntensity;

    /** Smoke amount (0.0 - 1.0) */
    float m_smokeAmount;
};

#endif // MAINVIEW3D_H
