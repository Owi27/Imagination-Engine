#include "D:/GitHub/Imagination-Engine/Refactor/Imagination - Current/build/CMakeFiles/Imagination.dir/Debug/cmake_pch.hxx"
#include "ImgnGui.h"

void ImgnGui::CreateResources()
{
	ImGuiIO& io = ImGui::GetIO();

	uint8_t* fontData;
	int texWidth, texHeight;
	//io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

	vk::DeviceSize uploadSize = texWidth * texHeight * 4 * sizeof(char);

	//font image
	//vk::ImageCreateInfo imageCreateInfo
	//{
	//	.imageType = vk::ImageType::e2D,
	//	.format = vk::Format::eR8G8B8A8Unorm,
	//	.extent = {static_cast<uint32_t>(texWidth) , static_cast<uint32_t>(texHeight), 1},
	//	.mipLevels = 1,
	//	.arrayLayers = 1,
	//	.samples = vk::SampleCountFlagBits::e1,
	//	.tiling = vk::ImageTiling::eOptimal,
	//	.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
	//	.sharingMode = vk::SharingMode::eExclusive
	//};

	//_font.image = vk::raii::Image(Device::Inst().GetDevice(), imageCreateInfo);

	//vk::MemoryRequirements memRequirements = _font.image.getMemoryRequirements();
	//vk::MemoryAllocateInfo allocInfo
	//{
	//	.allocationSize = memRequirements.size,
	//	.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
	//};

	//_font.memory = vk::raii::DeviceMemory(Device::Inst().GetDevice(), allocInfo);
	//_font.image.bindMemory(_font.memory, 0);

	//image view
	Device::Inst().CreateImage(texWidth, texHeight, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::MemoryPropertyFlagBits::eDeviceLocal, _font);
	Device::Inst().CreateImageView(_font, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor);

	//vk::ImageViewCreateInfo imageViewCreateInfo
	//{
	//	.image = _font.image,
	//	.viewType = vk::ImageViewType::e2D,
	//	.format = vk::Format::eR8G8B8A8Unorm,
	//	.components
	//	{
	//		.r = vk::ComponentSwizzle::eIdentity,
	//		.g = vk::ComponentSwizzle::eIdentity,
	//		.b = vk::ComponentSwizzle::eIdentity,
	//		.a = vk::ComponentSwizzle::eIdentity
	//	},
	//	.subresourceRange
	//	{
	//		.aspectMask = vk::ImageAspectFlagBits::eColor,
	//		.baseMipLevel = 0,
	//		.levelCount = 1,
	//		.baseArrayLayer = 0,
	//		.layerCount = 1
	//	}
	//};

	//_font.imageView = vk::raii::ImageView(Device::Inst().GetDevice(), imageViewCreateInfo);

	Buffer staging;
	Device::Inst().CreateBuffer(uploadSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging);

	void* data = staging.memory.mapMemory(0, uploadSize);
	memcpy(data, fontData, uploadSize);
	staging.memory.unmapMemory();

	Device::Inst().TransitionImageLayout(_font.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
	Device::Inst().CopyBufferToImage(staging.buffer, _font.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
	Device::Inst().TransitionImageLayout(_font.image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	vk::SamplerCreateInfo samplerInfo
	{
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eLinear,
		.addressModeU = vk::SamplerAddressMode::eClampToEdge,
		.addressModeV = vk::SamplerAddressMode::eClampToEdge,
		.addressModeW = vk::SamplerAddressMode::eClampToEdge,
		.borderColor = vk::BorderColor::eFloatOpaqueWhite
	};

	_sampler = Device::Inst().GetDevice().createSampler(samplerInfo);

	vk::DescriptorPoolSize poolSize
	{
		.type = vk::DescriptorType::eCombinedImageSampler,
		.descriptorCount = 1
	};

	vk::DescriptorPoolCreateInfo poolInfo
	{
		.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		.maxSets = 3,
		.poolSizeCount = 1,
		.pPoolSizes = &poolSize
	};

	_descriptorPool = Device::Inst().GetDevice().createDescriptorPool(poolInfo);

	vk::DescriptorSetLayoutBinding binding
	{
		.binding = 0,
		.descriptorType = vk::DescriptorType::eCombinedImageSampler,
		.descriptorCount = 1,
		.stageFlags = vk::ShaderStageFlagBits::eFragment,
	};

	vk::DescriptorSetLayoutCreateInfo layoutInfo
	{
		.bindingCount = 1,
		.pBindings = &binding
	};

	_descriptorSetLayout = Device::Inst().GetDevice().createDescriptorSetLayout(layoutInfo);

	vk::DescriptorSetAllocateInfo allocInfo
	{
		.descriptorPool = *_descriptorPool,
		.descriptorSetCount = 1,
		.pSetLayouts = &*_descriptorSetLayout
	};

	_descriptorSet = std::move(Device::Inst().GetDevice().allocateDescriptorSets(allocInfo).front());

	vk::DescriptorImageInfo imageInfo
	{
		.sampler = *_sampler,
		.imageView = _font.imageView,
		.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
	};

	vk::WriteDescriptorSet writeSet
	{
		.dstSet = *_descriptorSet,
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = vk::DescriptorType::eCombinedImageSampler,
		.pImageInfo = &imageInfo,
	};

	Device::Inst().GetDevice().updateDescriptorSets(writeSet, 0);

	vk::PipelineCacheCreateInfo pipelineCacheInfo;
	_pipelineCache = Device::Inst().GetDevice().createPipelineCache(pipelineCacheInfo);

	vk::PushConstantRange pushConstantRange
	{
		.stageFlags = vk::ShaderStageFlagBits::eVertex,
		.offset = 0,
		.size = sizeof(ImGuiPushConst)
	};

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo
	{
		.setLayoutCount = 1,
		.pSetLayouts = &*_descriptorSetLayout,
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &pushConstantRange
	};

	_pipelineLayout = Device::Inst().GetDevice().createPipelineLayout(pipelineLayoutInfo);

}

uint32_t ImgnGui::FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps)
{
	vk::PhysicalDeviceMemoryProperties memProperties = Device::Inst().GetPhysicalDevice().getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((pTypeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & pProps) == pProps)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

void ImgnGui::OnMouseButtonPressedEvent(MouseButtonPressedEvent& e)
{
}

void ImgnGui::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent& e)
{
}

void ImgnGui::OnMouseMovedEvent(MouseMovedEvent& e)
{
}

void ImgnGui::OnMouseScrolledEvent(MouseScrolledEvent& e)
{
}

void ImgnGui::OnKeyPressedEvent(KeyPressedEvent& e)
{
}

void ImgnGui::OnKeyReleasedEvent(KeyReleasedEvent& e)
{
}

void ImgnGui::OnKeyTypedEvent(KeyTypedEvent& e)
{
}

void ImgnGui::OnWindowResizeEvent(WindowResizedEvent& e)
{
}

void ImgnGui::Init(ImgnWindow& pWin, float pWidth, float pHeight)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange; // Optional
	ImGui::GetIO().MouseDrawCursor = true; // or false, depending on your needs
	io.DisplaySize = ImVec2(pWidth, pHeight);
	io.DisplayFramebufferScale = ImVec2(1.f, 1.f);
	ImGui::StyleColorsDark();

	/*  Make our own style later
	vulkanStyle = ImGui::GetStyle();
	vulkanStyle.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
	vulkanStyle.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	vulkanStyle.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	vulkanStyle.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	vulkanStyle.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);*/

	//CreateResources();

	ImGui_ImplWin32_Init(pWin.GetHandle());

	VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;

	ImGui_ImplVulkan_InitInfo initInfo
	{
		.ApiVersion = vk::ApiVersion14,
		.Instance = *Device::Inst().GetVkInstance(),
		.PhysicalDevice = *Device::Inst().GetPhysicalDevice(),
		.Device = *Device::Inst().GetDevice(),
		.QueueFamily = 0,
		.Queue = *Device::Inst().GetQueue(),
		//.DescriptorPool = *_descriptorPool,
		.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE,
		.MinImageCount = 3,
		.ImageCount = 3,
		.PipelineCache = *_pipelineCache,
		.PipelineInfoMain
		{
			.PipelineRenderingCreateInfo =
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &format,
			}
		},
		.UseDynamicRendering = true,
	};

	ImGui_ImplVulkan_Init(&initInfo);
	//ImGui_ImplVulkan_CreateFontsTexture();
}

