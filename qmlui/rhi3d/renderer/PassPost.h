/*
  Q Light Controller Plus
  PassPost.h

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
#include <rhi/qrhi.h>

struct Mesh;

class PassPost final : public RenderPass
{
public:
    void prepare(FrameContext &ctx) override;
    void execute(FrameContext &ctx) override;

private:
    void ensureGizmoPipeline(FrameContext &ctx);
    void ensureGizmoMeshBuffers(FrameContext &ctx, Mesh &mesh, QRhiResourceUpdateBatch *u);

    QRhiTexture *m_bloomTex = nullptr;
    QRhiTextureRenderTarget *m_bloomRt = nullptr;
    QRhiRenderPassDescriptor *m_bloomRpDesc = nullptr;
    QRhiTexture *m_bloomBlurTex = nullptr;
    QRhiTextureRenderTarget *m_bloomBlurRt = nullptr;
    QRhiRenderPassDescriptor *m_bloomBlurRpDesc = nullptr;
    QRhiBuffer *m_postUbo = nullptr;
    QRhiShaderResourceBindings *m_bloomSrb = nullptr;
    QRhiShaderResourceBindings *m_bloomUpsampleSrb = nullptr;
    QRhiShaderResourceBindings *m_combineSrb = nullptr;
    QRhiGraphicsPipeline *m_bloomPipeline = nullptr;
    QRhiGraphicsPipeline *m_bloomUpsamplePipeline = nullptr;
    QRhiGraphicsPipeline *m_combinePipeline = nullptr;
    QRhiGraphicsPipeline *m_gizmoPipeline = nullptr;
    QRhiShaderResourceBindings *m_gizmoLayoutSrb = nullptr;
    QRhiBuffer *m_gizmoModelUbo = nullptr;
    QRhiBuffer *m_gizmoMaterialUbo = nullptr;
    QRhiSampler *m_sampler = nullptr;
    QRhiRenderPassDescriptor *m_swapRpDesc = nullptr;
    QRhiBuffer *m_gizmoCameraUbo = nullptr;
    QSize m_lastSize;
    bool m_combineUsesGBuffer = false;
};
