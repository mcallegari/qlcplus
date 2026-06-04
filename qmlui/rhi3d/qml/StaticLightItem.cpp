/*
  Q Light Controller Plus
  StaticLightItem.cpp

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

#include "qml/StaticLightItem.h"
#include "qlcfile.h"
#include "qlcconfig.h"

#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QUrl>

#include <QtMath>

static QString localPathForFileCheck(const QString &path)
{
    const QUrl url(path);
    if (url.isValid() && url.isLocalFile())
        return url.toLocalFile();
    return path;
}

StaticLightItem::StaticLightItem(QObject *parent)
    : MeshItem(parent)
{
    m_path = QDir::cleanPath(QLCFile::systemDirectory(MESHESDIR).path()
                             + QDir::separator() + "fixtures"
                             + QDir::separator() + "par.glb");
}

void StaticLightItem::setPath(const QString &path)
{
    if (m_path == path)
        return;

    if (path.isEmpty())
    {
        qWarning() << "StaticLightItem: empty mesh path";
    }
    else if (!QFileInfo::exists(localPathForFileCheck(path)))
    {
        qWarning() << "StaticLightItem: mesh path does not exist:" << path;
    }

    m_path = path;
    emit pathChanged();
    notifyParent();
}

void StaticLightItem::setColor(const QVector3D &color)
{
    if (m_color == color)
        return;
    m_color = color;
    emit colorChanged();
    notifyParent();
}

void StaticLightItem::setIntensity(float intensity)
{
    if (qFuzzyCompare(m_intensity, intensity))
        return;
    m_intensity = intensity;
    emit intensityChanged();
    notifyParent();
}

void StaticLightItem::setZoom(float zoom)
{
    if (qFuzzyCompare(m_zoom, zoom))
        return;
    m_zoom = zoom;
    emit zoomChanged();
    notifyParent();
}

void StaticLightItem::setBeamMode(bool enabled)
{
    if (m_beamMode == enabled)
        return;
    m_beamMode = enabled;
    emit beamModeChanged();
    notifyParent();
}

void StaticLightItem::setBeamRadius(float radius)
{
    if (qFuzzyCompare(m_beamRadius, radius))
        return;
    m_beamRadius = radius;
    emit beamRadiusChanged();
    notifyParent();
}

void StaticLightItem::setGoboPath(const QString &path)
{
    if (m_goboPath == path)
        return;
    m_goboPath = path;
    emit goboPathChanged();
    notifyParent();
}

Light StaticLightItem::toLight() const
{
    Light light;
    light.type = Light::Type::Spot;
    light.position = QVector3D(0.0f, 0.0f, 0.0f);
    light.direction = QVector3D(0.0f, -1.0f, 0.0f);
    light.color = m_color;
    light.intensity = m_intensity;
    light.range = 20.0f;
    light.beamRadius = m_beamRadius;
    light.beamShape = m_beamMode
            ? Light::BeamShapeType::BeamShape
            : Light::BeamShapeType::ConeShape;
    const float halfAngle = qDegreesToRadians(m_zoom * 0.5f);
    light.outerCone = halfAngle;
    light.innerCone = halfAngle * 0.8f;
    light.castShadows = true;
    light.goboPath = m_goboPath;
    return light;
}