void ImgnGui::Init(HWND pWin, float pWidth, float pHeight)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange; // Optional
	ImGui::GetIO().MouseDrawCursor = true; // or false, depending on your needs
	io.DisplaySize = ImVec2(pWidth, pHeight);
	io.DisplayFramebufferScale = ImVec2(1.f, 1.f);
	ImGui::StyleColorsDark();

	/*  Make our own style later
	vulkanStyle = ImGui::GetStyle();
	vulkanStyle.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
	vulkanStyle.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	vulkanStyle.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	vulkanStyle.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	vulkanStyle.Colors[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);*/

	//CreateResources();

	ImGui_ImplWin32_Init(pWin);

	VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;

	ImGui_ImplVulkan_InitInfo initInfo
	{
		.ApiVersion = vk::ApiVersion14,
		.Instance = *Device::Inst().GetVkInstance(),
		.PhysicalDevice = *Device::Inst().GetPhysicalDevice(),
		.Device = *Device::Inst().GetDevice(),
		.QueueFamily = 0,
		.Queue = *Device::Inst().GetQueue(),
		//.DescriptorPool = *_descriptorPool,
		.DescriptorPoolSize = IMGUI_IMPL_VULKAN_MINIMUM_IMAGE_SAMPLER_POOL_SIZE,
		.MinImageCount = 3,
		.ImageCount = 3,
		.PipelineCache = *_pipelineCache,
		.PipelineInfoMain
		{
			.PipelineRenderingCreateInfo =
			{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
				.colorAttachmentCount = 1,
				.pColorAttachmentFormats = &format,
			}
		},
		.UseDynamicRendering = true,
	};

	ImGui_ImplVulkan_Init(&initInfo);
	//ImGui_ImplVulkan_CreateFontsTexture();
}

