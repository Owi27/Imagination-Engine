#pragma once
#include "RenderPass.h"
#include "PipelineBuilder.h"
#include "Resource.hpp"

class TrianglePass : public RenderPass
{
	Texture _depth;

public:
	Buffer pos, col;

	TrianglePass(VulkanContext& pCtx) : RenderPass(pCtx), _depth(pCtx)
	{
		_depth.Swapchain(_vk.depthImage, _vk.depthImageView, _vk.depthFormat, { _vk.win.GetWidth(), _vk.win.GetHeight(), 1 }, ImageAspect::DEPTH | ImageAspect::STENCIL);
		_depth.TransitionImageLayout(ImageLayout::DEPTH_STENCIL);

		GraphicsPipelineBuilder pipelineBuilder(pCtx);

		std::vector<std::pair<std::string, ShaderType>> shaders
		{
			{ "PixelShader", ShaderType::FRAGMENT },
			{ "VertexShader", ShaderType::VERTEX }
		};

		std::vector<VertexInputDescription> vertexInputDescriptions
		{
			VertexInputDescription
			{
				.binding = 0,
				.location = 0,
				.stride = sizeof(float) * 2,
				.format = PipelineFormat::FLOAT2,
				.offset = 0,
			},
			VertexInputDescription
			{
				.binding = 1,
				.location = 1,
				.stride = sizeof(unsigned char) * 4,
				.format = PipelineFormat::COLOR,
				.offset = 0,
			},
		};

		std::vector<PipelineAttachment> pipelineAttachments
		{
			PipelineAttachment{},
		};

		RenderingInfo renderInfo
		{
			.colorAttachmentFormats
			{
				PipelineFormat::SWAPCHAIN,
			},
			.depthStencilFormat = pCtx.depthFormat
		};

		_pipeline = Attempt(pipelineBuilder.AddShaders(shaders)
			.AddVertexBindingDescriptions(vertexInputDescriptions)
			.AddDepthTest()
			.AddDepthWrite()
			.AddPipelineAttachments(pipelineAttachments)
			.SetRenderingInfo(renderInfo)
			.AddPushConstantRange(VkPushConstantRange{ .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, /* or VERTEX_BIT | FRAGMENT_BIT if both use it*/ .offset = 0, .size = sizeof(float) })
			.BuildPipeline(_pipelineLayout));
	}

	void Record(VkCommandBuffer pCommandBuffer) override;
};

class GBufferPass : public RenderPass
{
	enum GBuffer
	{
		Normal,
		Albedo,
		Emissive,
		AO_Rough_Metal,
		Depth,
		Max
	};

	SceneMatrices _sceneMatrices;

