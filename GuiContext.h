#pragma once
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_vulkan.h"

class Buffer;
class Texture;
//class VulkanContext;
class GuiContext
{
	struct ImGuiPushConst
	{
		vec2 scale, translate;
	} _imGuiPushConst;

	VulkanContext& _vk;
	VkCommandBuffer _commandBuffer;
	VkSemaphore _semaphore;
	VkDescriptorPool _descriptorPool;
	VkDescriptorSetLayout _descriptorSetLayout;
	VkDescriptorSet _descriptorSet;
	std::unique_ptr<Texture> _fontTexture;
	VkSampler _sampler;
	VkPipelineLayout _pipelineLayout;
	VkPipeline _pipeline;
	std::array<std::shared_ptr<Shader>, 2> _shaders;
	std::array<std::unique_ptr<Buffer>, 2> _buffers;
	unsigned vertexCount = 0, indexCount = 0;

public:
	std::vector<std::function<void()>> imGuiCalls;

	GuiContext() : _vk(*VulkanContext::GetInst())
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		vkCreateSemaphore(_vk.GetDevice(), &semaphoreCreateInfo, nullptr, &_semaphore);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange; // Optional
		ImGui::GetIO().MouseDrawCursor = true; // or false, depending on your needs
		io.DisplaySize = ImVec2(_vk.GetWidth(), _vk.GetHeight());
		io.DisplayFramebufferScale = ImVec2(1.f, 1.f);
		ImGui::StyleColorsDark();

		unsigned char* fontData;
		int texWidth, texHeight;
		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);
		//VkDeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

		_fontTexture = std::make_unique<Texture>(fontData, texWidth, texHeight);

		VkSamplerCreateInfo samplerCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
			.magFilter = VK_FILTER_LINEAR,
			.minFilter = VK_FILTER_LINEAR,
			.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
			.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
			.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE
		};

		vkCreateSampler(_vk.GetDevice(), &samplerCreateInfo, nullptr, &_sampler);

		//descriptor pool
		VkDescriptorPoolSize descriptorPoolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};
		//{
		//	.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		//	.descriptorCount = 1
		//};

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo
		{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
			.maxSets = 1000,
			.poolSizeCount = std::size(descriptorPoolSizes),
			.pPoolSizes = descriptorPoolSizes,
		};

		vkCreateDescriptorPool(_vk.GetDevice(), &descriptorPoolCreateInfo, nullptr, &_descriptorPool);

		////descriptor set layout
		//VkDescriptorSetLayoutBinding descriptorSetLayoutBinding
		//{
		//	.binding = 0,
		//	.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		//	.descriptorCount = 1,
		//	.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
		//};

		//VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo
		//{
		//	.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		//	.bindingCount = 1,
		//	.pBindings = &descriptorSetLayoutBinding
		//};

		//vkCreateDescriptorSetLayout(_vk.GetDevice(), &descriptorSetLayoutCreateInfo, nullptr, &_descriptorSetLayout);

		////descriptor set
		//VkDescriptorSetAllocateInfo descriptorSetAllocateInfo
		//{
		//	.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		//	.descriptorPool = _descriptorPool,
		//	.descriptorSetCount = 1,
		//	.pSetLayouts = &_descriptorSetLayout
		//};

		//vkAllocateDescriptorSets(_vk.GetDevice(), &descriptorSetAllocateInfo, &_descriptorSet);

		////write to descriptor
		//VkDescriptorImageInfo descriptorImageInfo
		//{
		//	.sampler = _sampler,
		//	.imageView = _fontTexture->GetImageView(),
		//	.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		//};

		//VkWriteDescriptorSet writeDescriptorSet
		//{
		//	.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		//	.dstSet = _descriptorSet,
		//	.dstBinding = 0,
		//	.descriptorCount = 1,
		//	.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		//	.pImageInfo = &descriptorImageInfo,
		//};

		//vkUpdateDescriptorSets(_vk.GetDevice(), 1, &writeDescriptorSet, 0, nullptr);

		//VkPushConstantRange pushConstantRange
		//{
		//	.stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
		//	.offset = 0,
		//	.size = sizeof(ImGuiPushConst),
		//};

		//_shaders[0] = std::make_shared<Shader>("UIFragmentShader", FRAGMENT_SHADER);
		//_shaders[1] = std::make_shared<Shader>("UIVertexShader", VERTEX_SHADER);

		//PipelineDescription pipelineDescription
		//{
		//	.vertexInput = POSITION | TEXCOORD | COLOR,
		//	.vertexShader = _shaders[1].get(),
		//	.fragmentShader = _shaders[0].get(),
		//	.pipelineLayoutCreateInfo
		//	{
		//		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		//		.setLayoutCount = 1,
		//		.pSetLayouts = &_descriptorSetLayout,
		//		.pushConstantRangeCount = 1,
		//		.pPushConstantRanges = &pushConstantRange,
		//	}
		//};
		//pipelineDescription.offsets[0] = offsetof(ImDrawVert, pos);
		//pipelineDescription.offsets[1] = offsetof(ImDrawVert, uv);
		//pipelineDescription.offsets[2] = offsetof(ImDrawVert, col);
		//pipelineDescription.colorAttachmentFormats.push_back(VK_FORMAT_R8G8B8A8_UNORM);

		//_pipeline = _vk.CreateGuiGraphicsPipeline(pipelineDescription, _pipelineLayout);

		VkFormat formats[] = { VK_FORMAT_R8G8B8A8_UNORM };
		ImGui_ImplWin32_Init(_vk.GetWindowHandle());
		ImGui_ImplVulkan_InitInfo initInfo
		{
			.Instance = _vk.GetInstance(),
			.PhysicalDevice = _vk.GetPhysicalDevice(),
			.Device = _vk.GetDevice(),
			.Queue = _vk.GetGraphicsQueue(),
			.DescriptorPool = _descriptorPool,
			.MinImageCount = _vk.GetMaxFrames(),
			.ImageCount = _vk.GetMaxFrames(),
			.UseDynamicRendering = true,
			.PipelineRenderingCreateInfo =
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = formats,
			}
		};

		ImGui_ImplVulkan_Init(&initInfo);

		//GvkHelper::signal_command_start(_vk.GetDevice(), _vk.GetCommandPool(), &_commandBuffer);

		ImGui_ImplVulkan_CreateFontsTexture();
		//GvkHelper::signal_command_end(_vk.GetDevice(), _vk.GetGraphicsQueue(), _vk.GetCommandPool(), &_commandBuffer);
	}

	~GuiContext()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void BuildCommandBuffer(VkCommandBuffer& commandBuffer);

	//VkCommandBuffer& GetCommandBuffer() { return _commandBuffer; }
	//VkSemaphore& GetSemaphore() { return _semaphore; }
	void ReadInputs(GInput& input);

	//void DrawFrame();
	void Render();
};

