#pragma once

#include <QtGui/QMatrix4x4>
#include <QtGui/QVector4D>
#include <memory>
#include <vector>

class RhiContext;
class RenderTargetCache;
class ShaderManager;
class RhiScene;
class QRhiTexture;
class QRhiBuffer;

inline constexpr int kMaxLights = 100;
inline constexpr int kMaxSpotShadows = 32;

struct ShadowData
{
    int cascadeCount = 0;
    QVector4D splits;
    QVector4D dirLightDir;
    QVector4D dirLightColorIntensity;
    QMatrix4x4 lightViewProj[3];
    QRhiTexture *shadowMaps[3] = { nullptr, nullptr, nullptr };
    QMatrix4x4 spotLightViewProj[kMaxLights];
    QVector4D spotShadowParams[kMaxLights];
    int spotShadowCount = 0;
    QRhiTexture *spotShadowMapArray = nullptr;
    QVector4D shadowDepthParams;
};

struct LightCullingData
{
    QRhiTexture *clusterLightIndexTexture = nullptr;
    int clusterCountX = 0;
    int clusterCountY = 0;
    int clusterCountZ = 0;
    int clusterSize = 120;
    float logScale = 0.0f;
    float logBias = 0.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    bool enabled = false;
};

struct FrameContext
{
    RhiContext *rhi = nullptr;
    RenderTargetCache *targets = nullptr;
    ShaderManager *shaders = nullptr;
    RhiScene *scene = nullptr;
    ShadowData *shadows = nullptr;
    LightCullingData *lightCulling = nullptr;
    bool lightingEnabled = true;
};

class RenderPass
{
public:
    virtual ~RenderPass() = default;
    virtual void prepare(FrameContext &ctx) = 0;
    virtual void execute(FrameContext &ctx) = 0;
};

class RenderGraph
{
public:
    void addPass(std::unique_ptr<RenderPass> pass);
    void clear();
    void run(FrameContext &ctx);

private:
    std::vector<std::unique_ptr<RenderPass>> m_passes;
};
