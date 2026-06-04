/*
  Q Light Controller Plus
  PassLightCulling.h

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
#include <QtCore/QSize>
#include <rhi/qrhi.h>

class PassLightCulling final : public RenderPass
{
public:
    void prepare(FrameContext &ctx) override;
    void execute(FrameContext &ctx) override;

private:
    void ensurePipeline(FrameContext &ctx);
    void ensureBuffers(FrameContext &ctx, const QSize &size);

    QRhiComputePipeline *m_pipeline = nullptr;
    QRhiShaderResourceBindings *m_srb = nullptr;
    QRhiBuffer *m_lightUbo = nullptr;
    QRhiBuffer *m_cullUbo = nullptr;
    QRhiTexture *m_lightIndexTexture = nullptr;
    QSize m_lastSize;
    int m_clusterSize = 120;
    int m_clusterCountZ = 24;
    int m_lastClusterCountX = 0;
    int m_lastClusterCountY = 0;
};
