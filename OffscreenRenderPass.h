#pragma once
#include "RenderPass.h"
class OffscreenRenderPass :
    public RenderPass
{
public:
    OffscreenRenderPass();
    ~OffscreenRenderPass();

    void Setup() override;
    void Execute() override;
};