	std::unique_ptr<Buffer> _sceneMatriceBuffer;
	//std::array<Texture, GBuffer::Max> _gBuffer;

public:
	GBufferPass(VulkanContext& pCtx) : RenderPass(pCtx) /*Constructor*/
	{
		VkExtent3D extent = { pCtx.win.GetWidth(), pCtx.win.GetHeight(), 1 };

		//normal
		Texture normal(pCtx), albedo(pCtx), emissive(pCtx), aoRM(pCtx), depth(pCtx);
		normal.CreateImage(extent, 1, SampleCount::SAMPLE_1BIT, PipelineFormat::COLOR2, ImageTiling::OPTIMAL, ImageUsage::COLOR_ATTACHMENT | ImageUsage::SAMPLED, MemoryFlags::GPU).CreateImageView(ImageAspect::COLOR).TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);
		albedo.CreateImage(extent, 1, SampleCount::SAMPLE_1BIT, PipelineFormat::COLOR, ImageTiling::OPTIMAL, ImageUsage::COLOR_ATTACHMENT | ImageUsage::SAMPLED, MemoryFlags::GPU).CreateImageView(ImageAspect::COLOR).TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);
		emissive.CreateImage(extent, 1, SampleCount::SAMPLE_1BIT, PipelineFormat::COLOR, ImageTiling::OPTIMAL, ImageUsage::COLOR_ATTACHMENT | ImageUsage::SAMPLED, MemoryFlags::GPU).CreateImageView(ImageAspect::COLOR).TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);
		aoRM.CreateImage(extent, 1, SampleCount::SAMPLE_1BIT, PipelineFormat::COLOR, ImageTiling::OPTIMAL, ImageUsage::COLOR_ATTACHMENT | ImageUsage::SAMPLED, MemoryFlags::GPU).CreateImageView(ImageAspect::COLOR).TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);
		depth.Swapchain(pCtx.depthImage, pCtx.depthImageView, pCtx.depthFormat, extent, ImageAspect::DEPTH | ImageAspect::STENCIL).TransitionImageLayout(ImageLayout::DEPTH_STENCIL).SetOwnership(false);

		AddColorAttachment(AttachmentDesc
			{
				.texture = std::move(&normal),
				.loadOp = LoadOp::CLEAR,
				.storeOp = StoreOp::STORE,
				.clearValue
				{
					.color = {{ 0.f, 0.f, 0.f, 1.f }},
					//.depthStencil = ,
				}
			});
		AddColorAttachment(AttachmentDesc
			{
				.texture = std::move(&albedo),
				.loadOp = LoadOp::CLEAR,
				.storeOp = StoreOp::STORE,
				.clearValue
				{
					.color = {{ 0.f, 0.f, 0.f, 1.f }},
					//.depthStencil = ,
				}
			});
		AddColorAttachment(AttachmentDesc
			{
				.texture = std::move(&emissive),
				.loadOp = LoadOp::CLEAR,
				.storeOp = StoreOp::STORE,
				.clearValue
				{
					.color = {{ 0.f, 0.f, 0.f, 1.f }},
					//.depthStencil = ,
				}
			});
		AddColorAttachment(AttachmentDesc
			{
				.texture = std::move(&aoRM),
				.loadOp = LoadOp::CLEAR,
				.storeOp = StoreOp::STORE,
				.clearValue
				{
					.color = {{ 0.f, 0.f, 0.f, 1.f }},
					//.depthStencil = ,
				}
			});
		SetDepthAttachment(AttachmentDesc
			{
				.texture = std::move(&depth),
				.loadOp = LoadOp::CLEAR,
				.storeOp = StoreOp::STORE,
				.clearValue
				{
					//.color = {{ 0.f, 0.f, 0.f, 1.f }},
					.depthStencil = { 1.f, 0 },
				}
			});

		//graphics pipeline
		GraphicsPipelineBuilder pipelineBuilder(pCtx);
		std::vector<std::pair<std::string, ShaderType>> shaders
		{
			{ "GBufferFragmentShader", ShaderType::FRAGMENT },
			{ "GBufferVertexShader", ShaderType::VERTEX }
		};

		std::vector<VertexInputDescription> vertexInputDescriptions
		{
			VertexInputDescription
			{
				.binding = 0,
				.location = 0,
				.stride = sizeof(float) * 3,
				.format = PipelineFormat::FLOAT3,
				.offset = 0,
			},
			VertexInputDescription
			{
				.binding = 1,
				.location = 1,
				.stride = sizeof(unsigned char) * 2,
				.format = PipelineFormat::COLOR2,
				.offset = 0,
			},
			VertexInputDescription
			{
				.binding = 1,
				.location = 1,
				.stride = sizeof(unsigned char) * 3,
				.format = PipelineFormat::COLOR3,
				.offset = 0,
			},
			VertexInputDescription
			{
				.binding = 1,
				.location = 1,
				.stride = sizeof(unsigned char) * 3,
				.format = PipelineFormat::COLOR3,
				.offset = 0,
			},
			VertexInputDescription
			{
				.binding = 1,
				.location = 1,
				.stride = sizeof(unsigned char) * 3,
				.format = PipelineFormat::COLOR3,
				.offset = 0,
			},
		};

		std::vector<PipelineAttachment> pipelineAttachments
		{
			PipelineAttachment{},
		};

		RenderingInfo renderInfo
		{
			.colorAttachmentFormats
			{
				PipelineFormat::SWAPCHAIN,
			},
			.depthStencilFormat = pCtx.depthFormat
		};

		Buffer descriptor(pCtx);
		descriptor.CreateBuffer(2048, BufferUsage::DESCRIPTOR | BufferUsage::DESCRIPTOR_SAMPLER | BufferUsage::DEVICE_ADDRESS, MemoryFlags::CPU | MemoryFlags::CPU2GPU);

		// uniform buffer;
		_sceneMatrices.view = GW::MATH::GIdentityMatrixF;
		_sceneMatrices.proj = GW::MATH::GIdentityMatrixF;

		_sceneMatriceBuffer = std::make_unique<Buffer>(pCtx);
		_sceneMatriceBuffer->CreateBuffer(sizeof(SceneMatrices), BufferUsage::UNIFORM | BufferUsage::DEVICE_ADDRESS, MemoryFlags::CPU2GPU).WriteToBuffer(&_sceneMatrices);
		

		_pipeline = Attempt(pipelineBuilder.AddShaders(shaders)
			.AddVertexBindingDescriptions(vertexInputDescriptions)
			.AddDepthTest()
			.AddDepthWrite()
			.AddPipelineAttachments(pipelineAttachments)
			.SetRenderingInfo(renderInfo)
			.AddPushConstantRange(VkPushConstantRange{ .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, /* or VERTEX_BIT | FRAGMENT_BIT if both use it*/ .offset = 0, .size = sizeof(float) })
			.BuildPipeline(_pipelineLayout));
	}

	~GBufferPass() /*Destructor*/
	{
	}

	GBufferPass(const GBufferPass& pOther) /*Copy Constructor*/ = delete;
	GBufferPass& operator=(const GBufferPass& pOther) /*Copy Assignment Operator*/ = delete;

	GBufferPass(GBufferPass&& pOther) noexcept : RenderPass(pOther._vk) /*Move Constructor*/
	{
	}

	GBufferPass& operator=(GBufferPass&& pOther) noexcept /*Move Assignment Operator*/
	{
		if (this != &pOther)
		{
		}

		return *this;
	}

	// Inherited via RenderPass
	void Record(VkCommandBuffer pCommandBuffer) override;
};