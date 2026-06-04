/*
  Q Light Controller Plus
  PassPost.cpp

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

#include "renderer/PassPost.h"

#include "core/RhiContext.h"
#include "core/ShaderManager.h"
#include "scene/Scene.h"
#include "core/RenderTargetCache.h"

#include <QtGui/QColor>
#include <QtGui/QVector4D>
#include <cstring>

void PassPost::ensureGizmoPipeline(FrameContext &ctx)
{
    if (!ctx.rhi || !ctx.shaders)
        return;
    QRhi *rhi = ctx.rhi->rhi();
    if (!rhi)
        return;
    QRhiRenderTarget *swapRt = ctx.rhi->swapchainRenderTarget();
    if (!swapRt)
        return;

    if (!m_gizmoCameraUbo)
    {
        const quint32 mat4Size = 16 * sizeof(float);
        m_gizmoCameraUbo = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, mat4Size);
        if (!m_gizmoCameraUbo->create())
        {
            delete m_gizmoCameraUbo;
            m_gizmoCameraUbo = nullptr;
            return;
        }
        if (ctx.scene)
        {
            for (Mesh &mesh : ctx.scene->meshes())
            {
                delete mesh.gizmoSrb;
                mesh.gizmoSrb = nullptr;
            }
        }
    }

    if (!m_gizmoModelUbo)
    {
        const quint32 mat4Size = 16 * sizeof(float);
        m_gizmoModelUbo = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, mat4Size * 2);
        if (!m_gizmoModelUbo->create())
            return;
    }
    if (!m_gizmoMaterialUbo)
    {
        m_gizmoMaterialUbo = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(QVector4D) * 3);
        if (!m_gizmoMaterialUbo->create())
            return;
    }
    if (!m_gizmoLayoutSrb)
    {
        m_gizmoLayoutSrb = rhi->newShaderResourceBindings();
        m_gizmoLayoutSrb->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, m_gizmoCameraUbo),
            QRhiShaderResourceBinding::uniformBuffer(1, QRhiShaderResourceBinding::VertexStage, m_gizmoModelUbo),
            QRhiShaderResourceBinding::uniformBuffer(2, QRhiShaderResourceBinding::FragmentStage, m_gizmoMaterialUbo)
        });
        if (!m_gizmoLayoutSrb->create())
        {
            delete m_gizmoLayoutSrb;
            m_gizmoLayoutSrb = nullptr;
            return;
        }
    }

    if (m_gizmoPipeline && m_swapRpDesc == swapRt->renderPassDescriptor())
        return;

    delete m_gizmoPipeline;
    m_gizmoPipeline = nullptr;

    const QRhiShaderStage vs = ctx.shaders->loadStage(QRhiShaderStage::Vertex, QStringLiteral(":/shaders/gizmo.vert.qsb"));
    const QRhiShaderStage fs = ctx.shaders->loadStage(QRhiShaderStage::Fragment, QStringLiteral(":/shaders/gizmo.frag.qsb"));
    if (!vs.shader().isValid() || !fs.shader().isValid())
        return;

    QRhiGraphicsPipeline *pipeline = rhi->newGraphicsPipeline();
    pipeline->setShaderStages({ vs, fs });

    QRhiVertexInputLayout inputLayout;
    inputLayout.setBindings({
        QRhiVertexInputBinding(sizeof(Vertex))
    });
    inputLayout.setAttributes({
        QRhiVertexInputAttribute(0, 0, QRhiVertexInputAttribute::Float3, 0),
        QRhiVertexInputAttribute(0, 1, QRhiVertexInputAttribute::Float3, 12),
        QRhiVertexInputAttribute(0, 2, QRhiVertexInputAttribute::Float2, 24)
    });
    pipeline->setVertexInputLayout(inputLayout);
    pipeline->setSampleCount(1);
    pipeline->setCullMode(QRhiGraphicsPipeline::None);
    pipeline->setDepthTest(false);
    pipeline->setDepthWrite(false);
    pipeline->setShaderResourceBindings(m_gizmoLayoutSrb);
    pipeline->setRenderPassDescriptor(swapRt->renderPassDescriptor());

    if (!pipeline->create())
    {
        delete pipeline;
        return;
    }

    m_gizmoPipeline = pipeline;
}

void PassPost::ensureGizmoMeshBuffers(FrameContext &ctx, Mesh &mesh, QRhiResourceUpdateBatch *u)
{
    if (mesh.vertices.isEmpty() || mesh.indices.isEmpty())
        return;

    if (!mesh.vertexBuffer || !mesh.indexBuffer)
    {
        mesh.vertexBuffer = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::VertexBuffer, mesh.vertices.size() * sizeof(Vertex));
        mesh.indexBuffer = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Immutable, QRhiBuffer::IndexBuffer, mesh.indices.size() * sizeof(quint32));
        if (!mesh.vertexBuffer->create() || !mesh.indexBuffer->create())
            return;

        u->uploadStaticBuffer(mesh.vertexBuffer, mesh.vertices.constData());
        u->uploadStaticBuffer(mesh.indexBuffer, mesh.indices.constData());
        mesh.indexCount = mesh.indices.size();
    }
    if (mesh.indexCount == 0 && !mesh.indices.isEmpty())
        mesh.indexCount = mesh.indices.size();

    if (!mesh.modelUbo)
    {
        const quint32 mat4Size = 16 * sizeof(float);
        mesh.modelUbo = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, mat4Size * 2);
        if (!mesh.modelUbo->create())
            return;
    }
    if (!mesh.materialUbo)
    {
        mesh.materialUbo = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(QVector4D) * 4);
        if (!mesh.materialUbo->create())
            return;
    }
}

void PassPost::prepare(FrameContext &ctx)
{
    if (!ctx.rhi || !ctx.shaders || !ctx.targets)
        return;
    QRhi *rhi = ctx.rhi->rhi();
    if (!rhi)
        return;

    QRhiRenderTarget *swapRt = ctx.rhi->swapchainRenderTarget();
    if (!swapRt)
        return;
    const QSize size = swapRt->pixelSize();
    if (size.isEmpty())
        return;

    if (m_lastSize != size)
    {
        delete m_bloomRpDesc;
        m_bloomRpDesc = nullptr;
        delete m_bloomRt;
        m_bloomRt = nullptr;
        delete m_bloomTex;
        m_bloomTex = nullptr;
        delete m_bloomBlurRpDesc;
        m_bloomBlurRpDesc = nullptr;
        delete m_bloomBlurRt;
        m_bloomBlurRt = nullptr;
        delete m_bloomBlurTex;
        m_bloomBlurTex = nullptr;
        delete m_bloomSrb;
        m_bloomSrb = nullptr;
        delete m_bloomUpsampleSrb;
        m_bloomUpsampleSrb = nullptr;
        delete m_combineSrb;
        m_combineSrb = nullptr;
        delete m_bloomPipeline;
        m_bloomPipeline = nullptr;
        delete m_bloomUpsamplePipeline;
        m_bloomUpsamplePipeline = nullptr;
        delete m_combinePipeline;
        m_combinePipeline = nullptr;
        delete m_gizmoPipeline;
        m_gizmoPipeline = nullptr;
        m_swapRpDesc = nullptr;
        m_lastSize = size;
    }

    if (!m_sampler)
    {
        m_sampler = rhi->newSampler(QRhiSampler::Linear, QRhiSampler::Linear, QRhiSampler::None,
                                    QRhiSampler::ClampToEdge, QRhiSampler::ClampToEdge);
        if (!m_sampler->create())
            return;
    }

    if (!m_postUbo)
    {
        m_postUbo = rhi->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(QVector4D) * 2);
        if (!m_postUbo->create())
            return;
    }

    if (!m_bloomTex)
    {
        const QSize half = QSize(qMax(1, size.width() / 2), qMax(1, size.height() / 2));
        QRhiTexture::Format colorFormat = QRhiTexture::RGBA16F;
        if (!rhi->isTextureFormatSupported(colorFormat, QRhiTexture::RenderTarget))
            colorFormat = QRhiTexture::RGBA8;
        m_bloomTex = rhi->newTexture(colorFormat, half, 1, QRhiTexture::RenderTarget);
        if (!m_bloomTex->create())
            return;
        QRhiTextureRenderTargetDescription rtDesc;
        rtDesc.setColorAttachments({ QRhiColorAttachment(m_bloomTex) });
        m_bloomRt = rhi->newTextureRenderTarget(rtDesc);
        m_bloomRpDesc = m_bloomRt->newCompatibleRenderPassDescriptor();
        m_bloomRt->setRenderPassDescriptor(m_bloomRpDesc);
        if (!m_bloomRt->create())
            return;
    }
    if (!m_bloomBlurTex)
    {
        QRhiTexture::Format colorFormat = QRhiTexture::RGBA16F;
        if (!rhi->isTextureFormatSupported(colorFormat, QRhiTexture::RenderTarget))
            colorFormat = QRhiTexture::RGBA8;
        m_bloomBlurTex = rhi->newTexture(colorFormat, size, 1, QRhiTexture::RenderTarget);
        if (!m_bloomBlurTex->create())
            return;
        QRhiTextureRenderTargetDescription rtDesc;
        rtDesc.setColorAttachments({ QRhiColorAttachment(m_bloomBlurTex) });
        m_bloomBlurRt = rhi->newTextureRenderTarget(rtDesc);
        m_bloomBlurRpDesc = m_bloomBlurRt->newCompatibleRenderPassDescriptor();
        m_bloomBlurRt->setRenderPassDescriptor(m_bloomBlurRpDesc);
        if (!m_bloomBlurRt->create())
            return;
    }

    if (m_swapRpDesc != swapRt->renderPassDescriptor())
    {
        delete m_combinePipeline;
        m_combinePipeline = nullptr;
        m_swapRpDesc = swapRt->renderPassDescriptor();
    }

    const bool debugCombine = !ctx.lightingEnabled;
    if (!m_bloomSrb || !m_bloomUpsampleSrb || !m_combineSrb
            || !m_bloomPipeline || !m_bloomUpsamplePipeline || !m_combinePipeline
            || m_combineUsesGBuffer != debugCombine)
    {
        const RenderTargetCache::GBufferTargets gbuf = ctx.targets->getOrCreateGBuffer(size, 1);
        const RenderTargetCache::LightingTargets lighting = ctx.targets->getOrCreateLightingTarget(size, 1);
        if (!gbuf.color3 || !lighting.color)
            return;

        delete m_bloomSrb;
        m_bloomSrb = nullptr;
        delete m_bloomUpsampleSrb;
        m_bloomUpsampleSrb = nullptr;
        delete m_combineSrb;
        m_combineSrb = nullptr;
        delete m_bloomPipeline;
        m_bloomPipeline = nullptr;
        delete m_bloomUpsamplePipeline;
        m_bloomUpsamplePipeline = nullptr;
        delete m_combinePipeline;
        m_combinePipeline = nullptr;

        m_bloomSrb = rhi->newShaderResourceBindings();
        m_bloomSrb->setBindings({
            QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage, gbuf.color3, m_sampler),
            QRhiShaderResourceBinding::uniformBuffer(1, QRhiShaderResourceBinding::FragmentStage, m_postUbo)
        });
        if (!m_bloomSrb->create())
            return;

        m_bloomUpsampleSrb = rhi->newShaderResourceBindings();
        m_bloomUpsampleSrb->setBindings({
            QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage, m_bloomTex, m_sampler),
            QRhiShaderResourceBinding::uniformBuffer(1, QRhiShaderResourceBinding::FragmentStage, m_postUbo)
        });
        if (!m_bloomUpsampleSrb->create())
            return;

        m_combineSrb = rhi->newShaderResourceBindings();
        QRhiTexture *combineSource = debugCombine ? gbuf.color0 : lighting.color;
        m_combineSrb->setBindings({
            QRhiShaderResourceBinding::sampledTexture(0, QRhiShaderResourceBinding::FragmentStage, combineSource, m_sampler),
            QRhiShaderResourceBinding::sampledTexture(1, QRhiShaderResourceBinding::FragmentStage, m_bloomBlurTex, m_sampler),
            QRhiShaderResourceBinding::uniformBuffer(2, QRhiShaderResourceBinding::FragmentStage, m_postUbo)
        });
        if (!m_combineSrb->create())
            return;
        m_combineUsesGBuffer = debugCombine;

        const QRhiShaderStage vs = ctx.shaders->loadStage(QRhiShaderStage::Vertex, QStringLiteral(":/shaders/lighting.vert.qsb"));
        const QRhiShaderStage fsBloom = ctx.shaders->loadStage(QRhiShaderStage::Fragment, QStringLiteral(":/shaders/post_bloom_downsample.frag.qsb"));
        const QRhiShaderStage fsUpsample = ctx.shaders->loadStage(QRhiShaderStage::Fragment, QStringLiteral(":/shaders/post_bloom_upsample.frag.qsb"));
        const QRhiShaderStage fsCombine = ctx.shaders->loadStage(QRhiShaderStage::Fragment, QStringLiteral(":/shaders/post_combine.frag.qsb"));
        if (!vs.shader().isValid() || !fsBloom.shader().isValid() || !fsUpsample.shader().isValid()
                || !fsCombine.shader().isValid())
            return;

        QRhiGraphicsPipeline *bloomPipeline = rhi->newGraphicsPipeline();
        bloomPipeline->setShaderStages({ vs, fsBloom });
        bloomPipeline->setCullMode(QRhiGraphicsPipeline::None);
        bloomPipeline->setDepthTest(false);
        bloomPipeline->setDepthWrite(false);
        bloomPipeline->setShaderResourceBindings(m_bloomSrb);
        bloomPipeline->setRenderPassDescriptor(m_bloomRpDesc);
        if (!bloomPipeline->create())
            return;
        m_bloomPipeline = bloomPipeline;

        QRhiGraphicsPipeline *upsamplePipeline = rhi->newGraphicsPipeline();
        upsamplePipeline->setShaderStages({ vs, fsUpsample });
        upsamplePipeline->setCullMode(QRhiGraphicsPipeline::None);
        upsamplePipeline->setDepthTest(false);
        upsamplePipeline->setDepthWrite(false);
        upsamplePipeline->setShaderResourceBindings(m_bloomUpsampleSrb);
        upsamplePipeline->setRenderPassDescriptor(m_bloomBlurRpDesc);
        if (!upsamplePipeline->create())
            return;
        m_bloomUpsamplePipeline = upsamplePipeline;

        QRhiGraphicsPipeline *combinePipeline = rhi->newGraphicsPipeline();
        combinePipeline->setShaderStages({ vs, fsCombine });
        combinePipeline->setCullMode(QRhiGraphicsPipeline::None);
        combinePipeline->setDepthTest(false);
        combinePipeline->setDepthWrite(false);
        combinePipeline->setShaderResourceBindings(m_combineSrb);
        combinePipeline->setRenderPassDescriptor(m_swapRpDesc);
        if (!combinePipeline->create())
            return;
        m_combinePipeline = combinePipeline;
    }

    ensureGizmoPipeline(ctx);
}

void PassPost::execute(FrameContext &ctx)
{
    const bool debugCombine = !ctx.lightingEnabled;
    if (!ctx.rhi || !ctx.targets || !m_combinePipeline)
        return;
    if (!debugCombine && (!m_bloomRt || !m_bloomBlurRt
            || !m_bloomPipeline || !m_bloomUpsamplePipeline))
        return;
    QRhiCommandBuffer *cb = ctx.rhi->commandBuffer();
    QRhiRenderTarget *swapRt = ctx.rhi->swapchainRenderTarget();
    if (!cb || !swapRt)
        return;

    const QSize size = swapRt->pixelSize();
    struct PostParams
    {
        QVector4D pixelSize;
        QVector4D intensity;
    } params;

    const float bloomIntensity = ctx.scene ? ctx.scene->bloomIntensity() : 0.0f;
    const float bloomRadius = ctx.scene ? ctx.scene->bloomRadius() : 0.0f;
    const QColor clear(0, 0, 0);
    const QSize half = QSize(qMax(1, size.width() / 2), qMax(1, size.height() / 2));
    QRhiResourceUpdateBatch *u = ctx.rhi->rhi()->nextResourceUpdateBatch();
    params.pixelSize = QVector4D(1.0f / float(size.width()), 1.0f / float(size.height()), 0.0f, 0.0f);
    const bool flipSampleY = !ctx.rhi->rhi()->isYUpInFramebuffer();
    params.intensity = QVector4D(debugCombine ? 0.0f : bloomIntensity,
                                 debugCombine ? 0.0f : bloomRadius,
                                 flipSampleY ? 1.0f : 0.0f,
                                 flipSampleY ? 1.0f : 0.0f);
    u->updateDynamicBuffer(m_postUbo, 0, sizeof(PostParams), &params);
    cb->resourceUpdate(u);
    if (!debugCombine)
    {
        cb->beginPass(m_bloomRt, clear, {});
        cb->setGraphicsPipeline(m_bloomPipeline);
        cb->setViewport(QRhiViewport(0, 0, half.width(), half.height()));
        cb->setShaderResources(m_bloomSrb);
        cb->draw(3);
        cb->endPass();

        u = ctx.rhi->rhi()->nextResourceUpdateBatch();
        params.pixelSize = QVector4D(1.0f / float(half.width()), 1.0f / float(half.height()), 0.0f, 0.0f);
        params.intensity = QVector4D(bloomIntensity,
                                     bloomRadius,
                                     flipSampleY ? 1.0f : 0.0f,
                                     flipSampleY ? 1.0f : 0.0f);
        u->updateDynamicBuffer(m_postUbo, 0, sizeof(PostParams), &params);
        cb->resourceUpdate(u);
        cb->beginPass(m_bloomBlurRt, clear, {});
        cb->setGraphicsPipeline(m_bloomUpsamplePipeline);
        cb->setViewport(QRhiViewport(0, 0, size.width(), size.height()));
        cb->setShaderResources(m_bloomUpsampleSrb);
        cb->draw(3);
        cb->endPass();
    }

    cb->beginPass(swapRt, clear, {});
    cb->setGraphicsPipeline(m_combinePipeline);
    cb->setViewport(QRhiViewport(0, 0, size.width(), size.height()));
    cb->setShaderResources(m_combineSrb);
    cb->draw(3);

    if (m_gizmoPipeline && ctx.scene && m_gizmoCameraUbo)
    {
        struct GizmoCameraData
        {
            float viewProj[16];
        } camData;
        const QMatrix4x4 viewProj = ctx.rhi->rhi()->clipSpaceCorrMatrix()
                * ctx.scene->camera().projectionMatrix()
                * ctx.scene->camera().viewMatrix();
        std::memcpy(camData.viewProj, viewProj.constData(), sizeof(camData.viewProj));

        QRhiResourceUpdateBatch *gizmoUpdates = ctx.rhi->rhi()->nextResourceUpdateBatch();
        gizmoUpdates->updateDynamicBuffer(m_gizmoCameraUbo, 0, sizeof(GizmoCameraData), &camData);

        struct ModelData
        {
            float model[16];
            float normalMatrix[16];
        };
        struct MaterialData
        {
            QVector4D baseColorMetal;
            QVector4D roughnessOcclusion;
            QVector4D emissive;
        };

        for (Mesh &mesh : ctx.scene->meshes())
        {
            if (mesh.gizmoAxis < 0)
                continue;
            ensureGizmoMeshBuffers(ctx, mesh, gizmoUpdates);
            if (!mesh.modelUbo || !mesh.materialUbo)
                continue;

            QMatrix4x4 model = mesh.modelMatrix;
            QMatrix4x4 normalMatrix = model.inverted();
            normalMatrix = normalMatrix.transposed();
            ModelData modelData;
            std::memcpy(modelData.model, model.constData(), sizeof(modelData.model));
            std::memcpy(modelData.normalMatrix, normalMatrix.constData(), sizeof(modelData.normalMatrix));

            MaterialData matData;
            matData.baseColorMetal = QVector4D(mesh.material.baseColor, mesh.material.metalness);
            matData.roughnessOcclusion = QVector4D(mesh.material.roughness, mesh.material.occlusion, 0.0f, 0.0f);
            matData.emissive = QVector4D(mesh.material.emissive, 0.0f);

            gizmoUpdates->updateDynamicBuffer(mesh.modelUbo, 0, sizeof(ModelData), &modelData);
            gizmoUpdates->updateDynamicBuffer(mesh.materialUbo, 0, sizeof(MaterialData), &matData);
        }

        cb->resourceUpdate(gizmoUpdates);
        cb->setGraphicsPipeline(m_gizmoPipeline);
        cb->setViewport(QRhiViewport(0, 0, size.width(), size.height()));

        for (Mesh &mesh : ctx.scene->meshes())
        {
            if (mesh.gizmoAxis < 0)
                continue;
            if (!mesh.vertexBuffer || !mesh.indexBuffer || mesh.indexCount == 0)
                continue;
            if (!mesh.modelUbo || !mesh.materialUbo)
                continue;

            if (!mesh.gizmoSrb)
            {
                mesh.gizmoSrb = ctx.rhi->rhi()->newShaderResourceBindings();
                mesh.gizmoSrb->setBindings({
                    QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, m_gizmoCameraUbo),
                    QRhiShaderResourceBinding::uniformBuffer(1, QRhiShaderResourceBinding::VertexStage, mesh.modelUbo),
                    QRhiShaderResourceBinding::uniformBuffer(2, QRhiShaderResourceBinding::FragmentStage, mesh.materialUbo)
                });
                if (!mesh.gizmoSrb->create())
                {
                    delete mesh.gizmoSrb;
                    mesh.gizmoSrb = nullptr;
                }
            }

            if (!mesh.gizmoSrb)
                continue;

            cb->setShaderResources(mesh.gizmoSrb);
            const QRhiCommandBuffer::VertexInput vbufBinding(mesh.vertexBuffer, 0);
            cb->setVertexInput(0, 1, &vbufBinding, mesh.indexBuffer, 0, QRhiCommandBuffer::IndexUInt32);
            cb->drawIndexed(mesh.indexCount);
        }
    }

    cb->endPass();
}
