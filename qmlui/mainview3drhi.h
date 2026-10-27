/*
  Q Light Controller Plus
  mainview3drhi.h

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

#ifndef MAINVIEW3DRHI_H
#define MAINVIEW3DRHI_H

#include <QColor>
#include <QElapsedTimer>
#include <QHash>
#include <QObject>
#include <QQuickView>
#include <QVector3D>
#include <QMatrix4x4>
#include <QStringList>
#include <QList>
#include <QVariant>

#include "previewcontext.h"

class Doc;
class Fixture;
class ListModel;
class QTimer;
class MonitorProperties;

class MainView3D final : public PreviewContext
{
    Q_OBJECT

    Q_PROPERTY(QVector3D cameraPosition READ cameraPosition WRITE setCameraPosition NOTIFY cameraPositionChanged FINAL)
    Q_PROPERTY(QVector3D cameraViewCenter READ cameraViewCenter WRITE setCameraViewCenter NOTIFY cameraViewCenterChanged FINAL)

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
    Q_PROPERTY(QString genericItemsName READ genericItemsName WRITE setGenericItemsName NOTIFY genericItemsNameChanged)
    Q_PROPERTY(QColor genericItemsColor READ genericItemsColor WRITE setGenericItemsColor NOTIFY genericItemsColorChanged)

public:
    explicit MainView3D(QQuickView *view, Doc *doc, QObject *parent = nullptr);
    ~MainView3D() override;

    void enableContext(bool enable) override;
    void setUniverseFilter(quint32 universeFilter) override;

    void resetItems();
    void resetCameraPosition();

    QVector3D cameraPosition() const;
    void setCameraPosition(const QVector3D &newCameraPosition);

    QVector3D cameraViewCenter() const;
    void setCameraViewCenter(const QVector3D &newCameraViewCenter);

    bool frameCountEnabled() const;
    void setFrameCountEnabled(bool enable);

    int FPS() const { return m_frameCount; }
    int minFPS() const { return m_minFrameCount; }
    int maxFPS() const { return m_maxFrameCount; }
    float avgFPS() const { return m_avgFrameCount; }

    void createFixtureItems(quint32 fxID, QVector3D pos, bool mmCoords = true);
    void createFixtureItem(quint32 fxID, quint16 headIndex, quint16 linkedIndex, QVector3D pos,
                           bool mmCoords = true);
    void setFixtureFlags(quint32 itemID, quint32 flags);

    void updateFixture(Fixture *fixture, QByteArray &previous);
    void updateFixtureItem(Fixture *fixture, quint16 headIndex, quint16 linkedIndex,
                           const QByteArray &previous);

    void updateFixtureSelection(QList<quint32> fixtures);
    void updateFixtureSelection(quint32 itemID, bool enable);
    void updateFixturePosition(quint32 itemID, QVector3D pos);
    void updateFixtureRotation(quint32 itemID, QVector3D degrees);

    /** Bounding-box extents (metres) of the item with $itemID as drawn,
     *  or a zero vector if the item is not present in the 3D scene */
    QVector3D fixtureExtents(quint32 itemID) const;

    /** Full path of the mesh this view uses to draw $fixture, or an empty
     *  string for fixture types drawn without a mesh (LED bars, strobes,
     *  multi-head color changers) */
    QString fixtureMeshPath(const Fixture *fixture) const;

    /** Bounding-box extents (metres) of the mesh file at $meshPath.
     *  Results are cached per path. Returns a zero vector when the file
     *  cannot be loaded */
    QVector3D meshFileExtents(const QString &meshPath) const;

    /** Half-thickness (metres) of the truss bars of the current stage,
     *  or 0 when the current stage has no trusses */
    Q_INVOKABLE qreal trussHalfSize() const;

    /** Vertical span (metres, monitor space) of the truss bars: the
     *  underside and the top face. Both are 0 when there are no trusses */
    void trussVerticalSpan(qreal &bottomY, qreal &topY) const;

    /** Size (metres) $fixtureID will actually be drawn at: the mesh is
     *  fitted into the declared physical size keeping its aspect ratio */
    Q_INVOKABLE QVector3D fixtureDrawnSize(quint32 fixtureID) const;

    void removeFixtureItem(quint32 itemID);

    QVector3D lightPosition(quint32 itemID) const;
    QMatrix4x4 lightMatrix(quint32 itemID) const;

    Q_INVOKABLE void createGenericItem(QString filename, int itemID);
    Q_INVOKABLE void setItemSelection(int itemID, bool enable, int keyModifiers);
    /** Select/deselect a generic item by its row $index in the items list model */
    Q_INVOKABLE void setItemSelectionByIndex(int index, bool enable, int keyModifiers);
    Q_INVOKABLE int sceneItemMonitorID(QObject *item) const;
    Q_INVOKABLE bool isGenericSceneItem(QObject *item) const;
    Q_INVOKABLE bool isFixtureSceneItem(QObject *item) const;
    int genericSelectedCount() const;

    /** Returns true if at least one of the selected generic items is locked */
    bool genericSelectedLocked() const;

    /** Lock/unlock the position of the selected generic items. If any
     *  selected item is unlocked, all get locked; otherwise all get unlocked */
    Q_INVOKABLE void toggleGenericItemsLock();

    Q_INVOKABLE void removeSelectedGenericItems();
    Q_INVOKABLE void normalizeSelectedGenericItems();
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

    /** Get/Set the custom name of the selected generic item.
     *  Only meaningful when exactly one item is selected */
    void updateGenericItemName(quint32 itemID, QString name);
    QString genericItemsName() const;
    void setGenericItemsName(QString name);

    /** Get/Set the base color of the selected generic items.
     *  An invalid color restores the original mesh colors */
    void updateGenericItemColor(quint32 itemID, QColor color);
    QColor genericItemsColor() const;
    void setGenericItemsColor(QColor color);
    Q_INVOKABLE void resetGenericItemsColor();

    QStringList stagesList() const;
    int stageIndex() const;
    void setStageIndex(int stageIndex);

    float ambientIntensity() const;
    void setAmbientIntensity(float ambientIntensity);

    float smokeAmount() const;
    void setSmokeAmount(float smokeAmount);

