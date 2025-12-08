#pragma once
#include "Shader.h"

struct PipelineBuilder
{
	PipelineBuilder(VulkanContext& vk)
	{
		_vk = std::make_unique<VulkanContext>(vk);
	}

	~PipelineBuilder() = default;


	virtual RETURN(VkPipeline) BuildPipeline(VkPipelineLayout& pipelineLayout) = 0;

protected:
	std::unique_ptr<VulkanContext> _vk;
};


struct VertexInputDescription
{
	unsigned binding;
	unsigned location;
	unsigned stride;
	PipelineFormat format;
	unsigned offset;
};


struct RasterStateInfo
{
	PolygonMode polygonMode;
	CullMode cullMode;
	FrontFace frontFace;
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

struct RenderingInfo
{
	std::vector<PipelineFormat> colorAttachmentFormats;
	PipelineFormat depthStencilFormat = PipelineFormat::UNDEFINED;
};

struct GraphicsPipelineBuilder : PipelineBuilder
{
	GraphicsPipelineBuilder(VulkanContext& vk) : PipelineBuilder(vk)
	{
		VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.vertexBindingDescriptionCount = 0,
			.pVertexBindingDescriptions = nullptr,
			.vertexAttributeDescriptionCount = 0,
			.pVertexAttributeDescriptions = nullptr
		};

		VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = false
		};

		VkPipelineTessellationStateCreateInfo pipelineTessellationStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.patchControlPoints = 0,
		};

		_viewport =
		{
			.x = 0,
			.y = 0,
			.width = static_cast<float>(_vk->win.GetWidth()),
			.height = static_cast<float>(_vk->win.GetHeight()),
			.minDepth = 0,
			.maxDepth = 1,
		};

		_scissor =
		{
			.offset
			{
				.x = 0,
				.y = 0
			},
			.extent
			{
				.width = _vk->win.GetWidth(),
				.height = _vk->win.GetHeight()
			}
		};

		VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.viewportCount = 1,
			.pViewports = &_viewport,
			.scissorCount = 1,
			.pScissors = &_scissor,
		};

		VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo
		{
			//.pNext = ,
			//.flags = ,

			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.depthClampEnable = false,
			.rasterizerDiscardEnable = false,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.depthBiasEnable = false,
			.depthBiasConstantFactor = 0.0f,
			.depthBiasClamp = 0.0f,
			.depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.f,
		};

		VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = false,
			.minSampleShading = 1.f,
			.pSampleMask = nullptr,
			.alphaToCoverageEnable = false,
			.alphaToOneEnable = false,
		};

		VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.depthTestEnable = false,
			.depthWriteEnable = false,
			.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
			.depthBoundsTestEnable = false,
			.stencilTestEnable = false,
			//.front = ,
			//.back = ,
			.minDepthBounds = 0.f,
			.maxDepthBounds = 1.f,
		};

		VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.logicOpEnable = false,
			.logicOp = VK_LOGIC_OP_COPY,
			.attachmentCount = 0,
			.pAttachments = nullptr,
			.blendConstants
			{
				0.f, 0.f, 0.f, 0.f
			},
		};

		_dynamicStates =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			//.pNext = ,
			//.flags = ,
			.dynamicStateCount = (unsigned)_dynamicStates.size(),
			.pDynamicStates = _dynamicStates.data()
		};

		VkPipelineRenderingCreateInfoKHR pipelineRenderingCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
			//.pNext = ,
			//.viewMask = ,
			.colorAttachmentCount = 0,
			.pColorAttachmentFormats = nullptr,
			.depthAttachmentFormat = VK_FORMAT_UNDEFINED,
			.stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
		};

		_pipelineVertexInputStateCreateInfo = pipelineVertexInputStateCreateInfo;
		_pipelineInputAssemblyStateCreateInfo = pipelineInputAssemblyStateCreateInfo;
		_pipelineTessellationStateCreateInfo = pipelineTessellationStateCreateInfo;
		_pipelineViewportStateCreateInfo = pipelineViewportStateCreateInfo;
		_pipelineRasterizationStateCreateInfo = pipelineRasterizationStateCreateInfo;
		_pipelineMultisampleStateCreateInfo = pipelineMultisampleStateCreateInfo;
		_pipelineDepthStencilStateCreateInfo = pipelineDepthStencilStateCreateInfo;
		_pipelineColorBlendStateCreateInfo = pipelineColorBlendStateCreateInfo;
		_pipelineDynamicStateCreateInfo = pipelineDynamicStateCreateInfo;
		_pipelineRenderingCreateInfo = pipelineRenderingCreateInfo;
	}

	~GraphicsPipelineBuilder() = default;

	GraphicsPipelineBuilder& AddShaders(std::vector<std::pair<std::string, ShaderType>> shaders);
	GraphicsPipelineBuilder& AddVertexBindingDescriptions(std::vector<VertexInputDescription> inputDescriptions);
	GraphicsPipelineBuilder& BuildTessellationState(unsigned patchControlPoints);
	GraphicsPipelineBuilder& BuildViewportState(unsigned width, unsigned height);
	GraphicsPipelineBuilder& SetTopology(Topology topology);
	GraphicsPipelineBuilder& SetRasterizationState(RasterStateInfo rasterStateInfo);
	GraphicsPipelineBuilder& AddDepthTest();
	GraphicsPipelineBuilder& AddDepthWrite();
	GraphicsPipelineBuilder& AddStencilTest();
	GraphicsPipelineBuilder& AddPipelineAttachments(std::vector<PipelineAttachment> pipelineAttachments);
	GraphicsPipelineBuilder& AddDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout);
	GraphicsPipelineBuilder& AddPushConstantRange(VkPushConstantRange pushConstantRange);
	GraphicsPipelineBuilder& SetRenderingInfo(RenderingInfo renderingInfo);

	RETURN(VkPipeline) BuildPipeline(VkPipelineLayout& pipelineLayout) override;

