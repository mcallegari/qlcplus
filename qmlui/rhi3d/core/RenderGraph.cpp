#include "core/RenderGraph.h"


void RenderGraph::addPass(std::unique_ptr<RenderPass> pass)
{
    m_passes.push_back(std::move(pass));
}

void RenderGraph::clear()
{
    m_passes.clear();
}

void RenderGraph::run(FrameContext &ctx)
{
    if (!ctx.rhi)
    {
        qWarning() << "RenderGraph: missing RHI context";
        return;
    }

    for (const auto &pass : m_passes)
        pass->prepare(ctx);

    for (const auto &pass : m_passes)
        pass->execute(ctx);
}
