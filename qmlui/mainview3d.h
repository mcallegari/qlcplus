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

class GoboTextureImage final : public Qt3DRender::QPaintedTextureImage
{
public:
    GoboTextureImage(int w, int h, const QString& filename);

    /** Get/set the gobo source to use as texture */
    QString source() const;
    void setSource(const QString& filename);

protected:
    void paint(QPainter *painter) override;

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
    /** Reference to the root item transform component, to perform translations/rotations */
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
    /** The scene generation this item was created in. Mesh loading is
     *  asynchronous, so a SceneLoader callback can arrive after the scene
     *  has been reset (e.g. on project load). Callbacks carrying a stale
     *  generation must be discarded, otherwise they would resurrect
     *  already deleted entities */
    quint32 m_generation;
} SceneItem;

class MainView3D final : public PreviewContext
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
    Q_PROPERTY(bool genericSelectedLocked READ genericSelectedLocked NOTIFY genericSelectedLockedChanged)
    Q_PROPERTY(QVector3D genericItemsPosition READ genericItemsPosition WRITE setGenericItemsPosition NOTIFY genericItemsPositionChanged)
    Q_PROPERTY(QVector3D genericItemsRotation READ genericItemsRotation WRITE setGenericItemsRotation NOTIFY genericItemsRotationChanged)
    Q_PROPERTY(QVector3D genericItemsScale READ genericItemsScale WRITE setGenericItemsScale NOTIFY genericItemsScaleChanged)

    Q_PROPERTY(QVector3D position3DMarker READ position3DMarker WRITE setPosition3DMarker NOTIFY position3DMarkerChanged)
    Q_PROPERTY(bool position3DMarkerVisible READ position3DMarkerVisible WRITE setPosition3DMarkerVisible NOTIFY position3DMarkerVisibleChanged)

public:
    explicit MainView3D(QQuickView *view, Doc *doc, QObject *parent = 0);
    ~MainView3D();

    /** @reimp */
    void enableContext(bool enable) override;

    /** @reimp */
    void setUniverseFilter(quint32 universeFilter) override;

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
    void slotRefreshView() override;

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
    QQmlComponent *m_markerComponent;
    QQmlComponent *m_spotlightConeComponent;
    QQmlComponent *m_fillGBufferLayer;
    int m_createItemCount;

    /** Incremented on every scene reset. Used to detect and drop
     *  asynchronous mesh loading callbacks belonging to a previous scene */
    quint32 m_sceneGeneration;

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

private:
    /** Apply the FPS counter enabled state to the running scene (attach/detach
     *  the QFrameAction, reset counters, notify QML). Unlike setFrameCountEnabled()
     *  this does not persist the value nor mark the project modified, so it is
     *  safe to call while loading a project. */
    void applyFrameCountEnabled(bool enable);

    /** Create the QFrameAction (if needed) and attach it to the current
     *  scene root entity. Called both when the user enables the FPS counter
     *  and whenever the 3D scene is (re)initialized, so the setting survives
     *  switching to another view and back. */
    void attachFrameAction();

    /** Detach and destroy the QFrameAction. Must be called before the scene
     *  root entity is torn down, otherwise Qt3D would delete the action
     *  (it gets reparented on addComponent) and leave a dangling pointer. */
    void detachFrameAction();

signals:
    void frameCountEnabledChanged();
    void FPSChanged(int fps);
    void minFPSChanged(int fps);
    void maxFPSChanged(int fps);
    void avgFPSChanged(float fps);