private:
	VkViewport _viewport;
	VkRect2D   _scissor;
	//VkPipelineLayout _pipelineLayout;
	std::vector<Shader> _shaders;
	std::vector<VkDynamicState> _dynamicStates;          // dynamic states
	std::vector<VkFormat> _colorAttachmentFormats;   // real VkFormat storage
	std::vector<VkVertexInputBindingDescription> _inputBindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> _inputAttributeDescriptions;
	std::vector<VkPipelineColorBlendAttachmentState> _blendAttachments;
	std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
	std::vector<VkPushConstantRange> _pipelinePushConstantRanges;
	std::vector<VkDescriptorSetLayout> _pipelineDescriptorSetLayouts;
	std::vector<VkPipelineShaderStageCreateInfo> _pipelineShaderStageCreateInfos;
	VkPipelineVertexInputStateCreateInfo _pipelineVertexInputStateCreateInfo;
	VkPipelineInputAssemblyStateCreateInfo _pipelineInputAssemblyStateCreateInfo;
	VkPipelineTessellationStateCreateInfo _pipelineTessellationStateCreateInfo;
	VkPipelineViewportStateCreateInfo _pipelineViewportStateCreateInfo;
	VkPipelineRasterizationStateCreateInfo _pipelineRasterizationStateCreateInfo;
	VkPipelineMultisampleStateCreateInfo _pipelineMultisampleStateCreateInfo;
	VkPipelineDepthStencilStateCreateInfo _pipelineDepthStencilStateCreateInfo;
	VkPipelineColorBlendStateCreateInfo _pipelineColorBlendStateCreateInfo;
	VkPipelineDynamicStateCreateInfo _pipelineDynamicStateCreateInfo;
	VkPipelineRenderingCreateInfoKHR _pipelineRenderingCreateInfo;
};