void ImgnGui::DrawFrame(vk::raii::CommandBuffer& pCommandBuffer)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::SetWindowPos(ImVec2(20, 360), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_FirstUseEver);
	ImGui::ShowDemoWindow();

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *pCommandBuffer);
}

void ImgnGui::OnEvent(Event& event)
{
	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<MouseButtonPressedEvent>([&](MouseButtonPressedEvent& e)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.MouseDown[e.GetMouseButton()] = true;

			return false;
		});

	dispatcher.Dispatch<MouseButtonReleasedEvent>([&](MouseButtonReleasedEvent& e)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.MouseDown[e.GetMouseButton()] = false;


			return false;
		});

	dispatcher.Dispatch<MouseMovedEvent>([&](MouseMovedEvent& e)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.MousePos = ImVec2(e.GetX(), e.GetY());

			return false;
		});

	dispatcher.Dispatch<MouseScrolledEvent>([&](MouseScrolledEvent& e)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.MouseWheelH += e.GetXOffset();
			io.MouseWheel += e.GetYOffset();

			return false;
		});

	dispatcher.Dispatch<KeyPressedEvent>([&](KeyPressedEvent& e)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.KeysData[e.GetKeyCode()].Down = true;

			io.KeyCtrl = io.KeysData[G_KEY_LEFTCONTROL].Down || io.KeysData[G_KEY_RIGHTCONTROL].Down;
			io.KeyShift = io.KeysData[G_KEY_LEFTSHIFT].Down || io.KeysData[G_KEY_RIGHTSHIFT].Down;
			io.KeyAlt = io.KeysData[G_KEY_LEFTALT].Down || io.KeysData[G_KEY_RIGHTALT].Down;
			io.KeySuper = io.KeysData[G_KEY_COMMAND].Down;

			return false;
		});

	dispatcher.Dispatch<KeyReleasedEvent>([&](KeyReleasedEvent& e)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.KeysData[e.GetKeyCode()].Down = false;


			return false;
		});

	dispatcher.Dispatch<KeyTypedEvent>([&](KeyTypedEvent& e)
		{
			ImGuiIO& io = ImGui::GetIO();

			/*if (e > 0 && e < 0x10000)
			{

			}*/

			return false;
		});

	dispatcher.Dispatch<WindowResizedEvent>([&](WindowResizedEvent& e)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = ImVec2(e.GetWidth(), e.GetHeight());
			io.DisplayFramebufferScale = ImVec2(1.f, 1.f);

			return false;
		});
}
