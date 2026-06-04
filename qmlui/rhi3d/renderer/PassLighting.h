/*
  Q Light Controller Plus
  PassLighting.h

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

#pragma once

#include "core/RenderGraph.h"
#include <QtCore/QString>
#include <QtCore/QSize>
#include <QtCore/QHash>
#include <QtGui/QImage>
#include <QtGui/QVector4D>

class QRhiBuffer;
class QRhiGraphicsPipeline;
class QRhiRenderTarget;
class QRhiRenderPassDescriptor;
class QRhiResourceUpdateBatch;
class QRhiSampler;
class QRhiShaderResourceBindings;
class QRhiTexture;

class PassLighting final : public RenderPass
{
public:
    void prepare(FrameContext &ctx) override;
    void execute(FrameContext &ctx) override;

private:
    void ensurePipeline(FrameContext &ctx);
    void ensureSelectionBoxesPipeline(FrameContext &ctx, QRhiRenderTarget *rt);
    void updateGoboTextures(FrameContext &ctx, QRhiResourceUpdateBatch *u);
    QImage loadGoboCached(const QString &path);

    QRhiGraphicsPipeline *m_pipeline = nullptr;
    QRhiShaderResourceBindings *m_srb = nullptr;
    QRhiSampler *m_sampler = nullptr;
    QRhiSampler *m_shadowSampler = nullptr;
    QRhiSampler *m_spotShadowSampler = nullptr;
    QRhiSampler *m_goboSampler = nullptr;
    QRhiSampler *m_lightIndexSampler = nullptr;
    QRhiBuffer *m_lightsUbo = nullptr;
    QRhiBuffer *m_cameraUbo = nullptr;
    QRhiBuffer *m_shadowUbo = nullptr;
    QRhiBuffer *m_flipUbo = nullptr;
    QRhiBuffer *m_lightCullUbo = nullptr;
    QRhiTexture *m_lightIndexTexture = nullptr;
    QRhiRenderPassDescriptor *m_rpDesc = nullptr;
    QRhiTexture *m_shadowMapRefs[3] = { nullptr, nullptr, nullptr };
    QRhiTexture *m_gbufColor0 = nullptr;
    QRhiTexture *m_gbufColor1 = nullptr;
    QRhiTexture *m_gbufColor2 = nullptr;
    QRhiTexture *m_gbufColor3 = nullptr;
    QRhiTexture *m_gbufDepth = nullptr;
    bool m_gbufWorldPosFloat = false;
    QRhiTexture *m_spotShadowMapArray = nullptr;
    QRhiTexture *m_spotGoboMap = nullptr;
    QString m_spotGoboPaths[kMaxLights];
    QSize m_spotGoboSize = QSize(256, 256);
    QHash<QString, QImage> m_goboCache;
    bool m_reverseZ = false;
    bool m_useLightCulling = false;
    QVector4D m_lastFlip = QVector4D(-1.0f, -1.0f, 0.0f, 0.0f);
    QVector4D m_lastLightCullScreen = QVector4D();
    QVector4D m_lastLightCullCluster = QVector4D();
    QVector4D m_lastLightCullZParams = QVector4D();
    QVector4D m_lastLightCullFlags = QVector4D();
    bool m_lightCullParamsValid = false;

    QRhiGraphicsPipeline *m_selectionPipeline = nullptr;
    QRhiShaderResourceBindings *m_selectionSrb = nullptr;
    QRhiBuffer *m_selectionUbo = nullptr;
    QRhiBuffer *m_selectionDepthUbo = nullptr;
    QRhiBuffer *m_selectionScreenUbo = nullptr;
    QRhiBuffer *m_selectionVbuf = nullptr;
    QRhiRenderPassDescriptor *m_selectionRpDesc = nullptr;
    int m_selectionMaxVertices = 0;
    int m_selectionVertexCount = 0;
    bool m_selectionCacheValid = false;
    bool m_selectionAnySelected = false;


};
