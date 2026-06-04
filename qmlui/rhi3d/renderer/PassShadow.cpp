/*
  Q Light Controller Plus
  PassShadow.cpp

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

#include "renderer/PassShadow.h"

#include <QtGui/QMatrix4x4>
#include <QtGui/QVector3D>
#include <QtMath>
#include <algorithm>
#include <cstring>
#include <limits>
#include <vector>
#include <QtCore/QtGlobal>
#include <rhi/qrhi.h>

#include "core/RhiContext.h"
#include "core/ShaderManager.h"
#include "scene/Scene.h"

static QVector3D safeUp(const QVector3D &dir)
{
    const QVector3D up(0.0f, 1.0f, 0.0f);
    if (qAbs(QVector3D::dotProduct(dir.normalized(), up)) > 0.99f)
        return QVector3D(0.0f, 0.0f, 1.0f);
    return up;
}

static float spotApexOffset(const Light &light)
{
    if (light.type != Light::Type::Spot)
        return 0.0f;
    if (light.beamShape != Light::BeamShapeType::ConeShape)
        return 0.0f;
    if (light.beamRadius <= 0.0f || light.outerCone <= 1e-4f || light.direction.isNull())
        return 0.0f;
    const float tanOuter = qTan(light.outerCone);
    if (tanOuter <= 1e-4f)
        return 0.0f;
    return light.beamRadius / tanOuter;
}

static QVector3D spotShadowOrigin(const Light &light)
{
    if (light.direction.isNull())
        return light.position;
    return light.position - light.direction.normalized() * spotApexOffset(light);
}

void PassShadow::prepare(FrameContext &ctx)
{
    if (!ctx.rhi || !ctx.shaders)
        return;

    if (m_spotShadowSlots != m_maxSpotShadows)
    {
        m_spotShadowSlots = m_maxSpotShadows;
        m_spotShaderVersion = 0;
    }
    ensureResources(ctx);

    if (ctx.shadows)
    {
        const bool depthZeroToOne = ctx.rhi->rhi()->isClipDepthZeroToOne();
        const float depthScale = depthZeroToOne ? 1.0f : 0.5f;
        const float depthBias = depthZeroToOne ? 0.0f : 0.5f;
        bool reverseZ = ctx.rhi->rhi()->clipSpaceCorrMatrix()(2, 2) < 0.0f;
        if (ctx.rhi->rhi()->backend() == QRhi::D3D11 || ctx.rhi->rhi()->backend() == QRhi::Metal)
            reverseZ = false;
        ctx.shadows->cascadeCount = 0;
        ctx.shadows->splits = {};
        ctx.shadows->dirLightDir = {};
        ctx.shadows->dirLightColorIntensity = {};
        for (int i = 0; i < 3; ++i)
            ctx.shadows->shadowMaps[i] = m_cascades[i].color;
        for (int i = 0; i < kMaxLights; ++i)
        {
            ctx.shadows->spotLightViewProj[i] = QMatrix4x4();
            ctx.shadows->spotShadowParams[i] = QVector4D(-1.0f, 0.0f, 0.0f, 0.0f);
        }
        ctx.shadows->spotShadowCount = 0;
        ctx.shadows->spotShadowMapArray = m_spotShadowMapArray;
        ctx.shadows->shadowDepthParams = QVector4D(depthScale, depthBias,
                                                   reverseZ ? 1.0f : 0.0f, 0.0f);
    }
}

void PassShadow::execute(FrameContext &ctx)
{
    if (!ctx.scene || !ctx.shadows)
        return;

    const bool shadowsEnabled = ctx.scene->shadowsEnabled();
    const Light *dirLight = nullptr;
    const Light *dirShadowLight = nullptr;
    for (const Light &l : ctx.scene->lights())
    {
        if (l.type != Light::Type::Directional)
            continue;
        if (!dirLight)
            dirLight = &l;
        if (!dirShadowLight && l.castShadows)
        {
            dirShadowLight = &l;
        }
    }
    const QMatrix4x4 clipCorr = ctx.rhi->rhi()->clipSpaceCorrMatrix();
    if (ctx.shadows)
    {
        const bool depthZeroToOne = ctx.rhi->rhi()->isClipDepthZeroToOne();
        const float depthScale = depthZeroToOne ? 1.0f : 0.5f;
        const float depthBias = depthZeroToOne ? 0.0f : 0.5f;
        bool reverseZ = clipCorr(2, 2) < 0.0f;
        if (ctx.rhi->rhi()->backend() == QRhi::D3D11 || ctx.rhi->rhi()->backend() == QRhi::Metal)
            reverseZ = false;
        ctx.shadows->shadowDepthParams = QVector4D(depthScale, depthBias,
                                                   reverseZ ? 1.0f : 0.0f, 0.0f);
    }
    if (dirLight)
    {
        ctx.shadows->dirLightDir = QVector4D(dirLight->direction.normalized(), 0.0f);
        ctx.shadows->dirLightColorIntensity = QVector4D(dirLight->color, dirLight->intensity);
    }

    if (dirShadowLight && shadowsEnabled)
    {
        const Camera &cam = ctx.scene->camera();
        const float nearPlane = cam.nearPlane();
        const float farPlane = cam.farPlane();
        const int cascadeCount = 3;
        const float lambda = 0.5f;

        float splits[4];
        splits[0] = nearPlane;
        for (int i = 1; i <= cascadeCount; ++i)
        {
            float p = float(i) / float(cascadeCount);
            float logSplit = nearPlane * std::pow(farPlane / nearPlane, p);
            float uniSplit = nearPlane + (farPlane - nearPlane) * p;
            splits[i] = logSplit * lambda + uniSplit * (1.0f - lambda);
        }

        ctx.shadows->cascadeCount = cascadeCount;
        ctx.shadows->splits = QVector4D(splits[1], splits[2], splits[3], farPlane);

        for (int i = 0; i < cascadeCount; ++i)
        {
            const float cNear = splits[i];
            const float cFar = splits[i + 1];
            QMatrix4x4 lightViewProj = clipCorr * computeLightViewProj(cam, dirShadowLight->direction, cNear, cFar);
            m_cascades[i].lightViewProj = lightViewProj;
            ctx.shadows->lightViewProj[i] = lightViewProj;
            renderCascade(ctx, m_cascades[i], lightViewProj);
        }
    }
    else
    {
        ctx.shadows->cascadeCount = 0;
    }

    for (int i = 0; i < kMaxLights; ++i)
    {
        ctx.shadows->spotLightViewProj[i] = QMatrix4x4();
        ctx.shadows->spotShadowParams[i] = QVector4D(-1.0f, 0.0f, 0.0f, 0.0f);
    }
    ctx.shadows->spotShadowCount = 0;

    const auto &lights = ctx.scene->lights();
    for (int i = 0; i < lights.size(); ++i)
    {
        const Light &light = lights[i];
        if (light.type != Light::Type::Spot || light.range <= 0.0f)
            continue;
        const float apexOffset = spotApexOffset(light);
        const float nearPlane = 1.0f;
        const float farPlane = qMax(light.range + apexOffset, nearPlane + 0.1f);
        const QMatrix4x4 lightViewProj = clipCorr * computeSpotViewProj(light, nearPlane, farPlane);
        ctx.shadows->spotLightViewProj[i] = lightViewProj;
    }

    if (shadowsEnabled && !m_spotRts.isEmpty() && m_spotShadowMapArray)
    {
        const int maxSlots = qMin(int(m_spotRts.size()), m_spotShadowSlots);
        struct SpotCandidate
        {
            int index = -1;
            float score = 0.0f;
            float nearPlane = 1.0f;
            float farPlane = 1.0f;
        };
        QVector<SpotCandidate> candidates;
        candidates.reserve(lights.size());
        const QVector3D camPos = ctx.scene->camera().position();
        for (int i = 0; i < lights.size(); ++i)
        {
            const Light &light = lights[i];
            if (light.type != Light::Type::Spot || !light.castShadows || light.range <= 0.0f)
                continue;
            const float apexOffset = spotApexOffset(light);
            const float nearPlane = 1.0f;
            const float farPlane = qMax(light.range + apexOffset, nearPlane + 0.1f);
            const float range = qMax(light.range, 0.01f);
            const float dist = (camPos - spotShadowOrigin(light)).length();
            const float falloff = 1.0f / (1.0f + (dist * dist) / (range * range));
            const float luminance = 0.2126f * light.color.x()
                                    + 0.7152f * light.color.y()
                                    + 0.0722f * light.color.z();
            const float intensity = qMax(light.intensity, 0.0f);
            const float score = intensity * luminance * falloff;
            if (score <= 0.0f)
                continue;
            candidates.push_back({ i, score, nearPlane, farPlane });
        }
        std::stable_sort(candidates.begin(), candidates.end(),
                         [](const SpotCandidate &a, const SpotCandidate &b) { return a.score > b.score; });
        int slot = 0;
        for (const SpotCandidate &candidate : candidates)
        {
            if (slot >= maxSlots)
                break;
            const int lightIndex = candidate.index;
            const Light &light = lights[lightIndex];
            const QMatrix4x4 lightViewProj = ctx.shadows->spotLightViewProj[lightIndex];
            ctx.shadows->spotShadowParams[lightIndex] = QVector4D(float(slot), 1.0f, candidate.nearPlane, candidate.farPlane);
            renderSpot(ctx, m_spotRts[slot], lightViewProj, spotShadowOrigin(light), candidate.nearPlane, candidate.farPlane, slot);
            ++slot;
        }
        ctx.shadows->spotShadowCount = slot;
        // Debug/capture block removed.
    }
}

void PassShadow::ensureResources(FrameContext &ctx)
{
    constexpr int kSpotShaderVersion = 3;
    const QMatrix4x4 clipCorr = ctx.rhi->rhi()->clipSpaceCorrMatrix();
    bool reverseZ = clipCorr(2, 2) < 0.0f;
    if (ctx.rhi->rhi()->backend() == QRhi::D3D11 || ctx.rhi->rhi()->backend() == QRhi::Metal)
        reverseZ = false;
    const bool haveResources = m_pipeline && m_spotPipeline
                               && m_cascades[0].rt && m_cascades[0].color
                               && m_spotShadowMapArray
                               && !m_spotDepthStencils.isEmpty()
                               && !m_spotRts.isEmpty();
    if (haveResources && m_reverseZ == reverseZ && m_spotShaderVersion == kSpotShaderVersion)
        return;

    if (!ctx.rhi || !ctx.rhi->rhi())
        return;

    if (m_pipeline || m_spotPipeline || m_cascades[0].rt || m_spotShadowMapArray || !m_spotRts.isEmpty())
    {
        delete m_pipeline;
        m_pipeline = nullptr;
        delete m_spotPipeline;
        m_spotPipeline = nullptr;
        delete m_srb;
        m_srb = nullptr;
        delete m_shadowUbo;
        m_shadowUbo = nullptr;
        for (QRhiBuffer *ubo : m_spotShadowUbos)
            delete ubo;
        m_spotShadowUbos.clear();
        delete m_modelUbo;
        m_modelUbo = nullptr;
        for (Cascade &c : m_cascades)
        {
            delete c.rpDesc;
            c.rpDesc = nullptr;
            delete c.rt;
            c.rt = nullptr;
            delete c.color;
            c.color = nullptr;
            delete c.depthStencil;
            c.depthStencil = nullptr;
        }
        delete m_spotRpDesc;
        m_spotRpDesc = nullptr;
        for (QRhiTextureRenderTarget *rt : m_spotRts)
            delete rt;
        m_spotRts.clear();
        delete m_spotShadowMapArray;
        m_spotShadowMapArray = nullptr;
        for (QRhiRenderBuffer *buf : m_spotDepthStencils)
            delete buf;
        m_spotDepthStencils.clear();
    }
    if (ctx.scene)
    {
        for (Mesh &mesh : ctx.scene->meshes())
        {
            delete mesh.shadowSrb;
            mesh.shadowSrb = nullptr;
            for (QRhiShaderResourceBindings *srb : mesh.spotShadowSrbs)
                delete srb;
            mesh.spotShadowSrbs.clear();
        }
    }

    m_reverseZ = reverseZ;
    m_spotShaderVersion = kSpotShaderVersion;

    QRhiTexture::Format colorFormat = QRhiTexture::RGBA16F;
    if (!ctx.rhi->rhi()->isTextureFormatSupported(colorFormat, QRhiTexture::RenderTarget))
        colorFormat = QRhiTexture::RGBA8;
    QRhiTexture::Format spotColorFormat = QRhiTexture::R16F;
    if (!ctx.rhi->rhi()->isTextureFormatSupported(spotColorFormat, QRhiTexture::RenderTarget))
        spotColorFormat = QRhiTexture::RGBA8;

    for (Cascade &c : m_cascades)
    {
        c.color = ctx.rhi->rhi()->newTexture(colorFormat, QSize(m_shadowSize, m_shadowSize), 1, QRhiTexture::RenderTarget);
        if (!c.color->create())
            return;
        c.depthStencil = ctx.rhi->rhi()->newRenderBuffer(QRhiRenderBuffer::DepthStencil,
                                                         QSize(m_shadowSize, m_shadowSize), 1);
        if (!c.depthStencil->create())
            return;
        QRhiTextureRenderTargetDescription rtDesc;
        rtDesc.setColorAttachments({ QRhiColorAttachment(c.color) });
        rtDesc.setDepthStencilBuffer(c.depthStencil);
        c.rt = ctx.rhi->rhi()->newTextureRenderTarget(rtDesc);
        c.rpDesc = c.rt->newCompatibleRenderPassDescriptor();
        c.rt->setRenderPassDescriptor(c.rpDesc);
        if (!c.rt->create())
            return;
    }

    m_spotShadowMapArray = ctx.rhi->rhi()->newTextureArray(spotColorFormat,
                                                           m_spotShadowSlots,
                                                           QSize(m_spotShadowSize, m_spotShadowSize),
                                                           1,
                                                           QRhiTexture::RenderTarget | QRhiTexture::UsedAsTransferSource);
    if (!m_spotShadowMapArray->create())
    {
        delete m_spotShadowMapArray;
        m_spotShadowMapArray = nullptr;
        return;
    }
    m_spotRts.reserve(m_spotShadowSlots);
    m_spotDepthStencils.reserve(m_spotShadowSlots);
    for (int i = 0; i < m_spotShadowSlots; ++i)
    {
        QRhiRenderBuffer *depthStencil = ctx.rhi->rhi()->newRenderBuffer(QRhiRenderBuffer::DepthStencil,
                                                                         QSize(m_spotShadowSize, m_spotShadowSize), 1);
        if (!depthStencil->create())
        {
            delete depthStencil;
            return;
        }
        QRhiColorAttachment colorAttachment(m_spotShadowMapArray);
        colorAttachment.setLayer(i);
        QRhiTextureRenderTargetDescription spotDesc;
        spotDesc.setColorAttachments({ colorAttachment });
        spotDesc.setDepthStencilBuffer(depthStencil);
        QRhiTextureRenderTarget *rt = ctx.rhi->rhi()->newTextureRenderTarget(spotDesc);
        if (!m_spotRpDesc)
            m_spotRpDesc = rt->newCompatibleRenderPassDescriptor();
        rt->setRenderPassDescriptor(m_spotRpDesc);
        if (!rt->create())
        {
            delete rt;
            delete depthStencil;
            return;
        }
        m_spotRts.push_back(rt);
        m_spotDepthStencils.push_back(depthStencil);
    }

    const quint32 mat4Size = 16 * sizeof(float);
    const quint32 shadowUboSize = mat4Size + 3 * 4 * sizeof(float);
    m_shadowUbo = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, shadowUboSize);
    m_modelUbo = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, mat4Size);
    if (!m_shadowUbo->create() || !m_modelUbo->create())
        return;
    m_spotShadowUbos.reserve(m_spotShadowSlots);
    for (int i = 0; i < m_spotShadowSlots; ++i)
    {
        QRhiBuffer *ubo = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, shadowUboSize);
        if (!ubo->create())
        {
            delete ubo;
            return;
        }
        m_spotShadowUbos.push_back(ubo);
    }

    m_srb = ctx.rhi->rhi()->newShaderResourceBindings();
    m_srb->setBindings({
        QRhiShaderResourceBinding::uniformBuffer(0,
                                                 QRhiShaderResourceBinding::VertexStage
                                                 | QRhiShaderResourceBinding::FragmentStage,
                                                 m_shadowUbo),
        QRhiShaderResourceBinding::uniformBuffer(1, QRhiShaderResourceBinding::VertexStage, m_modelUbo)
    });
    if (!m_srb->create())
        return;

    const QRhiShaderStage vs = ctx.shaders->loadStage(QRhiShaderStage::Vertex, QStringLiteral(":/shaders/shadow.vert.qsb"));
    const QRhiShaderStage fsShadow = ctx.shaders->loadStage(QRhiShaderStage::Fragment, QStringLiteral(":/shaders/shadow.frag.qsb"));
    if (!vs.shader().isValid() || !fsShadow.shader().isValid())
        return;

    auto createPipeline = [&](QRhiRenderPassDescriptor *rpDesc,
                              bool enableDepth,
                              bool useMinBlend,
                              QRhiGraphicsPipeline::CompareOp depthOp,
                              const QRhiShaderStage &vsStage,
                              const QRhiShaderStage &fsStage) -> QRhiGraphicsPipeline *
                              {
        if (!rpDesc)
            return nullptr;
        QRhiGraphicsPipeline *pipeline = ctx.rhi->rhi()->newGraphicsPipeline();
        pipeline->setShaderStages({ vsStage, fsStage });
        QRhiVertexInputLayout inputLayout;
        inputLayout.setBindings({ QRhiVertexInputBinding(sizeof(Vertex)) });
        inputLayout.setAttributes({
            QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float3, 0)
        });
        pipeline->setVertexInputLayout(inputLayout);
        pipeline->setDepthTest(enableDepth);
        pipeline->setDepthWrite(enableDepth);
        pipeline->setDepthOp(depthOp);
        pipeline->setCullMode(QRhiGraphicsPipeline::None);
        if (useMinBlend)
        {
            QRhiGraphicsPipeline::TargetBlend blend;
            blend.enable = true;
            blend.srcColor = QRhiGraphicsPipeline::One;
            blend.dstColor = QRhiGraphicsPipeline::One;
            blend.opColor = QRhiGraphicsPipeline::Min;
            blend.srcAlpha = QRhiGraphicsPipeline::One;
            blend.dstAlpha = QRhiGraphicsPipeline::One;
            blend.opAlpha = QRhiGraphicsPipeline::Min;
            pipeline->setTargetBlends({ blend });
        }
        else
        {
            pipeline->setTargetBlends({ QRhiGraphicsPipeline::TargetBlend() });
        }
        pipeline->setShaderResourceBindings(m_srb);
        pipeline->setRenderPassDescriptor(rpDesc);
        if (!pipeline->create())
            return nullptr;
        return pipeline;
    };

    m_pipeline = createPipeline(m_cascades[0].rpDesc, true, false,
                                m_reverseZ ? QRhiGraphicsPipeline::GreaterOrEqual
                                           : QRhiGraphicsPipeline::LessOrEqual,
                                vs, fsShadow);
    if (!m_pipeline)
        return;
    const QRhiShaderStage fsSpot = ctx.shaders->loadStage(QRhiShaderStage::Fragment, QStringLiteral(":/shaders/shadow_spot.frag.qsb"));
    if (!fsSpot.shader().isValid())
    {
        qWarning() << "PassShadow: failed to load spot shadow fragment shader";
        return;
    }
    m_spotPipeline = createPipeline(m_spotRpDesc, true, false,
                                    m_reverseZ ? QRhiGraphicsPipeline::GreaterOrEqual
                                               : QRhiGraphicsPipeline::LessOrEqual,
                                    vs, fsSpot);
    if (!m_spotPipeline)
        return;
}

QRhiShaderResourceBindings *PassShadow::shadowSrbForMesh(FrameContext &ctx, Mesh &mesh)
{
    if (!m_shadowUbo || !mesh.modelUbo)
        return nullptr;
    if (mesh.shadowSrb)
        return mesh.shadowSrb;
    mesh.shadowSrb = ctx.rhi->rhi()->newShaderResourceBindings();
    mesh.shadowSrb->setBindings({
        QRhiShaderResourceBinding::uniformBuffer(0,
                                                 QRhiShaderResourceBinding::VertexStage
                                                 | QRhiShaderResourceBinding::FragmentStage,
                                                 m_shadowUbo),
        QRhiShaderResourceBinding::uniformBuffer(1, QRhiShaderResourceBinding::VertexStage, mesh.modelUbo)
    });
    if (!mesh.shadowSrb->create())
    {
        delete mesh.shadowSrb;
        mesh.shadowSrb = nullptr;
    }
    return mesh.shadowSrb;
}

QRhiShaderResourceBindings *PassShadow::spotShadowSrbForMesh(FrameContext &ctx, Mesh &mesh, int slot)
{
    if (slot < 0 || slot >= m_spotShadowUbos.size())
        return nullptr;
    if (!m_spotShadowUbos[slot] || !mesh.modelUbo)
        return nullptr;
    if (mesh.spotShadowSrbs.size() != m_spotShadowUbos.size())
        mesh.spotShadowSrbs.resize(m_spotShadowUbos.size());
    if (mesh.spotShadowSrbs[slot])
        return mesh.spotShadowSrbs[slot];
    QRhiShaderResourceBindings *srb = ctx.rhi->rhi()->newShaderResourceBindings();
    srb->setBindings({
        QRhiShaderResourceBinding::uniformBuffer(0,
                                                 QRhiShaderResourceBinding::VertexStage
                                                 | QRhiShaderResourceBinding::FragmentStage,
                                                 m_spotShadowUbos[slot]),
        QRhiShaderResourceBinding::uniformBuffer(1, QRhiShaderResourceBinding::VertexStage, mesh.modelUbo)
    });
    if (!srb->create())
    {
        delete srb;
        return nullptr;
    }
    mesh.spotShadowSrbs[slot] = srb;
    return srb;
}

void PassShadow::renderCascade(FrameContext &ctx, Cascade &cascade, const QMatrix4x4 &lightViewProj)
{
    if (!ctx.scene || !m_pipeline || !cascade.rt)
        return;
    QRhiCommandBuffer *cb = ctx.rhi->commandBuffer();
    if (!cb)
        return;

    const QRhiDepthStencilClearValue dsClear(m_reverseZ ? 0.0f : 1.0f, 0);
    cb->beginPass(cascade.rt, Qt::white, dsClear);
    cb->setGraphicsPipeline(m_pipeline);
    cb->setViewport(QRhiViewport(0, 0, cascade.rt->pixelSize().width(), cascade.rt->pixelSize().height()));
    struct ShadowUboData
    {
        float lightViewProj[16];
        float shadowDepthParams[4];
        float lightPosNear[4];
        float lightParams[4];
    } shadowData;
    std::memcpy(shadowData.lightViewProj, lightViewProj.constData(), sizeof(shadowData.lightViewProj));
    shadowData.shadowDepthParams[0] = ctx.shadows ? ctx.shadows->shadowDepthParams.x() : 1.0f;
    shadowData.shadowDepthParams[1] = ctx.shadows ? ctx.shadows->shadowDepthParams.y() : 0.0f;
    shadowData.shadowDepthParams[2] = ctx.shadows ? ctx.shadows->shadowDepthParams.z() : 0.0f;
    shadowData.shadowDepthParams[3] = ctx.shadows ? ctx.shadows->shadowDepthParams.w() : 0.0f;
    shadowData.lightPosNear[0] = 0.0f;
    shadowData.lightPosNear[1] = 0.0f;
    shadowData.lightPosNear[2] = 0.0f;
    shadowData.lightPosNear[3] = 0.0f;
    shadowData.lightParams[0] = 1.0f;
    shadowData.lightParams[1] = 0.0f;
    shadowData.lightParams[2] = 0.0f;
    shadowData.lightParams[3] = 0.0f;
    QRhiResourceUpdateBatch *u = ctx.rhi->rhi()->nextResourceUpdateBatch();
    u->updateDynamicBuffer(m_shadowUbo, 0, sizeof(ShadowUboData), &shadowData);
    cb->resourceUpdate(u);

    const auto &meshes = ctx.scene->meshes();
    for (int i = meshes.size() - 1; i >= 0; --i)
    {
        Mesh &mesh = const_cast<Mesh &>(meshes[i]);
        if (mesh.gizmoAxis >= 0)
            continue;
        if (!mesh.visible)
            continue;
        if (!mesh.vertexBuffer || !mesh.indexBuffer || mesh.indexCount == 0)
            continue;
        QRhiShaderResourceBindings *meshSrb = shadowSrbForMesh(ctx, mesh);
        if (!meshSrb)
            continue;
        cb->setShaderResources(meshSrb);

        const QRhiCommandBuffer::VertexInput vbufBinding(mesh.vertexBuffer, 0);
        cb->setVertexInput(0, 1, &vbufBinding, mesh.indexBuffer, 0, QRhiCommandBuffer::IndexUInt32);
        cb->drawIndexed(mesh.indexCount);
    }
    cb->endPass();
}

void PassShadow::renderSpot(FrameContext &ctx,
                            QRhiTextureRenderTarget *rt,
                            const QMatrix4x4 &lightViewProj,
                            const QVector3D &lightPos, float nearPlane, float farPlane,
                            int slot)
{
    if (!ctx.scene || !m_spotPipeline || !rt)
        return;
    QRhiCommandBuffer *cb = ctx.rhi->commandBuffer();
    if (!cb)
        return;

    const QColor clearColor = Qt::white;
    const QRhiDepthStencilClearValue dsClear(m_reverseZ ? 0.0f : 1.0f, 0);
    cb->beginPass(rt, clearColor, dsClear);
    cb->setGraphicsPipeline(m_spotPipeline);
    cb->setViewport(QRhiViewport(0, 0, rt->pixelSize().width(), rt->pixelSize().height()));
    struct ShadowUboData
    {
        float lightViewProj[16];
        float shadowDepthParams[4];
        float lightPosNear[4];
        float lightParams[4];
    } shadowData;
    std::memcpy(shadowData.lightViewProj, lightViewProj.constData(), sizeof(shadowData.lightViewProj));
    shadowData.shadowDepthParams[0] = ctx.shadows ? ctx.shadows->shadowDepthParams.x() : 1.0f;
    shadowData.shadowDepthParams[1] = ctx.shadows ? ctx.shadows->shadowDepthParams.y() : 0.0f;
    shadowData.shadowDepthParams[2] = ctx.shadows ? ctx.shadows->shadowDepthParams.z() : 0.0f;
    shadowData.shadowDepthParams[3] = ctx.shadows ? ctx.shadows->shadowDepthParams.w() : 0.0f;
    shadowData.lightPosNear[0] = lightPos.x();
    shadowData.lightPosNear[1] = lightPos.y();
    shadowData.lightPosNear[2] = lightPos.z();
    shadowData.lightPosNear[3] = nearPlane;
    shadowData.lightParams[0] = farPlane;
    shadowData.lightParams[1] = 0.0f;
    shadowData.lightParams[2] = 0.0f;
    shadowData.lightParams[3] = 0.0f;
    if (slot < 0 || slot >= m_spotShadowUbos.size())
        return;
    QRhiResourceUpdateBatch *u = ctx.rhi->rhi()->nextResourceUpdateBatch();
    u->updateDynamicBuffer(m_spotShadowUbos[slot], 0, sizeof(ShadowUboData), &shadowData);
    cb->resourceUpdate(u);

    for (Mesh &mesh : ctx.scene->meshes())
    {
        if (mesh.gizmoAxis >= 0)
            continue;
        if (!mesh.visible)
            continue;
        if (!mesh.vertexBuffer || !mesh.indexBuffer || mesh.indexCount == 0)
            continue;
        QRhiShaderResourceBindings *meshSrb = spotShadowSrbForMesh(ctx, mesh, slot);
        if (!meshSrb)
            continue;
        cb->setShaderResources(meshSrb);

        const QRhiCommandBuffer::VertexInput vbufBinding(mesh.vertexBuffer, 0);
        cb->setVertexInput(0, 1, &vbufBinding, mesh.indexBuffer, 0, QRhiCommandBuffer::IndexUInt32);
        cb->drawIndexed(mesh.indexCount);
    }
    cb->endPass();
}

QMatrix4x4 PassShadow::computeLightViewProj(const Camera &camera, const QVector3D &lightDir, float nearPlane, float farPlane)
{
    const float fovY = qDegreesToRadians(camera.fovYDegrees());
    const float tanFov = std::tan(fovY * 0.5f);
    const float nearH = tanFov * nearPlane;
    const float nearW = nearH * camera.aspect();
    const float farH = tanFov * farPlane;
    const float farW = farH * camera.aspect();

    QVector<QVector3D> corners;
    corners.reserve(8);
    corners << QVector3D(-nearW,  nearH, -nearPlane);
    corners << QVector3D( nearW,  nearH, -nearPlane);
    corners << QVector3D( nearW, -nearH, -nearPlane);
    corners << QVector3D(-nearW, -nearH, -nearPlane);
    corners << QVector3D(-farW,  farH, -farPlane);
    corners << QVector3D( farW,  farH, -farPlane);
    corners << QVector3D( farW, -farH, -farPlane);
    corners << QVector3D(-farW, -farH, -farPlane);

    QMatrix4x4 invView = camera.viewMatrix().inverted();
    for (QVector3D &c : corners)
        c = (invView * QVector4D(c, 1.0f)).toVector3D();

    QVector3D center(0.0f, 0.0f, 0.0f);
    for (const QVector3D &c : corners)
        center += c;
    center /= float(corners.size());

    QVector3D dir = lightDir.normalized();
    QVector3D lightPos = center - dir * 50.0f;
    QMatrix4x4 view;
    view.lookAt(lightPos, center, safeUp(dir));

    float minX = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float minY = std::numeric_limits<float>::max();
    float maxY = std::numeric_limits<float>::lowest();
    float minZ = std::numeric_limits<float>::max();
    float maxZ = std::numeric_limits<float>::lowest();

    for (const QVector3D &c : corners)
    {
        QVector3D lc = (view * QVector4D(c, 1.0f)).toVector3D();
        minX = qMin(minX, lc.x());
        maxX = qMax(maxX, lc.x());
        minY = qMin(minY, lc.y());
        maxY = qMax(maxY, lc.y());
        minZ = qMin(minZ, lc.z());
        maxZ = qMax(maxZ, lc.z());
    }

    const float zMargin = 50.0f;
    float orthoNear = -maxZ - zMargin;
    float orthoFar = -minZ + zMargin;
    if (orthoNear < 0.1f)
        orthoNear = 0.1f;
    if (orthoFar <= orthoNear + 0.1f)
        orthoFar = orthoNear + 0.1f;
    QMatrix4x4 proj;
    proj.ortho(minX, maxX, minY, maxY, orthoNear, orthoFar);
    return proj * view;
}

QMatrix4x4 PassShadow::computeSpotViewProj(const Light &light, float nearPlane, float farPlane)
{
    const QVector3D dir = light.direction.normalized();
    const QVector3D pos = spotShadowOrigin(light);
    const QVector3D up = safeUp(dir);
    const float fovY = qRadiansToDegrees(qMax(light.outerCone * 2.0f, 0.01f));

    QMatrix4x4 view;
    view.lookAt(pos, pos + dir, up);
    QMatrix4x4 proj;
    proj.perspective(fovY, 1.0f, nearPlane, farPlane);
    return proj * view;
}
