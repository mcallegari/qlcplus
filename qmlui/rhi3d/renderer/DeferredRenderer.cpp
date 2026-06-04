/*
  Q Light Controller Plus
  DeferredRenderer.cpp

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

#include "renderer/DeferredRenderer.h"

#include "core/RhiContext.h"
#include "core/RenderTargetCache.h"
#include "core/ShaderManager.h"
#include "renderer/PassDepth.h"
#include "renderer/PassGBuffer.h"
#include "renderer/PassLightCulling.h"
#include "renderer/PassLighting.h"
#include "renderer/PassPost.h"
#include "renderer/PassShadow.h"
#include "scene/Scene.h"

DeferredRenderer::DeferredRenderer() = default;

void DeferredRenderer::initialize(RhiContext *rhi, RenderTargetCache *targets, ShaderManager *shaders)
{
    m_frameCtx.rhi = rhi;
    m_frameCtx.targets = targets;
    m_frameCtx.shaders = shaders;
    m_frameCtx.shadows = &m_shadowData;
    m_frameCtx.lightCulling = &m_lightCulling;

    const bool skipLighting = qEnvironmentVariableIsSet("RHIPIPELINE_SKIP_LIGHTING");
    const bool skipPost = qEnvironmentVariableIsSet("RHIPIPELINE_SKIP_POST");
    m_frameCtx.lightingEnabled = !skipLighting;
    m_graph.clear();
    m_graph.addPass(std::make_unique<PassDepth>());
    m_graph.addPass(std::make_unique<PassGBuffer>());
    m_graph.addPass(std::make_unique<PassShadow>());
    m_graph.addPass(std::make_unique<PassLightCulling>());
    if (skipLighting)
    {
        qWarning() << "DeferredRenderer: skipping PassLighting (RHIPIPELINE_SKIP_LIGHTING)";
    }
    else
    {
        m_graph.addPass(std::make_unique<PassLighting>());
    }
    if (skipPost)
    {
        qWarning() << "DeferredRenderer: skipping PassPost (RHIPIPELINE_SKIP_POST)";
    }
    else
    {
        m_graph.addPass(std::make_unique<PassPost>());
    }
}

void DeferredRenderer::resize(const QSize &size)
{
    Q_UNUSED(size);
    // TODO: notify passes and target cache.
}

void DeferredRenderer::render(RhiScene *scene)
{
    m_frameCtx.scene = scene;
    m_graph.run(m_frameCtx);
}