private:
    QElapsedTimer m_fpsElapsed;
    QFrameAction *m_frameAction;
    /** User-requested state of the FPS counter, kept independently from
     *  m_frameAction so it persists across 3D scene teardown/rebuild */
    bool m_frameCountEnabled;
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

    Q_INVOKABLE void initializeFixture(quint32 itemID, QEntity *fxEntity, const QSceneLoader *loader);

    Q_INVOKABLE QString makeShader(QString str);

    /** Update the fixture preview items when some channels have changed */
    void updateFixture(Fixture *fixture, QByteArray &previous);

    /** Update a single fixture item for a specific Fixture ID, head index and linked index */
    void updateFixtureItem(Fixture *fixture, quint16 headIndex, quint16 linkedIndex, const QByteArray &previous);

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

    /** Loaded mesh bounding-box extents (in metres) for the item with $itemID,
     *  or a zero vector if the item is not (yet) present in the 3D scene. */
    QVector3D fixtureExtents(quint32 itemID) const;

    /** Full path of the generic mesh this view uses to draw $fixture, or an
     *  empty string for fixture types drawn without a mesh (LED bars). The file
     *  name comes from FixtureUtils::fixtureLightResource(). */
    QString fixtureMeshPath(const Fixture *fixture) const;

    /** Bounding-box extents (metres) of the mesh file at $meshPath, parsed
     *  straight from the geometry. Results are cached per path. Returns a zero
     *  vector when the file cannot be read or holds no vertex data. */
    QVector3D meshFileExtents(const QString &meshPath) const;

    /** Half-thickness (metres) of the truss bars of the current stage.
     *
     *  Read from the live stage entity's `trussHalfSize` QML property, so it
     *  tracks whatever the stage model declares instead of being duplicated in
     *  C++. Returns 0 when the current stage has no trusses (or the 3D view has
     *  never been created), which callers must treat as "no truss to snap to".
     */
    Q_INVOKABLE qreal trussHalfSize() const;

    /** Vertical span (metres, monitor space) of the truss bars: the underside
     *  and the top face. The bars sit ABOVE the environment box, so the
     *  underside is the grid height and the top is one bar-thickness higher.
     *  Both are 0 when the current stage has no trusses. */
    void trussVerticalSpan(qreal &bottomY, qreal &topY) const;

    /** Size (metres) $fixture will actually be DRAWN at.
     *
     *  Mirrors what createFixtureItem() + updateFixtureScale() do: the generic
     *  per-type mesh is fitted into the fixture's declared physical box with a
     *  single uniform scale, so it keeps its aspect ratio. Callers that need to
     *  position a fixture against real geometry (the Stage Wizard snapping to a
     *  truss) must use this, not the declared size — the two differ whenever the
     *  mesh aspect does not match the declared box.
     *
     *  Falls back to the live scene item's extents when the mesh is already
     *  loaded, and to the declared physical size when there is no mesh at all. */
    Q_INVOKABLE QVector3D fixtureDrawnSize(quint32 fixtureID) const;

    /** Remove a Fixture item with the provided $itemID from the preview */
    void removeFixtureItem(quint32 itemID);

    /** Get the Fixture light 3D position for the provided $itemID */
    QVector3D lightPosition(quint32 itemID) const;

    /** Get the Fixture light matrix for the provided $itemID */
    QMatrix4x4 lightMatrix(quint32 itemID) const;

protected:
    /** First time 3D view variables initializations */
    bool initialize3DProperties();
    void scheduleInitializeRetry();

    /** Bounding box volume calculation methods */
    //void getMeshCorners(QGeometryRenderer *mesh, QVector3D &minCorner, QVector3D &maxCorner);
    void addVolumes(SceneItem *meshRef, QVector3D minCorner, QVector3D maxCorner);

    /** Recursive method to get/set all the information of a scene */
    QEntity *inspectEntity(QEntity *entity, SceneItem *meshRef,
                           QLayer *layer, QEffect *effect,
                           bool calculateVolume, QVector3D translation);

    void walkNode(QNode *e, int depth) const;

private:
    Qt3DCore::QTransform *getTransform(const QEntity *entity) const;
    QMaterial *getMaterial(const QEntity *entity) const;
    void updateLightMatrix(SceneItem *mesh, quint32 itemID);

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

    /** Bounding extents (metres) parsed from a mesh FILE, keyed by path. Filled
     *  lazily by meshFileExtents() so the geometry can be queried even when the
     *  3D view has never been shown and no scene item exists. */
    mutable QHash<QString, QVector3D> m_meshFileExtents;

    /*********************************************************************
     * Generic items
     *********************************************************************/
