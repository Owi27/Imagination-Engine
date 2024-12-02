#include "pch.h"
#include "Renderer.h"

Renderer::Renderer(GWindow win)
{
	_win = win;
	_win.GetClientWidth(_width);
	_win.GetClientHeight(_height);
}

Renderer::~Renderer()
{
}

void VulkanRenderer::CreateOffscreenFrameBuffer()
{
	auto CreateAttachment = [this](VkFormat format, VkImageUsageFlagBits usage, FrameBufferTexture* frameBufferTexture)
		{
			VkImageAspectFlags aspectMask = 0;
			VkImageLayout imageLayout;

			frameBufferTexture->format = format;

			if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			{
				aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}

			if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

				if (format >= VK_FORMAT_D16_UNORM_S8_UINT) aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

				imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}

			assert(aspectMask > 0);

			GvkHelper::create_image(_physicalDevice, _device, { _width, _height, 1 }, 1, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, usage | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr, &frameBufferTexture->texture.texImage.image, &frameBufferTexture->texture.texImage.memory);
			GvkHelper::create_image_view(_device, frameBufferTexture->texture.texImage.image, format, aspectMask, 1, nullptr, &frameBufferTexture->texture.texImageView);
		};

	CreateAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &_offscreenFrameBuffer.position);
	CreateAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &_offscreenFrameBuffer.normal);
	CreateAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &_offscreenFrameBuffer.albedo);

	std::vector<VkFormat> formats =
	{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	VkFormat depthFormat;
	GvkHelper::find_depth_format(_physicalDevice, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, formats.data(), &depthFormat);
	CreateAttachment(depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &_offscreenFrameBuffer.depth);

	std::array<VkAttachmentDescription, 4> attachmentDescription;

	for (size_t i = 0; i < 4; i++)
	{
		attachmentDescription[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescription[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		if (i == 3)
		{
			attachmentDescription[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescription[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			attachmentDescription[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescription[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}

	//formats
	attachmentDescription[0].format = _offscreenFrameBuffer.position.format;
	attachmentDescription[1].format = _offscreenFrameBuffer.normal.format;
	attachmentDescription[2].format = _offscreenFrameBuffer.albedo.format;
	attachmentDescription[3].format = _offscreenFrameBuffer.depth.format;

	std::vector<VkAttachmentReference> colorAttachmentReference;
	colorAttachmentReference.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorAttachmentReference.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorAttachmentReference.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	VkAttachmentReference depthAttachmentReference;
	depthAttachmentReference.attachment = 3;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.pColorAttachments = colorAttachmentReference.data();
	subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReference.size());
	subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;

	// Use subpass dependencies for attachment layout transitions
	std::array<VkSubpassDependency, 2> subpassDependencies;

	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassCreateInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassCreateInfo.pAttachments = attachmentDescription.data();
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachmentDescription.size());
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = 2;
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	vkCreateRenderPass(_device, &renderPassCreateInfo, nullptr, &_offscreenFrameBuffer.renderPass);

	std::array<VkImageView, 4> imageViews;
	imageViews[0] = _offscreenFrameBuffer.position.texture.texImageView;
	imageViews[1] = _offscreenFrameBuffer.normal.texture.texImageView;
	imageViews[2] = _offscreenFrameBuffer.albedo.texture.texImageView;
	imageViews[3] = _offscreenFrameBuffer.depth.texture.texImageView;

	VkFramebufferCreateInfo frameBufferCreateInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	frameBufferCreateInfo.renderPass = _offscreenFrameBuffer.renderPass;
	frameBufferCreateInfo.pAttachments = imageViews.data();
	frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
	frameBufferCreateInfo.width = _width;
	frameBufferCreateInfo.height = _height;
	frameBufferCreateInfo.layers = 1;

	vkCreateFramebuffer(_device, &frameBufferCreateInfo, nullptr, &_offscreenFrameBuffer.frameBuffer);

	//create sampler
	VkSamplerCreateInfo samplerCreateInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.maxAnisotropy = 1.f;
	samplerCreateInfo.maxLod = 1.f;
	samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.minLod = 0;
	samplerCreateInfo.mipLodBias = 0;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	vkCreateSampler(_device, &samplerCreateInfo, nullptr, &_colorSampler);
}

void VulkanRenderer::CreateUniformBuffers()
{
	_vlk.GetAspectRatio(_aspect);

	_offscreenData.world = GW::MATH::GIdentityMatrixF;
	GMatrix::LookAtLHF(vec4{ 0.f, 3.f, -1.5f }, vec4{ 0.f, 0.f, 0.f }, vec4{ 0, 1, 0 }, _offscreenData.view);
	GMatrix::ProjectionVulkanLHF(G_DEGREE_TO_RADIAN(65), _aspect, .1f, 100.f, _offscreenData.proj);

	GvkHelper::create_buffer(_physicalDevice, _device, sizeof(UniformBufferOffscreen), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &_uniformBuffer.buffer, &_uniformBuffer.memory);
	GvkHelper::write_to_buffer(_device, _uniformBuffer.memory, &_offscreenData, sizeof(UniformBufferOffscreen));
}

VulkanRenderer::VulkanRenderer(GWindow win) : Renderer(win)
{
	if (-_vlk.Create(_win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT)) return; //return if creation didn't work

	_vlk.GetDevice((void**)&_device);
	_vlk.GetPhysicalDevice((void**)&_physicalDevice);

	CreateOffscreenFrameBuffer();
	CreateUniformBuffers();
}

VulkanRenderer::~VulkanRenderer()
{
}

void VulkanRenderer::Render()
{
}

DX12Renderer::DX12Renderer(GWindow win) : Renderer(win)
{
	if (-_dxs.Create(_win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT)) return; //return if creation didn't work
}

DX12Renderer::~DX12Renderer()
{
}

void DX12Renderer::Render()
{
}
