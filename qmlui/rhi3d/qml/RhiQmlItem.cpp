/*
  Q Light Controller Plus
  RhiQmlItem.cpp

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

#include "qml/RhiQmlItem.h"

#include <QtCore/QElapsedTimer>
#include <QtCore/QDateTime>
#include <QtCore/QTimer>
#include <QtCore/QMetaObject>
#include <QtCore/QVariant>
#include <QtCore/QLatin1String>
#include <QtMath>
#include <memory>
#include <QtGui/QMatrix4x4>
#include <QtGui/QGuiApplication>
#include <QtGui/QMouseEvent>
#include <QtGui/QQuaternion>
#include <QtGui/QKeyEvent>
#include <QtGui/QImage>
#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <rhi/qrhi.h>

#include "core/RhiContext.h"
#include "core/RenderTargetCache.h"
#include "core/ShaderManager.h"
#include "renderer/DeferredRenderer.h"
#include "scene/AssimpLoader.h"
#include "qml/CameraItem.h"
#include "qml/LightItem.h"
#include "qml/HazerItem.h"
#include "qml/ModelItem.h"
#include "qml/StaticLightItem.h"
#include "qml/MovingHeadItem.h"
#include "qml/CubeItem.h"
#include "qml/SphereItem.h"
#include "qml/PixelBarItem.h"
#include "qml/BeamBarItem.h"
#include "qml/VideoItem.h"
#include "qml/MeshItem.h"
#include "qml/MeshUtils.h"
#include "qml/PickingUtils.h"

namespace
{
using namespace RhiQmlUtils;
static constexpr float kMovingHeadTopRadiusMin = 0.050f;
static constexpr float kMovingHeadTopRadiusScale = 1.00f;

struct EmitterData
{
    QVector3D position;
    QVector3D direction;
    float diameter = 0.0f;
};

struct MeshRecord
{
    const MeshItem *item = nullptr;
    MeshItem::MeshType type = MeshItem::MeshType::Model;
    QString path;
    QVector3D position;
    QVector3D rotationDegrees;
    QVector3D scale;
    QVector3D baseColor = QVector3D(0.7f, 0.7f, 0.7f);
    QVector3D emissiveColor = QVector3D(0.0f, 0.0f, 0.0f);
    float metalness = 0.0f;
    float roughness = 0.5f;
    float pan = 0.0f;
    float tilt = 0.0f;
    int emitterCount = 0;
    QSize headsLayout = QSize(1, 1);
    QVector3D lightColor = QVector3D(1.0f, 1.0f, 1.0f);
    float intensity = 1.0f;
    float range = 20.0f;
    float beamRadius = 0.2f;
    bool castShadows = false;
    QVector<QVector3D> emitterColors;
    QVector<float> emitterIntensities;
    QImage videoFrame;
    QSize videoSize;
    QRhiTexture *videoTexture = nullptr;
    bool videoDirty = false;
    bool selected = false;
    bool selectable = true;
    bool visible = true;
    int firstMesh = 0;
    int meshCount = 0;
    int armMesh = -1;
    int headMesh = -1;
    QVector3D armPivot = QVector3D(0.0f, 0.0f, 0.0f);
    QVector3D headPivot = QVector3D(0.0f, 0.0f, 0.0f);
    bool pivotsValid = false;
};

static bool isEmitterName(const QString &name)
{
    return name.contains(QLatin1String("emitter"), Qt::CaseInsensitive);
}

static const char *meshTypeName(MeshItem::MeshType type)
{
    switch (type)
    {
        case MeshItem::MeshType::Model:
            return "Model";
        case MeshItem::MeshType::StaticLight:
            return "StaticLight";
        case MeshItem::MeshType::MovingHead:
            return "MovingHead";
        case MeshItem::MeshType::Cube:
            return "Cube";
        case MeshItem::MeshType::Sphere:
            return "Sphere";
        case MeshItem::MeshType::Video:
            return "Video";
        case MeshItem::MeshType::PixelBar:
            return "PixelBar";
        case MeshItem::MeshType::BeamBar:
            return "BeamBar";
        default:
            return "Unknown";
    }
}

static QString localPathForFileCheck(const QString &path)
{
    const QUrl url(path);
    if (url.isValid() && url.isLocalFile())
        return url.toLocalFile();
    return path;
}

static QVector3D averageNormal(const Mesh &mesh)
{
    QVector3D sum(0.0f, 0.0f, 0.0f);
    for (const Vertex &v : mesh.vertices)
        sum += QVector3D(v.nx, v.ny, v.nz);
    if (sum.isNull())
        return QVector3D(0.0f, -1.0f, 0.0f);
    return sum.normalized();
}

static float maxPlaneDiameter(const Mesh &mesh, const QVector3D &localNormal)
{
    if (!mesh.boundsValid)
        return 0.0f;
    const QVector3D extents = mesh.boundsMax - mesh.boundsMin;
    const QVector3D col0 = mesh.modelMatrix.column(0).toVector3D();
    const QVector3D col1 = mesh.modelMatrix.column(1).toVector3D();
    const QVector3D col2 = mesh.modelMatrix.column(2).toVector3D();
    const QVector3D scaled(extents.x() * col0.length(),
                           extents.y() * col1.length(),
                           extents.z() * col2.length());
    const QVector3D absN(qAbs(localNormal.x()), qAbs(localNormal.y()), qAbs(localNormal.z()));
    if (absN.x() >= absN.y() && absN.x() >= absN.z())
        return qMax(scaled.y(), scaled.z());
    if (absN.y() >= absN.x() && absN.y() >= absN.z())
        return qMax(scaled.x(), scaled.z());
    return qMax(scaled.x(), scaled.y());
}

static void projectedEmitterMetrics(const Mesh &mesh,
                                    const QVector3D &worldCenter,
                                    const QVector3D &worldAxis,
                                    float &diameter,
                                    float &frontOffset)
{
    diameter = 0.0f;
    frontOffset = 0.0f;
    if (!mesh.boundsValid || worldAxis.isNull())
        return;

    const QVector3D axis = worldAxis.normalized();
    const QVector3D bmin = mesh.boundsMin;
    const QVector3D bmax = mesh.boundsMax;
    const QVector3D corners[8] = {
        QVector3D(bmin.x(), bmin.y(), bmin.z()),
        QVector3D(bmax.x(), bmin.y(), bmin.z()),
        QVector3D(bmin.x(), bmax.y(), bmin.z()),
        QVector3D(bmax.x(), bmax.y(), bmin.z()),
        QVector3D(bmin.x(), bmin.y(), bmax.z()),
        QVector3D(bmax.x(), bmin.y(), bmax.z()),
        QVector3D(bmin.x(), bmax.y(), bmax.z()),
        QVector3D(bmax.x(), bmax.y(), bmax.z())
    };

    bool havePoint = false;
    float radialMax = 0.0f;
    float axialMax = 0.0f;
    for (const QVector3D &corner : corners)
    {
        const QVector3D worldCorner = mesh.modelMatrix.map(corner);
        const QVector3D delta = worldCorner - worldCenter;
        const float axial = QVector3D::dotProduct(delta, axis);
        const QVector3D radialVec = delta - axis * axial;
        const float radial = radialVec.length();
        if (!havePoint)
        {
            radialMax = radial;
            axialMax = axial;
            havePoint = true;
        }
        else
        {
            radialMax = qMax(radialMax, radial);
            axialMax = qMax(axialMax, axial);
        }
    }

    if (!havePoint)
        return;

    diameter = radialMax * 2.0f;
    frontOffset = axialMax;
}

static QVector<EmitterData> collectEmitters(const QVector<Mesh> &meshes,
                                            int firstMesh,
                                            int meshCount,
                                            const QVector3D &directionOverride = QVector3D())
{
    QVector<EmitterData> emitters;
    emitters.reserve(meshCount);
    for (int i = firstMesh; i < firstMesh + meshCount; ++i)
    {
        const Mesh &mesh = meshes[i];
        if (!isEmitterName(mesh.name))
            continue;
        if (!mesh.boundsValid)
            continue;
        const QVector3D localCenter = (mesh.boundsMin + mesh.boundsMax) * 0.5f;
        const QVector3D localNormal = averageNormal(mesh);
        const QVector3D worldPos = mesh.modelMatrix.map(localCenter);
        const QMatrix3x3 normalMat = mesh.modelMatrix.normalMatrix();
        const QVector3D worldNormal = QVector3D(
            normalMat(0, 0) * localNormal.x() + normalMat(0, 1) * localNormal.y() + normalMat(0, 2) * localNormal.z(),
            normalMat(1, 0) * localNormal.x() + normalMat(1, 1) * localNormal.y() + normalMat(1, 2) * localNormal.z(),
            normalMat(2, 0) * localNormal.x() + normalMat(2, 1) * localNormal.y() + normalMat(2, 2) * localNormal.z()
        ).normalized();
        QVector3D emitterDirection = directionOverride;
        if (emitterDirection.isNull())
            emitterDirection = worldNormal;
        if (emitterDirection.isNull())
            emitterDirection = QVector3D(0.0f, -1.0f, 0.0f);
        emitterDirection.normalize();

        float diameter = 0.0f;
        float frontOffset = 0.0f;
        projectedEmitterMetrics(mesh, worldPos, emitterDirection, diameter, frontOffset);
        if (diameter <= 1e-5f)
            diameter = maxPlaneDiameter(mesh, localNormal);

        const QVector3D emitterPos = worldPos + emitterDirection * frontOffset;
        emitters.push_back({ emitterPos, emitterDirection, diameter });
    }
    return emitters;
}

struct TransformInfo
{
    QMatrix4x4 matrix;
    QQuaternion rotation;
};

static QQuaternion rotationFromAxesOrderXYZ(const QVector3D &rotationDegrees)
{
    // Match legacy Qt3D behavior from QTransform::fromAxesAndAngles(X, Y, Z).
    return QQuaternion::fromAxisAndAngle(0.0f, 0.0f, 1.0f, rotationDegrees.z())
            * QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, rotationDegrees.y())
            * QQuaternion::fromAxisAndAngle(1.0f, 0.0f, 0.0f, rotationDegrees.x());
}

static TransformInfo makeTransform(const QVector3D &position,
                                   const QVector3D &rotationDegrees,
                                   const QVector3D &scale)
{
    TransformInfo info;
    info.matrix.translate(position);
    if (!rotationDegrees.isNull())
    {
        info.rotation = rotationFromAxesOrderXYZ(rotationDegrees);
        info.matrix.rotate(info.rotation);
    }
    if (scale != QVector3D(1.0f, 1.0f, 1.0f))
        info.matrix.scale(scale);
    return info;
}

static void applyTransform(QVector<Mesh> &meshes,
                           int firstMesh,
                           int meshCount,
                           const TransformInfo &transform,
                           const QVector3D &position,
                           bool setBase)
{
    for (int i = firstMesh; i < firstMesh + meshCount; ++i)
    {
        Mesh &mesh = meshes[i];
        if (setBase)
            mesh.baseModelMatrix = mesh.modelMatrix;
        mesh.modelMatrix = transform.matrix * mesh.baseModelMatrix;
        mesh.userOffset = position;
        mesh.modelDirty = true;
        mesh.worldBoundsDirty = true;
        if (mesh.selected)
            mesh.selectionDirty = true;
    }
}

static QMatrix4x4 rotateAroundPivot(const QVector3D &pivot, const QQuaternion &rotation)
{
    QMatrix4x4 mat;
    mat.translate(pivot);
    mat.rotate(rotation);
    mat.translate(-pivot);
    return mat;
}

enum class MovingHeadPart
{
    Base,
    Arm,
    Head,
    HeadChild,
    Other
};

static MovingHeadPart movingHeadPartForName(const QString &name)
{
    const QString lower = name.toLower();
    if (lower.contains(QLatin1String("emitter")) || lower.contains(QLatin1String("handle")))
        return MovingHeadPart::HeadChild;
    if (lower.contains(QLatin1String("head")))
        return MovingHeadPart::Head;
    if (lower.contains(QLatin1String("arm")))
        return MovingHeadPart::Arm;
    if (lower.contains(QLatin1String("base")))
        return MovingHeadPart::Base;
    return MovingHeadPart::Other;
}

static void applySelectionVisibility(QVector<Mesh> &meshes,
                                     int firstMesh,
                                     int meshCount,
                                     bool selected,
                                     bool selectable,
                                     bool visible)
{
    for (int i = firstMesh; i < firstMesh + meshCount; ++i)
    {
        Mesh &mesh = meshes[i];
        const bool changed = mesh.selected != selected
                || mesh.selectable != selectable
                || mesh.visible != visible;
        mesh.selected = selected;
        mesh.selectable = selectable;
        mesh.visible = visible;
        if (changed)
            mesh.selectionDirty = true;
    }
}

static void applySelectionGroup(QVector<Mesh> &meshes,
                                int firstMesh,
                                int meshCount,
                                int groupId)
{
    for (int i = firstMesh; i < firstMesh + meshCount; ++i)
    {
        Mesh &mesh = meshes[i];
        if (mesh.selectionGroup != groupId)
        {
            mesh.selectionGroup = groupId;
            mesh.selectionDirty = true;
        }
    }
}

static Mesh::SelectionDomain selectionDomainForMeshType(MeshItem::MeshType type)
{
    return (type == MeshItem::MeshType::Model)
            ? Mesh::SelectionDomain::GenericItem
            : Mesh::SelectionDomain::FixtureItem;
}

static void applySelectionDomain(QVector<Mesh> &meshes,
                                 int firstMesh,
                                 int meshCount,
                                 Mesh::SelectionDomain domain)
{
    for (int i = firstMesh; i < firstMesh + meshCount; ++i)
    {
        Mesh &mesh = meshes[i];
        if (mesh.selectionDomain != domain)
        {
            mesh.selectionDomain = domain;
            if (mesh.selected)
                mesh.selectionDirty = true;
        }
    }
}

static void applyMaterial(QVector<Mesh> &meshes,
                          int firstMesh,
                          int meshCount,
                          const QVector3D &baseColor,
                          const QVector3D &emissiveColor,
                          float metalness,
                          float roughness)
{
    for (int i = firstMesh; i < firstMesh + meshCount; ++i)
    {
        Mesh &mesh = meshes[i];
        if (mesh.material.baseColor != baseColor
                || mesh.material.emissive != emissiveColor
                || mesh.material.metalness != metalness
                || mesh.material.roughness != roughness)
        {
            mesh.materialDirty = true;
        }
        mesh.material.baseColor = baseColor;
        mesh.material.emissive = emissiveColor;
        mesh.material.metalness = metalness;
        mesh.material.roughness = roughness;
    }
}

template <typename RecordT>
static void syncSelectionVisibilityFromItem(RecordT &record,
                                            const MeshItem *item,
                                            QVector<Mesh> &meshes)
{
    const bool selected = item->isSelected();
    const bool selectable = item->selectable();
    const bool visible = item->visible();
    if (record.selected == selected && record.selectable == selectable && record.visible == visible)
        return;
    record.selected = selected;
    record.selectable = selectable;
    record.visible = visible;
    applySelectionVisibility(meshes, record.firstMesh, record.meshCount,
                             record.selected, record.selectable, record.visible);
}

template <typename RecordT>
static bool syncTransformFromItem(RecordT &record,
                                  const MeshItem *item,
                                  QVector<Mesh> &meshes)
{
    const QVector3D position = item->position();
    const QVector3D rotationDegrees = item->rotationDegrees();
    const QVector3D scale = item->scale();
    if (record.position == position
            && record.rotationDegrees == rotationDegrees
            && record.scale == scale)
        return false;
    record.position = position;
    record.rotationDegrees = rotationDegrees;
    record.scale = scale;
    const TransformInfo transform = makeTransform(record.position,
                                                  record.rotationDegrees,
                                                  record.scale);
    applyTransform(meshes, record.firstMesh, record.meshCount,
                   transform, record.position, false);
    return true;
}

template <typename RecordT>
static TransformInfo transformFromRecord(const RecordT &record)
{
    return makeTransform(record.position, record.rotationDegrees, record.scale);
}

static QMatrix4x4 makeBasisMatrix(const QVector3D &position, const QVector3D &rotationDegrees)
{
    QMatrix4x4 mat;
    mat.translate(position);
    if (!rotationDegrees.isNull())
        mat.rotate(rotationFromAxesOrderXYZ(rotationDegrees));
    return mat;
}

static bool physicalSizeFromItem(const MeshItem *item, QVector3D &size)
{
    if (!item)
        return false;
    const QVariant sizeVar = item->property("physicalSize");
    if (!sizeVar.isValid() || !sizeVar.canConvert<QVector3D>())
        return false;

    const QVector3D value = sizeVar.value<QVector3D>();
    if (value.x() <= 0.0f || value.y() <= 0.0f || value.z() <= 0.0f)
        return false;

    size = value;
    return true;
}

static bool computeMeshGroupExtents(const QVector<Mesh> &meshes,
                                    int firstMesh,
                                    int meshCount,
                                    QVector3D &extents)
{
    QVector3D minV;
    QVector3D maxV;
    bool haveBounds = false;

    const int begin = qMax(0, firstMesh);
    const int end = qMin(firstMesh + meshCount, meshes.size());
    for (int i = begin; i < end; ++i)
    {
        const Mesh &mesh = meshes[i];
        if (!mesh.boundsValid)
            continue;

        const QVector3D bmin = mesh.boundsMin;
        const QVector3D bmax = mesh.boundsMax;
        const QVector3D corners[8] = {
            QVector3D(bmin.x(), bmin.y(), bmin.z()),
            QVector3D(bmax.x(), bmin.y(), bmin.z()),
            QVector3D(bmin.x(), bmax.y(), bmin.z()),
            QVector3D(bmax.x(), bmax.y(), bmin.z()),
            QVector3D(bmin.x(), bmin.y(), bmax.z()),
            QVector3D(bmax.x(), bmin.y(), bmax.z()),
            QVector3D(bmin.x(), bmax.y(), bmax.z()),
            QVector3D(bmax.x(), bmax.y(), bmax.z())
        };

        for (const QVector3D &corner : corners)
        {
            const QVector3D p = (mesh.modelMatrix * QVector4D(corner, 1.0f)).toVector3D();
            if (!haveBounds)
            {
                minV = p;
                maxV = p;
                haveBounds = true;
            }
            else
            {
                minV.setX(qMin(minV.x(), p.x()));
                minV.setY(qMin(minV.y(), p.y()));
                minV.setZ(qMin(minV.z(), p.z()));
                maxV.setX(qMax(maxV.x(), p.x()));
                maxV.setY(qMax(maxV.y(), p.y()));
                maxV.setZ(qMax(maxV.z(), p.z()));
            }
        }
    }

    if (!haveBounds)
        return false;

    extents = maxV - minV;
    return extents.x() > 1e-4f && extents.y() > 1e-4f && extents.z() > 1e-4f;
}

template <typename RecordT>
static bool applyPhysicalScaleFromItem(RecordT &record,
                                       const MeshItem *item,
                                       const QVector<Mesh> &meshes)
{
    QVector3D targetSize;
    if (!physicalSizeFromItem(item, targetSize))
        return false;

    QVector3D meshExtents;
    if (!computeMeshGroupExtents(meshes, record.firstMesh, record.meshCount, meshExtents))
        return false;

    const float xScale = targetSize.x() / meshExtents.x();
    const float yScale = targetSize.y() / meshExtents.y();
    const float zScale = targetSize.z() / meshExtents.z();
    const float minScale = qMin(xScale, qMin(yScale, zScale));
    if (!qIsFinite(minScale) || minScale <= 0.0f)
        return false;

    record.scale *= minScale;
    MeshItem *mutableItem = const_cast<MeshItem *>(item);
    if (mutableItem && mutableItem->scale() != record.scale)
        mutableItem->setScale(record.scale);
    return true;
}

template <typename RecordT, typename ItemT>
static void initCommonRecord(RecordT &record, const ItemT *item)
{
    record.position = item->position();
    record.rotationDegrees = item->rotationDegrees();
    record.scale = item->scale();
    record.selected = item->isSelected();
    record.selectable = item->selectable();
    record.visible = item->visible();
}

template <typename RecordT>
static TransformInfo applyCommonRecordTransforms(RecordT &record,
                                                 QVector<Mesh> &meshes,
                                                 bool setBase)
{
    const TransformInfo transform = transformFromRecord(record);
    applyTransform(meshes, record.firstMesh, record.meshCount, transform, record.position, setBase);
    applySelectionVisibility(meshes, record.firstMesh, record.meshCount,
                             record.selected, record.selectable, record.visible);
    applySelectionGroup(meshes, record.firstMesh, record.meshCount, record.firstMesh);
    applySelectionDomain(meshes, record.firstMesh, record.meshCount,
                         selectionDomainForMeshType(record.type));
    return transform;
}

static bool syncCommonFields(MeshRecord &record, const MeshItem *item)
{
    bool changed = false;
    if (record.position != item->position())
    {
        record.position = item->position();
        changed = true;
    }
    if (record.rotationDegrees != item->rotationDegrees())
    {
        record.rotationDegrees = item->rotationDegrees();
        changed = true;
    }
    if (record.scale != item->scale())
    {
        record.scale = item->scale();
        changed = true;
    }
    return changed;
}

static void applyPixelBarLayout(MeshRecord &record, QVector<Mesh> &meshes)
{
    if (record.meshCount <= 0)
        return;

    const int availableEmitters = qMax(0, record.meshCount - 1);
    const int emitterCount = qBound(0, record.emitterCount, availableEmitters);
    if (record.emitterCount != emitterCount)
        record.emitterCount = emitterCount;

    QMatrix4x4 base = makeBasisMatrix(record.position, record.rotationDegrees);
    const float length = qMax(0.001f, 0.1f * float(qMax(1, emitterCount)) * record.scale.x());
    const float height = qMax(0.001f, 0.1f * record.scale.y());
    const float depth = qMax(0.001f, 0.1f * record.scale.z());

    Mesh &body = meshes[record.firstMesh];
    QMatrix4x4 bodyLocal;
    bodyLocal.scale(length, height, depth);
    body.modelMatrix = base * bodyLocal;
    body.userOffset = record.position;
    body.material.baseColor = record.baseColor;
    body.material.emissive = QVector3D(0.0f, 0.0f, 0.0f);
    body.materialDirty = true;
    body.modelDirty = true;
    body.worldBoundsDirty = true;

    const float segment = emitterCount > 0 ? (length / emitterCount) : length;
    const float emitterX = segment * 0.8f;
    const float emitterY = qMax(0.001f, 0.01f * record.scale.y());
    const float emitterZ = depth * 0.8f;
    const float start = -length * 0.5f + segment * 0.5f;

    const float topOffset = (height * 0.5f) + (emitterY * 0.5f);
    for (int i = 0; i < emitterCount; ++i)
    {
        const QVector3D color = (i < record.emitterColors.size()) ? record.emitterColors[i] : record.emissiveColor;
        const float intensity = (i < record.emitterIntensities.size()) ? record.emitterIntensities[i] : 1.0f;
        const float emissiveBoost = 3.0f;
        const QVector3D emissive = color * (intensity * emissiveBoost);
        Mesh &mesh = meshes[record.firstMesh + 1 + i];
        QMatrix4x4 local;
        // Keep parity with Qt3D PixelBar3DItem: emitters sit on the +Y face.
        local.translate(start + i * segment, topOffset, 0.0f);
        local.scale(emitterX, emitterY, emitterZ);
        mesh.modelMatrix = base * local;
        mesh.userOffset = record.position;
        mesh.material.baseColor = color;
        mesh.material.emissive = emissive;
        mesh.materialDirty = true;
        mesh.modelDirty = true;
        mesh.worldBoundsDirty = true;
    }

    for (int i = record.firstMesh + 1 + emitterCount; i < record.firstMesh + record.meshCount; ++i)
    {
        Mesh &mesh = meshes[i];
        QMatrix4x4 hidden;
        hidden.setToIdentity();
        hidden.scale(0.0f);
        mesh.modelMatrix = hidden;
        mesh.userOffset = record.position;
        mesh.visible = false;
        mesh.selectable = false;
        mesh.selected = false;
        mesh.modelDirty = true;
        mesh.worldBoundsDirty = true;
        mesh.selectionDirty = true;
    }
}

static void applyBeamBarBody(MeshRecord &record, QVector<Mesh> &meshes)
{
    if (record.meshCount <= 0)
        return;
    QMatrix4x4 base = makeBasisMatrix(record.position, record.rotationDegrees);
    int cols = qMax(1, record.headsLayout.width());
    int rows = qMax(1, record.headsLayout.height());
    if (cols * rows < qMax(1, record.emitterCount))
    {
        cols = qMax(1, record.emitterCount);
        rows = 1;
    }
    const float length = qMax(0.001f, 0.1f * float(cols) * record.scale.x());
    const float height = qMax(0.001f, 0.1f * record.scale.y());
    const float depth = qMax(0.001f, 0.1f * float(rows) * record.scale.z());
    Mesh &body = meshes[record.firstMesh];
    QMatrix4x4 bodyLocal;
    bodyLocal.scale(length, height, depth);
    body.modelMatrix = base * bodyLocal;
    body.userOffset = record.position;
    body.material.baseColor = record.baseColor;
    body.material.emissive = QVector3D(0.0f, 0.0f, 0.0f);
    body.materialDirty = true;
    body.modelDirty = true;
    body.worldBoundsDirty = true;
}

template <typename RecordT>
static void applyMovingHeadTransforms(RecordT &record,
                                      QVector<Mesh> &meshes,
                                      const TransformInfo &transform,
                                      const QVector3D &position,
                                      float panDegrees,
                                      float tiltDegrees,
                                      bool setBase)
{
    if (setBase)
    {
        for (int i = record.firstMesh; i < record.firstMesh + record.meshCount; ++i)
            meshes[i].baseModelMatrix = meshes[i].modelMatrix;
    }

    if (!record.pivotsValid)
    {
        record.armMesh = -1;
        record.headMesh = -1;
        for (int i = record.firstMesh; i < record.firstMesh + record.meshCount; ++i)
        {
            const MovingHeadPart part = movingHeadPartForName(meshes[i].name);
            if (part == MovingHeadPart::Arm && record.armMesh < 0)
                record.armMesh = i;
            if (part == MovingHeadPart::Head && record.headMesh < 0)
                record.headMesh = i;
        }
        if (record.armMesh >= 0)
            record.armPivot = meshes[record.armMesh].baseModelMatrix.column(3).toVector3D();
        else
            record.armPivot = QVector3D(0.0f, 0.0f, 0.0f);
        if (record.headMesh >= 0)
            record.headPivot = meshes[record.headMesh].baseModelMatrix.column(3).toVector3D();
        else
            record.headPivot = QVector3D(0.0f, 0.0f, 0.0f);
        record.pivotsValid = true;
    }

    const QQuaternion panRot = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, panDegrees);
    const QVector3D tiltAxis = panRot.rotatedVector(QVector3D(1.0f, 0.0f, 0.0f));
    const QQuaternion tiltRot = QQuaternion::fromAxisAndAngle(tiltAxis, tiltDegrees);
    const QMatrix4x4 panMatrix = rotateAroundPivot(record.armPivot, panRot);
    const QVector3D headPivotRotated = (panMatrix * QVector4D(record.headPivot, 1.0f)).toVector3D();
    const QMatrix4x4 tiltMatrix = rotateAroundPivot(headPivotRotated, tiltRot);

    for (int i = record.firstMesh; i < record.firstMesh + record.meshCount; ++i)
    {
        Mesh &mesh = meshes[i];
        const MovingHeadPart part = movingHeadPartForName(mesh.name);
        QMatrix4x4 local = mesh.baseModelMatrix;
        if (part == MovingHeadPart::Arm)
            local = panMatrix * local;
        else if (part == MovingHeadPart::Head || part == MovingHeadPart::HeadChild)
            local = tiltMatrix * (panMatrix * local);
        mesh.modelMatrix = transform.matrix * local;
        mesh.userOffset = position;
        mesh.modelDirty = true;
        mesh.worldBoundsDirty = true;
        if (mesh.selected)
            mesh.selectionDirty = true;
    }
}

static int hideEmitterMeshes(QVector<Mesh> &meshes, int firstMesh, int meshCount)
{
    int hiddenCount = 0;
    for (int i = firstMesh; i < firstMesh + meshCount; ++i)
    {
        Mesh &mesh = meshes[i];
        if (isEmitterName(mesh.name))
        {
            mesh.visible = false;
            mesh.selectable = false;
            ++hiddenCount;
        }
    }
    return hiddenCount;
}

static MeshItem *pickHitForRecords(const QVector<MeshRecord> &records, int meshIndex)
{
    for (const auto &record : records)
    {
        if (meshIndex >= record.firstMesh && meshIndex < record.firstMesh + record.meshCount)
            return const_cast<MeshItem *>(record.item);
    }
    return nullptr;
}

static void pruneMissingRecords(QVector<MeshRecord> &records,
                                const QList<MeshItem *> &liveItems,
                                QVector<Mesh> &meshes)
{
    QSet<const MeshItem *> live;
    live.reserve(liveItems.size());
    for (const MeshItem *item : liveItems)
        live.insert(item);

    for (int i = records.size() - 1; i >= 0; --i)
    {
        auto &record = records[i];
        if (live.contains(record.item))
            continue;
        applySelectionVisibility(meshes, record.firstMesh, record.meshCount, false, false, false);
        if (record.videoTexture)
        {
            delete record.videoTexture;
            record.videoTexture = nullptr;
        }
        records.removeAt(i);
    }
}

class RhiQmlItemRenderer final : public QQuickRhiItemRenderer
{
public:
    enum DragType
    {
        DragBegin = 0,
        DragMove = 1,
        DragEnd = 2
    };

    void initialize(QRhiCommandBuffer *cb) override
    {
        Q_UNUSED(cb);
        if (m_initialized)
            return;

        if (!m_rhiContext.initializeExternal(rhi()))
        {
            qWarning() << "RhiQmlItemRenderer: failed to initialize external RHI context";
            return;
        }

        m_targets = std::make_unique<RenderTargetCache>(rhi());
        m_shaders = std::make_unique<ShaderManager>(rhi());
        m_renderer.initialize(&m_rhiContext, m_targets.get(), m_shaders.get());
        m_initialized = true;
    }

    void synchronize(QQuickRhiItem *item) override
    {
        auto *qmlItem = static_cast<RhiQmlItem *>(item);
        struct LightTransform
        {
            QMatrix4x4 matrix;
            QQuaternion rotation;
        };
        QHash<const LightItem *, LightTransform> staticLightTransforms;
        QHash<const StaticLightItem *, LightTransform> staticLightItemTransforms;
        QHash<const StaticLightItem *, QVector<EmitterData>> staticLightEmitters;
        QHash<const MovingHeadItem *, QVector<EmitterData>> movingHeadEmitters;
        QHash<const BeamBarItem *, QVector<Light>> beamBarLights;
        const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();

        QVector<RhiQmlItem::PendingModel> models;
        QVector<QObject *> selectableItems;
        qmlItem->takePendingModels(models);
        if (!models.isEmpty())
        {
            qDebug() << "RhiQmlItemRenderer: processing pending models:" << models.size()
                     << "scene mesh count before:" << m_scene.meshes().size();
        }
        for (const auto &entry : models)
        {
            const int beforeCount = m_scene.meshes().size();
            if (!m_loader.loadModel(entry.path, m_scene, true))
            {
                qWarning() << "RhiQmlItemRenderer: failed to load model" << entry.path;
                continue;
            }
            for (int i = beforeCount; i < m_scene.meshes().size(); ++i)
            {
                Mesh &mesh = m_scene.meshes()[i];
                mesh.baseModelMatrix = mesh.modelMatrix;
                QMatrix4x4 transform;
                transform.translate(entry.position);
                if (!entry.rotationDegrees.isNull())
                {
                    const QQuaternion rot = rotationFromAxesOrderXYZ(entry.rotationDegrees);
                    transform.rotate(rot);
                }
                if (entry.scale != QVector3D(1.0f, 1.0f, 1.0f))
                    transform.scale(entry.scale);
                mesh.modelMatrix = transform * mesh.baseModelMatrix;
                mesh.userOffset = entry.position;
                mesh.modelDirty = true;
                mesh.worldBoundsDirty = true;
                if (!mesh.vertices.isEmpty())
                {
                    QVector3D minV(mesh.vertices[0].px, mesh.vertices[0].py, mesh.vertices[0].pz);
                    QVector3D maxV = minV;
                    for (const Vertex &v : mesh.vertices)
                    {
                        minV.setX(qMin(minV.x(), v.px));
                        minV.setY(qMin(minV.y(), v.py));
                        minV.setZ(qMin(minV.z(), v.pz));
                        maxV.setX(qMax(maxV.x(), v.px));
                        maxV.setY(qMax(maxV.y(), v.py));
                        maxV.setZ(qMax(maxV.z(), v.pz));
                    }
                }
            }
        }

        QVector<Light> lights;
        qmlItem->takePendingLights(lights);
        for (const Light &light : lights)
            m_staticLights.push_back(light);

        const auto qmlMeshItems = qmlItem->findChildren<MeshItem *>(QString(), Qt::FindChildrenRecursively);
        pruneMissingRecords(m_qmlMeshes, qmlMeshItems, m_scene.meshes());

        QSet<const MeshItem *> liveItems;
        liveItems.reserve(qmlMeshItems.size());
        for (const MeshItem *itemPtr : qmlMeshItems)
            liveItems.insert(itemPtr);
        for (auto it = m_loggedCreatedItems.begin(); it != m_loggedCreatedItems.end(); )
        {
            if (!liveItems.contains(*it))
                it = m_loggedCreatedItems.erase(it);
            else
                ++it;
        }

        if (qmlMeshItems.isEmpty())
        {
            if (!m_warnedNoQmlMeshItems)
            {
                m_warnedNoQmlMeshItems = true;
                qWarning() << "RhiQmlItemRenderer: no MeshItem instances found in scene graph";
            }
        }
        else
        {
            m_warnedNoQmlMeshItems = false;
        }

        if (m_lastSyncSummaryMs == 0 || (nowMs - m_lastSyncSummaryMs) >= 1500)
        {
            int staticCount = 0;
            int movingCount = 0;
            int modelCount = 0;
            for (const MeshItem *meshItem : qmlMeshItems)
            {
                switch (meshItem->type())
                {
                    case MeshItem::MeshType::StaticLight:
                        ++staticCount;
                    break;
                    case MeshItem::MeshType::MovingHead:
                        ++movingCount;
                    break;
                    case MeshItem::MeshType::Model:
                        ++modelCount;
                    break;
                    default:
                    break;
                }
            }
#if 0
            qDebug() << "RhiQmlItemRenderer: sync summary"
                     << "qmlItems" << qmlMeshItems.size()
                     << "models" << modelCount
                     << "staticLights" << staticCount
                     << "movingHeads" << movingCount
                     << "records" << m_qmlMeshes.size()
                     << "sceneMeshes" << m_scene.meshes().size()
                     << "pendingModels" << models.size();
#endif
            m_lastSyncSummaryMs = nowMs;
        }

        auto findRecord = [&](const MeshItem *item) -> MeshRecord * {
            for (MeshRecord &record : m_qmlMeshes)
            {
                if (record.item == item)
                    return &record;
            }
            return nullptr;
        };

        for (MeshItem *meshItem : qmlMeshItems)
        {
            const MeshItem::MeshType type = meshItem->type();
            MeshRecord *record = findRecord(meshItem);
            QString path;
            bool needsPath = false;

            if (type == MeshItem::MeshType::Model)
            {
                const ModelItem *modelItem = qobject_cast<ModelItem *>(meshItem);
                if (!modelItem)
                {
                    qWarning() << "RhiQmlItemRenderer: failed to cast mesh item to ModelItem" << meshItem;
                    continue;
                }
                path = modelItem->path();
                needsPath = true;
            }
            else if (type == MeshItem::MeshType::StaticLight)
            {
                const StaticLightItem *staticLight = qobject_cast<StaticLightItem *>(meshItem);
                if (!staticLight)
                {
                    qWarning() << "RhiQmlItemRenderer: failed to cast mesh item to StaticLightItem" << meshItem;
                    continue;
                }
                path = staticLight->path();
                needsPath = true;
            }
            else if (type == MeshItem::MeshType::MovingHead)
            {
                const MovingHeadItem *movingHead = qobject_cast<MovingHeadItem *>(meshItem);
                if (!movingHead)
                {
                    qWarning() << "RhiQmlItemRenderer: failed to cast mesh item to MovingHeadItem" << meshItem;
                    continue;
                }
                path = movingHead->path();
                needsPath = true;
            }

            if (needsPath && path.isEmpty())
            {
                if (!m_warnedEmptyPathItems.contains(meshItem))
                {
                    m_warnedEmptyPathItems.insert(meshItem);
                    qWarning() << "RhiQmlItemRenderer: mesh item has empty model path"
                               << meshTypeName(type) << meshItem;
                }
                continue;
            }
            m_warnedEmptyPathItems.remove(meshItem);

            if (needsPath)
            {
                const QFileInfo modelInfo(localPathForFileCheck(path));
                if (!modelInfo.exists())
                {
                    if (!m_warnedMissingModelPaths.contains(path))
                    {
                        m_warnedMissingModelPaths.insert(path);
                        qWarning() << "RhiQmlItemRenderer: model path does not exist" << path
                                   << "for" << meshTypeName(type);
                    }
                }
            }

            if (meshItem->selectable())
                selectableItems.push_back(meshItem);

            if (record)
            {
                record->type = type;
                if (needsPath && record->path != path)
                {
                    if (type == MeshItem::MeshType::Model)
                        qWarning() << "RhiQmlItemRenderer: model path changed after load for" << path;
                    else if (type == MeshItem::MeshType::StaticLight)
                        qWarning() << "RhiQmlItemRenderer: static light path changed after load for" << path;
                    else if (type == MeshItem::MeshType::MovingHead)
                        qWarning() << "RhiQmlItemRenderer: moving head path changed after load for" << path;
                    continue;
                }
                applySelectionGroup(m_scene.meshes(), record->firstMesh, record->meshCount, record->firstMesh);
                applySelectionDomain(m_scene.meshes(), record->firstMesh, record->meshCount,
                                     selectionDomainForMeshType(record->type));

                if (type == MeshItem::MeshType::Model)
                {
                    syncTransformFromItem(*record, meshItem, m_scene.meshes());
                    syncSelectionVisibilityFromItem(*record, meshItem, m_scene.meshes());
                }
                else if (type == MeshItem::MeshType::StaticLight)
                {
                    syncTransformFromItem(*record, meshItem, m_scene.meshes());
                    syncSelectionVisibilityFromItem(*record, meshItem, m_scene.meshes());
                    const int hiddenCount = hideEmitterMeshes(m_scene.meshes(), record->firstMesh, record->meshCount);
                    if (hiddenCount >= record->meshCount && record->meshCount > 0)
                    {
                        qWarning() << "RhiQmlItemRenderer: all meshes hidden as emitters for static light"
                                   << record->path
                                   << "(meshCount:" << record->meshCount << ")";
                    }
                    const StaticLightItem *staticLight = static_cast<const StaticLightItem *>(meshItem);
                    staticLightEmitters.insert(staticLight,
                                               collectEmitters(m_scene.meshes(), record->firstMesh, record->meshCount));
                    const TransformInfo transform = transformFromRecord(*record);
                    staticLightItemTransforms.insert(staticLight, { transform.matrix, transform.rotation });
                    const auto lights = staticLight->findChildren<LightItem *>(QString(), Qt::FindChildrenRecursively);
                    for (const LightItem *lightItem : lights)
                        staticLightTransforms.insert(lightItem, { transform.matrix, transform.rotation });
                }
                else if (type == MeshItem::MeshType::MovingHead)
                {
                    const MovingHeadItem *movingHead = static_cast<const MovingHeadItem *>(meshItem);
                    const bool transformDirty = syncTransformFromItem(*record, meshItem, m_scene.meshes());
                    syncSelectionVisibilityFromItem(*record, meshItem, m_scene.meshes());
                    const float pan = movingHead->pan();
                    const float tilt = movingHead->tilt();
                    if (transformDirty || !qFuzzyCompare(record->pan, pan) || !qFuzzyCompare(record->tilt, tilt))
                    {
                        record->pan = pan;
                        record->tilt = tilt;
                        const TransformInfo transform = transformFromRecord(*record);
                        applyMovingHeadTransforms(*record, m_scene.meshes(), transform,
                                                  record->position, record->pan, record->tilt, false);
                    }
                    const int hiddenCount = hideEmitterMeshes(m_scene.meshes(), record->firstMesh, record->meshCount);
                    if (hiddenCount >= record->meshCount && record->meshCount > 0)
                    {
                        qWarning() << "RhiQmlItemRenderer: all meshes hidden as emitters for moving head"
                                   << record->path
                                   << "(meshCount:" << record->meshCount << ")";
                    }
                    QVector3D emitterDirectionOverride;
                    if (record->headMesh >= 0
                            && record->headMesh < m_scene.meshes().size())
                    {
                        const QVector3D axis = -m_scene.meshes().at(record->headMesh).modelMatrix.column(1).toVector3D();
                        const QQuaternion baseRot = rotationFromAxesOrderXYZ(record->rotationDegrees);
                        const QQuaternion panRot = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, record->pan);
                        const QVector3D tiltAxis = panRot.rotatedVector(QVector3D(1.0f, 0.0f, 0.0f));
                        const QQuaternion tiltRot = QQuaternion::fromAxisAndAngle(tiltAxis, record->tilt);
                        QVector3D logicalDir = (baseRot * (tiltRot * panRot)).rotatedVector(QVector3D(0.0f, -1.0f, 0.0f));
                        if (!logicalDir.isNull())
                        {
                            logicalDir.normalize();
                            emitterDirectionOverride = logicalDir;
                        }
                        else if (!axis.isNull())
                        {
                            emitterDirectionOverride = axis.normalized();
                        }
                    }
                    QVector<EmitterData> emitters = collectEmitters(m_scene.meshes(),
                                                                    record->firstMesh,
                                                                    record->meshCount,
                                                                    emitterDirectionOverride);
                    movingHeadEmitters.insert(movingHead, emitters);
                }
                else if (type == MeshItem::MeshType::Cube)
                {
                    const CubeItem *cubeItem = static_cast<const CubeItem *>(meshItem);
                    syncTransformFromItem(*record, meshItem, m_scene.meshes());
                    const QVector3D baseColor = cubeItem->baseColor();
                    const QVector3D emissiveColor = cubeItem->emissiveColor();
                    const float metalness = cubeItem->metalness();
                    const float roughness = cubeItem->roughness();
                    if (record->baseColor != baseColor || record->emissiveColor != emissiveColor
                            || !qFuzzyCompare(record->metalness, metalness)
                            || !qFuzzyCompare(record->roughness, roughness))
                    {
                        record->baseColor = baseColor;
                        record->emissiveColor = emissiveColor;
                        record->metalness = metalness;
                        record->roughness = roughness;
                        applyMaterial(m_scene.meshes(), record->firstMesh, record->meshCount,
                                      baseColor, emissiveColor, metalness, roughness);
                    }
                    syncSelectionVisibilityFromItem(*record, meshItem, m_scene.meshes());
                }
                else if (type == MeshItem::MeshType::Sphere)
                {
                    const SphereItem *sphereItem = static_cast<const SphereItem *>(meshItem);
                    syncTransformFromItem(*record, meshItem, m_scene.meshes());
                    const QVector3D baseColor = sphereItem->baseColor();
                    const QVector3D emissiveColor = sphereItem->emissiveColor();
                    const float metalness = sphereItem->metalness();
                    const float roughness = sphereItem->roughness();
                    if (record->baseColor != baseColor || record->emissiveColor != emissiveColor
                            || !qFuzzyCompare(record->metalness, metalness)
                            || !qFuzzyCompare(record->roughness, roughness))
                    {
                        record->baseColor = baseColor;
                        record->emissiveColor = emissiveColor;
                        record->metalness = metalness;
                        record->roughness = roughness;
                        applyMaterial(m_scene.meshes(), record->firstMesh, record->meshCount,
                                      baseColor, emissiveColor, metalness, roughness);
                    }
                    syncSelectionVisibilityFromItem(*record, meshItem, m_scene.meshes());
                }
                else if (type == MeshItem::MeshType::Video)
                {
                    VideoItem *videoItem = qobject_cast<VideoItem *>(meshItem);
                    if (!videoItem)
                        continue;
                    syncTransformFromItem(*record, meshItem, m_scene.meshes());
                    syncSelectionVisibilityFromItem(*record, meshItem, m_scene.meshes());
                    const QVector3D baseColor(0.0f, 0.0f, 0.0f);
                    const QVector3D emissive(1.0f, 1.0f, 1.0f);
                    if (record->baseColor != baseColor || record->emissiveColor != emissive
                            || !qFuzzyCompare(record->metalness, 0.0f)
                            || !qFuzzyCompare(record->roughness, 1.0f))
                    {
                        record->baseColor = baseColor;
                        record->emissiveColor = emissive;
                        record->metalness = 0.0f;
                        record->roughness = 1.0f;
                        applyMaterial(m_scene.meshes(), record->firstMesh, record->meshCount,
                                      record->baseColor, record->emissiveColor,
                                      record->metalness, record->roughness);
                    }
                    QImage frame;
                    if (videoItem->takeFrame(frame))
                    {
                        record->videoFrame = frame;
                        record->videoDirty = true;
                    }
                }
                else if (type == MeshItem::MeshType::PixelBar)
                {
                    const PixelBarItem *pixelBar = static_cast<const PixelBarItem *>(meshItem);
                    const int nextCount = qMax(1, pixelBar->emitterCount());
                    record->emitterCount = record->meshCount > 0
                            ? qMin(nextCount, record->meshCount - 1)
                            : 1;
                    record->baseColor = pixelBar->baseColor();
                    record->emissiveColor = pixelBar->emissiveColor();
                    record->emitterColors = pixelBar->emitterColorsVector();
                    record->emitterIntensities = pixelBar->emitterIntensitiesVector();
                    syncCommonFields(*record, meshItem);
                    syncSelectionVisibilityFromItem(*record, meshItem, m_scene.meshes());
                    applyPixelBarLayout(*record, m_scene.meshes());
                }
                else if (type == MeshItem::MeshType::BeamBar)
                {
                    const BeamBarItem *beamBar = static_cast<const BeamBarItem *>(meshItem);
                    record->emitterCount = qMax(1, beamBar->emitterCount());
                    record->headsLayout = beamBar->headsLayout();
                    record->baseColor = beamBar->baseColor();
                    record->lightColor = beamBar->color();
                    record->intensity = beamBar->intensity();
                    record->range = beamBar->range();
                    record->beamRadius = beamBar->beamRadius();
                    record->castShadows = beamBar->castShadows();
                    record->pan = beamBar->pan();
                    record->tilt = beamBar->tilt();
                    record->emitterColors = beamBar->emitterColorsVector();
                    record->emitterIntensities = beamBar->emitterIntensitiesVector();
                    syncCommonFields(*record, meshItem);
                    syncSelectionVisibilityFromItem(*record, meshItem, m_scene.meshes());
                    applyBeamBarBody(*record, m_scene.meshes());

                    QVector<Light> lights;
                    lights.reserve(record->emitterCount);
                    int cols = qMax(1, record->headsLayout.width());
                    int rows = qMax(1, record->headsLayout.height());
                    if (cols * rows < qMax(1, record->emitterCount))
                    {
                        cols = qMax(1, record->emitterCount);
                        rows = 1;
                    }
                    const float length = qMax(0.001f, 0.1f * float(cols) * record->scale.x());
                    const float depth = qMax(0.001f, 0.1f * float(rows) * record->scale.z());
                    const float segmentX = length / float(cols);
                    const float segmentZ = depth / float(rows);
                    const float startX = -length * 0.5f + segmentX * 0.5f;
                    const float startZ = -depth * 0.5f + segmentZ * 0.5f;
                    const QQuaternion baseRot = rotationFromAxesOrderXYZ(record->rotationDegrees);
                    const QQuaternion panRot = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, record->pan);
                    const QVector3D tiltAxis = panRot.rotatedVector(QVector3D(1.0f, 0.0f, 0.0f));
                    const QQuaternion tiltRot = QQuaternion::fromAxisAndAngle(tiltAxis, record->tilt);
                    const QQuaternion headRot = tiltRot * panRot;
                    const QQuaternion lightRot = baseRot * headRot;
                    for (int i = 0; i < record->emitterCount; ++i)
                    {
                        const int col = i % cols;
                        const int row = i / cols;
                        const QVector3D localPos(startX + float(col) * segmentX,
                                                 0.0f,
                                                 startZ + float(row) * segmentZ);
                        Light light = beamBar->toLight();
                        const QVector3D color = (i < record->emitterColors.size()) ? record->emitterColors[i] : record->lightColor;
                        const float intensity = (i < record->emitterIntensities.size()) ? record->emitterIntensities[i] : record->intensity;
                        const QVector3D worldPos = record->position
                                + baseRot.rotatedVector(headRot.rotatedVector(localPos));
                        light.position = worldPos;
                        light.direction = lightRot.rotatedVector(QVector3D(0.0f, -1.0f, 0.0f)).normalized();
                        light.color = color;
                        light.intensity = intensity;
                        light.range = record->range;
                        light.beamRadius = record->beamRadius;
                        light.castShadows = record->castShadows;
                        lights.push_back(light);
                    }
                    beamBarLights.insert(beamBar, lights);
                }
                continue;
            }

            const int beforeCount = m_scene.meshes().size();
            bool created = false;

            if (type == MeshItem::MeshType::Model
                    || type == MeshItem::MeshType::StaticLight
                    || type == MeshItem::MeshType::MovingHead)
            {
                if (!m_loader.loadModel(path, m_scene, true))
                {
                    if (type == MeshItem::MeshType::Model)
                        qWarning() << "RhiQmlItemRenderer: failed to load model" << path;
                    else if (type == MeshItem::MeshType::StaticLight)
                        qWarning() << "RhiQmlItemRenderer: failed to load static light model" << path;
                    else if (type == MeshItem::MeshType::MovingHead)
                        qWarning() << "RhiQmlItemRenderer: failed to load moving head model" << path;
                    continue;
                }
                created = true;
            }
            else if (type == MeshItem::MeshType::Cube)
            {
                m_scene.meshes().push_back(createUnitCubeMesh());
                created = true;
            }
            else if (type == MeshItem::MeshType::Sphere)
            {
                m_scene.meshes().push_back(createSphereMesh());
                created = true;
            }
            else if (type == MeshItem::MeshType::PixelBar)
            {
                const PixelBarItem *pixelBar = static_cast<const PixelBarItem *>(meshItem);
                const int emitters = qMax(0, pixelBar->emitterCount());
                for (int i = 0; i < emitters + 1; ++i)
                    m_scene.meshes().push_back(createUnitCubeMesh());
                created = true;
            }
            else if (type == MeshItem::MeshType::BeamBar)
            {
                m_scene.meshes().push_back(createUnitCubeMesh());
                created = true;
            }
            else if (type == MeshItem::MeshType::Video)
            {
                m_scene.meshes().push_back(createUnitQuadMesh());
                m_scene.meshes().last().name = QStringLiteral("VideoQuad");
                created = true;
            }

            if (!created)
                continue;

            const int meshCount = m_scene.meshes().size() - beforeCount;
            if (meshCount <= 0)
            {
                qWarning() << "RhiQmlItemRenderer: mesh item created with zero meshes"
                           << meshTypeName(type)
                           << "path" << path;
                continue;
            }
            MeshRecord newRecord;
            newRecord.item = meshItem;
            newRecord.type = type;
            newRecord.path = path;
            initCommonRecord(newRecord, meshItem);
            newRecord.firstMesh = beforeCount;
            newRecord.meshCount = meshCount;

            if (type == MeshItem::MeshType::StaticLight
                    || type == MeshItem::MeshType::MovingHead)
            {
                applyPhysicalScaleFromItem(newRecord, meshItem, m_scene.meshes());
            }

            if (!m_loggedCreatedItems.contains(meshItem))
            {
                m_loggedCreatedItems.insert(meshItem);
                qDebug() << "RhiQmlItemRenderer: created mesh record"
                         << meshTypeName(type)
                         << "item" << meshItem
                         << "path" << path
                         << "firstMesh" << newRecord.firstMesh
                         << "meshCount" << newRecord.meshCount;
            }

            if (type == MeshItem::MeshType::MovingHead)
            {
                const MovingHeadItem *movingHead = static_cast<const MovingHeadItem *>(meshItem);
                newRecord.pan = movingHead->pan();
                newRecord.tilt = movingHead->tilt();
                const TransformInfo transform = transformFromRecord(newRecord);
                applyMovingHeadTransforms(newRecord, m_scene.meshes(), transform,
                                          newRecord.position, newRecord.pan, newRecord.tilt, true);
                applySelectionVisibility(m_scene.meshes(), newRecord.firstMesh, newRecord.meshCount,
                                         newRecord.selected, newRecord.selectable, newRecord.visible);
                applySelectionGroup(m_scene.meshes(), newRecord.firstMesh, newRecord.meshCount, newRecord.firstMesh);
                applySelectionDomain(m_scene.meshes(), newRecord.firstMesh, newRecord.meshCount,
                                     selectionDomainForMeshType(newRecord.type));
                const int hiddenCount = hideEmitterMeshes(m_scene.meshes(), newRecord.firstMesh, newRecord.meshCount);
                if (hiddenCount >= newRecord.meshCount && newRecord.meshCount > 0)
                {
                    qWarning() << "RhiQmlItemRenderer: all meshes hidden as emitters for new moving head"
                               << newRecord.path
                               << "(meshCount:" << newRecord.meshCount << ")";
                }
                QVector3D emitterDirectionOverride;
                if (newRecord.headMesh >= 0
                        && newRecord.headMesh < m_scene.meshes().size())
                {
                    const QVector3D axis = -m_scene.meshes().at(newRecord.headMesh).modelMatrix.column(1).toVector3D();
                    const QQuaternion baseRot = rotationFromAxesOrderXYZ(newRecord.rotationDegrees);
                    const QQuaternion panRot = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, newRecord.pan);
                    const QVector3D tiltAxis = panRot.rotatedVector(QVector3D(1.0f, 0.0f, 0.0f));
                    const QQuaternion tiltRot = QQuaternion::fromAxisAndAngle(tiltAxis, newRecord.tilt);
                    QVector3D logicalDir = (baseRot * (tiltRot * panRot)).rotatedVector(QVector3D(0.0f, -1.0f, 0.0f));
                    if (!logicalDir.isNull())
                    {
                        logicalDir.normalize();
                        emitterDirectionOverride = logicalDir;
                    }
                    else if (!axis.isNull())
                    {
                        emitterDirectionOverride = axis.normalized();
                    }
                }
                QVector<EmitterData> emitters = collectEmitters(m_scene.meshes(),
                                                                newRecord.firstMesh,
                                                                newRecord.meshCount,
                                                                emitterDirectionOverride);
                movingHeadEmitters.insert(static_cast<const MovingHeadItem *>(meshItem), emitters);
            }
            else
            {
                TransformInfo transform;
                const bool useCommonTransform = (type == MeshItem::MeshType::Model
                                                 || type == MeshItem::MeshType::StaticLight
                                                 || type == MeshItem::MeshType::Cube
                                                 || type == MeshItem::MeshType::Sphere
                                                 || type == MeshItem::MeshType::Video);
                if (useCommonTransform)
                    transform = applyCommonRecordTransforms(newRecord, m_scene.meshes(), true);
                if (type == MeshItem::MeshType::StaticLight)
                {
                    const StaticLightItem *staticLight = static_cast<const StaticLightItem *>(meshItem);
                    const int hiddenCount = hideEmitterMeshes(m_scene.meshes(), newRecord.firstMesh, newRecord.meshCount);
                    if (hiddenCount >= newRecord.meshCount && newRecord.meshCount > 0)
                    {
                        qWarning() << "RhiQmlItemRenderer: all meshes hidden as emitters for new static light"
                                   << newRecord.path
                                   << "(meshCount:" << newRecord.meshCount << ")";
                    }
                    const QVector<EmitterData> emitters = collectEmitters(m_scene.meshes(), newRecord.firstMesh, newRecord.meshCount);
                    staticLightEmitters.insert(staticLight, emitters);
                    staticLightItemTransforms.insert(staticLight, { transform.matrix, transform.rotation });
                    const auto lights = staticLight->findChildren<LightItem *>(QString(), Qt::FindChildrenRecursively);
                    for (const LightItem *lightItem : lights)
                        staticLightTransforms.insert(lightItem, { transform.matrix, transform.rotation });
                }
                else if (type == MeshItem::MeshType::Cube)
                {
                    const CubeItem *cubeItem = static_cast<const CubeItem *>(meshItem);
                    newRecord.baseColor = cubeItem->baseColor();
                    newRecord.emissiveColor = cubeItem->emissiveColor();
                    newRecord.metalness = cubeItem->metalness();
                    newRecord.roughness = cubeItem->roughness();
                    applyMaterial(m_scene.meshes(), newRecord.firstMesh, newRecord.meshCount,
                                  newRecord.baseColor, newRecord.emissiveColor,
                                  newRecord.metalness, newRecord.roughness);
                }
                else if (type == MeshItem::MeshType::Sphere)
                {
                    const SphereItem *sphereItem = static_cast<const SphereItem *>(meshItem);
                    newRecord.baseColor = sphereItem->baseColor();
                    newRecord.emissiveColor = sphereItem->emissiveColor();
                    newRecord.metalness = sphereItem->metalness();
                    newRecord.roughness = sphereItem->roughness();
                    applyMaterial(m_scene.meshes(), newRecord.firstMesh, newRecord.meshCount,
                                  newRecord.baseColor, newRecord.emissiveColor,
                                  newRecord.metalness, newRecord.roughness);
                }
                else if (type == MeshItem::MeshType::Video)
                {
                    VideoItem *videoItem = qobject_cast<VideoItem *>(meshItem);
                    if (videoItem)
                    {
                        newRecord.baseColor = QVector3D(1.0f, 1.0f, 1.0f);
                        newRecord.emissiveColor = QVector3D(1.0f, 1.0f, 1.0f);
                        newRecord.metalness = 0.0f;
                        newRecord.roughness = 1.0f;
                        applyMaterial(m_scene.meshes(), newRecord.firstMesh, newRecord.meshCount,
                                      newRecord.baseColor, newRecord.emissiveColor,
                                      newRecord.metalness, newRecord.roughness);
                        QImage frame;
                        if (videoItem->takeFrame(frame))
                        {
                            newRecord.videoFrame = frame;
                            newRecord.videoDirty = true;
                        }
                    }
                }
                else if (type == MeshItem::MeshType::PixelBar)
                {
                    const PixelBarItem *pixelBar = static_cast<const PixelBarItem *>(meshItem);
                    newRecord.emitterCount = qMax(1, pixelBar->emitterCount());
                    newRecord.baseColor = pixelBar->baseColor();
                    newRecord.emissiveColor = pixelBar->emissiveColor();
                    newRecord.emitterColors = pixelBar->emitterColorsVector();
                    newRecord.emitterIntensities = pixelBar->emitterIntensitiesVector();
                    applyPixelBarLayout(newRecord, m_scene.meshes());
                    applySelectionVisibility(m_scene.meshes(), newRecord.firstMesh, newRecord.meshCount,
                                             newRecord.selected, newRecord.selectable, newRecord.visible);
                    applySelectionGroup(m_scene.meshes(), newRecord.firstMesh, newRecord.meshCount, newRecord.firstMesh);
                    applySelectionDomain(m_scene.meshes(), newRecord.firstMesh, newRecord.meshCount,
                                         selectionDomainForMeshType(newRecord.type));
                }
                else if (type == MeshItem::MeshType::BeamBar)
                {
                    const BeamBarItem *beamBar = static_cast<const BeamBarItem *>(meshItem);
                    newRecord.emitterCount = qMax(1, beamBar->emitterCount());
                    newRecord.headsLayout = beamBar->headsLayout();
                    newRecord.baseColor = beamBar->baseColor();
                    newRecord.lightColor = beamBar->color();
                    newRecord.intensity = beamBar->intensity();
                    newRecord.range = beamBar->range();
                    newRecord.beamRadius = beamBar->beamRadius();
                    newRecord.castShadows = beamBar->castShadows();
                    newRecord.pan = beamBar->pan();
                    newRecord.tilt = beamBar->tilt();
                    newRecord.emitterColors = beamBar->emitterColorsVector();
                    newRecord.emitterIntensities = beamBar->emitterIntensitiesVector();
                    applyBeamBarBody(newRecord, m_scene.meshes());
                    applySelectionVisibility(m_scene.meshes(), newRecord.firstMesh, newRecord.meshCount,
                                             newRecord.selected, newRecord.selectable, newRecord.visible);
                    applySelectionGroup(m_scene.meshes(), newRecord.firstMesh, newRecord.meshCount, newRecord.firstMesh);
                    applySelectionDomain(m_scene.meshes(), newRecord.firstMesh, newRecord.meshCount,
                                         selectionDomainForMeshType(newRecord.type));

                    QVector<Light> lights;
                    lights.reserve(newRecord.emitterCount);
                    int cols = qMax(1, newRecord.headsLayout.width());
                    int rows = qMax(1, newRecord.headsLayout.height());
                    if (cols * rows < qMax(1, newRecord.emitterCount))
                    {
                        cols = qMax(1, newRecord.emitterCount);
                        rows = 1;
                    }
                    const float length = qMax(0.001f, 0.1f * float(cols) * newRecord.scale.x());
                    const float depth = qMax(0.001f, 0.1f * float(rows) * newRecord.scale.z());
                    const float segmentX = length / float(cols);
                    const float segmentZ = depth / float(rows);
                    const float startX = -length * 0.5f + segmentX * 0.5f;
                    const float startZ = -depth * 0.5f + segmentZ * 0.5f;
                    const QQuaternion baseRot = rotationFromAxesOrderXYZ(newRecord.rotationDegrees);
                    const QQuaternion panRot = QQuaternion::fromAxisAndAngle(0.0f, 1.0f, 0.0f, newRecord.pan);
                    const QVector3D tiltAxis = panRot.rotatedVector(QVector3D(1.0f, 0.0f, 0.0f));
                    const QQuaternion tiltRot = QQuaternion::fromAxisAndAngle(tiltAxis, newRecord.tilt);
                    const QQuaternion headRot = tiltRot * panRot;
                    const QQuaternion lightRot = baseRot * headRot;
                    for (int i = 0; i < newRecord.emitterCount; ++i)
                    {
                        const int col = i % cols;
                        const int row = i / cols;
                        const QVector3D localPos(startX + float(col) * segmentX,
                                                 0.0f,
                                                 startZ + float(row) * segmentZ);
                        Light light = beamBar->toLight();
                        const QVector3D color = (i < newRecord.emitterColors.size()) ? newRecord.emitterColors[i] : newRecord.lightColor;
                        const float intensity = (i < newRecord.emitterIntensities.size()) ? newRecord.emitterIntensities[i] : newRecord.intensity;
                        const QVector3D worldPos = newRecord.position
                                + baseRot.rotatedVector(headRot.rotatedVector(localPos));
                        light.position = worldPos;
                        light.direction = lightRot.rotatedVector(QVector3D(0.0f, -1.0f, 0.0f)).normalized();
                        light.color = color;
                        light.intensity = intensity;
                        lights.push_back(light);
                    }
                    beamBarLights.insert(beamBar, lights);
                }
            }

            m_qmlMeshes.push_back(newRecord);
        }

        qmlItem->updateSelectableItems(selectableItems);

        QVector<Light> newLights;
        newLights.reserve(m_staticLights.size() + staticLightItemTransforms.size());
        newLights = m_staticLights;
        QSet<quint64> seenEmitterDebugKeys;
        auto logEmitterDebug = [this, &seenEmitterDebugKeys](const char *fixtureKind,
                                                             const void *fixturePtr,
                                                             int emitterIndex,
                                                             const EmitterData &emitter,
                                                             const Light &light,
                                                             float itemTopRadius)
        {
            const quint64 ptrKey = quint64(reinterpret_cast<quintptr>(fixturePtr));
            const quint64 key = (ptrKey * 1315423911ULL)
                    ^ (quint64(emitterIndex & 0xFFFF) << 1)
                    ^ quint64((fixtureKind && fixtureKind[0] == 'm') ? 1 : 0);
            seenEmitterDebugKeys.insert(key);

            const int mode = (light.beamShape == Light::BeamShapeType::BeamShape) ? 1 : 0;
            const BeamDebugState prev = m_beamDebugState.value(key);
            const bool changed = (prev.mode != mode)
                    || !qFuzzyCompare(prev.emitterDiameter + 1.0f, emitter.diameter + 1.0f)
                    || !qFuzzyCompare(prev.topRadius + 1.0f, light.beamRadius + 1.0f);
            if (!changed)
                return;

            BeamDebugState next;
            next.emitterDiameter = emitter.diameter;
            next.topRadius = light.beamRadius;
            next.mode = mode;
            m_beamDebugState.insert(key, next);

            const char *lightTypeName = "unknown";
            switch (light.type)
            {
                case Light::Type::Directional: lightTypeName = "directional"; break;
                case Light::Type::Point: lightTypeName = "point"; break;
                case Light::Type::Spot: lightTypeName = "spot"; break;
                case Light::Type::Area: lightTypeName = "area"; break;
            }

            qDebug() << "RhiQmlItemRenderer: emitter debug"
                     << "fixtureKind" << fixtureKind
                     << "fixturePtr" << fixturePtr
                     << "emitter" << emitterIndex
                     << "detectedEmitterSize" << emitter.diameter
                     << "topRadius" << light.beamRadius
                     << "position" << light.position
                     << "direction" << light.direction
                     << "innerConeDeg" << qRadiansToDegrees(light.innerCone * 2.0f)
                     << "outerConeDeg" << qRadiansToDegrees(light.outerCone * 2.0f)
                     << "lightType" << lightTypeName
                     << "beamType" << (mode == 1 ? "beam" : "cone");

#ifdef QLC_RHI_DEBUG_CONE_RADIUS
            qDebug().noquote()
                    << QString("RhiQmlItemRenderer[cone-radius]: kind=%1 emitter=%2 itemTopRadius=%3 emitterDiameter=%4 emitterRadius=%5 finalTopRadius=%6 beamType=%7")
                          .arg(QString::fromLatin1(fixtureKind))
                          .arg(emitterIndex)
                          .arg(itemTopRadius, 0, 'f', 4)
                          .arg(emitter.diameter, 0, 'f', 4)
                          .arg(emitter.diameter > 0.0f ? emitter.diameter * 0.5f : -1.0f, 0, 'f', 4)
                          .arg(light.beamRadius, 0, 'f', 4)
                          .arg(mode == 1 ? QStringLiteral("beam") : QStringLiteral("cone"));
#else
            Q_UNUSED(itemTopRadius);
#endif
        };
        QVector3D ambientTotal = qmlItem->ambientLight() * qmlItem->ambientIntensity();
        for (auto it = staticLightItemTransforms.cbegin(); it != staticLightItemTransforms.cend(); ++it)
        {
            const StaticLightItem *itemPtr = it.key();
            const QVector<EmitterData> emitters = staticLightEmitters.value(itemPtr);
            if (!emitters.isEmpty())
            {
                for (int emitterIndex = 0; emitterIndex < emitters.size(); ++emitterIndex)
                {
                    const EmitterData &emitter = emitters[emitterIndex];
                    Light light = itemPtr->toLight();
                    const float itemTopRadius = light.beamRadius;
                    light.position = emitter.position;
                    if (!emitter.direction.isNull())
                        light.direction = emitter.direction.normalized();
                    if (emitter.diameter > 0.0f)
                        light.beamRadius = qMax(0.005f, emitter.diameter * 0.5f);
                    logEmitterDebug("static", itemPtr, emitterIndex, emitter, light, itemTopRadius);
                    newLights.push_back(light);
                }
                continue;
            }

            Light light = itemPtr->toLight();
            QVector4D worldPos = it.value().matrix * QVector4D(light.position, 1.0f);
            light.position = worldPos.toVector3D();
            if (!light.direction.isNull())
                light.direction = it.value().rotation.rotatedVector(light.direction).normalized();
            newLights.push_back(light);
        }
        for (auto it = movingHeadEmitters.cbegin(); it != movingHeadEmitters.cend(); ++it)
        {
            const MovingHeadItem *itemPtr = it.key();
            const QVector<EmitterData> emitters = it.value();
            if (emitters.isEmpty())
                continue;
            for (int emitterIndex = 0; emitterIndex < emitters.size(); ++emitterIndex)
            {
                const EmitterData &emitter = emitters[emitterIndex];
                Light light = itemPtr->toLight();
                const float itemTopRadius = light.beamRadius;
                light.position = emitter.position;
                if (!emitter.direction.isNull())
                    light.direction = emitter.direction.normalized();
                if (emitter.diameter > 0.0f)
                    light.beamRadius = emitter.diameter * 0.5f * kMovingHeadTopRadiusScale;
                else
                    light.beamRadius = qMax(kMovingHeadTopRadiusMin, light.beamRadius);
                logEmitterDebug("moving", itemPtr, emitterIndex, emitter, light, itemTopRadius);
                newLights.push_back(light);
            }
        }
        for (auto it = m_beamDebugState.begin(); it != m_beamDebugState.end(); )
        {
            if (!seenEmitterDebugKeys.contains(it.key()))
                it = m_beamDebugState.erase(it);
            else
                ++it;
        }
        for (auto it = beamBarLights.cbegin(); it != beamBarLights.cend(); ++it)
        {
            const QVector<Light> lights = it.value();
            for (const Light &light : lights)
                newLights.push_back(light);
        }
        const auto qmlLights = qmlItem->findChildren<LightItem *>(QString(), Qt::FindChildrenRecursively);
        for (const LightItem *lightItem : qmlLights)
        {
            if (lightItem->type() == LightItem::Ambient)
            {
                ambientTotal += lightItem->color() * lightItem->intensity();
                continue;
            }
            Light light = lightItem->toLight();
            const auto it = staticLightTransforms.constFind(lightItem);
            if (it != staticLightTransforms.constEnd())
            {
                QVector4D worldPos = it->matrix * QVector4D(light.position, 1.0f);
                light.position = worldPos.toVector3D();
                if (!light.direction.isNull())
                    light.direction = it->rotation.rotatedVector(light.direction).normalized();
            }
            newLights.push_back(light);
        }

        int directionalCount = 0;
        int pointCount = 0;
        int areaCount = 0;
        int spotConeCount = 0;
        int spotBeamCount = 0;
        for (const Light &light : std::as_const(newLights))
        {
            switch (light.type)
            {
                case Light::Type::Directional:
                    ++directionalCount;
                    break;
                case Light::Type::Point:
                    ++pointCount;
                    break;
                case Light::Type::Area:
                    ++areaCount;
                    break;
                case Light::Type::Spot:
                    if (light.beamShape == Light::BeamShapeType::BeamShape)
                        ++spotBeamCount;
                    else
                        ++spotConeCount;
                    break;
            }
        }
        const bool summaryChanged = (m_lightSummary.directional != directionalCount)
                || (m_lightSummary.point != pointCount)
                || (m_lightSummary.area != areaCount)
                || (m_lightSummary.spotCone != spotConeCount)
                || (m_lightSummary.spotBeam != spotBeamCount);
        if (summaryChanged)
        {
            m_lightSummary.directional = directionalCount;
            m_lightSummary.point = pointCount;
            m_lightSummary.area = areaCount;
            m_lightSummary.spotCone = spotConeCount;
            m_lightSummary.spotBeam = spotBeamCount;
            qDebug() << "RhiQmlItemRenderer: light summary"
                     << "directional" << directionalCount
                     << "point" << pointCount
                     << "area" << areaCount
                     << "spotCone" << spotConeCount
                     << "spotBeam" << spotBeamCount;
        }
        m_scene.setLights(newLights);

        const QSize size = qmlItem->effectiveColorBufferSize();
        const float aspect = size.height() > 0 ? float(size.width()) / float(size.height()) : 1.0f;
        const CameraItem *cameraItem = qmlItem->findChild<CameraItem *>(QString(), Qt::FindChildrenRecursively);
        if (cameraItem && !qmlItem->freeCameraEnabled())
        {
            m_scene.camera().setPosition(cameraItem->position());
            m_scene.camera().setPerspective(cameraItem->fov(), aspect, cameraItem->nearPlane(), cameraItem->farPlane());
            m_scene.camera().lookAt(cameraItem->target());
        }
        else
        {
            m_scene.camera().setPosition(qmlItem->cameraPosition());
            m_scene.camera().setPerspective(qmlItem->cameraFov(), aspect, 0.01f, 300.0f);
            m_scene.camera().lookAt(qmlItem->cameraTarget());
        }
        m_scene.setAmbientLight(ambientTotal);
        m_scene.setAmbientIntensity(1.0f);
        m_scene.setSmokeAmount(qmlItem->smokeAmount());
        m_scene.setBeamModel(static_cast<RhiScene::BeamModel>(qmlItem->beamModel()));
        m_scene.setBloomIntensity(qmlItem->bloomIntensity());
        m_scene.setBloomRadius(qmlItem->bloomRadius());
        m_scene.setTimeSeconds(qmlItem->smokeTimeSeconds());
        m_scene.setVolumetricEnabled(qmlItem->volumetricEnabled());
        m_scene.setShadowsEnabled(qmlItem->shadowsEnabled());
        m_scene.setSmokeNoiseEnabled(qmlItem->smokeNoiseEnabled());

        const HazerItem *hazer = qmlItem->findChild<HazerItem *>(QString(), Qt::FindChildrenRecursively);
        if (hazer && hazer->enabled())
        {
            m_scene.setHazeEnabled(true);
            m_scene.setHazePosition(hazer->position());
            m_scene.setHazeDirection(hazer->direction());
            m_scene.setHazeLength(hazer->length());
            m_scene.setHazeRadius(hazer->radius());
            m_scene.setHazeDensity(hazer->density());
        }
        else
        {
            m_scene.setHazeEnabled(false);
            m_scene.setHazeDensity(0.0f);
        }

        QVector3D selectedPos;
        bool hasSelected = computeSelectionCenter(selectedPos);

        ensureGizmoMeshes();

        bool skipPick = false;
        QVector<RhiQmlItem::DragRequest> drags;
        qmlItem->takePendingDragRequests(drags);
        for (const auto &drag : drags)
        {
            if (drag.type == DragBegin && hasSelected)
            {
                PickHit gizmoHit;
                const bool hitOk = pickSceneMesh(m_scene, m_rhiContext.rhi(), drag.normPos,
                                                 PickFilter::GizmosOnly, gizmoHit);
                if (hitOk)
                {
                    const Mesh &mesh = m_scene.meshes()[gizmoHit.meshIndex];
                    if (mesh.gizmoAxis >= 0)
                    {
                        const QVector3D axisDir = axisVector(mesh.gizmoAxis);
                        QVector3D rayOrigin;
                        QVector3D rayDir;
                        if (computeRay(m_scene, m_rhiContext.rhi(), drag.normPos, rayOrigin, rayDir))
                        {
                            m_axisDragActive = false;
                            m_rotateDragActive = false;
                            m_axisDrag = -1;
                            m_rotateAxis = -1;
                            buildDragSelection();
                            if (mesh.gizmoType == 2)
                            {
                                QVector3D hitPoint;
                                if (rayPlaneIntersection(rayOrigin, rayDir, selectedPos, axisDir, hitPoint))
                                {
                                    QVector3D startVec = hitPoint - selectedPos;
                                    startVec -= axisDir * QVector3D::dotProduct(startVec, axisDir);
                                    if (!startVec.isNull())
                                    {
                                        startVec.normalize();
                                        m_rotateDragActive = true;
                                        m_rotateAxis = mesh.gizmoAxis;
                                        m_rotateStartVec = startVec;
                                        m_dragOrigin = selectedPos;
                                        skipPick = true;
                                    }
                                }
                            }
                            else
                            {
                                m_axisDragActive = true;
                                m_axisDrag = mesh.gizmoAxis;
                                m_dragOrigin = selectedPos;
                                m_dragStartT = closestAxisT(rayOrigin, rayDir, selectedPos, axisDir);
                                skipPick = true;
                            }
                        }
                    }
                }
            }
            else if (drag.type == DragMove && m_axisDragActive && !m_dragSelection.isEmpty())
            {
                const QVector3D axisDir = axisVector(m_axisDrag);
                QVector3D rayOrigin;
                QVector3D rayDir;
                if (computeRay(m_scene, m_rhiContext.rhi(), drag.normPos, rayOrigin, rayDir))
                {
                    const float t = closestAxisT(rayOrigin, rayDir, m_dragOrigin, axisDir);
                    const float delta = t - m_dragStartT;
                    const QVector3D newCenter = m_dragOrigin + axisDir * delta;
                    selectedPos = newCenter;
                    hasSelected = true;
                    for (const SelectedTransform &entry : m_dragSelection)
                    {
                        const QVector3D newPos = entry.startPos + axisDir * delta;
                        QMetaObject::invokeMethod(qmlItem, "setObjectPosition", Qt::QueuedConnection,
                                                  Q_ARG(QObject *, entry.item),
                                                  Q_ARG(QVector3D, newPos));
                    }
                }
            }
            else if (drag.type == DragMove && m_rotateDragActive && !m_dragSelection.isEmpty())
            {
                const QVector3D axisDir = axisVector(m_rotateAxis);
                QVector3D rayOrigin;
                QVector3D rayDir;
                if (computeRay(m_scene, m_rhiContext.rhi(), drag.normPos, rayOrigin, rayDir))
                {
                    QVector3D hitPoint;
                    if (rayPlaneIntersection(rayOrigin, rayDir, m_dragOrigin, axisDir, hitPoint))
                    {
                        QVector3D currVec = hitPoint - m_dragOrigin;
                        currVec -= axisDir * QVector3D::dotProduct(currVec, axisDir);
                        if (!currVec.isNull())
                        {
                            currVec.normalize();
                            const float dot = QVector3D::dotProduct(m_rotateStartVec, currVec);
                            const QVector3D cross = QVector3D::crossProduct(m_rotateStartVec, currVec);
                            const float sign = QVector3D::dotProduct(axisDir, cross) >= 0.0f ? 1.0f : -1.0f;
                            const float angleRad = qAtan2(cross.length() * sign, qBound(-1.0f, dot, 1.0f));
                            const float angleDeg = qRadiansToDegrees(angleRad);
                            const QQuaternion rot = QQuaternion::fromAxisAndAngle(axisDir, angleDeg);
                            for (const SelectedTransform &entry : m_dragSelection)
                            {
                                QVector3D newRot = entry.startRot;
                                if (m_rotateAxis == 0)
                                    newRot.setX(entry.startRot.x() + angleDeg);
                                else if (m_rotateAxis == 1)
                                    newRot.setY(entry.startRot.y() + angleDeg);
                                else
                                    newRot.setZ(entry.startRot.z() + angleDeg);
                                const QVector3D offset = entry.startPos - m_dragOrigin;
                                const QVector3D newPos = m_dragOrigin + rot.rotatedVector(offset);
                                QMetaObject::invokeMethod(qmlItem, "setObjectPosition", Qt::QueuedConnection,
                                                          Q_ARG(QObject *, entry.item),
                                                          Q_ARG(QVector3D, newPos));
                                QMetaObject::invokeMethod(qmlItem, "setObjectRotation", Qt::QueuedConnection,
                                                          Q_ARG(QObject *, entry.item),
                                                          Q_ARG(QVector3D, newRot));
                            }
                            selectedPos = m_dragOrigin;
                        }
                    }
                }
            }
            else if (drag.type == DragEnd)
            {
                m_axisDragActive = false;
                m_rotateDragActive = false;
                m_axisDrag = -1;
                m_rotateAxis = -1;
                m_dragSelection.clear();
            }
        }

        updateGizmoMeshes(hasSelected ? selectedPos : QVector3D(),
                          m_scene.camera().position(), hasSelected);

        QVector<RhiQmlItem::PickRequest> picks;
        qmlItem->takePendingPickRequests(picks);
        if (!picks.isEmpty() && !skipPick)
        {
            for (const auto &pick : picks)
            {
                PickHit hit;
                const PickFilter pickFilter = qmlItem->positionPicking()
                        ? PickFilter::All
                        : PickFilter::SelectableOnly;
                const bool hitOk = pickSceneMesh(m_scene, m_rhiContext.rhi(), pick.normPos,
                                                 pickFilter, hit);

                QObject *hitItem = nullptr;
                if (hitOk)
                {
                    hitItem = pickHitForRecords(m_qmlMeshes, hit.meshIndex);
                }

                QMetaObject::invokeMethod(qmlItem, "dispatchPickResult", Qt::QueuedConnection,
                                          Q_ARG(QObject *, hitItem),
                                          Q_ARG(QVector3D, hitOk ? hit.worldPos : QVector3D()),
                                          Q_ARG(bool, hitOk),
                                          Q_ARG(int, int(pick.modifiers)));
            }
        }
    }

    void render(QRhiCommandBuffer *cb) override
    {
        if (!m_initialized)
            return;
        QRhiRenderTarget *rt = renderTarget();
        if (!cb || !rt)
        {
            qWarning() << "RhiQmlItemRenderer: missing cb/rt" << cb << rt;
            return;
        }

        m_rhiContext.setExternalFrame(cb, rt);
        uploadVideoFrames(cb);
        m_renderer.render(&m_scene);
        m_rhiContext.clearExternalFrame();
    }

private:
    void uploadVideoFrames(QRhiCommandBuffer *cb)
    {
        if (!cb)
            return;
        QRhiResourceUpdateBatch *u = nullptr;
        for (MeshRecord &record : m_qmlMeshes)
        {
            if (record.type != MeshItem::MeshType::Video || !record.videoDirty)
                continue;
            if (record.firstMesh < 0 || record.firstMesh >= m_scene.meshes().size())
            {
                record.videoDirty = false;
                record.videoFrame = QImage();
                continue;
            }
            QImage frame = record.videoFrame;
            record.videoFrame = QImage();
            record.videoDirty = false;
            if (frame.isNull())
                continue;
            if (frame.format() != QImage::Format_RGBA8888)
                frame = frame.convertToFormat(QImage::Format_RGBA8888);
            if (!record.videoTexture || record.videoSize != frame.size())
            {
                if (record.videoTexture)
                {
                    delete record.videoTexture;
                    record.videoTexture = nullptr;
                }
                record.videoTexture = rhi()->newTexture(QRhiTexture::RGBA8, frame.size(), 1);
                if (!record.videoTexture || !record.videoTexture->create())
                {
                    delete record.videoTexture;
                    record.videoTexture = nullptr;
                    continue;
                }
                record.videoSize = frame.size();
                Mesh &mesh = m_scene.meshes()[record.firstMesh];
                bool srbDirty = false;
                if (mesh.baseColorTexture == record.videoTexture)
                {
                    mesh.baseColorTexture = nullptr;
                    mesh.baseColorSampler = nullptr;
                    srbDirty = true;
                }
                if (mesh.emissiveTexture != record.videoTexture)
                {
                    mesh.emissiveTexture = record.videoTexture;
                    srbDirty = true;
                }
                if (srbDirty)
                {
                    if (mesh.srb)
                    {
                        delete mesh.srb;
                        mesh.srb = nullptr;
                    }
                    mesh.gpuReady = false;
                }
            }
            if (!u)
                u = rhi()->nextResourceUpdateBatch();
            QRhiTextureUploadDescription upload(QRhiTextureUploadEntry(
                0, 0, QRhiTextureSubresourceUploadDescription(frame)));
            u->uploadTexture(record.videoTexture, upload);
        }
        if (u)
            cb->resourceUpdate(u);
    }

    bool m_initialized = false;
    RhiContext m_rhiContext;
    std::unique_ptr<RenderTargetCache> m_targets;
    std::unique_ptr<ShaderManager> m_shaders;
    DeferredRenderer m_renderer;
    RhiScene m_scene;
    AssimpLoader m_loader;
    QVector<Light> m_staticLights;
    QVector<MeshRecord> m_qmlMeshes;
    QSet<const MeshItem *> m_warnedEmptyPathItems;
    QSet<QString> m_warnedMissingModelPaths;
    QSet<const MeshItem *> m_loggedCreatedItems;
    struct BeamDebugState
    {
        float emitterDiameter = -1.0f;
        float topRadius = -1.0f;
        int mode = -1; // 0 = spot, 1 = beam
    };
    struct LightSummaryState
    {
        int directional = -1;
        int point = -1;
        int area = -1;
        int spotCone = -1;
        int spotBeam = -1;
    };
    QHash<quint64, BeamDebugState> m_beamDebugState;
    LightSummaryState m_lightSummary;
    qint64 m_lastSyncSummaryMs = 0;
    bool m_warnedNoQmlMeshItems = false;
    struct GizmoPart
    {
        int meshIndex = -1;
        int axis = -1;
        int type = 0;
    };
    struct SelectedTransform
    {
        QObject *item = nullptr;
        QVector3D startPos;
        QVector3D startRot;
    };
    QVector<GizmoPart> m_gizmoParts;
    QVector<SelectedTransform> m_dragSelection;
    bool m_axisDragActive = false;
    int m_axisDrag = -1;
    bool m_rotateDragActive = false;
    int m_rotateAxis = -1;
    QVector3D m_dragOrigin;
    float m_dragStartT = 0.0f;
    QVector3D m_rotateStartVec;

    QVector3D axisVector(int axis) const
    {
        switch (axis)
        {
        case 0: return QVector3D(1.0f, 0.0f, 0.0f);
        case 1: return QVector3D(0.0f, 1.0f, 0.0f);
        case 2: return QVector3D(0.0f, 0.0f, 1.0f);
        default: return QVector3D(0.0f, 0.0f, 0.0f);
        }
    }

    bool accumulateMeshBounds(const Mesh &mesh, QVector3D &minV, QVector3D &maxV, bool &hasBounds) const
    {
        QVector3D localMin = mesh.boundsMin;
        QVector3D localMax = mesh.boundsMax;
        if (!mesh.boundsValid)
        {
            if (mesh.vertices.isEmpty())
                return false;
            localMin = QVector3D(mesh.vertices[0].px, mesh.vertices[0].py, mesh.vertices[0].pz);
            localMax = localMin;
            for (const Vertex &v : mesh.vertices)
            {
                localMin.setX(qMin(localMin.x(), v.px));
                localMin.setY(qMin(localMin.y(), v.py));
                localMin.setZ(qMin(localMin.z(), v.pz));
                localMax.setX(qMax(localMax.x(), v.px));
                localMax.setY(qMax(localMax.y(), v.py));
                localMax.setZ(qMax(localMax.z(), v.pz));
            }
        }

        const QVector3D corners[8] = {
            QVector3D(localMin.x(), localMin.y(), localMin.z()),
            QVector3D(localMax.x(), localMin.y(), localMin.z()),
            QVector3D(localMin.x(), localMax.y(), localMin.z()),
            QVector3D(localMax.x(), localMax.y(), localMin.z()),
            QVector3D(localMin.x(), localMin.y(), localMax.z()),
            QVector3D(localMax.x(), localMin.y(), localMax.z()),
            QVector3D(localMin.x(), localMax.y(), localMax.z()),
            QVector3D(localMax.x(), localMax.y(), localMax.z())
        };

        for (const QVector3D &corner : corners)
        {
            const QVector3D world = mesh.modelMatrix.map(corner);
            if (!hasBounds)
            {
                minV = world;
                maxV = world;
                hasBounds = true;
            }
            else
            {
                minV.setX(qMin(minV.x(), world.x()));
                minV.setY(qMin(minV.y(), world.y()));
                minV.setZ(qMin(minV.z(), world.z()));
                maxV.setX(qMax(maxV.x(), world.x()));
                maxV.setY(qMax(maxV.y(), world.y()));
                maxV.setZ(qMax(maxV.z(), world.z()));
            }
        }
        return true;
    }

    bool computeSelectionCenter(QVector3D &center)
    {
        QVector3D sum;
        int count = 0;

        for (const MeshRecord &record : m_qmlMeshes)
        {
            if (!record.selected)
                continue;
            QVector3D minV;
            QVector3D maxV;
            bool hasBounds = false;
            for (int i = record.firstMesh; i < record.firstMesh + record.meshCount; ++i)
            {
                const Mesh &mesh = m_scene.meshes()[i];
                if (!mesh.visible)
                    continue;
                if (!mesh.selectable)
                    continue;
                accumulateMeshBounds(mesh, minV, maxV, hasBounds);
            }
            if (hasBounds)
            {
                sum += (minV + maxV) * 0.5f;
                ++count;
            }
        }

        if (count == 0)
            return false;
        center = sum / float(count);
        return true;
    }

    void buildDragSelection()
    {
        m_dragSelection.clear();
        for (const MeshRecord &record : m_qmlMeshes)
        {
            if (!record.selected)
                continue;
            m_dragSelection.push_back({ const_cast<MeshItem *>(record.item),
                                        record.position,
                                        record.rotationDegrees });
        }
    }

    void ensureGizmoMeshes()
    {
        if (!m_gizmoParts.isEmpty())
            return;
        const QVector3D colors[3] = {
            QVector3D(1.0f, 0.1f, 0.1f),
            QVector3D(0.1f, 1.0f, 0.1f),
            QVector3D(0.1f, 0.3f, 1.0f)
        };
        const float arcRadius = 1.0f;
        const float arcThickness = 0.03f;
        const float arcSpan = float(M_PI) * 0.5f;
        const int arcSegments = 24;
        const int arcSides = 10;
        for (int axis = 0; axis < 3; ++axis)
        {
            for (int part = 0; part < 2; ++part)
            {
                Mesh mesh = createUnitCubeMesh();
                mesh.selectable = false;
                mesh.gizmoAxis = axis;
                mesh.gizmoType = part;
                mesh.material.baseColor = colors[axis];
                mesh.material.emissive = colors[axis] * 2.0f;
                const int meshIndex = m_scene.meshes().size();
                m_scene.meshes().push_back(mesh);
                m_gizmoParts.push_back({ meshIndex, axis, part });
            }
            Mesh arc = createArcMesh(arcRadius, arcThickness, 0.0f, arcSpan,
                                     arcSegments, arcSides);
            arc.selectable = false;
            arc.gizmoAxis = axis;
            arc.gizmoType = 2;
            arc.material.baseColor = colors[axis];
            arc.material.emissive = colors[axis] * 2.0f;
            const int arcIndex = m_scene.meshes().size();
            m_scene.meshes().push_back(arc);
            m_gizmoParts.push_back({ arcIndex, axis, 2 });
        }
    }

    void updateGizmoMeshes(const QVector3D &pos, const QVector3D &cameraPos, bool visible)
    {
        const float distance = (cameraPos - pos).length();
        const float axisLength = qMax(0.2f, distance * 0.2f);
        const float shaftRadius = axisLength * 0.06f;
        const float headSize = axisLength * 0.14f;
        const float arcRadius = axisLength * 0.5f;
        const float arcScale = arcRadius;

        for (const GizmoPart &part : m_gizmoParts)
        {
            Mesh &mesh = m_scene.meshes()[part.meshIndex];
            if (!visible)
            {
                QMatrix4x4 hidden;
                hidden.setToIdentity();
                hidden.scale(0.0f);
                mesh.modelMatrix = hidden;
                mesh.modelDirty = true;
                mesh.worldBoundsDirty = true;
                continue;
            }

            const int axis = part.axis;
            const QVector3D dir = axisVector(axis);
            if (part.type == 0)
            {
                QMatrix4x4 shaftM;
                shaftM.translate(pos + dir * (axisLength * 0.5f));
                if (axis == 0)
                    shaftM.scale(axisLength, shaftRadius, shaftRadius);
                else if (axis == 1)
                    shaftM.scale(shaftRadius, axisLength, shaftRadius);
                else
                    shaftM.scale(shaftRadius, shaftRadius, axisLength);
                mesh.modelMatrix = shaftM;
                mesh.modelDirty = true;
                mesh.worldBoundsDirty = true;
            }
            else if (part.type == 1)
            {
                QMatrix4x4 headM;
                headM.translate(pos + dir * (axisLength + headSize * 0.5f));
                headM.scale(headSize, headSize, headSize);
                mesh.modelMatrix = headM;
                mesh.modelDirty = true;
                mesh.worldBoundsDirty = true;
            }
            else
            {
                QVector3D u(1.0f, 0.0f, 0.0f);
                QVector3D v(0.0f, 1.0f, 0.0f);
                QVector3D axisDir = dir;
                if (axis == 0) {
                    axisDir = QVector3D(1.0f, 0.0f, 0.0f);
                    u = QVector3D(0.0f, 1.0f, 0.0f);
                    v = QVector3D(0.0f, 0.0f, 1.0f);
                } else if (axis == 1) {
                    axisDir = QVector3D(0.0f, 1.0f, 0.0f);
                    u = QVector3D(1.0f, 0.0f, 0.0f);
                    v = QVector3D(0.0f, 0.0f, 1.0f);
                } else {
                    axisDir = QVector3D(0.0f, 0.0f, 1.0f);
                    u = QVector3D(1.0f, 0.0f, 0.0f);
                    v = QVector3D(0.0f, 1.0f, 0.0f);
                }

                QMatrix4x4 arcM;
                arcM.setToIdentity();
                arcM.setColumn(0, QVector4D(u * arcScale, 0.0f));
                arcM.setColumn(1, QVector4D(v * arcScale, 0.0f));
                arcM.setColumn(2, QVector4D(axisDir * arcScale, 0.0f));
                arcM.setColumn(3, QVector4D(pos, 1.0f));
                mesh.modelMatrix = arcM;
                mesh.modelDirty = true;
                mesh.worldBoundsDirty = true;
            }
        }
    }
};

} // namespace

RhiQmlItem::RhiQmlItem(QQuickItem *parent)
    : QQuickRhiItem(parent)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton | Qt::MiddleButton);
    setFlag(ItemIsFocusScope, true);
    m_cameraTick = new QTimer(this);
    m_cameraTick->setInterval(16);
    connect(m_cameraTick, &QTimer::timeout, this, [this]()
    {
        if (!m_freeCameraEnabled)
            return;
        if (!m_cameraTimer.isValid())
            m_cameraTimer.start();
        const qint64 ms = m_cameraTimer.restart();
        const float dt = ms > 0 ? float(ms) / 1000.0f : 0.0f;
        if (dt > 0.0f)
            updateFreeCamera(dt);
    });
    m_smokeTick = new QTimer(this);
    m_smokeTick->setInterval(16);
    connect(m_smokeTick, &QTimer::timeout, this, [this]()
    {
        if (m_smokeAmount <= 0.0f || !m_smokeNoiseEnabled)
            return;
        if (!m_smokeTimer.isValid())
            m_smokeTimer.start();
        update();
    });
}

void RhiQmlItem::setCameraPosition(const QVector3D &pos)
{
    if (m_cameraPosition == pos)
        return;
    m_cameraPosition = pos;
    emit cameraPositionChanged();
    update();
}

void RhiQmlItem::setCameraTarget(const QVector3D &target)
{
    if (m_cameraTarget == target)
        return;
    m_cameraTarget = target;
    emit cameraTargetChanged();
    update();
}

void RhiQmlItem::setCameraFov(float fov)
{
    if (qFuzzyCompare(m_cameraFov, fov))
        return;
    m_cameraFov = fov;
    emit cameraFovChanged();
    update();
}

void RhiQmlItem::zoomAlongView(float delta)
{
    const QVector3D dir = (m_freeCameraEnabled ? forwardVector() : (m_cameraTarget - m_cameraPosition).normalized());
    if (dir.isNull())
        return;
    m_cameraPosition += dir * delta;
    if (!m_freeCameraEnabled)
        m_cameraTarget += dir * delta;
    emit cameraPositionChanged();
    emit cameraTargetChanged();
    update();
}

void RhiQmlItem::setAmbientLight(const QVector3D &ambient)
{
    if (m_ambientLight == ambient)
        return;
    m_ambientLight = ambient;
    emit ambientLightChanged();
    update();
}

void RhiQmlItem::setAmbientIntensity(float intensity)
{
    if (qFuzzyCompare(m_ambientIntensity, intensity))
        return;
    m_ambientIntensity = intensity;
    emit ambientIntensityChanged();
    update();
}

void RhiQmlItem::setSmokeAmount(float amount)
{
    if (qFuzzyCompare(m_smokeAmount, amount))
        return;
    m_smokeAmount = amount;
    emit smokeAmountChanged();
    updateSmokeTicker();
    update();
}

void RhiQmlItem::setBeamModel(BeamModel mode)
{
    if (m_beamModel == mode)
        return;
    m_beamModel = mode;
    emit beamModelChanged();
    update();
}

void RhiQmlItem::setBloomIntensity(float intensity)
{
    if (qFuzzyCompare(m_bloomIntensity, intensity))
        return;
    m_bloomIntensity = intensity;
    emit bloomIntensityChanged();
    update();
}

void RhiQmlItem::setBloomRadius(float radius)
{
    if (qFuzzyCompare(m_bloomRadius, radius))
        return;
    m_bloomRadius = radius;
    emit bloomRadiusChanged();
    update();
}

void RhiQmlItem::setVolumetricEnabled(bool enabled)
{
    if (m_volumetricEnabled == enabled)
        return;
    m_volumetricEnabled = enabled;
    emit volumetricEnabledChanged();
    update();
}

void RhiQmlItem::setShadowsEnabled(bool enabled)
{
    if (m_shadowsEnabled == enabled)
        return;
    m_shadowsEnabled = enabled;
    emit shadowsEnabledChanged();
    update();
}

void RhiQmlItem::setSmokeNoiseEnabled(bool enabled)
{
    if (m_smokeNoiseEnabled == enabled)
        return;
    m_smokeNoiseEnabled = enabled;
    emit smokeNoiseEnabledChanged();
    updateSmokeTicker();
    update();
}

void RhiQmlItem::setFreeCameraEnabled(bool enabled)
{
    if (m_freeCameraEnabled == enabled)
        return;
    m_freeCameraEnabled = enabled;
    if (m_freeCameraEnabled)
    {
        const QVector3D dir = (m_cameraTarget - m_cameraPosition).normalized();
        updateYawPitchFromDirection(dir);
        m_cameraTimer.restart();
        m_cameraTick->start();
    }
    else
    {
        m_cameraTick->stop();
    }
    emit freeCameraEnabledChanged();
    update();
}

void RhiQmlItem::setPositionPicking(bool enabled)
{
    if (m_positionPicking == enabled)
        return;
    m_positionPicking = enabled;
    emit positionPickingChanged();
}

void RhiQmlItem::setMoveSpeed(float speed)
{
    if (qFuzzyCompare(m_moveSpeed, speed))
        return;
    m_moveSpeed = speed;
    emit moveSpeedChanged();
}

void RhiQmlItem::setLookSensitivity(float sensitivity)
{
    if (qFuzzyCompare(m_lookSensitivity, sensitivity))
        return;
    m_lookSensitivity = sensitivity;
    emit lookSensitivityChanged();
}

float RhiQmlItem::smokeTimeSeconds() const
{
    if (!m_smokeTimer.isValid())
        return 0.0f;
    return float(m_smokeTimer.elapsed()) / 1000.0f;
}

void RhiQmlItem::updateSmokeTicker()
{
    if (m_smokeAmount > 0.0f && m_smokeNoiseEnabled)
    {
        if (!m_smokeTimer.isValid())
            m_smokeTimer.start();
        if (!m_smokeTick->isActive())
            m_smokeTick->start();
    }
    else
    {
        if (m_smokeTick->isActive())
            m_smokeTick->stop();
    }
}

void RhiQmlItem::keyPressEvent(QKeyEvent *event)
{
    if (!m_freeCameraEnabled)
    {
        QQuickRhiItem::keyPressEvent(event);
        return;
    }
    if (event->key() == Qt::Key_W)
        m_moveForward = true;
    else if (event->key() == Qt::Key_S)
        m_moveBackward = true;
    else if (event->key() == Qt::Key_A)
        m_moveLeft = true;
    else if (event->key() == Qt::Key_D)
        m_moveRight = true;
    else if (event->key() == Qt::Key_Q)
        m_moveDown = true;
    else if (event->key() == Qt::Key_E)
        m_moveUp = true;
    else
        QQuickRhiItem::keyPressEvent(event);
}

void RhiQmlItem::keyReleaseEvent(QKeyEvent *event)
{
    if (!m_freeCameraEnabled)
    {
        QQuickRhiItem::keyReleaseEvent(event);
        return;
    }
    if (event->key() == Qt::Key_W)
        m_moveForward = false;
    else if (event->key() == Qt::Key_S)
        m_moveBackward = false;
    else if (event->key() == Qt::Key_A)
        m_moveLeft = false;
    else if (event->key() == Qt::Key_D)
        m_moveRight = false;
    else if (event->key() == Qt::Key_Q)
        m_moveDown = false;
    else if (event->key() == Qt::Key_E)
        m_moveUp = false;
    else
        QQuickRhiItem::keyReleaseEvent(event);
}

void RhiQmlItem::mousePressEvent(QMouseEvent *event)
{
    forceActiveFocus(Qt::MouseFocusReason);
    if (event->button() == Qt::MiddleButton)
    {
        m_panning = true;
        m_lastPanPos = event->position();
        event->accept();
        return;
    }
    if (event->button() == Qt::LeftButton)
    {
        const float w = width();
        const float h = height();
        if (w > 0.0f && h > 0.0f)
        {
            const QPointF pos = event->position();
            const QPointF normPos(pos.x() / w, pos.y() / h);
            m_pendingPickRequests.push_back({ normPos, QGuiApplication::keyboardModifiers() });
            m_pendingDragRequests.push_back({ normPos, 0 });
            update();
        }
        m_leftDown = true;
        event->accept();
        return;
    }

    if (!m_freeCameraEnabled || event->button() != Qt::RightButton)
    {
        QQuickRhiItem::mousePressEvent(event);
        return;
    }
    m_looking = true;
    m_lastMousePos = event->position();
    event->accept();
}

void RhiQmlItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton)
    {
        m_panning = false;
        event->accept();
        return;
    }
    if (event->button() == Qt::LeftButton)
    {
        const float w = width();
        const float h = height();
        if (w > 0.0f && h > 0.0f)
        {
            const QPointF pos = event->position();
            const QPointF normPos(pos.x() / w, pos.y() / h);
            m_pendingDragRequests.push_back({ normPos, 2 });
            update();
        }
        m_leftDown = false;
        event->accept();
        return;
    }
    if (!m_freeCameraEnabled || event->button() != Qt::RightButton)
    {
        QQuickRhiItem::mouseReleaseEvent(event);
        return;
    }
    m_looking = false;
    event->accept();
}

void RhiQmlItem::mouseMoveEvent(QMouseEvent *event)
{
    if (m_panning)
    {
        const QPointF pos = event->position();
        const QPointF delta = pos - m_lastPanPos;
        m_lastPanPos = pos;

        QVector3D forward = m_freeCameraEnabled
                ? forwardVector()
                : (m_cameraTarget - m_cameraPosition).normalized();
        if (forward.isNull())
            forward = QVector3D(0.0f, 0.0f, -1.0f);
        const QVector3D right = QVector3D::crossProduct(forward, QVector3D(0.0f, 1.0f, 0.0f)).normalized();
        const QVector3D up = QVector3D::crossProduct(right, forward).normalized();
        const float dist = (m_cameraTarget - m_cameraPosition).length();
        const float scale = qMax(0.001f, dist) * 0.006f;
        const QVector3D pan = (-right * float(delta.x()) + up * float(delta.y())) * scale;
        m_cameraPosition += pan;
        m_cameraTarget += pan;
        emit cameraPositionChanged();
        emit cameraTargetChanged();
        update();
        event->accept();
        return;
    }
    if (m_leftDown)
    {
        const float w = width();
        const float h = height();
        if (w > 0.0f && h > 0.0f)
        {
            const QPointF pos = event->position();
            const QPointF normPos(pos.x() / w, pos.y() / h);
            m_pendingDragRequests.push_back({ normPos, 1 });
            update();
        }
    }
    if (!m_freeCameraEnabled || !m_looking)
    {
        QQuickRhiItem::mouseMoveEvent(event);
        return;
    }
    const QPointF pos = event->position();
    const QPointF delta = pos - m_lastMousePos;
    m_lastMousePos = pos;

    const float sensitivity = m_lookSensitivity * 5.0f;
    float yawDelta = -float(delta.x()) * sensitivity;
    float pitchDelta = float(delta.y()) * sensitivity;

    if (event->modifiers() & Qt::ShiftModifier)
    {
        if (qAbs(delta.x()) >= qAbs(delta.y()))
            pitchDelta = 0.0f;
        else
            yawDelta = 0.0f;
    }

    orbitCameraAroundTarget(yawDelta, pitchDelta);
    event->accept();
}

void RhiQmlItem::updateFreeCamera(float dtSeconds)
{
    QVector3D pos = m_cameraPosition;
    const QVector3D fwd = forwardVector();
    const QVector3D right = rightVector();
    const QVector3D up(0.0f, 1.0f, 0.0f);
    const float speed = m_moveSpeed * dtSeconds;

    if (m_moveForward)
        pos += fwd * speed;
    if (m_moveBackward)
        pos -= fwd * speed;
    if (m_moveLeft)
        pos -= right * speed;
    if (m_moveRight)
        pos += right * speed;
    if (m_moveUp)
        pos += up * speed;
    if (m_moveDown)
        pos -= up * speed;

    if (pos != m_cameraPosition)
    {
        m_cameraPosition = pos;
        m_cameraTarget = m_cameraPosition + fwd;
        emit cameraPositionChanged();
        emit cameraTargetChanged();
        update();
    }
}

void RhiQmlItem::updateYawPitchFromDirection(const QVector3D &dir)
{
    const QVector3D nd = dir.normalized();
    m_pitchDeg = qRadiansToDegrees(std::asin(nd.y()));
    m_yawDeg = qRadiansToDegrees(std::atan2(nd.z(), nd.x()));
}

QVector3D RhiQmlItem::forwardVector() const
{
    const float yaw = qDegreesToRadians(m_yawDeg);
    const float pitch = qDegreesToRadians(m_pitchDeg);
    QVector3D f(std::cos(yaw) * std::cos(pitch),
                std::sin(pitch),
                std::sin(yaw) * std::cos(pitch));
    return f.normalized();
}

QVector3D RhiQmlItem::rightVector() const
{
    const QVector3D fwd = forwardVector();
    const QVector3D up(0.0f, 1.0f, 0.0f);
    return QVector3D::crossProduct(fwd, up).normalized();
}

void RhiQmlItem::addModel(const QString &path)
{
    if (path.isEmpty())
        return;
    addModel(path, QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector3D(1.0f, 1.0f, 1.0f));
}

void RhiQmlItem::addModel(const QString &path, const QVector3D &position)
{
    if (path.isEmpty())
        return;
    addModel(path, position, QVector3D(0.0f, 0.0f, 0.0f), QVector3D(1.0f, 1.0f, 1.0f));
}

void RhiQmlItem::addModel(const QString &path,
                          const QVector3D &position,
                          const QVector3D &rotationDegrees,
                          const QVector3D &scale)
{
    if (path.isEmpty())
        return;
    PendingModel model;
    model.path = path;
    model.position = position;
    model.rotationDegrees = rotationDegrees;
    model.scale = scale;
    m_pendingModels.push_back(model);
    update();
}

void RhiQmlItem::addPointLight(const QVector3D &position,
                               const QVector3D &color,
                               float intensity,
                               float range)
{
    Light light;
    light.type = Light::Type::Point;
    light.position = position;
    light.color = color;
    light.intensity = intensity;
    light.range = range;
    light.castShadows = true;
    m_pendingLights.push_back(light);
    update();
}

void RhiQmlItem::addDirectionalLight(const QVector3D &direction,
                                     const QVector3D &color,
                                     float intensity)
{
    Light light;
    light.type = Light::Type::Directional;
    light.direction = direction;
    light.color = color;
    light.intensity = intensity;
    light.castShadows = true;
    m_pendingLights.push_back(light);
    update();
}

void RhiQmlItem::addSpotLight(const QVector3D &position,
                              const QVector3D &direction,
                              const QVector3D &color,
                              float intensity,
                              float coneAngleDegrees)
{
    Light light;
    light.type = Light::Type::Spot;
    light.position = position;
    light.direction = direction;
    light.color = color;
    light.intensity = intensity;
    light.range = 20.0f;
    const float halfAngle = qDegreesToRadians(coneAngleDegrees * 0.5f);
    light.innerCone = halfAngle * 0.8f;
    light.outerCone = halfAngle;
    light.castShadows = true;
    m_pendingLights.push_back(light);
    update();
}

void RhiQmlItem::addAreaLight(const QVector3D &position,
                              const QVector3D &direction,
                              const QVector3D &color,
                              float intensity,
                              const QSizeF &size)
{
    Light light;
    light.type = Light::Type::Area;
    light.position = position;
    light.direction = direction;
    light.color = color;
    light.intensity = intensity;
    light.range = qMax(size.width(), size.height());
    light.areaSize = QVector2D(float(size.width()), float(size.height()));
    light.castShadows = false;
    m_pendingLights.push_back(light);
    update();
}

void RhiQmlItem::takePendingModels(QVector<PendingModel> &out)
{
    out = std::move(m_pendingModels);
    m_pendingModels.clear();
}

void RhiQmlItem::takePendingLights(QVector<Light> &out)
{
    out = std::move(m_pendingLights);
    m_pendingLights.clear();
}

void RhiQmlItem::takePendingPickRequests(QVector<PickRequest> &out)
{
    out = std::move(m_pendingPickRequests);
    m_pendingPickRequests.clear();
}

void RhiQmlItem::takePendingDragRequests(QVector<DragRequest> &out)
{
    out = std::move(m_pendingDragRequests);
    m_pendingDragRequests.clear();
}

void RhiQmlItem::dispatchPickResult(QObject *item, const QVector3D &worldPos, bool hit, int modifiers)
{
    const QVariant itemId = item ? item->property("itemID") : QVariant();
    qDebug() << "RhiQmlItem: pick result"
             << "hit" << hit
             << "item" << item
             << "class" << (item ? item->metaObject()->className() : "null")
             << "itemIDValid" << itemId.isValid()
             << "itemID" << (itemId.isValid() ? itemId.toUInt() : 0u)
             << "selectable" << (item ? item->property("selectable") : QVariant())
             << "worldPos" << worldPos
             << "modifiers" << modifiers;
    emit meshPicked(item, worldPos, hit, modifiers);
}

void RhiQmlItem::handlePick(QObject *item, bool hit, int modifiers)
{
    const bool multi = (modifiers & Qt::ShiftModifier) != 0;
    bool canSelect = item && item->property("isSelected").isValid();
    if (canSelect && item->property("selectable").isValid())
        canSelect = item->property("selectable").toBool();

    if (!multi && (!hit || canSelect))
    {
        for (QObject *entry : m_selectableItems)
        {
            if (!entry)
                continue;
            if (entry->property("selectable").isValid() && !entry->property("selectable").toBool())
                continue;
            entry->setProperty("isSelected", false);
        }
    }

    if (item && hit && canSelect)
    {
        if (multi)
            item->setProperty("isSelected", !item->property("isSelected").toBool());
        else
            item->setProperty("isSelected", true);
        setSelectedItem(item);
    }
    else if (!multi && !hit)
    {
        setSelectedItem(nullptr);
    }
}

void RhiQmlItem::removeSelectedItems()
{
    const auto meshItems = findChildren<MeshItem *>(QString(), Qt::FindChildrenRecursively);
    QVector<QObject *> toDelete;
    toDelete.reserve(meshItems.size());
    for (MeshItem *item : meshItems)
    {
        if (!item)
            continue;
        if (!item->isSelected())
            continue;
        toDelete.push_back(item);
    }

    if (toDelete.isEmpty())
        return;

    bool clearedSelection = false;
    for (QObject *item : toDelete)
    {
        item->setProperty("isSelected", false);
        m_selectableItems.remove(item);
        if (item == m_selectedItem)
        {
            m_selectedItem = nullptr;
            clearedSelection = true;
        }
        item->deleteLater();
    }

    if (clearedSelection)
        emit selectedItemChanged();
    update();
}

void RhiQmlItem::updateSelectableItems(const QVector<QObject *> &items)
{
    m_selectableItems.clear();
    for (QObject *item : items)
    {
        if (!item)
            continue;
        m_selectableItems.insert(item);
    }
}

void RhiQmlItem::setCameraDirection(const QVector3D &dir)
{
    if (dir.isNull())
        return;
    updateYawPitchFromDirection(dir);
    m_cameraTarget = m_cameraPosition + forwardVector();
    emit cameraTargetChanged();
    update();
}

void RhiQmlItem::orbitCameraAroundTarget(float yawDeltaDeg, float pitchDeltaDeg)
{
    QVector3D offset = m_cameraPosition - m_cameraTarget;
    if (offset.lengthSquared() <= 0.000001f)
        return;

    const QVector3D worldUp(0.0f, 1.0f, 0.0f);
    if (!qFuzzyIsNull(yawDeltaDeg))
    {
        const QQuaternion yawRot = QQuaternion::fromAxisAndAngle(worldUp, yawDeltaDeg);
        offset = yawRot.rotatedVector(offset);
    }

    if (!qFuzzyIsNull(pitchDeltaDeg))
    {
        const QVector3D viewDir = (-offset).normalized();
        QVector3D right = QVector3D::crossProduct(viewDir, worldUp);
        if (right.lengthSquared() > 0.000001f)
        {
            right.normalize();
            const QQuaternion pitchRot = QQuaternion::fromAxisAndAngle(right, pitchDeltaDeg);
            const QVector3D pitchedOffset = pitchRot.rotatedVector(offset);
            const QVector3D pitchedViewDir = (-pitchedOffset).normalized();
            const float upDot = QVector3D::dotProduct(pitchedViewDir, worldUp);
            if (qAbs(upDot) < 0.995f)
                offset = pitchedOffset;
        }
    }

    m_cameraPosition = m_cameraTarget + offset;
    updateYawPitchFromDirection((m_cameraTarget - m_cameraPosition).normalized());
    emit cameraPositionChanged();
    update();
}

void RhiQmlItem::rotateFreeCamera(float yawDelta, float pitchDelta)
{
    if (!m_freeCameraEnabled)
        return;
    orbitCameraAroundTarget(-yawDelta, -pitchDelta);
}

void RhiQmlItem::setSelectedItem(QObject *item)
{
    if (m_selectedItem == item)
        return;
    m_selectedItem = item;
    emit selectedItemChanged();
}

void RhiQmlItem::setObjectPosition(QObject *item, const QVector3D &pos)
{
    if (!item)
        return;
    const QVariant current = item->property("position");
    if (current.isValid() && current.canConvert<QVector3D>() && current.value<QVector3D>() == pos)
        return;
    item->setProperty("position", QVariant::fromValue(pos));
    emit objectPositionEdited(item, pos);
}

void RhiQmlItem::setObjectRotation(QObject *item, const QVector3D &rotation)
{
    if (!item)
        return;
    const QVariant current = item->property("rotationDegrees");
    if (current.isValid() && current.canConvert<QVector3D>() && current.value<QVector3D>() == rotation)
        return;
    item->setProperty("rotationDegrees", QVariant::fromValue(rotation));
    emit objectRotationEdited(item, rotation);
}

QQuickRhiItemRenderer *RhiQmlItem::createRenderer()
{
    return new RhiQmlItemRenderer();
}
