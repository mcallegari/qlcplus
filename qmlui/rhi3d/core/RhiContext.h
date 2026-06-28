#pragma once

#include <QtGui/QWindow>
#include <QtGui/QOpenGLContext>
#include <QtGui/QOffscreenSurface>
#include <rhi/qrhi.h>
#if QT_CONFIG(vulkan)
#include <QtGui/QVulkanInstance>
#endif

class RhiContext
{
public:
    RhiContext();

    bool initialize(QWindow *window);
    bool initializeExternal(QRhi *rhi);
    void shutdown();

    void beginFrame();
    void endFrame();

    void resize(const QSize &size);
    void setExternalFrame(QRhiCommandBuffer *cb, QRhiRenderTarget *rt);
    void clearExternalFrame();

    QRhi *rhi() const { return m_rhi; }
    QRhiCommandBuffer *commandBuffer() const { return m_externalCb ? m_externalCb : m_cb; }
    QRhiRenderTarget *swapchainRenderTarget() const;

private:
    QWindow *m_window = nullptr;
    QRhi *m_rhi = nullptr;
    QRhiSwapChain *m_swapChain = nullptr;
    QRhiRenderBuffer *m_swapChainDepthStencil = nullptr;
    QRhiRenderPassDescriptor *m_swapChainRpDesc = nullptr;
    QRhiCommandBuffer *m_cb = nullptr;
    QRhiCommandBuffer *m_externalCb = nullptr;
    QRhiRenderTarget *m_externalRt = nullptr;
#if QT_CONFIG(vulkan)
    QVulkanInstance m_vkInstance;
#endif
    QOpenGLContext *m_glContext = nullptr;
    QOffscreenSurface *m_glOffscreenSurface = nullptr;
    QSize m_swapChainSize;
    QRhi::Implementation m_backend = QRhi::Null;
    bool m_ownsRhi = true;
};
