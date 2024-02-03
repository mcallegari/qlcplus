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
#include <QElapsedTimer>

#include <Qt3DCore/QEntity>
#include <Qt3DCore/QTransform>
#include <Qt3DLogic/QFrameAction>
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
class ListModel;
class QSvgRenderer;
class MonitorProperties;

using namespace Qt3DCore;
using namespace Qt3DRender;
using namespace Qt3DLogic;

class GoboTextureImage : public Qt3DRender::QPaintedTextureImage
{
public:
    GoboTextureImage(int w, int h, QString filename);

    /** Get/set the gobo source to use as texture */
    QString source() const;
    void setSource(QString filename);

protected:
    void paint(QPainter *painter);

private:
    QSvgRenderer *m_renderer;
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
    /** The bounding volume information */
    BoundingVolume m_volume;
    /** The selection box entity */
    QEntity *m_selectionBox;
    /** Reference to the texture used to render the
     *  currently selected gobo picture */
    GoboTextureImage *m_goboTexture;
} SceneItem;

class MainView3D : public PreviewContext
{
    Q_OBJECT

    Q_PROPERTY(QVector3D cameraPosition READ cameraPosition WRITE setCameraPosition NOTIFY cameraPositionChanged FINAL)
    Q_PROPERTY(QVector3D cameraUpVector READ cameraUpVector WRITE setCameraUpVector NOTIFY cameraUpVectorChanged FINAL)
    Q_PROPERTY(QVector3D cameraViewCenter READ cameraViewCenter WRITE setCameraViewCenter NOTIFY cameraViewCenterChanged FINAL)

    Q_PROPERTY(RenderQuality renderQuality READ renderQuality WRITE setRenderQuality NOTIFY renderQualityChanged)
    Q_PROPERTY(QString meshDirectory READ meshDirectory CONSTANT)
    Q_PROPERTY(QStringList stagesList READ stagesList CONSTANT)
    Q_PROPERTY(int stageIndex READ stageIndex WRITE setStageIndex NOTIFY stageIndexChanged)
    Q_PROPERTY(float ambientIntensity READ ambientIntensity WRITE setAmbientIntensity NOTIFY ambientIntensityChanged)
    Q_PROPERTY(float smokeAmount READ smokeAmount WRITE setSmokeAmount NOTIFY smokeAmountChanged)

    Q_PROPERTY(bool frameCountEnabled READ frameCountEnabled WRITE setFrameCountEnabled NOTIFY frameCountEnabledChanged)
    Q_PROPERTY(int FPS READ FPS NOTIFY FPSChanged)
    Q_PROPERTY(int minFPS READ minFPS NOTIFY minFPSChanged)
    Q_PROPERTY(int maxFPS READ maxFPS NOTIFY maxFPSChanged)
    Q_PROPERTY(float avgFPS READ avgFPS NOTIFY avgFPSChanged)

    Q_PROPERTY(QVariant genericItemsList READ genericItemsList NOTIFY genericItemsListChanged)
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

    /** Cleanup all the items in the scene */
    void resetItems();

    /** Reset the camera position to initial values */
    void resetCameraPosition();

    /** Get set the scene camera position */
    QVector3D cameraPosition() const;
    void setCameraPosition(const QVector3D &newCameraPosition);

    /** Get set the scene camera position */
    QVector3D cameraUpVector() const;
    void setCameraUpVector(const QVector3D &newCameraUpVector);

    /** Get set the scene camera position */
    QVector3D cameraViewCenter() const;
    void setCameraViewCenter(const QVector3D &newCameraViewCenter);

protected:
    /** Returns a string with the mesh location, suitable to be used by QML */
    QString meshDirectory() const;
    /** Returns a string with the gobo location, cross platform */
    QString goboDirectory() const;

public slots:
    /** @reimp */
    void slotRefreshView();

signals:
    void cameraPositionChanged();
    void cameraUpVectorChanged();
    void cameraViewCenterChanged();

private:
    /** Reference to the Doc Monitor properties */
    MonitorProperties *m_monProps;

