/*
  Q Light Controller Plus
  DeferredRenderer.h

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

#include <memory>

#include "core/RenderGraph.h"

class RhiContext;
class RenderTargetCache;
class ShaderManager;
class RhiScene;

class DeferredRenderer
{
public:
    DeferredRenderer();

    void initialize(RhiContext *rhi, RenderTargetCache *targets, ShaderManager *shaders);
    void resize(const QSize &size);
    void render(RhiScene *scene);

private:
    RenderGraph m_graph;
    FrameContext m_frameCtx;
    ShadowData m_shadowData;
    LightCullingData m_lightCulling;
};
