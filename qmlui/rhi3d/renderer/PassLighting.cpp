/*
  Q Light Controller Plus
  PassLighting.cpp

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

#include "renderer/PassLighting.h"

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QUrl>
#include <QtGui/QImage>
#include <QtGui/QColor>
#include <QtGui/QPainter>
#include <QtSvg/QSvgRenderer>
#include <rhi/qrhi.h>
#include <vector>
#include <cstring>
#include <QtCore/QVector>
#include <QtCore/QHash>

#include "core/RhiContext.h"
#include "core/RenderTargetCache.h"
#include "core/ShaderManager.h"
#include "scene/Scene.h"

static QString resolveGoboPath(const QString &path)
{
    if (path.isEmpty())
        return QString();
    if (path.startsWith(QLatin1String("colorwheel://"), Qt::CaseInsensitive))
        return path;
    const QUrl url(path);
    if (url.isValid() && url.isLocalFile())
        return url.toLocalFile();
    const QFileInfo info(path);
    if (info.isAbsolute())
        return path;
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString candidateCwd = QDir::current().filePath(path);
    if (QFileInfo::exists(candidateCwd))
        return candidateCwd;
    const QString candidateApp = QDir(appDir).filePath(path);
    if (QFileInfo::exists(candidateApp))
        return candidateApp;
    const QString candidateAppParent = QDir(appDir).filePath(QStringLiteral("../") + path);
    if (QFileInfo::exists(candidateAppParent))
        return candidateAppParent;
    return candidateCwd;
}

static bool decodeDoubleColorGoboPath(const QString &path, QColor &colorA, QColor &colorB,
                                      QString *embeddedGoboPath = nullptr)
{
    static const QString prefix = QStringLiteral("colorwheel://double/");
    if (!path.startsWith(prefix, Qt::CaseInsensitive))
        return false;

    const QString tail = path.mid(prefix.length());
    const int queryPos = tail.indexOf(QLatin1Char('?'));
    const QString wheelPart = queryPos >= 0 ? tail.left(queryPos) : tail;
    const QString queryPart = queryPos >= 0 ? tail.mid(queryPos + 1) : QString();

    const QStringList parts = wheelPart.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    if (parts.size() != 2)
        return false;

    const QColor c1(QStringLiteral("#") + parts[0]);
    const QColor c2(QStringLiteral("#") + parts[1]);
    if (!c1.isValid() || !c2.isValid())
        return false;

    colorA = c1;
    colorB = c2;

    if (embeddedGoboPath != nullptr)
    {
        embeddedGoboPath->clear();
        if (queryPart.startsWith(QLatin1String("gobo="), Qt::CaseInsensitive))
        {
            const QString encoded = queryPart.mid(5);
            *embeddedGoboPath = QUrl::fromPercentEncoding(encoded.toUtf8());
        }
    }
    return true;
}

static QImage makeDoubleColorGoboImage(const QSize &size, const QColor &colorA, const QColor &colorB)
{
    QImage image(size, QImage::Format_RGBA8888);
    image.fill(Qt::black);
    if (size.width() <= 0 || size.height() <= 0)
        return image;

    const QRectF inner(2.0, 2.0,
                       qreal(qMax(0, size.width() - 4)),
                       qreal(qMax(0, size.height() - 4)));
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(colorA));
    painter.drawPie(inner, 90 * 16, 180 * 16);
    painter.setBrush(QBrush(colorB));
    painter.drawPie(inner, -90 * 16, 180 * 16);
    return image;
}

static QImage multiplyGoboImages(const QImage &base, const QImage &mask)
{
    QImage result = base.convertToFormat(QImage::Format_RGBA8888);
    if (result.isNull())
        return result;

    QImage overlay = mask.convertToFormat(QImage::Format_RGBA8888);
    if (overlay.isNull())
        return result;
    if (overlay.size() != result.size())
        overlay = overlay.scaled(result.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    QPainter painter(&result);
    painter.setCompositionMode(QPainter::CompositionMode_Multiply);
    painter.drawImage(QPoint(0, 0), overlay);
    return result;
}

static QImage loadGoboImage(const QString &path, const QSize &size)
{
    QColor colorA;
    QColor colorB;
    QString embeddedGoboPath;
    if (decodeDoubleColorGoboPath(path, colorA, colorB, &embeddedGoboPath))
    {
        QImage colorWheelImage = makeDoubleColorGoboImage(size, colorA, colorB);
        if (!embeddedGoboPath.isEmpty()
            && !embeddedGoboPath.startsWith(QLatin1String("colorwheel://"), Qt::CaseInsensitive))
        {
            const QImage goboImage = loadGoboImage(embeddedGoboPath, size);
            colorWheelImage = multiplyGoboImages(colorWheelImage, goboImage);
        }
        return colorWheelImage;
    }

    QImage image;
    if (!path.isEmpty())
    {
        const QString resolved = resolveGoboPath(path);
        if (resolved.endsWith(QLatin1String(".svg"), Qt::CaseInsensitive))
        {
            QSvgRenderer renderer(resolved);
            if (renderer.isValid())
            {
                const int w = size.width();
                const int h = size.height();
                image = QImage(size, QImage::Format_RGBA8888);
                // Match existing gobo pipeline: black background + circular mask + inset render.
                image.fill(Qt::black);
                QPainter painter(&image);
                painter.setBrush(QBrush(Qt::white));
                painter.setPen(Qt::NoPen);
                // Mask out a 2px border to avoid edge bleed from the rasterized SVG.
                painter.drawEllipse(2, 2, qMax(0, w - 4), qMax(0, h - 4));
                // Render SVG 1px inset to keep it inside the mask.
                renderer.render(&painter, QRect(1, 1, qMax(0, w - 2), qMax(0, h - 2)));
            }
        }
        else
        {
            const int w = size.width();
            const int h = size.height();
            image = QImage(size, QImage::Format_RGBA8888);
            // Apply the same mask/inset behavior for non-SVG gobos.
            image.fill(Qt::black);
            QPainter painter(&image);
            painter.setBrush(QBrush(Qt::white));
            painter.setPen(Qt::NoPen);
            painter.drawEllipse(2, 2, qMax(0, w - 4), qMax(0, h - 4));
            QIcon goboFile(resolved);
            painter.drawPixmap(1, 1, qMax(0, w - 2), qMax(0, h - 2),
                               goboFile.pixmap(QSize(qMax(0, w - 2), qMax(0, h - 2))));
        }
    }
    if (image.isNull())
    {
        image = QImage(size, QImage::Format_RGBA8888);
        image.fill(Qt::black);
    }
    else
    {
        image = image.convertToFormat(QImage::Format_RGBA8888);
        if (image.size() != size)
            image = image.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }
    return image;
}

QImage PassLighting::loadGoboCached(const QString &path)
{
    if (path.isEmpty())
        return QImage();
    const QString resolved = resolveGoboPath(path);
    const auto it = m_goboCache.constFind(resolved);
    if (it != m_goboCache.constEnd())
        return it.value();
    const QImage image = loadGoboImage(resolved, m_spotGoboSize);
    m_goboCache.insert(resolved, image);
    return image;
}

void PassLighting::prepare(FrameContext &ctx)
{
    if (qEnvironmentVariableIsSet("RHIPIPELINE_SKIP_LIGHTING"))
    {
        qWarning() << "PassLighting: skipping lighting (RHIPIPELINE_SKIP_LIGHTING)";
        return;
    }
    ensurePipeline(ctx);
}

void PassLighting::updateGoboTextures(FrameContext &ctx, QRhiResourceUpdateBatch *u)
{
    if (!ctx.scene || !m_spotGoboMap || !u)
        return;
    QVector<QRhiTextureUploadEntry> entries;
    entries.reserve(kMaxLights);
    const auto &lights = ctx.scene->lights();
    for (int i = 0; i < kMaxLights; ++i)
    {
        const QString path = i < lights.size() ? lights[i].goboPath : QString();
        if (!m_spotGoboPaths[i].isNull() && path == m_spotGoboPaths[i])
            continue;
        m_spotGoboPaths[i] = path;
        QImage image = loadGoboCached(path);
        if (image.isNull())
            image = loadGoboImage(QString(), m_spotGoboSize);
        entries.push_back(QRhiTextureUploadEntry(i, 0, QRhiTextureSubresourceUploadDescription(image)));
    }
    if (entries.isEmpty())
        return;
    QRhiTextureUploadDescription desc;
    desc.setEntries(entries.begin(), entries.end());
    u->uploadTexture(m_spotGoboMap, desc);
}

void PassLighting::execute(FrameContext &ctx)
{
    if (!ctx.rhi)
        return;
    if (!ctx.scene || !m_pipeline || !m_srb)
        return;
    QRhiCommandBuffer *cb = ctx.rhi->commandBuffer();
    QRhiRenderTarget *swapRt = ctx.rhi->swapchainRenderTarget();
    if (!cb || !swapRt)
        return;
    const RenderTargetCache::LightingTargets lighting = ctx.targets->getOrCreateLightingTarget(swapRt->pixelSize(), 1);
    QRhiRenderTarget *rt = lighting.rt;
    if (!rt)
        return;

    const bool lightDataDirty = ctx.scene->lightsDirty() || ctx.scene->lightParamsDirty();
    struct LightsData
    {
        QVector4D lightCount;
        QVector4D lightParams;
        QVector4D lightFlags;
        QVector4D lightBeam[kMaxLights];
        QVector4D posRange[kMaxLights];
        QVector4D colorIntensity[kMaxLights];
        QVector4D dirInner[kMaxLights];
        QVector4D other[kMaxLights];
    } lightData{};
    if (lightDataDirty)
    {
        const int maxLights = qMin(kMaxLights, ctx.scene->lights().size());
        const QVector3D ambient = ctx.scene->ambientLight() * ctx.scene->ambientIntensity();
        lightData.lightCount = QVector4D(float(maxLights), ambient.x(), ambient.y(), ambient.z());
        lightData.lightParams = QVector4D(ctx.scene->smokeAmount(),
                                          float(static_cast<int>(ctx.scene->beamModel())),
                                          ctx.scene->bloomIntensity(),
                                          ctx.scene->bloomRadius());
        lightData.lightFlags = QVector4D(ctx.scene->volumetricEnabled() ? 1.0f : 0.0f,
                                         ctx.scene->smokeNoiseEnabled() ? 1.0f : 0.0f,
                                         ctx.scene->shadowsEnabled() ? 1.0f : 0.0f,
                                         0.0f);
        for (int i = 0; i < maxLights; ++i)
        {
            const Light &l = ctx.scene->lights()[i];
            const QVector3D dir = l.direction.normalized();
            lightData.posRange[i] = QVector4D(l.position, l.range);
            lightData.colorIntensity[i] = QVector4D(l.color, l.intensity);
            lightData.dirInner[i] = QVector4D(dir, qCos(l.innerCone));
            lightData.lightBeam[i] = QVector4D(l.beamRadius, float(l.beamShape), 0.0f, 0.0f);
            float extraZ = 0.0f;
            float extraW = 0.0f;
            if (l.type == Light::Type::Area)
            {
                extraZ = l.areaSize.x();
                extraW = l.areaSize.y();
            } else if (l.type == Light::Type::Spot)
            {
                extraZ = l.goboPath.isEmpty() ? -1.0f : float(i);
                extraW = float(l.qualitySteps);
            }
            lightData.other[i] = QVector4D(qCos(l.outerCone),
                                           float(l.type),
                                           extraZ,
                                           extraW);
        }
    }

    const bool cameraDirty = ctx.scene->cameraDirty() || ctx.scene->timeDirty();
    struct CameraData
    {
        float view[16];
        float invViewProj[16];
        QVector4D cameraPos;
    } camData;
    if (cameraDirty)
    {
        const QMatrix4x4 view = ctx.scene->camera().viewMatrix();
        const QMatrix4x4 viewProj = ctx.rhi->rhi()->clipSpaceCorrMatrix()
                * ctx.scene->camera().projectionMatrix()
                * ctx.scene->camera().viewMatrix();
        const QMatrix4x4 invViewProj = viewProj.inverted();
        std::memcpy(camData.view, view.constData(), sizeof(camData.view));
        std::memcpy(camData.invViewProj, invViewProj.constData(), sizeof(camData.invViewProj));
        camData.cameraPos = QVector4D(ctx.scene->camera().position(), ctx.scene->timeSeconds());
    }

    struct ShadowDataGpu
    {
        float lightViewProj[3][16];
        float splits[4];
        float dirLightDir[4];
        float dirLightColorIntensity[4];
        float spotLightViewProj[kMaxLights][16];
        float spotShadowParams[kMaxLights][4];
        float shadowDepthParams[4];
    } shadowData;
    memset(&shadowData, 0, sizeof(ShadowDataGpu));
    if (ctx.shadows && ctx.shadows->cascadeCount > 0)
    {
        for (int i = 0; i < ctx.shadows->cascadeCount; ++i)
            std::memcpy(shadowData.lightViewProj[i], ctx.shadows->lightViewProj[i].constData(), sizeof(shadowData.lightViewProj[i]));
        shadowData.splits[0] = ctx.shadows->splits.x();
        shadowData.splits[1] = ctx.shadows->splits.y();
        shadowData.splits[2] = ctx.shadows->splits.z();
        shadowData.splits[3] = ctx.shadows->splits.w();
        shadowData.dirLightDir[0] = ctx.shadows->dirLightDir.x();
        shadowData.dirLightDir[1] = ctx.shadows->dirLightDir.y();
        shadowData.dirLightDir[2] = ctx.shadows->dirLightDir.z();
        shadowData.dirLightDir[3] = ctx.shadows->dirLightDir.w();
        shadowData.dirLightColorIntensity[0] = ctx.shadows->dirLightColorIntensity.x();
        shadowData.dirLightColorIntensity[1] = ctx.shadows->dirLightColorIntensity.y();
        shadowData.dirLightColorIntensity[2] = ctx.shadows->dirLightColorIntensity.z();
        shadowData.dirLightColorIntensity[3] = ctx.shadows->dirLightColorIntensity.w();
    }

    if (shadowData.dirLightColorIntensity[3] <= 0.0f && ctx.scene)
    {
        for (const Light &l : ctx.scene->lights())
        {
            if (l.type != Light::Type::Directional)
                continue;
            const QVector3D dir = l.direction.normalized();
            shadowData.dirLightDir[0] = dir.x();
            shadowData.dirLightDir[1] = dir.y();
            shadowData.dirLightDir[2] = dir.z();
            shadowData.dirLightDir[3] = 0.0f;
            shadowData.dirLightColorIntensity[0] = l.color.x();
            shadowData.dirLightColorIntensity[1] = l.color.y();
            shadowData.dirLightColorIntensity[2] = l.color.z();
            shadowData.dirLightColorIntensity[3] = l.intensity;
            break;
        }
    }
    if (ctx.shadows)
    {
        for (int i = 0; i < kMaxLights; ++i)
        {
            std::memcpy(shadowData.spotLightViewProj[i], ctx.shadows->spotLightViewProj[i].constData(), sizeof(shadowData.spotLightViewProj[i]));
            shadowData.spotShadowParams[i][0] = ctx.shadows->spotShadowParams[i].x();
            shadowData.spotShadowParams[i][1] = ctx.shadows->spotShadowParams[i].y();
            shadowData.spotShadowParams[i][2] = ctx.shadows->spotShadowParams[i].z();
            shadowData.spotShadowParams[i][3] = ctx.shadows->spotShadowParams[i].w();
        }
        shadowData.shadowDepthParams[0] = ctx.shadows->shadowDepthParams.x();
        shadowData.shadowDepthParams[1] = ctx.shadows->shadowDepthParams.y();
        shadowData.shadowDepthParams[2] = ctx.shadows->shadowDepthParams.z();
        shadowData.shadowDepthParams[3] = m_gbufWorldPosFloat ? 1.0f : 0.0f;
    }

    QRhiResourceUpdateBatch *u = ctx.rhi->rhi()->nextResourceUpdateBatch();
    if (m_lightsUbo)
    {
        if (lightDataDirty)
        {
            if (ctx.rhi->rhi()->backend() == QRhi::D3D11)
                u->updateDynamicBuffer(m_lightsUbo, 0, sizeof(LightsData), &lightData);
            else
                u->uploadStaticBuffer(m_lightsUbo, 0, sizeof(LightsData), &lightData);
        }
    }
    if (m_cameraUbo)
    {
        if (cameraDirty)
            u->updateDynamicBuffer(m_cameraUbo, 0, sizeof(CameraData), &camData);
    }
    if (m_shadowUbo)
    {
        if (ctx.rhi->rhi()->backend() == QRhi::D3D11)
            u->updateDynamicBuffer(m_shadowUbo, 0, sizeof(ShadowDataGpu), &shadowData);
        else
            u->uploadStaticBuffer(m_shadowUbo, 0, sizeof(ShadowDataGpu), &shadowData);
    }
    bool flipSampleY = !ctx.rhi->rhi()->isYUpInFramebuffer();
    bool flipNdcY = !ctx.rhi->rhi()->isYUpInNDC();
    const float cameraFar = ctx.scene ? qMax(1.0f, ctx.scene->camera().farPlane()) : 300.0f;
    const QVector4D flipData(flipSampleY ? 1.0f : 0.0f,
                             flipNdcY ? 1.0f : 0.0f,
                             cameraFar,
                             0.0f);
    if (flipData != m_lastFlip)
    {
        u->updateDynamicBuffer(m_flipUbo, 0, sizeof(flipData), &flipData);
        m_lastFlip = flipData;
    }
    if (m_useLightCulling && m_lightCullUbo && ctx.lightCulling)
    {
        struct LightCullParams
        {
            QVector4D screen;
            QVector4D cluster;
            QVector4D zParams;
            QVector4D flags;
        } params;
        const QSize size = rt->pixelSize();
        params.screen = QVector4D(float(size.width()), float(size.height()),
                                  1.0f / float(size.width()), 1.0f / float(size.height()));
        params.cluster = QVector4D(float(ctx.lightCulling->clusterCountX),
                                   float(ctx.lightCulling->clusterCountY),
                                   float(ctx.lightCulling->clusterCountZ),
                                   float(ctx.lightCulling->clusterSize));
        params.zParams = QVector4D(ctx.lightCulling->logScale,
                                   ctx.lightCulling->logBias,
                                   ctx.lightCulling->nearPlane,
                                   ctx.lightCulling->farPlane);
        params.flags = QVector4D(ctx.lightCulling->enabled ? 1.0f : 0.0f,
                                 0.0f, 0.0f, 0.0f);
        if (!m_lightCullParamsValid
                || params.screen != m_lastLightCullScreen
                || params.cluster != m_lastLightCullCluster
                || params.zParams != m_lastLightCullZParams
                || params.flags != m_lastLightCullFlags)
        {
            u->updateDynamicBuffer(m_lightCullUbo, 0, sizeof(LightCullParams), &params);
            m_lastLightCullScreen = params.screen;
            m_lastLightCullCluster = params.cluster;
            m_lastLightCullZParams = params.zParams;
            m_lastLightCullFlags = params.flags;
            m_lightCullParamsValid = true;
        }
    }
    updateGoboTextures(ctx, u);

    cb->resourceUpdate(u);

    const QColor clear(0, 0, 0);
    const QRhiDepthStencilClearValue dsClear(1.0f, 0);
    cb->beginPass(rt, clear, dsClear);
    cb->setGraphicsPipeline(m_pipeline);
    cb->setViewport(QRhiViewport(0, 0, rt->pixelSize().width(), rt->pixelSize().height()));
    cb->setShaderResources(m_srb);
    cb->draw(3);

    bool anySelected = false;
    bool selectionDirty = !m_selectionCacheValid;
    for (const Mesh &mesh : ctx.scene->meshes())
    {
        if (!mesh.selected || !mesh.visible)
            continue;
        anySelected = true;
        if (mesh.selectionDirty || mesh.worldBoundsDirty)
            selectionDirty = true;
    }
    if (anySelected != m_selectionAnySelected)
        selectionDirty = true;

    if (anySelected)
        ensureSelectionBoxesPipeline(ctx, rt);

    if (anySelected && m_selectionPipeline && m_selectionSrb && m_selectionVbuf)
    {
        struct SelectionVertex
        {
            float x;
            float y;
            float z;
            float r;
            float g;
            float b;
        };
        struct GroupBounds
        {
            QVector3D minV;
            QVector3D maxV;
            bool hasBounds = false;
            bool hasGeneric = false;
        };

        const auto &meshes = ctx.scene->meshes();
        const int maxVertices = m_selectionMaxVertices > 0 ? m_selectionMaxVertices : 0;
        std::vector<SelectionVertex> vertices;
        if (selectionDirty && maxVertices > 0)
            vertices.reserve(qMin(int(meshes.size()) * 24, maxVertices));

        const int edges[12][2] = {
            {0, 1}, {1, 2}, {2, 3}, {3, 0},
            {4, 5}, {5, 6}, {6, 7}, {7, 4},
            {0, 4}, {1, 5}, {2, 6}, {3, 7}
        };

        auto getLocalBounds = [](Mesh &mesh, QVector3D &minV, QVector3D &maxV) -> bool {
            if (mesh.boundsValid)
            {
                minV = mesh.boundsMin;
                maxV = mesh.boundsMax;
                return true;
            }
            if (mesh.vertices.isEmpty())
                return false;
            minV = QVector3D(mesh.vertices[0].px, mesh.vertices[0].py, mesh.vertices[0].pz);
            maxV = minV;
            for (const Vertex &v : mesh.vertices)
            {
                minV.setX(qMin(minV.x(), v.px));
                minV.setY(qMin(minV.y(), v.py));
                minV.setZ(qMin(minV.z(), v.pz));
                maxV.setX(qMax(maxV.x(), v.px));
                maxV.setY(qMax(maxV.y(), v.py));
                maxV.setZ(qMax(maxV.z(), v.pz));
            }
            return true;
        };

        auto updateWorldBounds = [&](Mesh &mesh) -> bool {
            QVector3D localMin;
            QVector3D localMax;
            if (!getLocalBounds(mesh, localMin, localMax))
                return false;
            QVector3D corners[8] = {
                { localMin.x(), localMin.y(), localMin.z() },
                { localMax.x(), localMin.y(), localMin.z() },
                { localMax.x(), localMax.y(), localMin.z() },
                { localMin.x(), localMax.y(), localMin.z() },
                { localMin.x(), localMin.y(), localMax.z() },
                { localMax.x(), localMin.y(), localMax.z() },
                { localMax.x(), localMax.y(), localMax.z() },
                { localMin.x(), localMax.y(), localMax.z() }
            };
            QVector3D minV;
            QVector3D maxV;
            for (int i = 0; i < 8; ++i)
            {
                const QVector3D world = (mesh.modelMatrix * QVector4D(corners[i], 1.0f)).toVector3D();
                if (i == 0)
                {
                    minV = world;
                    maxV = world;
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
            mesh.worldBoundsMin = minV;
            mesh.worldBoundsMax = maxV;
            mesh.worldBoundsValid = true;
            mesh.worldBoundsDirty = false;
            return true;
        };

        if (selectionDirty)
        {
            QHash<int, GroupBounds> boundsByGroup;
            boundsByGroup.reserve(meshes.size());

            for (int meshIndex = 0; meshIndex < meshes.size(); ++meshIndex)
            {
                Mesh &mesh = const_cast<Mesh &>(meshes[meshIndex]);
                if (!mesh.selected || !mesh.visible)
                    continue;
                if (!mesh.worldBoundsValid || mesh.worldBoundsDirty)
                {
                    if (!updateWorldBounds(mesh))
                        continue;
                }
                const int groupId = mesh.selectionGroup >= 0 ? mesh.selectionGroup : meshIndex;
                GroupBounds &group = boundsByGroup[groupId];
                if (!group.hasBounds)
                {
                    group.minV = mesh.worldBoundsMin;
                    group.maxV = mesh.worldBoundsMax;
                    group.hasBounds = true;
                }
                else
                {
                    group.minV.setX(qMin(group.minV.x(), mesh.worldBoundsMin.x()));
                    group.minV.setY(qMin(group.minV.y(), mesh.worldBoundsMin.y()));
                    group.minV.setZ(qMin(group.minV.z(), mesh.worldBoundsMin.z()));
                    group.maxV.setX(qMax(group.maxV.x(), mesh.worldBoundsMax.x()));
                    group.maxV.setY(qMax(group.maxV.y(), mesh.worldBoundsMax.y()));
                    group.maxV.setZ(qMax(group.maxV.z(), mesh.worldBoundsMax.z()));
                }
                if (mesh.selectionDomain == Mesh::SelectionDomain::GenericItem)
                    group.hasGeneric = true;
                mesh.selectionDirty = false;
            }

            for (auto it = boundsByGroup.cbegin(); it != boundsByGroup.cend(); ++it)
            {
                if (!it.value().hasBounds)
                    continue;
                const QVector3D minV = it.value().minV;
                const QVector3D maxV = it.value().maxV;
                const QVector3D color = it.value().hasGeneric
                        ? QVector3D(0.0f, 1.0f, 0.0f)
                        : QVector3D(1.0f, 1.0f, 0.0f);
                const QVector3D corners[8] = {
                    { minV.x(), minV.y(), minV.z() },
                    { maxV.x(), minV.y(), minV.z() },
                    { maxV.x(), maxV.y(), minV.z() },
                    { minV.x(), maxV.y(), minV.z() },
                    { minV.x(), minV.y(), maxV.z() },
                    { maxV.x(), minV.y(), maxV.z() },
                    { maxV.x(), maxV.y(), maxV.z() },
                    { minV.x(), maxV.y(), maxV.z() }
                };
                for (const auto &edge : edges)
                {
                    if (maxVertices > 0 && int(vertices.size() + 2) > maxVertices)
                        break;
                    const QVector3D a = corners[edge[0]];
                    const QVector3D b = corners[edge[1]];
                    vertices.push_back({ a.x(), a.y(), a.z(), color.x(), color.y(), color.z() });
                    vertices.push_back({ b.x(), b.y(), b.z(), color.x(), color.y(), color.z() });
                }
            }

            m_selectionVertexCount = int(vertices.size());
            m_selectionCacheValid = true;
        }

        if (m_selectionVertexCount > 0)
        {
            QRhiResourceUpdateBatch *u = ctx.rhi->rhi()->nextResourceUpdateBatch();
            QMatrix4x4 viewProj = ctx.rhi->rhi()->clipSpaceCorrMatrix()
                    * ctx.scene->camera().projectionMatrix()
                    * ctx.scene->camera().viewMatrix();
            const quint32 mat4Size = 16 * sizeof(float);
            bool flipSampleY = !ctx.rhi->rhi()->isYUpInFramebuffer();
            const QVector4D depthParams(ctx.shadows ? ctx.shadows->shadowDepthParams.x() : 1.0f,
                                        ctx.shadows ? ctx.shadows->shadowDepthParams.y() : 0.0f,
                                        ctx.shadows ? ctx.shadows->shadowDepthParams.z() : 0.0f,
                                        flipSampleY ? 1.0f : 0.0f);
            const QSize rtSize = rt->pixelSize();
            const QVector4D screenParams(float(rtSize.width()),
                                         float(rtSize.height()),
                                         1.0f / float(rtSize.width()),
                                         1.0f / float(rtSize.height()));
            u->updateDynamicBuffer(m_selectionUbo, 0, mat4Size, viewProj.constData());
            u->updateDynamicBuffer(m_selectionDepthUbo, 0, sizeof(depthParams), &depthParams);
            u->updateDynamicBuffer(m_selectionScreenUbo, 0, sizeof(screenParams), &screenParams);
            if (selectionDirty && !vertices.empty())
            {
                u->updateDynamicBuffer(m_selectionVbuf, 0,
                                       int(vertices.size() * sizeof(SelectionVertex)), vertices.data());
            }
            cb->resourceUpdate(u);

            cb->setGraphicsPipeline(m_selectionPipeline);
            cb->setViewport(QRhiViewport(0, 0, rt->pixelSize().width(), rt->pixelSize().height()));
            cb->setShaderResources(m_selectionSrb);
            const QRhiCommandBuffer::VertexInput vbufBinding(m_selectionVbuf, 0);
            cb->setVertexInput(0, 1, &vbufBinding);
            cb->draw(quint32(m_selectionVertexCount));
        }
    }
    else if (!anySelected && m_selectionAnySelected)
    {
        m_selectionVertexCount = 0;
        m_selectionCacheValid = true;
    }
    m_selectionAnySelected = anySelected;

    cb->endPass();
}

void PassLighting::ensurePipeline(FrameContext &ctx)
{
    if (!ctx.rhi || !ctx.targets || !ctx.shaders)
        return;

    QRhiRenderTarget *swapRt = ctx.rhi->swapchainRenderTarget();
    if (!swapRt)
        return;

    const QSize size = swapRt->pixelSize();
    const RenderTargetCache::LightingTargets lighting = ctx.targets->getOrCreateLightingTarget(size, 1);
    QRhiRenderTarget *rt = lighting.rt;
    if (!rt)
        return;
    const RenderTargetCache::GBufferTargets gbuf = ctx.targets->getOrCreateGBuffer(size, 1);
    const bool gbufChanged = gbuf.color0 != m_gbufColor0 ||
                             gbuf.color1 != m_gbufColor1 ||
                             gbuf.color2 != m_gbufColor2 ||
                             gbuf.color3 != m_gbufColor3 ||
                             gbuf.depth != m_gbufDepth;
    const bool reverseZ = ctx.rhi->rhi()->clipSpaceCorrMatrix()(2, 2) < 0.0f;
    const bool d3d11 = ctx.rhi->rhi()->backend() == QRhi::D3D11;
    const bool metal = ctx.rhi->rhi()->backend() == QRhi::Metal;
    const bool useLightCulling = ctx.lightCulling
            && ctx.lightCulling->enabled
            && ctx.lightCulling->clusterLightIndexTexture;
    const bool effectiveLightCulling = useLightCulling;
    bool spotChanged = false;
    if (ctx.shadows)
    {
        spotChanged = m_spotShadowMapArray != ctx.shadows->spotShadowMapArray;
    }
    else
    {
        spotChanged = m_spotShadowMapArray != nullptr;
    }

    bool shadowsChanged = false;
    if (ctx.shadows)
    {
        for (int i = 0; i < 3; ++i)
        {
            if (m_shadowMapRefs[i] != ctx.shadows->shadowMaps[i])
                shadowsChanged = true;
        }
    }
    if (m_pipeline && m_rpDesc == rt->renderPassDescriptor() && !shadowsChanged && !gbufChanged
            && !spotChanged && m_reverseZ == reverseZ && m_useLightCulling == effectiveLightCulling
            && (!effectiveLightCulling || m_lightIndexTexture == ctx.lightCulling->clusterLightIndexTexture))
        return;

    delete m_pipeline;
    m_pipeline = nullptr;
    delete m_srb;
    m_srb = nullptr;
    delete m_sampler;
    m_sampler = nullptr;
    delete m_shadowSampler;
    m_shadowSampler = nullptr;
    delete m_spotShadowSampler;
    m_spotShadowSampler = nullptr;
    delete m_goboSampler;
    m_goboSampler = nullptr;
    delete m_selectionPipeline;
    m_selectionPipeline = nullptr;
    delete m_selectionSrb;
    m_selectionSrb = nullptr;
    m_selectionRpDesc = nullptr;
    m_selectionCacheValid = false;
    m_selectionVertexCount = 0;
    m_selectionAnySelected = false;
    delete m_lightsUbo;
    m_lightsUbo = nullptr;
    delete m_cameraUbo;
    m_cameraUbo = nullptr;
    delete m_shadowUbo;
    m_shadowUbo = nullptr;
    delete m_flipUbo;
    m_flipUbo = nullptr;
    delete m_lightCullUbo;
    m_lightCullUbo = nullptr;
    delete m_lightIndexSampler;
    m_lightIndexSampler = nullptr;
    m_lightIndexTexture = nullptr;
    delete m_spotGoboMap;
    m_spotGoboMap = nullptr;
    m_spotShadowMapArray = nullptr;
    for (QString &path : m_spotGoboPaths)
        path = QString();
    m_lastFlip = QVector4D(-1.0f, -1.0f, 0.0f, 0.0f);
    m_lightCullParamsValid = false;

    m_sampler = ctx.rhi->rhi()->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                           QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);
    if (!m_sampler->create())
        return;

    if (!metal)
    {
        m_shadowSampler = ctx.rhi->rhi()->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                                     QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);
        if (!m_shadowSampler->create())
            return;

        m_spotShadowSampler = ctx.rhi->rhi()->newSampler(QRhiSampler::Nearest, QRhiSampler::Nearest, QRhiSampler::None,
                                                         QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);
        if (!m_spotShadowSampler->create())
            return;
        m_goboSampler = ctx.rhi->rhi()->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                                   QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);
        if (!m_goboSampler->create())
            return;
    }
    else
    {
        m_spotShadowSampler = ctx.rhi->rhi()->newSampler(QRhiSampler::Nearest, QRhiSampler::Nearest, QRhiSampler::None,
                                                         QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);
        if (!m_spotShadowSampler->create())
            return;
        m_goboSampler = ctx.rhi->rhi()->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                                   QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);
        if (!m_goboSampler->create())
            return;
    }
    m_reverseZ = reverseZ;
    m_useLightCulling = effectiveLightCulling;

    struct LightsDataSize
    {
        QVector4D lightCount;
        QVector4D lightParams;
        QVector4D lightFlags;
        QVector4D lightBeam[kMaxLights];
        QVector4D posRange[kMaxLights];
        QVector4D colorIntensity[kMaxLights];
        QVector4D dirInner[kMaxLights];
        QVector4D other[kMaxLights];
    };
    m_lightsUbo = ctx.rhi->rhi()->newBuffer(d3d11 ? QRhiBuffer::Dynamic : QRhiBuffer::Static,
                                            d3d11 ? QRhiBuffer::UniformBuffer : QRhiBuffer::StorageBuffer,
                                            sizeof(LightsDataSize));
    struct CameraDataSize
    {
        float view[16];
        float invViewProj[16];
        QVector4D cameraPos;
    };
    m_cameraUbo = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer,
                                            sizeof(CameraDataSize));
    struct ShadowDataGpu
    {
        float lightViewProj[3][16];
        float splits[4];
        float dirLightDir[4];
        float dirLightColorIntensity[4];
        float spotLightViewProj[kMaxLights][16];
        float spotShadowParams[kMaxLights][4];
        float shadowDepthParams[4];
    };
    m_shadowUbo = ctx.rhi->rhi()->newBuffer(d3d11 ? QRhiBuffer::Dynamic : QRhiBuffer::Static,
                                            d3d11 ? QRhiBuffer::UniformBuffer : QRhiBuffer::StorageBuffer,
                                            sizeof(ShadowDataGpu));
    m_flipUbo = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(QVector4D));
    if (m_useLightCulling)
    {
        m_lightCullUbo = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer,
                                                   sizeof(QVector4D) * 4);
        if (!m_lightCullUbo->create())
            return;
        m_lightIndexTexture = ctx.lightCulling ? ctx.lightCulling->clusterLightIndexTexture : nullptr;
        if (!m_lightIndexTexture)
            return;
        if (!m_lightIndexSampler)
        {
            m_lightIndexSampler = ctx.rhi->rhi()->newSampler(QRhiSampler::Nearest,
                                                             QRhiSampler::Nearest,
                                                             QRhiSampler::None,
                                                             QRhiSampler::ClampToEdge,
                                                             QRhiSampler::ClampToEdge);
            if (!m_lightIndexSampler->create())
                return;
        }
    }
    if (!m_flipUbo->create())
        return;
    if (!m_lightsUbo->create() || !m_cameraUbo->create() || !m_shadowUbo->create())
        return;

    if (!gbuf.color0 || !gbuf.color1 || !gbuf.color2 || !gbuf.color3)
    {
        qWarning() << "PassLighting: missing GBuffer textures";
        return;
    }
    m_gbufColor0 = gbuf.color0;
    m_gbufColor1 = gbuf.color1;
    m_gbufColor2 = gbuf.color2;
    m_gbufColor3 = gbuf.color3;
    m_gbufDepth = gbuf.depth;
    m_gbufWorldPosFloat = (gbuf.colorFormat == QRhiTexture::RGBA16F
                           || gbuf.colorFormat == QRhiTexture::RGBA32F);
    m_spotShadowMapArray = ctx.shadows ? ctx.shadows->spotShadowMapArray : nullptr;
    if (!m_spotGoboMap)
    {
        m_spotGoboMap = ctx.rhi->rhi()->newTextureArray(QRhiTexture::RGBA8, kMaxLights, m_spotGoboSize);
        if (!m_spotGoboMap->create())
            return;
    }

    m_srb = ctx.rhi->rhi()->newShaderResourceBindings();
    QVector<QRhiShaderResourceBinding> bindings;
    if (metal)
    {
        bindings = {
            QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::FragmentStage, m_flipUbo),
            QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, gbuf.color0, m_sampler),
            QRhiShaderResourceBinding::sampledTexture(2, QRhiShaderResourceBinding::FragmentStage, gbuf.color3, m_sampler),
            QRhiShaderResourceBinding::sampledTexture(3, QRhiShaderResourceBinding::FragmentStage, gbuf.color1, m_sampler),
            QRhiShaderResourceBinding::sampledTexture(4, QRhiShaderResourceBinding::FragmentStage, gbuf.color2, m_sampler),
            QRhiShaderResourceBinding::sampledTexture(5, QRhiShaderResourceBinding::FragmentStage, gbuf.depth, m_sampler),
            QRhiShaderResourceBinding::bufferLoad(6, QRhiShaderResourceBinding::FragmentStage, m_lightsUbo),
            QRhiShaderResourceBinding::uniformBuffer(7, QRhiShaderResourceBinding::FragmentStage, m_cameraUbo),
            QRhiShaderResourceBinding::bufferLoad(8, QRhiShaderResourceBinding::FragmentStage, m_shadowUbo)
        };
        if (m_useLightCulling)
        {
            bindings.push_back(QRhiShaderResourceBinding::uniformBuffer(10, QRhiShaderResourceBinding::FragmentStage,
                                                                        m_lightCullUbo));
            bindings.push_back(QRhiShaderResourceBinding::sampledTexture(11, QRhiShaderResourceBinding::FragmentStage,
                                                                         m_lightIndexTexture, m_lightIndexSampler));
        }
        QRhiTexture *spotTex = ctx.shadows ? ctx.shadows->spotShadowMapArray : nullptr;
        if (!spotTex)
            return;
        bindings.push_back(QRhiShaderResourceBinding::sampledTexture(9, QRhiShaderResourceBinding::FragmentStage,
                                                                     spotTex, m_spotShadowSampler));
        bindings.push_back(QRhiShaderResourceBinding::sampledTexture(16, QRhiShaderResourceBinding::FragmentStage,
                                                                     m_spotGoboMap, m_goboSampler));
    }
    else if (d3d11)
    {
        bindings = {
            QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage, gbuf.color0, m_sampler),
            QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, gbuf.color1, m_sampler),
            QRhiShaderResourceBinding::sampledTexture(2, QRhiShaderResourceBinding::FragmentStage, gbuf.color2, m_sampler),
            QRhiShaderResourceBinding::uniformBuffer(20, QRhiShaderResourceBinding::FragmentStage, m_lightsUbo),
            QRhiShaderResourceBinding::uniformBuffer(21, QRhiShaderResourceBinding::FragmentStage, m_cameraUbo),
            QRhiShaderResourceBinding::uniformBuffer(22, QRhiShaderResourceBinding::FragmentStage, m_shadowUbo),
            QRhiShaderResourceBinding::uniformBuffer(23, QRhiShaderResourceBinding::FragmentStage, m_flipUbo)
        };
        if (m_useLightCulling)
        {
            bindings.push_back(QRhiShaderResourceBinding::uniformBuffer(24, QRhiShaderResourceBinding::FragmentStage,
                                                                        m_lightCullUbo));
            bindings.push_back(QRhiShaderResourceBinding::sampledTexture(25, QRhiShaderResourceBinding::FragmentStage,
                                                                         m_lightIndexTexture, m_lightIndexSampler));
        }
    }
    else
    {
        bindings = {
            QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage, gbuf.color0, m_sampler),
            QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, gbuf.color1, m_sampler),
            QRhiShaderResourceBinding::sampledTexture(2, QRhiShaderResourceBinding::FragmentStage, gbuf.color2, m_sampler),
            QRhiShaderResourceBinding::sampledTexture(20, QRhiShaderResourceBinding::FragmentStage, gbuf.color3, m_sampler),
            QRhiShaderResourceBinding::bufferLoad(3, QRhiShaderResourceBinding::FragmentStage, m_lightsUbo),
            QRhiShaderResourceBinding::uniformBuffer(4, QRhiShaderResourceBinding::FragmentStage, m_cameraUbo),
            QRhiShaderResourceBinding::bufferLoad(5, QRhiShaderResourceBinding::FragmentStage, m_shadowUbo),
            QRhiShaderResourceBinding::uniformBuffer(19, QRhiShaderResourceBinding::FragmentStage, m_flipUbo)
        };
        if (m_useLightCulling)
        {
            bindings.push_back(QRhiShaderResourceBinding::uniformBuffer(21, QRhiShaderResourceBinding::FragmentStage,
                                                                        m_lightCullUbo));
            bindings.push_back(QRhiShaderResourceBinding::sampledTexture(22, QRhiShaderResourceBinding::FragmentStage,
                                                                         m_lightIndexTexture, m_lightIndexSampler));
        }
    }

    if (!metal && ctx.shadows && ctx.shadows->shadowMaps[0])
    {
        const int base = d3d11 ? 3 : 6;
        bindings.push_back(QRhiShaderResourceBinding::sampledTexture(base + 0, QRhiShaderResourceBinding::FragmentStage, ctx.shadows->shadowMaps[0], m_shadowSampler));
        bindings.push_back(QRhiShaderResourceBinding::sampledTexture(base + 1, QRhiShaderResourceBinding::FragmentStage, ctx.shadows->shadowMaps[1], m_shadowSampler));
        bindings.push_back(QRhiShaderResourceBinding::sampledTexture(base + 2, QRhiShaderResourceBinding::FragmentStage, ctx.shadows->shadowMaps[2], m_shadowSampler));
        for (int i = 0; i < 3; ++i)
            m_shadowMapRefs[i] = ctx.shadows->shadowMaps[i];
    }
    else if (!metal)
    {
        const int base = d3d11 ? 3 : 6;
        bindings.push_back(QRhiShaderResourceBinding::sampledTexture(base + 0, QRhiShaderResourceBinding::FragmentStage, gbuf.depth, m_shadowSampler));
        bindings.push_back(QRhiShaderResourceBinding::sampledTexture(base + 1, QRhiShaderResourceBinding::FragmentStage, gbuf.depth, m_shadowSampler));
        bindings.push_back(QRhiShaderResourceBinding::sampledTexture(base + 2, QRhiShaderResourceBinding::FragmentStage, gbuf.depth, m_shadowSampler));
        for (int i = 0; i < 3; ++i)
            m_shadowMapRefs[i] = nullptr;
    }
    if (!metal)
    {
        QRhiTexture *spotTex = ctx.shadows ? ctx.shadows->spotShadowMapArray : nullptr;
        if (!spotTex)
            return;
        const int base = d3d11 ? 6 : 9;
        bindings.push_back(QRhiShaderResourceBinding::sampledTexture(base, QRhiShaderResourceBinding::FragmentStage,
                                                                     spotTex, m_spotShadowSampler));
        if (d3d11)
        {
            bindings.push_back(QRhiShaderResourceBinding::sampledTexture(13, QRhiShaderResourceBinding::FragmentStage,
                                                                         gbuf.depth, m_sampler));
            bindings.push_back(QRhiShaderResourceBinding::sampledTexture(14, QRhiShaderResourceBinding::FragmentStage,
                                                                         m_spotGoboMap, m_goboSampler));
        }
        else
        {
            bindings.push_back(QRhiShaderResourceBinding::sampledTexture(17, QRhiShaderResourceBinding::FragmentStage,
                                                                         gbuf.depth, m_sampler));
            bindings.push_back(QRhiShaderResourceBinding::sampledTexture(18, QRhiShaderResourceBinding::FragmentStage,
                                                                         m_spotGoboMap, m_goboSampler));
        }
    }

    m_srb->setBindings(bindings.begin(), bindings.end());
    if (!m_srb->create())
    {
        qWarning() << "PassLighting: failed to create SRB";
        return;
    }
    const QRhiShaderStage vs = ctx.shaders->loadStage(QRhiShaderStage::Vertex, QStringLiteral(":/shaders/lighting.vert.qsb"));
    QString fragPath;
    if (d3d11)
        fragPath = m_useLightCulling
                ? QStringLiteral(":/shaders/lighting_cull_d3d.frag.qsb")
                : QStringLiteral(":/shaders/lighting_d3d.frag.qsb");
    else if (metal)
        fragPath = m_useLightCulling
                ? QStringLiteral(":/shaders/lighting_cull_metal.frag.qsb")
                : QStringLiteral(":/shaders/lighting_metal.frag.qsb");
    else
        fragPath = m_useLightCulling
                ? QStringLiteral(":/shaders/lighting_cull.frag.qsb")
                : QStringLiteral(":/shaders/lighting.frag.qsb");
    const QRhiShaderStage fs = ctx.shaders->loadStage(QRhiShaderStage::Fragment, fragPath);
    if (!vs.shader().isValid() || !fs.shader().isValid())
        return;

    QRhiGraphicsPipeline *pipeline = ctx.rhi->rhi()->newGraphicsPipeline();
    pipeline->setShaderStages({ vs, fs });
    pipeline->setSampleCount(1);
    pipeline->setCullMode(QRhiGraphicsPipeline::None);
    pipeline->setDepthTest(false);
    pipeline->setDepthWrite(false);
    pipeline->setShaderResourceBindings(m_srb);
    pipeline->setRenderPassDescriptor(rt->renderPassDescriptor());

    if (!pipeline->create())
    {
        qWarning() << "PassLighting: failed to create pipeline";
        return;
    }

    m_pipeline = pipeline;
    m_rpDesc = rt->renderPassDescriptor();
}

void PassLighting::ensureSelectionBoxesPipeline(FrameContext &ctx, QRhiRenderTarget *rt)
{
    if (!ctx.rhi || !ctx.shaders || !rt || !ctx.targets)
        return;
    if (!m_sampler)
        return;

    if (m_selectionPipeline && m_selectionRpDesc == rt->renderPassDescriptor())
        return;

    delete m_selectionPipeline;
    m_selectionPipeline = nullptr;
    delete m_selectionSrb;
    m_selectionSrb = nullptr;
    delete m_selectionUbo;
    m_selectionUbo = nullptr;
    delete m_selectionVbuf;
    m_selectionVbuf = nullptr;
    delete m_selectionDepthUbo;
    m_selectionDepthUbo = nullptr;
    delete m_selectionScreenUbo;
    m_selectionScreenUbo = nullptr;
    m_selectionCacheValid = false;
    m_selectionVertexCount = 0;

    const quint32 mat4Size = 16 * sizeof(float);
    const int selectionVertexStride = int(sizeof(float) * 6);
    m_selectionUbo = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, mat4Size);
    if (!m_selectionUbo->create())
        return;

    m_selectionDepthUbo = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(QVector4D));
    if (!m_selectionDepthUbo->create())
        return;
    m_selectionScreenUbo = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(QVector4D));
    if (!m_selectionScreenUbo->create())
        return;

    const int maxMeshes = 256;
    m_selectionMaxVertices = maxMeshes * 24;
    m_selectionVbuf = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::VertexBuffer,
                                            m_selectionMaxVertices * selectionVertexStride);
    if (!m_selectionVbuf->create())
        return;

    QRhiTexture *gbufDepth = ctx.targets->getOrCreateGBuffer(rt->pixelSize(), 1).depth;
    if (!gbufDepth)
        return;

    m_selectionSrb = ctx.rhi->rhi()->newShaderResourceBindings();
    m_selectionSrb->setBindings({
        QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, m_selectionUbo),
        QRhiShaderResourceBinding::uniformBuffer(1, QRhiShaderResourceBinding::FragmentStage, m_selectionDepthUbo),
        QRhiShaderResourceBinding::uniformBuffer(2, QRhiShaderResourceBinding::FragmentStage, m_selectionScreenUbo),
        QRhiShaderResourceBinding::sampledTexture(3, QRhiShaderResourceBinding::FragmentStage,
                                                  gbufDepth,
                                                  m_sampler)
    });
    if (!m_selectionSrb->create())
        return;

    const QRhiShaderStage vs = ctx.shaders->loadStage(QRhiShaderStage::Vertex, QStringLiteral(":/shaders/selection_box.vert.qsb"));
    const QRhiShaderStage fs = ctx.shaders->loadStage(QRhiShaderStage::Fragment, QStringLiteral(":/shaders/selection_box.frag.qsb"));
    if (!vs.shader().isValid() || !fs.shader().isValid())
        return;

    QRhiGraphicsPipeline *pipeline = ctx.rhi->rhi()->newGraphicsPipeline();
    pipeline->setShaderStages({ vs, fs });
    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({ QRhiVertexInputBinding(selectionVertexStride) });
    inputLayout.setAttributes({
        QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float3, 0),
        QRhiVertexInputAttribute(0, 1, QRhiVertexInputAttribute::Float3, 3 * int(sizeof(float)))
    });
    pipeline->setVertexInputLayout(inputLayout);
    pipeline->setTopology(QRhiGraphicsPipeline::Lines);
    pipeline->setCullMode(QRhiGraphicsPipeline::None);
    pipeline->setDepthTest(false);
    pipeline->setDepthWrite(false);
    pipeline->setShaderResourceBindings(m_selectionSrb);
    pipeline->setRenderPassDescriptor(rt->renderPassDescriptor());

    if (!pipeline->create())
        return;

    m_selectionPipeline = pipeline;
    m_selectionRpDesc = rt->renderPassDescriptor();
}