    /** Pre-cached QML components for quick item creation */
    QQmlComponent *m_fixtureComponent;
    QQmlComponent *m_genericComponent;
    QQmlComponent *m_selectionComponent;
    QQmlComponent *m_spotlightConeComponent;
    QQmlComponent *m_fillGBufferLayer;
    int m_createItemCount;

    QVector3D m_cameraPosition;
    QVector3D m_cameraUpVector;
    QVector3D m_cameraViewCenter;

    /*********************************************************************
     * Frame counter
     *********************************************************************/
public:
    /** Enable/Disable a frame count signal */
    bool frameCountEnabled() const;
    void setFrameCountEnabled(bool enable);

    int FPS() const { return m_frameCount; }
    int minFPS() const { return m_minFrameCount; }
    int maxFPS() const { return m_maxFrameCount; }
    float avgFPS() const { return m_avgFrameCount; }

protected slots:
    void slotFrameProcessed();

signals:
    void frameCountEnabledChanged();
    void FPSChanged(int fps);
    void minFPSChanged(int fps);
    void maxFPSChanged(int fps);
    void avgFPSChanged(float fps);

private:
    QElapsedTimer m_fpsElapsed;
    QFrameAction *m_frameAction;
    int m_frameCount;
    int m_minFrameCount;
    int m_maxFrameCount;
    int m_avgFrameCount;

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
    void updateFixture(Fixture *fixture, QByteArray &previous);

    /** Update a single fixture item for a specific Fixture ID, head index and linked index */
    void updateFixtureItem(Fixture *fixture, quint16 headIndex, quint16 linkedIndex, QByteArray &previous);

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

    /** Get the Fixture light matrix for the provided $itemID */
    QMatrix4x4 lightMatrix(quint32 itemID);

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

    void walkNode(QNode *e, int depth);

private:
    Qt3DCore::QTransform *getTransform(QEntity *entity);
    QMaterial *getMaterial(QEntity *entity);
    void updateLightMatrix(SceneItem *mesh);

private:
    /** Reference to the Scene3D component */
    QQuickItem *m_scene3D;

    /** Reference to the entity containing everything */
    QEntity *m_scene3DEntity;

    /** Reference to the scene root entity for items creation */
    QEntity *m_sceneRootEntity;

    /** Reference to the light pass entity and material for uniform updates */
    QEntity *m_quadEntity;

    /** Reference to the render targets used for scattering */
    QRenderTarget *m_gBuffer;

    /** Map of QLC+ item IDs and SceneItem references */
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

    /** Get the number of generic items currently selected */
    int genericSelectedCount() const;

    /** Remove the currently selected generic items
     *  from the 3D scene */
    Q_INVOKABLE void removeSelectedGenericItems();

    /** Some generic items can be huge.
     *  Normalize them to be 2 meters big maximum */
    Q_INVOKABLE void normalizeSelectedGenericItems();

    /** Get a list of generic items currently in the 3D scene,
     *  to be displayed in QML */
    QVariant genericItemsList() const;

    void updateGenericItemPosition(quint32 itemID, QVector3D pos);
    QVector3D genericItemsPosition();
    void setGenericItemsPosition(QVector3D pos);

    void updateGenericItemRotation(quint32 itemID, QVector3D rot);
    QVector3D genericItemsRotation();
    void setGenericItemsRotation(QVector3D rot);

    void updateGenericItemScale(quint32 itemID, QVector3D scale);
    QVector3D genericItemsScale();
    void setGenericItemsScale(QVector3D scale);

protected:
    void updateGenericItemsList();

signals:
    void genericItemsListChanged();
    void genericSelectedCountChanged();
    void genericItemsPositionChanged();
    void genericItemsRotationChanged();
    void genericItemsScaleChanged();

private:
    /** Counter used to give unique IDs to generic items */
    int m_latestGenericID;

    /** QML model for generic items */
    ListModel *m_genericItemsList;

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
        NoMeshType = 0,
        ParMeshType,
        MovingHeadMeshType,
        ScannerMeshType,
        LEDBarMeshType,
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

    /** Get/Set the amount of smoke in the environment */
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
