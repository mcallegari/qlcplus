/*
  Q Light Controller Plus
  PassGBuffer.cpp

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

#include "renderer/PassGBuffer.h"

#include <QtGui/QImage>
#include <QtGui/QMatrix4x4>
#include <cstring>
#include <rhi/qrhi.h>

#include "core/RhiContext.h"
#include "core/ShaderManager.h"
#include "scene/Scene.h"

void PassGBuffer::prepare(FrameContext &ctx)
{
    if (!ctx.targets || !ctx.rhi)
        return;
    const QSize size = ctx.rhi->swapchainRenderTarget()->pixelSize();
    m_gbuffer = ctx.targets->getOrCreateGBuffer(size, 1);
    if (!m_gbuffer.rt)
        qWarning() << "PassGBuffer: failed to acquire GBuffer render target";

    ensurePipeline(ctx);
}

void PassGBuffer::execute(FrameContext &ctx)
{
    if (!ctx.rhi || !m_gbuffer.rt)
        return;
    if (!ctx.scene || !m_pipeline || !m_srb)
        return;
    QRhiCommandBuffer *cb = ctx.rhi->commandBuffer();
    if (!cb)
        return;

    const QColor clear0(0, 0, 0, 0);
    const QRhiDepthStencilClearValue dsClear(1.0f, 0);

    struct CameraData
    {
        float viewProj[16];
        QVector4D cameraPos;
    } camData;

    const bool cameraDirty = ctx.scene && ctx.scene->cameraDirty();
    if (cameraDirty)
    {
        const QMatrix4x4 viewProj = ctx.rhi->rhi()->clipSpaceCorrMatrix()
                * ctx.scene->camera().projectionMatrix()
                * ctx.scene->camera().viewMatrix();
        std::memcpy(camData.viewProj, viewProj.constData(), sizeof(camData.viewProj));
        camData.cameraPos = QVector4D(ctx.scene->camera().position(), 1.0f);
    }

    static bool s_dumped = false;
    if (!s_dumped)
    {
        // Optional one-shot mesh diagnostics can be re-enabled here when needed.
        s_dumped = true;
    }

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
        QVector4D miscParams;
    };

    QRhiResourceUpdateBatch *u = ctx.rhi->rhi()->nextResourceUpdateBatch();
    if (cameraDirty)
        u->updateDynamicBuffer(m_cameraUbo, 0, sizeof(CameraData), &camData);

    for (Mesh &mesh : ctx.scene->meshes())
    {
        if (mesh.gizmoAxis >= 0)
            continue;
        if (!mesh.visible)
            continue;
        ensureMeshBuffers(ctx, mesh, u);
        if (!mesh.modelUbo || !mesh.materialUbo)
            continue;
        const QMatrix4x4 model = mesh.modelMatrix;
        QMatrix4x4 normalMatrix = model.inverted();
        normalMatrix = normalMatrix.transposed();

        if (mesh.modelDirty)
        {
            ModelData modelData;
            std::memcpy(modelData.model, model.constData(), sizeof(modelData.model));
            std::memcpy(modelData.normalMatrix, normalMatrix.constData(), sizeof(modelData.normalMatrix));
            u->updateDynamicBuffer(mesh.modelUbo, 0, sizeof(ModelData), &modelData);
            mesh.modelDirty = false;
        }

        if (mesh.materialDirty)
        {
            MaterialData matData;
            matData.baseColorMetal = QVector4D(mesh.material.baseColor, mesh.material.metalness);
            matData.roughnessOcclusion = QVector4D(mesh.material.roughness, mesh.material.occlusion, 0.0f, 0.0f);
            matData.emissive = QVector4D(mesh.material.emissive, 0.0f);
            matData.miscParams = QVector4D(mesh.material.baseAlpha,
                                           mesh.material.alphaCutoff,
                                           float(mesh.material.alphaMode),
                                           mesh.material.normalMap.isNull() ? 0.0f : 1.0f);
            u->updateDynamicBuffer(mesh.materialUbo, 0, sizeof(MaterialData), &matData);
            mesh.materialDirty = false;
        }
    }

    cb->resourceUpdate(u);

    cb->beginPass(m_gbuffer.rt, clear0, dsClear);
    cb->setViewport(QRhiViewport(0, 0, m_gbuffer.rt->pixelSize().width(), m_gbuffer.rt->pixelSize().height()));

    for (Mesh &mesh : ctx.scene->meshes())
    {
        if (mesh.gizmoAxis >= 0)
            continue;
        if (!mesh.visible)
            continue;
        if (!mesh.vertexBuffer || !mesh.indexBuffer || mesh.indexCount == 0)
            continue;
        if (!mesh.srb)
            continue;
        QRhiGraphicsPipeline *pipeline = mesh.material.doubleSided ? m_pipelineTwoSided : m_pipeline;
        if (!pipeline)
            continue;
        cb->setGraphicsPipeline(pipeline);
        cb->setShaderResources(mesh.srb);
        const QRhiCommandBuffer::VertexInput vbufBinding(mesh.vertexBuffer, 0);
        cb->setVertexInput(0, 1, &vbufBinding, mesh.indexBuffer, 0, QRhiCommandBuffer::IndexUInt32);
        cb->drawIndexed(mesh.indexCount);
    }

    cb->endPass();
}

void PassGBuffer::ensurePipeline(FrameContext &ctx)
{
    if (!ctx.rhi || !ctx.shaders || !m_gbuffer.rpDesc)
        return;
    if (m_pipeline && m_rpDesc == m_gbuffer.rpDesc)
        return;

    delete m_pipeline;
    m_pipeline = nullptr;
    delete m_pipelineTwoSided;
    m_pipelineTwoSided = nullptr;
    delete m_srb;
    m_srb = nullptr;
    delete m_cameraUbo;
    delete m_modelUbo;
    delete m_materialUbo;
    delete m_videoSampler;
    m_videoSampler = nullptr;

    if (ctx.scene)
    {
        for (Mesh &mesh : ctx.scene->meshes())
        {
            delete mesh.srb;
            mesh.srb = nullptr;
            mesh.gpuReady = false;
        }
    }

    const quint32 mat4Size = 16 * sizeof(float);
    m_cameraUbo = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, mat4Size + sizeof(QVector4D));
    m_modelUbo = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, mat4Size * 2);
    m_materialUbo = ctx.rhi->rhi()->newBuffer(QRhiBuffer::Dynamic, QRhiBuffer::UniformBuffer, sizeof(QVector4D) * 4);

    if (!m_cameraUbo->create() || !m_modelUbo->create() || !m_materialUbo->create())
        return;

    if (!m_linearSampler)
    {
        m_linearSampler = ctx.rhi->rhi()->newSampler(QRhiSampler::Linear,
                                                     QRhiSampler::Linear,
                                                     QRhiSampler::None,
                                                     QRhiSampler::Repeat,
                                                     QRhiSampler::Repeat);
        if (!m_linearSampler->create())
            return;
    }
    if (!m_videoSampler)
    {
        m_videoSampler = ctx.rhi->rhi()->newSampler(QRhiSampler::Linear,
                                                    QRhiSampler::Linear,
                                                    QRhiSampler::None,
                                                    QRhiSampler::ClampToEdge,
                                                    QRhiSampler::ClampToEdge);
        if (!m_videoSampler->create())
            return;
    }
    if (!m_defaultBaseColor)
    {
        m_defaultBaseColor = ctx.rhi->rhi()->newTexture(QRhiTexture::RGBA8, QSize(1, 1), 1);
        if (!m_defaultBaseColor->create())
            return;
        m_defaultBaseColorUploaded = false;
    }
    if (!m_defaultNormal)
    {
        m_defaultNormal = ctx.rhi->rhi()->newTexture(QRhiTexture::RGBA8, QSize(1, 1), 1);
        if (!m_defaultNormal->create())
            return;
        m_defaultNormalUploaded = false;
    }
    if (!m_defaultMetallicRoughness)
    {
        m_defaultMetallicRoughness = ctx.rhi->rhi()->newTexture(QRhiTexture::RGBA8, QSize(1, 1), 1);
        if (!m_defaultMetallicRoughness->create())
            return;
        m_defaultMetallicRoughnessUploaded = false;
    }
    if (!m_defaultOcclusion)
    {
        m_defaultOcclusion = ctx.rhi->rhi()->newTexture(QRhiTexture::RGBA8, QSize(1, 1), 1);
        if (!m_defaultOcclusion->create())
            return;
        m_defaultOcclusionUploaded = false;
    }
    if (!m_defaultEmissive)
    {
        m_defaultEmissive = ctx.rhi->rhi()->newTexture(QRhiTexture::RGBA8, QSize(1, 1), 1);
        if (!m_defaultEmissive->create())
            return;
        m_defaultEmissiveUploaded = false;
    }

    m_srb = ctx.rhi->rhi()->newShaderResourceBindings();
    m_srb->setBindings({
        QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, m_cameraUbo),
        QRhiShaderResourceBinding::uniformBuffer(1, QRhiShaderResourceBinding::VertexStage, m_modelUbo),
        QRhiShaderResourceBinding::uniformBuffer(2, QRhiShaderResourceBinding::FragmentStage, m_materialUbo),
        QRhiShaderResourceBinding::sampledTexture(3, QRhiShaderResourceBinding::FragmentStage, m_defaultBaseColor, m_linearSampler),
        QRhiShaderResourceBinding::sampledTexture(4, QRhiShaderResourceBinding::FragmentStage, m_defaultNormal, m_linearSampler),
        QRhiShaderResourceBinding::sampledTexture(5, QRhiShaderResourceBinding::FragmentStage, m_defaultMetallicRoughness, m_linearSampler),
        QRhiShaderResourceBinding::sampledTexture(6, QRhiShaderResourceBinding::FragmentStage, m_defaultOcclusion, m_linearSampler),
        QRhiShaderResourceBinding::sampledTexture(7, QRhiShaderResourceBinding::FragmentStage, m_defaultEmissive, m_linearSampler)
    });
    if (!m_srb->create())
        return;

    m_pipeline = createPipeline(ctx, QRhiGraphicsPipeline::Back);
    m_pipelineTwoSided = createPipeline(ctx, QRhiGraphicsPipeline::None);
    if (!m_pipeline || !m_pipelineTwoSided)
    {
        qWarning() << "PassGBuffer: failed to create pipeline";
        return;
    }

    m_rpDesc = m_gbuffer.rpDesc;
}

QRhiGraphicsPipeline *PassGBuffer::createPipeline(FrameContext &ctx, QRhiGraphicsPipeline::CullMode cullMode)
{
    if (!ctx.rhi || !m_gbuffer.rpDesc || !m_srb)
        return nullptr;

    const QRhiShaderStage vs = ctx.shaders->loadStage(QRhiShaderStage::Vertex, QStringLiteral(":/shaders/gbuffer.vert.qsb"));
    const QRhiShaderStage fs = ctx.shaders->loadStage(QRhiShaderStage::Fragment, QStringLiteral(":/shaders/gbuffer.frag.qsb"));
    if (!vs.shader().isValid() || !fs.shader().isValid())
        return nullptr;

    QRhiGraphicsPipeline *pipeline = ctx.rhi->rhi()->newGraphicsPipeline();
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
    pipeline->setCullMode(cullMode);
    pipeline->setDepthTest(true);
    pipeline->setDepthWrite(true);
    pipeline->setTargetBlends({
        QRhiGraphicsPipeline::TargetBlend(),
        QRhiGraphicsPipeline::TargetBlend(),
        QRhiGraphicsPipeline::TargetBlend(),
        QRhiGraphicsPipeline::TargetBlend()
    });
    pipeline->setShaderResourceBindings(m_srb);
    pipeline->setRenderPassDescriptor(m_gbuffer.rpDesc);

    if (!pipeline->create())
        return nullptr;
    return pipeline;
}

void PassGBuffer::ensureMeshBuffers(FrameContext &ctx, Mesh &mesh, QRhiResourceUpdateBatch *u)
{
    if (mesh.vertices.isEmpty() || mesh.indices.isEmpty())
        return;
    if (mesh.gpuReady)
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
    if (m_defaultBaseColor && !m_defaultBaseColorUploaded)
    {
        QImage white(1, 1, QImage::Format_RGBA8888);
        white.fill(Qt::white);
        QRhiTextureUploadDescription upload(QRhiTextureUploadEntry(0, 0,
                                                                    QRhiTextureSubresourceUploadDescription(white)));
        u->uploadTexture(m_defaultBaseColor, upload);
        m_defaultBaseColorUploaded = true;
    }
    if (m_defaultNormal && !m_defaultNormalUploaded)
    {
        QImage normal(1, 1, QImage::Format_RGBA8888);
        normal.fill(QColor(128, 128, 255, 255));
        QRhiTextureUploadDescription upload(QRhiTextureUploadEntry(0, 0,
                                                                    QRhiTextureSubresourceUploadDescription(normal)));
        u->uploadTexture(m_defaultNormal, upload);
        m_defaultNormalUploaded = true;
    }
    if (m_defaultMetallicRoughness && !m_defaultMetallicRoughnessUploaded)
    {
        QImage mr(1, 1, QImage::Format_RGBA8888);
        // glTF packs roughness in G and metalness in B.
        // Use 1.0 defaults so scalar factors remain effective when no texture is present.
        mr.fill(QColor(0, 255, 255, 255));
        QRhiTextureUploadDescription upload(QRhiTextureUploadEntry(0, 0,
                                                                    QRhiTextureSubresourceUploadDescription(mr)));
        u->uploadTexture(m_defaultMetallicRoughness, upload);
        m_defaultMetallicRoughnessUploaded = true;
    }
    if (m_defaultOcclusion && !m_defaultOcclusionUploaded)
    {
        QImage occ(1, 1, QImage::Format_RGBA8888);
        occ.fill(Qt::white);
        QRhiTextureUploadDescription upload(QRhiTextureUploadEntry(0, 0,
                                                                    QRhiTextureSubresourceUploadDescription(occ)));
        u->uploadTexture(m_defaultOcclusion, upload);
        m_defaultOcclusionUploaded = true;
    }
    if (m_defaultEmissive && !m_defaultEmissiveUploaded)
    {
        QImage em(1, 1, QImage::Format_RGBA8888);
        em.fill(Qt::white);
        QRhiTextureUploadDescription upload(QRhiTextureUploadEntry(0, 0,
                                                                    QRhiTextureSubresourceUploadDescription(em)));
        u->uploadTexture(m_defaultEmissive, upload);
        m_defaultEmissiveUploaded = true;
    }
    if (!mesh.baseColorTexture)
    {
        if (!mesh.material.baseColorMap.isNull())
        {
            const QImage image = mesh.material.baseColorMap.convertToFormat(QImage::Format_RGBA8888);
            if (!image.isNull())
            {
                mesh.baseColorTexture = ctx.rhi->rhi()->newTexture(QRhiTexture::RGBA8, image.size(), 1);
                if (mesh.baseColorTexture->create())
                {
                    QRhiTextureUploadDescription upload(QRhiTextureUploadEntry(0, 0,
                                                                                QRhiTextureSubresourceUploadDescription(image)));
                    u->uploadTexture(mesh.baseColorTexture, upload);
                }
                else
                {
                    delete mesh.baseColorTexture;
                    mesh.baseColorTexture = nullptr;
                }
            }
        }
        if (!mesh.baseColorTexture)
            mesh.baseColorTexture = m_defaultBaseColor;
    }
    if (!mesh.baseColorSampler)
    {
        mesh.baseColorSampler = (mesh.name == QLatin1String("VideoQuad") && m_videoSampler)
                ? m_videoSampler
                : m_linearSampler;
    }
    if (!mesh.normalTexture)
    {
        if (!mesh.material.normalMap.isNull())
        {
            const QImage image = mesh.material.normalMap.convertToFormat(QImage::Format_RGBA8888);
            if (!image.isNull())
            {
                mesh.normalTexture = ctx.rhi->rhi()->newTexture(QRhiTexture::RGBA8, image.size(), 1);
                if (mesh.normalTexture->create())
                {
                    QRhiTextureUploadDescription upload(QRhiTextureUploadEntry(0, 0,
                                                                                QRhiTextureSubresourceUploadDescription(image)));
                    u->uploadTexture(mesh.normalTexture, upload);
                }
                else
                {
                    delete mesh.normalTexture;
                    mesh.normalTexture = nullptr;
                }
            }
        }
        if (!mesh.normalTexture)
            mesh.normalTexture = m_defaultNormal;
    }
    if (!mesh.normalSampler)
        mesh.normalSampler = m_linearSampler;
    if (!mesh.metallicRoughnessTexture)
    {
        if (!mesh.material.metallicRoughnessMap.isNull())
        {
            const QImage image = mesh.material.metallicRoughnessMap.convertToFormat(QImage::Format_RGBA8888);
            if (!image.isNull())
            {
                mesh.metallicRoughnessTexture = ctx.rhi->rhi()->newTexture(QRhiTexture::RGBA8, image.size(), 1);
                if (mesh.metallicRoughnessTexture->create())
                {
                    QRhiTextureUploadDescription upload(QRhiTextureUploadEntry(0, 0,
                                                                                QRhiTextureSubresourceUploadDescription(image)));
                    u->uploadTexture(mesh.metallicRoughnessTexture, upload);
                }
                else
                {
                    delete mesh.metallicRoughnessTexture;
                    mesh.metallicRoughnessTexture = nullptr;
                }
            }
        }
        if (!mesh.metallicRoughnessTexture)
            mesh.metallicRoughnessTexture = m_defaultMetallicRoughness;
    }
    if (!mesh.metallicRoughnessSampler)
        mesh.metallicRoughnessSampler = m_linearSampler;
    if (!mesh.occlusionTexture)
    {
        if (!mesh.material.occlusionMap.isNull())
        {
            const QImage image = mesh.material.occlusionMap.convertToFormat(QImage::Format_RGBA8888);
            if (!image.isNull())
            {
                mesh.occlusionTexture = ctx.rhi->rhi()->newTexture(QRhiTexture::RGBA8, image.size(), 1);
                if (mesh.occlusionTexture->create())
                {
                    QRhiTextureUploadDescription upload(QRhiTextureUploadEntry(0, 0,
                                                                                QRhiTextureSubresourceUploadDescription(image)));
                    u->uploadTexture(mesh.occlusionTexture, upload);
                }
                else
                {
                    delete mesh.occlusionTexture;
                    mesh.occlusionTexture = nullptr;
                }
            }
        }
        if (!mesh.occlusionTexture)
            mesh.occlusionTexture = m_defaultOcclusion;
    }
    if (!mesh.occlusionSampler)
        mesh.occlusionSampler = m_linearSampler;
    if (!mesh.emissiveTexture)
    {
        if (!mesh.material.emissiveMap.isNull())
        {
            const QImage image = mesh.material.emissiveMap.convertToFormat(QImage::Format_RGBA8888);
            if (!image.isNull())
            {
                mesh.emissiveTexture = ctx.rhi->rhi()->newTexture(QRhiTexture::RGBA8, image.size(), 1);
                if (mesh.emissiveTexture->create())
                {
                    QRhiTextureUploadDescription upload(QRhiTextureUploadEntry(0, 0,
                                                                                QRhiTextureSubresourceUploadDescription(image)));
                    u->uploadTexture(mesh.emissiveTexture, upload);
                }
                else
                {
                    delete mesh.emissiveTexture;
                    mesh.emissiveTexture = nullptr;
                }
            }
        }
        if (!mesh.emissiveTexture)
            mesh.emissiveTexture = m_defaultEmissive;
    }
    if (!mesh.emissiveSampler)
        mesh.emissiveSampler = m_linearSampler;
    if (!mesh.srb)
    {
        mesh.srb = ctx.rhi->rhi()->newShaderResourceBindings();
        mesh.srb->setBindings({
            QRhiShaderResourceBinding::uniformBuffer(0, QRhiShaderResourceBinding::VertexStage, m_cameraUbo),
            QRhiShaderResourceBinding::uniformBuffer(1, QRhiShaderResourceBinding::VertexStage, mesh.modelUbo),
            QRhiShaderResourceBinding::uniformBuffer(2, QRhiShaderResourceBinding::FragmentStage, mesh.materialUbo),
            QRhiShaderResourceBinding::sampledTexture(3, QRhiShaderResourceBinding::FragmentStage,
                                                      mesh.baseColorTexture, mesh.baseColorSampler),
            QRhiShaderResourceBinding::sampledTexture(4, QRhiShaderResourceBinding::FragmentStage,
                                                      mesh.normalTexture, mesh.normalSampler),
            QRhiShaderResourceBinding::sampledTexture(5, QRhiShaderResourceBinding::FragmentStage,
                                                      mesh.metallicRoughnessTexture, mesh.metallicRoughnessSampler),
            QRhiShaderResourceBinding::sampledTexture(6, QRhiShaderResourceBinding::FragmentStage,
                                                      mesh.occlusionTexture, mesh.occlusionSampler),
            QRhiShaderResourceBinding::sampledTexture(7, QRhiShaderResourceBinding::FragmentStage,
                                                      mesh.emissiveTexture, mesh.emissiveSampler)
        });
        if (!mesh.srb->create())
            return;
    }
    mesh.gpuReady = true;
}
