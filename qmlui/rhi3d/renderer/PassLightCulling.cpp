/*
  Q Light Controller Plus
  PassLightCulling.cpp

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

#include "renderer/PassLightCulling.h"

#include <QtCore/QVector>
#include <QtCore/QDebug>
#include <QtCore/QtGlobal>
#include <QtGui/QMatrix4x4>
#include <cstring>
#include <cmath>

#include "core/RhiContext.h"
#include "core/ShaderManager.h"
#include "scene/Scene.h"

namespace {

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
};

struct CullParams
{
    float view[16];
    float proj[16];
    QVector4D screen; // x=width y=height z=invW w=invH
    QVector4D cluster; // x=countX y=countY z=countZ w=clusterSize
    QVector4D zParams; // x=logScale y=logBias z=near w=far
    QVector4D flags;   // x=enabled
};

} // namespace

void PassLightCulling::prepare(FrameContext &ctx)
{
    if (!ctx.rhi || !ctx.shaders || !ctx.lightCulling)
        return;
    QRhi *rhi = ctx.rhi->rhi();
    if (!rhi)
        return;
    const bool supported = rhi->isFeatureSupported(QRhi::Compute);
    ctx.lightCulling->enabled = supported;
    if (!supported)
        ctx.lightCulling->clusterLightIndexTexture = nullptr;
    ctx.lightCulling->clusterSize = m_clusterSize;
    ctx.lightCulling->clusterCountZ = m_clusterCountZ;

    if (!supported)
        return;

    QRhiRenderTarget *swapRt = ctx.rhi->swapchainRenderTarget();
    if (!swapRt)
        return;
    const QSize size = swapRt->pixelSize();
    if (size.isEmpty())
    {
        ctx.lightCulling->clusterLightIndexTexture = nullptr;
        return;
    }

    ensureBuffers(ctx, size);
    ensurePipeline(ctx);
    if (m_lightIndexTexture)
    {
        ctx.lightCulling->clusterLightIndexTexture = m_lightIndexTexture;
        ctx.lightCulling->clusterCountX = m_lastClusterCountX;
        ctx.lightCulling->clusterCountY = m_lastClusterCountY;
        ctx.lightCulling->clusterCountZ = m_clusterCountZ;
    }
    if (!m_pipeline || !m_srb)
    {
        ctx.lightCulling->enabled = false;
        ctx.lightCulling->clusterLightIndexTexture = nullptr;
    }
}

void PassLightCulling::execute(FrameContext &ctx)
{
    if (!ctx.rhi || !ctx.scene || !ctx.lightCulling || !ctx.lightCulling->enabled)
        return;
    if (!m_pipeline || !m_srb || !m_lightUbo || !m_cullUbo || !m_lightIndexTexture)
        return;

    QRhiCommandBuffer *cb = ctx.rhi->commandBuffer();
    if (!cb)
        return;

    const int maxLights = qMin(kMaxLights, ctx.scene->lights().size());
    LightsData lightData = {};
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
        }
        else if (l.type == Light::Type::Spot)
        {
            extraZ = l.goboPath.isEmpty() ? -1.0f : float(i);
            extraW = float(l.qualitySteps);
        }
        lightData.other[i] = QVector4D(qCos(l.outerCone),
                                       float(l.type),
                                       extraZ,
                                       extraW);
    }

    const QSize size = m_lastSize;
    const int clusterCountX = m_lastClusterCountX;
    const int clusterCountY = m_lastClusterCountY;
    const int clusterCountZ = m_clusterCountZ;

    CullParams params = {};
    const QMatrix4x4 view = ctx.scene->camera().viewMatrix();
    const QMatrix4x4 proj = ctx.rhi->rhi()->clipSpaceCorrMatrix()
            * ctx.scene->camera().projectionMatrix();
    std::memcpy(params.view, view.constData(), sizeof(params.view));
    std::memcpy(params.proj, proj.constData(), sizeof(params.proj));
    params.screen = QVector4D(float(size.width()), float(size.height()),
                              1.0f / float(size.width()), 1.0f / float(size.height()));
    const float nearPlane = qMax(0.001f, ctx.scene->camera().nearPlane());
    const float farPlane = qMax(nearPlane + 0.001f, ctx.scene->camera().farPlane());
    const float logNear = std::log2(nearPlane);
    const float logFar = std::log2(farPlane);
    const float logScale = float(clusterCountZ) / qMax(0.0001f, logFar - logNear);
    const float logBias = -logNear * logScale;
    params.cluster = QVector4D(float(clusterCountX), float(clusterCountY), float(clusterCountZ), float(m_clusterSize));
    params.zParams = QVector4D(logScale, logBias, nearPlane, farPlane);
    params.flags = QVector4D(1.0f, 0.0f, 0.0f, 0.0f);

    QRhiResourceUpdateBatch *u = ctx.rhi->rhi()->nextResourceUpdateBatch();
    u->uploadStaticBuffer(m_lightUbo, 0, sizeof(LightsData), &lightData);
    u->updateDynamicBuffer(m_cullUbo, 0, sizeof(CullParams), &params);
    cb->resourceUpdate(u);

    cb->beginComputePass();
    cb->setComputePipeline(m_pipeline);
    cb->setShaderResources(m_srb);
    cb->dispatch(clusterCountX, clusterCountY, clusterCountZ);
    cb->endComputePass();

    ctx.lightCulling->clusterLightIndexTexture = m_lightIndexTexture;
    ctx.lightCulling->clusterCountX = clusterCountX;
    ctx.lightCulling->clusterCountY = clusterCountY;
    ctx.lightCulling->clusterCountZ = clusterCountZ;
    ctx.lightCulling->clusterSize = m_clusterSize;
    ctx.lightCulling->logScale = logScale;
    ctx.lightCulling->logBias = logBias;
    ctx.lightCulling->nearPlane = nearPlane;
    ctx.lightCulling->farPlane = farPlane;
}

void PassLightCulling::ensurePipeline(FrameContext &ctx)
{
    if (!ctx.rhi || !ctx.shaders || !m_lightIndexTexture)
        return;
    if (m_pipeline && m_srb)
        return;

    delete m_pipeline;
    m_pipeline = nullptr;
    delete m_srb;
    m_srb = nullptr;

    const QRhiShaderStage cs = ctx.shaders->loadStage(QRhiShaderStage::Compute,
                                                      QStringLiteral(":/shaders/light_cull.comp.qsb"));
    if (!cs.shader().isValid())
        return;

    if (!m_lightUbo)
    {
        m_lightUbo = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Static, QRhiBuffer::StorageBuffer, sizeof(LightsData));
        if (!m_lightUbo->create())
        {
            delete m_lightUbo;
            m_lightUbo = nullptr;
            return;
        }
    }
    if (!m_cullUbo)
    {
        m_cullUbo = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(CullParams));
        if (!m_cullUbo->create())
        {
            delete m_cullUbo;
            m_cullUbo = nullptr;
            return;
        }
    }

    m_srb = ctx.rhi->rhi()->newShaderResourceBindings();
    m_srb->setBindings({
        QRhiShaderResourceBinding::bufferLoad(0, QRhiShaderResourceBinding::ComputeStage, m_lightUbo),
        QRhiShaderResourceBinding::uniformBuffer(1, QRhiShaderResourceBinding::ComputeStage, m_cullUbo),
        QRhiShaderResourceBinding::imageStore(2, QRhiShaderResourceBinding::ComputeStage, m_lightIndexTexture, 0)
    });
    if (!m_srb->create())
    {
        delete m_srb;
        m_srb = nullptr;
        return;
    }

    m_pipeline = ctx.rhi->rhi()->newComputePipeline();
    m_pipeline->setShaderStage(cs);
    m_pipeline->setShaderResourceBindings(m_srb);
    if (!m_pipeline->create())
    {
        delete m_pipeline;
        m_pipeline = nullptr;
        return;
    }
}

void PassLightCulling::ensureBuffers(FrameContext &ctx, const QSize &size)
{
    const int clusterCountX = (size.width() + m_clusterSize - 1) / m_clusterSize;
    const int clusterCountY = (size.height() + m_clusterSize - 1) / m_clusterSize;
    if (size == m_lastSize && clusterCountX == m_lastClusterCountX && clusterCountY == m_lastClusterCountY && m_lightIndexTexture)
        return;

    m_lastSize = size;
    m_lastClusterCountX = clusterCountX;
    m_lastClusterCountY = clusterCountY;

    delete m_lightIndexTexture;
    m_lightIndexTexture = nullptr;
    const int width = kMaxLights + 1;
    const int height = clusterCountX * clusterCountY * m_clusterCountZ;
    if (width <= 0 || height <= 0)
    {
        ctx.lightCulling->enabled = false;
        ctx.lightCulling->clusterLightIndexTexture = nullptr;
        return;
    }
    QRhi *rhi = ctx.rhi->rhi();
    const QRhiTexture::Flags flags = QRhiTexture::UsedWithLoadStore;
    if (!rhi->isTextureFormatSupported(QRhiTexture::R32UI, flags))
    {
        qWarning() << "PassLightCulling: R32UI load/store not supported, disabling clustered culling";
        ctx.lightCulling->enabled = false;
        ctx.lightCulling->clusterLightIndexTexture = nullptr;
        return;
    }
    m_lightIndexTexture = rhi->newTexture(QRhiTexture::R32UI, QSize(width, height), 1, flags);
    if (!m_lightIndexTexture->create())
    {
        delete m_lightIndexTexture;
        m_lightIndexTexture = nullptr;
        ctx.lightCulling->enabled = false;
        ctx.lightCulling->clusterLightIndexTexture = nullptr;
        return;
    }

    if (m_srb)
    {
        delete m_srb;
        m_srb = nullptr;
    }
    if (m_pipeline)
    {
        delete m_pipeline;
        m_pipeline = nullptr;
    }
}
