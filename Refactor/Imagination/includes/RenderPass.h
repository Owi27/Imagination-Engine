#pragma once
#include "VkResources.h"
#include "PipelineBuilder.h"
#include "Window.h"
#include "Structs.h"

struct AttachmentDesc
{
	Texture* texture = nullptr;
	LoadOp loadOp = LoadOp::CLEAR;
	StoreOp storeOp = StoreOp::STORE;
	VkClearValue clearValue = {};
};

class RenderPass
{
	unsigned _width = 0, _height = 0;
	std::vector<AttachmentDesc> _colorAttachments;
	AttachmentDesc _depthAttachment;
	bool _hasDepthAttachment = false;
	//std::array<Texture, 3> _swapchain;// = { Texture(_vk), Texture(_vk), Texture(_vk) };

protected:
	VkPipeline _pipeline;
	VkPipelineLayout _pipelineLayout;
	bool _renderingToSwapchain = false;

	virtual void Record(VkCommandBuffer pCommandBuffer) = 0;

public:
	//RenderPass() = defualt;

	RenderPass()
	{
	}

	virtual ~RenderPass() = default;

	void AddColorAttachment(const AttachmentDesc& pDesc)
	{
		_colorAttachments.push_back(pDesc);
	}

	AttachmentDesc& GetColorAttachment(int pIndex)
	{
		return _colorAttachments[pIndex];
	}

	void SetDepthAttachment(const AttachmentDesc& pDesc)
	{
		_depthAttachment = pDesc;
		_hasDepthAttachment = true;
	}

	void Execute(VkCommandBuffer pCommandBuffer)
	{
		std::vector<VkRenderingAttachmentInfo> colorAttachmentInfos(_colorAttachments.size());

		int i = 0;
		for (AttachmentDesc& colorAttachment : _colorAttachments)
		{
			VkRenderingAttachmentInfo renderingAttachmentInfo
			{
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				//.pNext = ,
				.imageView = colorAttachment.texture->imageView,
				.imageLayout = (VkImageLayout)colorAttachment.texture->imageLayout,
				//.resolveMode = ,
				//.resolveImageView = ,
				//.resolveImageLayout = ,
				.loadOp = (VkAttachmentLoadOp)colorAttachment.loadOp,
				.storeOp = (VkAttachmentStoreOp)colorAttachment.storeOp,
				.clearValue = colorAttachment.clearValue,
			};

			colorAttachmentInfos[i] = renderingAttachmentInfo;
			i++;
		}

		std::optional<VkRenderingAttachmentInfo> depthAttachmentInfo;

		if (_hasDepthAttachment)
		{
			VkRenderingAttachmentInfo renderingAttachmentInfo
			{
				.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
				//.pNext = ,
				.imageView = VkCtx::Instance().depth->imageView,
				.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				//.resolveMode = ,
				//.resolveImageView = ,
				//.resolveImageLayout = ,
				.loadOp = (VkAttachmentLoadOp)_depthAttachment.loadOp,
				.storeOp = (VkAttachmentStoreOp)_depthAttachment.storeOp,
				.clearValue = _depthAttachment.clearValue,
			};

			depthAttachmentInfo = renderingAttachmentInfo;
		}

		if (_width == 0 || _height == 0)
		{
			_width = Window::GetInstance().GetWidth();
			_height = Window::GetInstance().GetHeight();
		}

		VkExtent2D extent
		{
			.width = _width,
			.height = _height,
		};

		VkRenderingInfo renderingInfo
		{
			.sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
			//.pNext = ,
			//.flags = ,
			.renderArea
			{
				.offset
				{
					.x = 0,
					.y = 0,
				},
				.extent = extent
			},
			.layerCount = 1,
			//.viewMask = ,
			.colorAttachmentCount = static_cast<unsigned>(colorAttachmentInfos.size()),
			.pColorAttachments = colorAttachmentInfos.data(),
			.pDepthAttachment = depthAttachmentInfo ? &*depthAttachmentInfo : nullptr,
			//.pStencilAttachment = ,
		};

		vkCmdBeginRendering(pCommandBuffer, &renderingInfo);
		Record(pCommandBuffer);
		vkCmdEndRendering(pCommandBuffer);
	}

	void SetSize(unsigned pWidth, unsigned pHeight)
	{
		_width = pWidth;
		_height = pHeight;
	}
};

class TrianglePass : public RenderPass
{
	Descriptor gBufferDescriptor; //0: scenedata, 1: material, 2: textures

public:
	Buffer pos, col;

