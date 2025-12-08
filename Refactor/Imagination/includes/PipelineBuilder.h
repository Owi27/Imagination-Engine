#pragma once
#include "VulkanCtx.h"
#include "Shader.h"

struct PipelineBuilder
{
	PipelineBuilder() = default;
	~PipelineBuilder() = default;

	virtual VkPipeline BuildPipeline(VkPipelineLayout& pPipelineLayout) = 0;
};

struct PipelineAttachment
{
    bool blend = false;

    BlendFactor colorSource = BlendFactor::SOURCE_ALPHA, colorDestination = BlendFactor::NEG_SOURCE_ALPHA;
    BlendOperation colorOperation = BlendOperation::ADD;

    BlendFactor alphaSource = BlendFactor::FULL, alphaDestination = BlendFactor::NONE;
    BlendOperation alphaOperation = BlendOperation::ADD;

    VkColorComponentFlags writeMask = (VkColorComponentFlags)ColorComponent::R | (VkColorComponentFlags)ColorComponent::G | (VkColorComponentFlags)ColorComponent::B | (VkColorComponentFlags)ColorComponent::A;
};

struct GraphicsPipelineBuilder : PipelineBuilder
{
    GraphicsPipelineBuilder() : PipelineBuilder()
    {
    }

    ~GraphicsPipelineBuilder() = default;

    GraphicsPipelineBuilder& AddShaders(std::vector<std::pair<std::string, ShaderType>> pShaders);
    GraphicsPipelineBuilder& AddDescriptorSetLayout(VkDescriptorSetLayout pLayout);
    GraphicsPipelineBuilder& AddPushConstantRange(VkPushConstantRange pPushConstantRange);
    GraphicsPipelineBuilder& SetRenderingFormats(const std::vector<PipelineFormat>& pFormats, PipelineFormat pDepthFormat);
    GraphicsPipelineBuilder& SetBlendAttachments(const std::vector<PipelineAttachment>& pAttachments);

    VkPipeline BuildPipeline(VkPipelineLayout& pOutLayout) override;

private:
    std::vector<Shader> _shaders;
    std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
    std::vector<VkDescriptorSetLayout> _setLayouts;
    std::vector<VkPushConstantRange>   _pushConstants;
    VkFormat _depthFormat;
    std::vector<VkFormat> _colorFormats;
    std::vector<VkPipelineColorBlendAttachmentState> _blendAttachments;
    //std::vector<VkDynamicState> _dynamicStates;

    VkPipelineVertexInputStateCreateInfo   _vertexInputStateCreateInfo;
    VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
    VkPipelineViewportStateCreateInfo      _viewportState;
    VkPipelineRasterizationStateCreateInfo _rasterState;
    VkPipelineMultisampleStateCreateInfo   _multisample;
    VkPipelineDepthStencilStateCreateInfo  _depthStencil;
    VkPipelineColorBlendStateCreateInfo    _colorBlendState;
    VkPipelineDynamicStateCreateInfo       _dynamicStateCI;
    VkPipelineRenderingCreateInfoKHR       _renderingInfo;
};