public slots:
    void slotRefreshView() override;

signals:
    void cameraPositionChanged();
    void cameraViewCenterChanged();

    void frameCountEnabledChanged();
    void FPSChanged(int fps);
    void minFPSChanged(int fps);
    void maxFPSChanged(int fps);
    void avgFPSChanged(float fps);

    void stageIndexChanged(int stageIndex);
    void ambientIntensityChanged(qreal ambientIntensity);
    void smokeAmountChanged(float smokeAmount);

    void genericItemsListChanged();
    void genericSelectedCountChanged();
    void genericSelectedLockedChanged();
    void genericItemsPositionChanged();
    void genericItemsRotationChanged();
    void genericItemsScaleChanged();
    void genericItemsNameChanged();
    void genericItemsColorChanged();

protected:
    QString meshDirectory() const;
    QString goboDirectory() const;

private:
    void initialize3DProperties();
    void updateGenericItemsList();
    /** Set the selection state of the row of $itemID in the QML items list */
    void setGenericItemListSelection(int itemID, bool selected);
    /** Push the lock state of $itemID to its scene item */
    void applyGenericItemLock(quint32 itemID);
    void syncSceneItemPosition(QObject *item, const QVector3D &scenePos);
    /** Start/stop the FPS sampling, depending on the user setting
     *  and on the 3D renderer availability */
    void updateFrameCounter();
    /** Read the frames rendered in the last period and update the FPS stats */
    void slotSampleFrameCount();
    void syncSceneItemRotation(QObject *item, const QVector3D &sceneRot);

private:
    MonitorProperties *m_monProps;

    QVector3D m_cameraPosition;
    QVector3D m_cameraViewCenter;

    bool m_frameCountEnabled;
    QTimer *m_fpsTimer;
    QElapsedTimer m_fpsElapsed;
    int m_frameCount;
    int m_minFrameCount;
    int m_maxFrameCount;
    float m_avgFrameCount;

    QStringList m_stagesList;
    QStringList m_stageResourceList;

    /** Mesh file extents (metres), keyed by path. Filled lazily by meshFileExtents() */
    mutable QHash<QString, QVector3D> m_meshFileExtents;

    float m_ambientIntensity;
    float m_smokeAmount;

    int m_latestGenericID;
    ListModel *m_genericItemsList;
    QList<int> m_genericSelectedItems;
};

#endif // MAINVIEW3DRHI_H
