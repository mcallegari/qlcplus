/*
  Q Light Controller Plus
  RhiQmlItem.h

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

#pragma once

#include <QtQuick/QQuickRhiItem>
#include <QtCore/QElapsedTimer>
#include <QtGui/QVector3D>
#include <QtCore/QVector>
#include <QtCore/QSet>
#include <QtCore/QString>
#include <QtCore/QSizeF>
#include <QtCore/QPointF>
#include <QtCore/Qt>

#include "scene/Scene.h"

class QTimer;

class RhiQmlItem : public QQuickRhiItem
{
    Q_OBJECT
    Q_PROPERTY(QVector3D cameraPosition READ cameraPosition WRITE setCameraPosition NOTIFY cameraPositionChanged)
    Q_PROPERTY(QVector3D cameraTarget READ cameraTarget WRITE setCameraTarget NOTIFY cameraTargetChanged)
    Q_PROPERTY(float cameraFov READ cameraFov WRITE setCameraFov NOTIFY cameraFovChanged)
    Q_PROPERTY(QVector3D ambientLight READ ambientLight WRITE setAmbientLight NOTIFY ambientLightChanged)
    Q_PROPERTY(float ambientIntensity READ ambientIntensity WRITE setAmbientIntensity NOTIFY ambientIntensityChanged)
    Q_PROPERTY(float smokeAmount READ smokeAmount WRITE setSmokeAmount NOTIFY smokeAmountChanged)
    Q_PROPERTY(BeamModel beamModel READ beamModel WRITE setBeamModel NOTIFY beamModelChanged)
    Q_PROPERTY(float bloomIntensity READ bloomIntensity WRITE setBloomIntensity NOTIFY bloomIntensityChanged)
    Q_PROPERTY(float bloomRadius READ bloomRadius WRITE setBloomRadius NOTIFY bloomRadiusChanged)
    Q_PROPERTY(bool volumetricEnabled READ volumetricEnabled WRITE setVolumetricEnabled NOTIFY volumetricEnabledChanged)
    Q_PROPERTY(bool shadowsEnabled READ shadowsEnabled WRITE setShadowsEnabled NOTIFY shadowsEnabledChanged)
    Q_PROPERTY(bool smokeNoiseEnabled READ smokeNoiseEnabled WRITE setSmokeNoiseEnabled NOTIFY smokeNoiseEnabledChanged)
    Q_PROPERTY(bool freeCameraEnabled READ freeCameraEnabled WRITE setFreeCameraEnabled NOTIFY freeCameraEnabledChanged)
    Q_PROPERTY(bool positionPicking READ positionPicking WRITE setPositionPicking NOTIFY positionPickingChanged)
    Q_PROPERTY(float moveSpeed READ moveSpeed WRITE setMoveSpeed NOTIFY moveSpeedChanged)
    Q_PROPERTY(float lookSensitivity READ lookSensitivity WRITE setLookSensitivity NOTIFY lookSensitivityChanged)

public:
    enum BeamModel
    {
        SoftHaze,
        Physical
    };
    Q_ENUM(BeamModel)

    explicit RhiQmlItem(QQuickItem *parent = nullptr);

    QVector3D cameraPosition() const { return m_cameraPosition; }
    void setCameraPosition(const QVector3D &pos);

    QVector3D cameraTarget() const { return m_cameraTarget; }
    void setCameraTarget(const QVector3D &target);

    float cameraFov() const { return m_cameraFov; }
    void setCameraFov(float fov);

    QVector3D ambientLight() const { return m_ambientLight; }
    void setAmbientLight(const QVector3D &ambient);
    float ambientIntensity() const { return m_ambientIntensity; }
    void setAmbientIntensity(float intensity);
    float smokeAmount() const { return m_smokeAmount; }
    void setSmokeAmount(float amount);
    BeamModel beamModel() const { return m_beamModel; }
    void setBeamModel(BeamModel mode);
    float bloomIntensity() const { return m_bloomIntensity; }
    void setBloomIntensity(float intensity);
    float bloomRadius() const { return m_bloomRadius; }
    void setBloomRadius(float radius);
    bool volumetricEnabled() const { return m_volumetricEnabled; }
    void setVolumetricEnabled(bool enabled);
    bool shadowsEnabled() const { return m_shadowsEnabled; }
    void setShadowsEnabled(bool enabled);
    bool smokeNoiseEnabled() const { return m_smokeNoiseEnabled; }
    void setSmokeNoiseEnabled(bool enabled);
    bool freeCameraEnabled() const { return m_freeCameraEnabled; }
    void setFreeCameraEnabled(bool enabled);
    bool positionPicking() const { return m_positionPicking; }
    void setPositionPicking(bool enabled);
    float moveSpeed() const { return m_moveSpeed; }
    void setMoveSpeed(float speed);
    float lookSensitivity() const { return m_lookSensitivity; }
    void setLookSensitivity(float sensitivity);
    float smokeTimeSeconds() const;

    Q_INVOKABLE void addModel(const QString &path);
    Q_INVOKABLE void addModel(const QString &path, const QVector3D &position);
    Q_INVOKABLE void addModel(const QString &path,
                              const QVector3D &position,
                              const QVector3D &rotationDegrees,
                              const QVector3D &scale);
    Q_INVOKABLE void addPointLight(const QVector3D &position,
                                   const QVector3D &color,
                                   float intensity,
                                   float range);
    Q_INVOKABLE void addDirectionalLight(const QVector3D &direction,
                                         const QVector3D &color,
                                         float intensity);
    Q_INVOKABLE void addSpotLight(const QVector3D &position,
                                  const QVector3D &direction,
                                  const QVector3D &color,
                                  float intensity,
                                  float coneAngleDegrees);
    Q_INVOKABLE void addAreaLight(const QVector3D &position,
                                  const QVector3D &direction,
                                  const QVector3D &color,
                                  float intensity,
                                  const QSizeF &size);
    Q_INVOKABLE void zoomAlongView(float delta);

    struct PendingModel
    {
        QString path;
        QVector3D position;
        QVector3D rotationDegrees;
        QVector3D scale = QVector3D(1.0f, 1.0f, 1.0f);
    };
    void takePendingModels(QVector<PendingModel> &out);
    void takePendingLights(QVector<Light> &out);
    struct PickRequest
    {
        QPointF normPos;
        Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    };
    void takePendingPickRequests(QVector<PickRequest> &out);
    struct DragRequest
    {
        QPointF normPos;
        int type = 0;
    };
    void takePendingDragRequests(QVector<DragRequest> &out);
    Q_INVOKABLE void dispatchPickResult(QObject *item, const QVector3D &worldPos, bool hit, int modifiers);
    Q_INVOKABLE void handlePick(QObject *item, bool hit, int modifiers);
    Q_INVOKABLE void removeSelectedItems();
    Q_INVOKABLE void setCameraDirection(const QVector3D &dir);
    Q_INVOKABLE void rotateFreeCamera(float yawDelta, float pitchDelta);
    Q_PROPERTY(QObject *selectedItem READ selectedItem WRITE setSelectedItem NOTIFY selectedItemChanged)
    QObject *selectedItem() const { return m_selectedItem; }
    void setSelectedItem(QObject *item);
    Q_INVOKABLE void setObjectPosition(QObject *item, const QVector3D &pos);
    Q_INVOKABLE void setObjectRotation(QObject *item, const QVector3D &rotation);
    void updateSelectableItems(const QVector<QObject *> &items);

Q_SIGNALS:
    void cameraPositionChanged();
    void cameraTargetChanged();
    void cameraFovChanged();
    void ambientLightChanged();
    void ambientIntensityChanged();
    void smokeAmountChanged();
    void beamModelChanged();
    void bloomIntensityChanged();
    void bloomRadiusChanged();
    void volumetricEnabledChanged();
    void shadowsEnabledChanged();
    void smokeNoiseEnabledChanged();
    void freeCameraEnabledChanged();
    void positionPickingChanged();
    void moveSpeedChanged();
    void lookSensitivityChanged();
    void meshPicked(QObject *item, const QVector3D &worldPos, bool hit, int modifiers);
    void selectedItemChanged();
    void objectPositionEdited(QObject *item, const QVector3D &pos);
    void objectRotationEdited(QObject *item, const QVector3D &rotation);

protected:
    QQuickRhiItemRenderer *createRenderer() override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void orbitCameraAroundTarget(float yawDeltaDeg, float pitchDeltaDeg);
    void updateFreeCamera(float dtSeconds);
    void updateYawPitchFromDirection(const QVector3D &dir);
    QVector3D forwardVector() const;
    QVector3D rightVector() const;
    void updateSmokeTicker();

    QVector3D m_cameraPosition = QVector3D(0.0f, 1.0f, 5.0f);
    QVector3D m_cameraTarget = QVector3D(0.0f, 0.0f, 0.0f);
    float m_cameraFov = 60.0f;
    QVector3D m_ambientLight = QVector3D(0.0f, 0.0f, 0.0f);
    float m_ambientIntensity = 1.0f;
    float m_smokeAmount = 0.0f;
    BeamModel m_beamModel = SoftHaze;
    float m_bloomIntensity = 0.6f;
    float m_bloomRadius = 6.0f;
    bool m_volumetricEnabled = true;
    bool m_shadowsEnabled = true;
    bool m_smokeNoiseEnabled = true;
    bool m_freeCameraEnabled = false;
    bool m_positionPicking = false;
    float m_moveSpeed = 5.0f;
    float m_lookSensitivity = 0.2f;
    bool m_moveForward = false;
    bool m_moveBackward = false;
    bool m_moveLeft = false;
    bool m_moveRight = false;
    bool m_moveUp = false;
    bool m_moveDown = false;
    bool m_looking = false;
    bool m_panning = false;
    QPointF m_lastMousePos = QPointF(0.0, 0.0);
    QPointF m_lastPanPos = QPointF(0.0, 0.0);
    float m_yawDeg = -90.0f;
    float m_pitchDeg = -10.0f;
    QElapsedTimer m_cameraTimer;
    QTimer *m_cameraTick = nullptr;
    QElapsedTimer m_smokeTimer;
    QTimer *m_smokeTick = nullptr;

    QVector<PendingModel> m_pendingModels;
    QVector<Light> m_pendingLights;
    QVector<PickRequest> m_pendingPickRequests;
    QVector<DragRequest> m_pendingDragRequests;
    QSet<QObject *> m_selectableItems;
    QObject *m_selectedItem = nullptr;
    bool m_leftDown = false;
};
