#pragma once

#include <QtCore/QHash>
#include <QtCore/QSize>
#include <QtCore/QVector>
#include <memory>
#include <rhi/qrhi.h>

class RenderTargetCache
{
public:
    explicit RenderTargetCache(QRhi *rhi);

    struct GBufferTargets
    {
        QRhiTexture *color0 = nullptr;
        QRhiTexture *color1 = nullptr;
        QRhiTexture *color2 = nullptr;
        QRhiTexture *color3 = nullptr;
        QRhiTexture *depth = nullptr;
        QRhiTextureRenderTarget *rt = nullptr;
        QRhiRenderPassDescriptor *rpDesc = nullptr;
        QRhiTexture::Format colorFormat = QRhiTexture::RGBA8;
    };

    struct LightingTargets
    {
        QRhiTexture *color = nullptr;
        QRhiTextureRenderTarget *rt = nullptr;
        QRhiRenderPassDescriptor *rpDesc = nullptr;
        QRhiTexture::Format colorFormat = QRhiTexture::RGBA8;
    };

    GBufferTargets getOrCreateGBuffer(const QSize &size, int sampleCount);
    LightingTargets getOrCreateLightingTarget(const QSize &size, int sampleCount);
    void releaseAll();

private:
    QRhi *m_rhi = nullptr;
    QSize m_lastSize;
    int m_lastSamples = 1;
    GBufferTargets m_gbuffer;
    LightingTargets m_lighting;
};
