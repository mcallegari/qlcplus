/*
  Q Light Controller Plus
  Scene.h

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

#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtGui/QVector3D>
#include <QtGui/QVector2D>

#include "scene/Camera.h"
#include "scene/Mesh.h"

struct Light
{
    enum class Type
    {
        Directional,
        Point,
        Spot,
        Area
    };
    enum class BeamShapeType
    {
        ConeShape,
        BeamShape
    };

    Type type = Type::Point;
    QVector3D color = QVector3D(1.0f, 1.0f, 1.0f);
    float intensity = 1.0f;
    QVector3D position;
    QVector3D direction;
    float range = 10.0f;
    float innerCone = 0.5f;
    float outerCone = 0.7f;
    QVector2D areaSize = QVector2D(1.0f, 1.0f);
    bool castShadows = true;
    int qualitySteps = 8;
    QString goboPath;
    float beamRadius = 0.15f;
    BeamShapeType beamShape = BeamShapeType::ConeShape;
};

class RhiScene
{
public:
    enum class BeamModel
    {
        SoftHaze,
        Physical
    };
    Camera &camera()
    {
        return m_camera;
    }
    const Camera &camera() const
    {
        return m_camera;
    }

    QVector<Mesh> &meshes()
    {
        return m_meshes;
    }
    const QVector<Mesh> &meshes() const
    {
        return m_meshes;
    }

    QVector<Light> &lights()
    {
        m_lightsDirty = true;
        return m_lights;
    }
    const QVector<Light> &lights() const
    {
        return m_lights;
    }
    bool setLights(const QVector<Light> &lights);
    void markLightsDirty() { m_lightsDirty = true; }
    bool lightsDirty() const { return m_lightsDirty; }
    void clearLightsDirty() { m_lightsDirty = false; }

    QVector3D ambientLight() const
    {
        return m_ambientLight;
    }
    void setAmbientLight(const QVector3D &ambient)
    {
        if (m_ambientLight == ambient)
            return;
        m_ambientLight = ambient;
        m_lightParamsDirty = true;
    }
    float ambientIntensity() const
    {
        return m_ambientIntensity;
    }
    void setAmbientIntensity(float intensity)
    {
        if (m_ambientIntensity == intensity)
            return;
        m_ambientIntensity = intensity;
        m_lightParamsDirty = true;
    }
    float smokeAmount() const
    {
        return m_smokeAmount;
    }
    void setSmokeAmount(float amount)
    {
        if (m_smokeAmount == amount)
            return;
        m_smokeAmount = amount;
        m_lightParamsDirty = true;
    }
    BeamModel beamModel() const
    {
        return m_beamModel;
    }
    void setBeamModel(BeamModel mode)
    {
        if (m_beamModel == mode)
            return;
        m_beamModel = mode;
        m_lightParamsDirty = true;
    }
    QVector3D hazePosition() const
    {
        return m_hazePosition;
    }
    void setHazePosition(const QVector3D &position)
    {
        if (m_hazePosition == position)
            return;
        m_hazePosition = position;
    }
    QVector3D hazeDirection() const
    {
        return m_hazeDirection;
    }
    void setHazeDirection(const QVector3D &direction)
    {
        if (m_hazeDirection == direction)
            return;
        m_hazeDirection = direction;
    }
    float hazeLength() const
    {
        return m_hazeLength;
    }
    void setHazeLength(float length)
    {
        if (m_hazeLength == length)
            return;
        m_hazeLength = length;
    }
    float hazeRadius() const
    {
        return m_hazeRadius;
    }
    void setHazeRadius(float radius)
    {
        if (m_hazeRadius == radius)
            return;
        m_hazeRadius = radius;
    }
    float hazeDensity() const
    {
        return m_hazeDensity;
    }
    void setHazeDensity(float density)
    {
        if (m_hazeDensity == density)
            return;
        m_hazeDensity = density;
    }
    bool hazeEnabled() const
    {
        return m_hazeEnabled;
    }
    void setHazeEnabled(bool enabled)
    {
        if (m_hazeEnabled == enabled)
            return;
        m_hazeEnabled = enabled;
    }
    float bloomIntensity() const
    {
        return m_bloomIntensity;
    }
    void setBloomIntensity(float intensity)
    {
        if (m_bloomIntensity == intensity)
            return;
        m_bloomIntensity = intensity;
        m_lightParamsDirty = true;
    }
    float bloomRadius() const
    {
        return m_bloomRadius;
    }
    void setBloomRadius(float radius)
    {
        if (m_bloomRadius == radius)
            return;
        m_bloomRadius = radius;
        m_lightParamsDirty = true;
    }
    bool volumetricEnabled() const
    {
        return m_volumetricEnabled;
    }
    void setVolumetricEnabled(bool enabled)
    {
        if (m_volumetricEnabled == enabled)
            return;
        m_volumetricEnabled = enabled;
        m_lightParamsDirty = true;
    }
    bool shadowsEnabled() const
    {
        return m_shadowsEnabled;
    }
    void setShadowsEnabled(bool enabled)
    {
        if (m_shadowsEnabled == enabled)
            return;
        m_shadowsEnabled = enabled;
        m_lightParamsDirty = true;
    }
    bool smokeNoiseEnabled() const
    {
        return m_smokeNoiseEnabled;
    }
    void setSmokeNoiseEnabled(bool enabled)
    {
        if (m_smokeNoiseEnabled == enabled)
            return;
        m_smokeNoiseEnabled = enabled;
        m_lightParamsDirty = true;
    }
    float timeSeconds() const
    {
        return m_timeSeconds;
    }
    void setTimeSeconds(float seconds)
    {
        if (m_timeSeconds == seconds)
            return;
        m_timeSeconds = seconds;
        m_timeDirty = true;
    }

    bool cameraDirty() const { return m_cameraDirty || m_camera.isDirty(); }
    void clearCameraDirty()
    {
        m_cameraDirty = false;
        m_camera.clearDirty();
    }
    bool timeDirty() const { return m_timeDirty; }
    void clearTimeDirty() { m_timeDirty = false; }
    bool lightParamsDirty() const { return m_lightParamsDirty; }
    void clearLightParamsDirty() { m_lightParamsDirty = false; }
    bool selectionDirty() const { return m_selectionDirty; }
    void markSelectionDirty() { m_selectionDirty = true; }
    void clearSelectionDirty() { m_selectionDirty = false; }
    void clearFrameDirty()
    {
        clearLightsDirty();
        clearLightParamsDirty();
        clearCameraDirty();
        clearTimeDirty();
        clearSelectionDirty();
    }

private:
    Camera m_camera;
    QVector<Mesh> m_meshes;
    QVector<Light> m_lights;
    QVector3D m_ambientLight = QVector3D(0.0f, 0.0f, 0.0f);
    float m_ambientIntensity = 1.0f;
    float m_smokeAmount = 0.0f;
    BeamModel m_beamModel = BeamModel::SoftHaze;
    float m_bloomIntensity = 0.0f;
    float m_bloomRadius = 4.0f;
    float m_timeSeconds = 0.0f;
    bool m_volumetricEnabled = true;
    bool m_shadowsEnabled = true;
    bool m_smokeNoiseEnabled = true;
    QVector3D m_hazePosition = QVector3D(0.0f, 0.0f, 0.0f);
    QVector3D m_hazeDirection = QVector3D(0.0f, 1.0f, 0.0f);
    float m_hazeLength = 3.0f;
    float m_hazeRadius = 1.0f;
    float m_hazeDensity = 0.0f;
    bool m_hazeEnabled = false;
    bool m_lightsDirty = true;
    bool m_lightParamsDirty = true;
    bool m_cameraDirty = true;
    bool m_timeDirty = true;
    bool m_selectionDirty = true;
};
