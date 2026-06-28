#include "core/RenderTargetCache.h"


RenderTargetCache::RenderTargetCache(QRhi *rhi)
    : m_rhi(rhi)
{
}

RenderTargetCache::GBufferTargets RenderTargetCache::getOrCreateGBuffer(const QSize &size, int sampleCount)
{
    if (m_gbuffer.rt && (m_lastSize == size) && (m_lastSamples == sampleCount))
        return m_gbuffer;

    releaseAll();

    m_lastSize = size;
    m_lastSamples = sampleCount;

    QRhiTexture::Format gbufFormat = QRhiTexture::RGBA16F;
    if (!m_rhi->isTextureFormatSupported(gbufFormat, QRhiTexture::RenderTarget))
    {
        gbufFormat = QRhiTexture::RGBA8;
        qWarning() << "RenderTargetCache: RGBA16F not supported, falling back to RGBA8";
    }

    QRhiTexture::Format depthFormat = QRhiTexture::D24S8;
    if (!m_rhi->isTextureFormatSupported(depthFormat, QRhiTexture::RenderTarget))
    {
        depthFormat = QRhiTexture::D32F;
        qWarning() << "RenderTargetCache: D24S8 not supported, falling back to D32F";
    }

    m_gbuffer.colorFormat = gbufFormat;
    m_gbuffer.color0 = m_rhi->newTexture(gbufFormat, size, sampleCount, QRhiTexture::RenderTarget);
    m_gbuffer.color1 = m_rhi->newTexture(gbufFormat, size, sampleCount, QRhiTexture::RenderTarget);
    m_gbuffer.color2 = m_rhi->newTexture(gbufFormat, size, sampleCount, QRhiTexture::RenderTarget);
    m_gbuffer.color3 = m_rhi->newTexture(gbufFormat, size, sampleCount, QRhiTexture::RenderTarget);
    m_gbuffer.depth = m_rhi->newTexture(depthFormat, size, sampleCount, QRhiTexture::RenderTarget);

    if (!m_gbuffer.color0->create() || !m_gbuffer.color1->create() ||
        !m_gbuffer.color2->create() || !m_gbuffer.color3->create() || !m_gbuffer.depth->create())
        {
        qWarning() << "RenderTargetCache: failed to create GBuffer textures";
        releaseAll();
        return m_gbuffer;
    }

    QRhiTextureRenderTargetDescription rtDesc;
    rtDesc.setColorAttachments({
        QRhiColorAttachment(m_gbuffer.color0),
        QRhiColorAttachment(m_gbuffer.color1),
        QRhiColorAttachment(m_gbuffer.color2),
        QRhiColorAttachment(m_gbuffer.color3)
    });
    rtDesc.setDepthTexture(m_gbuffer.depth);

    m_gbuffer.rt = m_rhi->newTextureRenderTarget(rtDesc);
    m_gbuffer.rpDesc = m_gbuffer.rt->newCompatibleRenderPassDescriptor();
    m_gbuffer.rt->setRenderPassDescriptor(m_gbuffer.rpDesc);

    if (!m_gbuffer.rt->create())
    {
        qWarning() << "RenderTargetCache: failed to create GBuffer render target";
        releaseAll();
        return m_gbuffer;
    }

    return m_gbuffer;
}

RenderTargetCache::LightingTargets RenderTargetCache::getOrCreateLightingTarget(const QSize &size, int sampleCount)
{
    if (m_lighting.rt && (m_lastSize == size) && (m_lastSamples == sampleCount))
        return m_lighting;

    if (m_lastSize != size || m_lastSamples != sampleCount)
        releaseAll();

    m_lastSize = size;
    m_lastSamples = sampleCount;

    QRhiTexture::Format colorFormat = QRhiTexture::RGBA16F;
    if (!m_rhi->isTextureFormatSupported(colorFormat, QRhiTexture::RenderTarget))
    {
        colorFormat = QRhiTexture::RGBA8;
        qWarning() << "RenderTargetCache: RGBA16F not supported, falling back to RGBA8";
    }

    m_lighting.colorFormat = colorFormat;
    m_lighting.color = m_rhi->newTexture(colorFormat, size, sampleCount, QRhiTexture::RenderTarget);
    if (!m_lighting.color->create())
    {
        qWarning() << "RenderTargetCache: failed to create lighting texture";
        releaseAll();
        return m_lighting;
    }

    QRhiTextureRenderTargetDescription rtDesc;
    rtDesc.setColorAttachments({ QRhiColorAttachment(m_lighting.color) });
    m_lighting.rt = m_rhi->newTextureRenderTarget(rtDesc);
    m_lighting.rpDesc = m_lighting.rt->newCompatibleRenderPassDescriptor();
    m_lighting.rt->setRenderPassDescriptor(m_lighting.rpDesc);
    if (!m_lighting.rt->create())
    {
        qWarning() << "RenderTargetCache: failed to create lighting render target";
        releaseAll();
        return m_lighting;
    }

    return m_lighting;
}

void RenderTargetCache::releaseAll()
{
    delete m_gbuffer.rpDesc;
    m_gbuffer.rpDesc = nullptr;
    delete m_gbuffer.rt;
    m_gbuffer.rt = nullptr;
    delete m_gbuffer.color0;
    delete m_gbuffer.color1;
    delete m_gbuffer.color2;
    delete m_gbuffer.color3;
    delete m_gbuffer.depth;
    m_gbuffer = {};

    delete m_lighting.rpDesc;
    m_lighting.rpDesc = nullptr;
    delete m_lighting.rt;
    m_lighting.rt = nullptr;
    delete m_lighting.color;
    m_lighting = {};
}