public:
    Q_INVOKABLE void createGenericItem(QString filename, int itemID);

    Q_INVOKABLE void initializeItem(int itemID, QEntity *fxEntity, QSceneLoader *loader);

    Q_INVOKABLE void setItemSelection(int itemID, bool enable, int keyModifiers);

    /** Select/deselect a generic item by its row $index in the items list model.
     *  Used to keep the 3D selection in sync with multi-row (range) selections
     *  performed on the QML list */
    Q_INVOKABLE void setItemSelectionByIndex(int index, bool enable, int keyModifiers);

    /** Get the number of generic items currently selected */
    int genericSelectedCount() const;

    /** Returns true if at least one of the currently selected
     *  generic items is locked */
    bool genericSelectedLocked() const;

    /** Lock/unlock the position of the currently selected generic items.
     *  If any selected item is unlocked, all get locked; otherwise all
     *  get unlocked */
    Q_INVOKABLE void toggleGenericItemsLock();

    /** Remove the currently selected generic items
     *  from the 3D scene */
    Q_INVOKABLE void removeSelectedGenericItems();

    /** Some generic items can be huge.
     *  Normalize them to be 2 meters big maximum */
    Q_INVOKABLE void normalizeSelectedGenericItems();

    /** Get a list of generic items currently in the 3D scene,
     *  to be displayed in QML */
    QVariant genericItemsList() const;

    void updateGenericItemPosition(quint32 itemID, QVector3D pos) const;
    QVector3D genericItemsPosition() const;
    void setGenericItemsPosition(QVector3D pos);

    void updateGenericItemRotation(quint32 itemID, QVector3D rot) const;
    QVector3D genericItemsRotation() const;
    void setGenericItemsRotation(QVector3D rot);

    void updateGenericItemScale(quint32 itemID, QVector3D scale) const;
    QVector3D genericItemsScale() const;
    void setGenericItemsScale(QVector3D scale);

    QVector3D position3DMarker() const;
    Q_INVOKABLE void setPosition3DMarker(QVector3D pos);
    bool position3DMarkerVisible() const;
    Q_INVOKABLE void setPosition3DMarkerVisible(bool visible);

protected:
    void updateGenericItemsList();

signals:
    void genericItemsListChanged();
    void genericSelectedCountChanged();
    void genericSelectedLockedChanged();
    void genericItemsPositionChanged();
    void genericItemsRotationChanged();
    void genericItemsScaleChanged();
    void position3DMarkerChanged();
    void position3DMarkerVisibleChanged();

private:
    /** Counter used to give unique IDs to generic items */
    int m_latestGenericID;
    int m_initRetryCount;

    /** QML model for generic items */
    ListModel *m_genericItemsList;

    QList<int> m_genericSelectedItems;

    /** Map of the generic items in the scene */
    QMap<quint32, SceneItem*> m_genericMap;

    QVector3D m_position3DMarker;
    bool m_position3DMarkerVisible;
    QEntity *m_markerEntity;

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
        StrobeMeshType,
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

    Q_INVOKABLE void pickEntity(const float &aspect, const QVector2D &ndcMousePos, int modifiers) const;

protected:
    void createStage();

    /** Re-apply the "Rendering" settings persisted in the project (MonitorProperties)
     *  to the running scene and notify the QML side. Called on project load /
     *  when the 3D view becomes visible. Does not mark the project modified. */
    void applyRenderSettings();
    QVector3D unprojectToWorld(const float &aspect, const QVector2D &ndcMousePos) const;
    bool rayIntersectsAABB(const QVector3D &rayOrigin, const QVector3D &rayDir,
                           const QVector3D &center, const QVector3D &extents, float &hitDistance) const;

    quint32 itemIntersection(const QVector3D &rayOrigin, const QVector3D &rayDir, const int &modifiers,
                             const QMap<quint32, SceneItem *> &map, bool generic) const;

signals:
    void renderQualityChanged(RenderQuality renderQuality);
    void stageIndexChanged(int stageIndex);
    void ambientIntensityChanged(qreal ambientIntensity);
    void smokeAmountChanged(float smokeAmount);

private:
    /* The "Rendering" settings (quality, ambient light, smoke, show FPS) are
       stored in the project through MonitorProperties (m_monProps), so they
       persist in the workspace file. The getters/setters below proxy to it,
       the same way stageIndex() does. */

    QStringList m_stagesList;
    QStringList m_stageResourceList;

    /** Reference to the selected stage Entity */
    QEntity *m_stageEntity;
};

#endif // MAINVIEW3D_H
