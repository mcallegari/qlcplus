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
    Q_PROPERTY(QVector3D genericItemsPosition READ genericItemsPosition WRITE setGenericItemsPosition NOTIFY genericItemsPositionChanged)
    Q_PROPERTY(QVector3D genericItemsRotation READ genericItemsRotation WRITE setGenericItemsRotation NOTIFY genericItemsRotationChanged)
    Q_PROPERTY(QVector3D genericItemsScale READ genericItemsScale WRITE setGenericItemsScale NOTIFY genericItemsScaleChanged)

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

    void removeFixtureItem(quint32 itemID);

    QVector3D lightPosition(quint32 itemID) const;
    QMatrix4x4 lightMatrix(quint32 itemID) const;

    Q_INVOKABLE void createGenericItem(QString filename, int itemID);
    Q_INVOKABLE void setItemSelection(int itemID, bool enable, int keyModifiers);
    Q_INVOKABLE int sceneItemMonitorID(QObject *item) const;
    Q_INVOKABLE bool isGenericSceneItem(QObject *item) const;
    Q_INVOKABLE bool isFixtureSceneItem(QObject *item) const;
    int genericSelectedCount() const;
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
    void genericItemsPositionChanged();
    void genericItemsRotationChanged();
    void genericItemsScaleChanged();

protected:
    QString meshDirectory() const;
    QString goboDirectory() const;

private:
    void initialize3DProperties();
    void updateGenericItemsList();
    void syncSceneItemPosition(QObject *item, const QVector3D &scenePos);
    void syncSceneItemRotation(QObject *item, const QVector3D &sceneRot);

private:
    MonitorProperties *m_monProps;

    QVector3D m_cameraPosition;
    QVector3D m_cameraViewCenter;

    bool m_frameCountEnabled;
    int m_frameCount;
    int m_minFrameCount;
    int m_maxFrameCount;
    float m_avgFrameCount;

    QStringList m_stagesList;

    float m_ambientIntensity;
    float m_smokeAmount;

    int m_latestGenericID;
    ListModel *m_genericItemsList;
    QList<int> m_genericSelectedItems;
};

#endif // MAINVIEW3DRHI_H