	TrianglePass() : RenderPass()
	{
		//_depth.Swapchain(_vk.depthImage, _vk.depthImageView, _vk.depthFormat, { _vk.win.GetWidth(), _vk.win.GetHeight(), 1 }, ImageAspect::DEPTH | ImageAspect::STENCIL);
		//_depth.TransitionImageLayout(ImageLayout::DEPTH_STENCIL);

		std::vector<std::pair<std::string, ShaderType>> shaders
		{
			{ "FragmentShader", ShaderType::FRAGMENT },
			{ "VertexShader", ShaderType::VERTEX }
		};

		Texture normal, albedo, emissive, aoRM;
		normal.CreateImage(VkCtx::Instance().swapchainExtent, 1, SampleCount::SAMPLE_1BIT, PipelineFormat::COLOR2, ImageTiling::OPTIMAL, ImageUsage::COLOR_ATTACHMENT | ImageUsage::SAMPLED, MemoryFlags::GPU).CreateImageView(ImageAspect::COLOR).TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);
		albedo.CreateImage(VkCtx::Instance().swapchainExtent, 1, SampleCount::SAMPLE_1BIT, PipelineFormat::COLOR, ImageTiling::OPTIMAL, ImageUsage::COLOR_ATTACHMENT | ImageUsage::SAMPLED, MemoryFlags::GPU).CreateImageView(ImageAspect::COLOR).TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);
		emissive.CreateImage(VkCtx::Instance().swapchainExtent, 1, SampleCount::SAMPLE_1BIT, PipelineFormat::COLOR, ImageTiling::OPTIMAL, ImageUsage::COLOR_ATTACHMENT | ImageUsage::SAMPLED, MemoryFlags::GPU).CreateImageView(ImageAspect::COLOR).TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);
		aoRM.CreateImage(VkCtx::Instance().swapchainExtent, 1, SampleCount::SAMPLE_1BIT, PipelineFormat::COLOR, ImageTiling::OPTIMAL, ImageUsage::COLOR_ATTACHMENT | ImageUsage::SAMPLED, MemoryFlags::GPU).CreateImageView(ImageAspect::COLOR).TransitionImageLayout(ImageLayout::COLOR_ATTACHMENT);

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
				.texture = VkCtx::Instance().depth.get(),
				.loadOp = LoadOp::CLEAR,
				.storeOp = StoreOp::STORE,
				.clearValue
				{
				//.color = {{ 0.f, 0.f, 0.f, 1.f }},
				.depthStencil = { 1.f, 0 },
			}
			});



		std::vector<PipelineFormat> colorFormats;
		colorFormats.push_back(PipelineFormat::COLOR2);
		colorFormats.push_back(PipelineFormat::COLOR);
		colorFormats.push_back(PipelineFormat::COLOR);
		colorFormats.push_back(PipelineFormat::COLOR);

		std::vector<PipelineAttachment> attachments =
		{
			PipelineAttachment(),
			PipelineAttachment(),
			PipelineAttachment(),
			PipelineAttachment(),
		};

		gBufferDescriptor
			.AddLayoutBinding(0, DescriptorType::UNIFORM_BUFFER, ShaderStage::VERTEX)
			.AddLayoutBinding(1, DescriptorType::STORAGE_BUFFER, ShaderStage::FRAGMENT)
			.AddLayoutBinding(2, DescriptorType::IMAGE_SAMPLER, ShaderStage::FRAGMENT, 256)
			.CreateDescriptorSetLayout();

		vkGetDescriptorSetLayoutBindingOffsetEXT(VkCtx::Instance().device, gBufferDescriptor.layout, 0, )

		//vkGetDescriptorSetLayoutSizeEXT(VkCtx::Instance().device, gBufferDescriptor.layout, sizeof(SceneData) + )
		
		GraphicsPipelineBuilder pipelineBuilder;

		_pipeline = pipelineBuilder
			.AddShaders(shaders)
			.AddDescriptorSetLayout(gBufferDescriptor.layout)
			.AddPushConstantRange(VkPushConstantRange
				{
					.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
					.offset = 0,
					.size = sizeof(mat4),
				})
			.SetRenderingFormats(colorFormats, VkCtx::Instance().depth->format)
			.SetBlendAttachments(attachments)
			.BuildPipeline(_pipelineLayout);


		//std::vector<VertexInputDescription> vertexInputDescriptions
		//{
		//	VertexInputDescription
		//	{
		//		.binding = 0,
		//		.location = 0,
		//		.stride = sizeof(float) * 2,
		//		.format = PipelineFormat::FLOAT2,
		//		.offset = 0,
		//	},
		//	VertexInputDescription
		//	{
		//		.binding = 1,
		//		.location = 1,
		//		.stride = sizeof(unsigned char) * 4,
		//		.format = PipelineFormat::COLOR,
		//		.offset = 0,
		//	},
		//};

		//std::vector<PipelineAttachment> pipelineAttachments
		//{
		//	PipelineAttachment{},
		//};

		//RenderingInfo renderInfo
		//{
		//	.colorAttachmentFormats
		//	{
		//		PipelineFormat::SWAPCHAIN,
		//	},
		//	.depthStencilFormat = pCtx.depthFormat
		//};

		//_pipeline = Attempt(pipelineBuilder.AddShaders(shaders)
		//	.AddVertexBindingDescriptions(vertexInputDescriptions)
		//	.AddDepthTest()
		//	.AddDepthWrite()
		//	.AddPipelineAttachments(pipelineAttachments)
		//	.SetRenderingInfo(renderInfo)
		//	.AddPushConstantRange(VkPushConstantRange{ .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, /* or VERTEX_BIT | FRAGMENT_BIT if both use it*/ .offset = 0, .size = sizeof(float) })
		//	.BuildPipeline(_pipelineLayout));
	}

	void Record(VkCommandBuffer pCommandBuffer) override;
};