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

void ImgnGui::AddImGuiSpecialKeyEvent(ImGuiIO& pIO, int pKeyCode, bool pPressed)
{
	switch (pKeyCode)
	{
	case IMGN_KEY_TAB:             pIO.AddKeyEvent(ImGuiKey_Tab, pPressed); break;
	case IMGN_KEY_LEFT:            pIO.AddKeyEvent(ImGuiKey_LeftArrow, pPressed); break;
	case IMGN_KEY_RIGHT:           pIO.AddKeyEvent(ImGuiKey_RightArrow, pPressed); break;
	case IMGN_KEY_UP:              pIO.AddKeyEvent(ImGuiKey_UpArrow, pPressed); break;
	case IMGN_KEY_DOWN:            pIO.AddKeyEvent(ImGuiKey_DownArrow, pPressed); break;
	case IMGN_KEY_PAGE_UP:         pIO.AddKeyEvent(ImGuiKey_PageUp, pPressed); break;
	case IMGN_KEY_PAGE_DOWN:       pIO.AddKeyEvent(ImGuiKey_PageDown, pPressed); break;
	case IMGN_KEY_HOME:            pIO.AddKeyEvent(ImGuiKey_Home, pPressed); break;
	case IMGN_KEY_END:             pIO.AddKeyEvent(ImGuiKey_End, pPressed); break;
	case IMGN_KEY_INSERT:          pIO.AddKeyEvent(ImGuiKey_Insert, pPressed); break;
	case IMGN_KEY_DELETE:          pIO.AddKeyEvent(ImGuiKey_Delete, pPressed); break;
	case IMGN_KEY_BACKSPACE:       pIO.AddKeyEvent(ImGuiKey_Backspace, pPressed); break;
	case IMGN_KEY_SPACE:           pIO.AddKeyEvent(ImGuiKey_Space, pPressed); break;
	case IMGN_KEY_ENTER:           pIO.AddKeyEvent(ImGuiKey_Enter, pPressed); break;
	case IMGN_KEY_ESCAPE:          pIO.AddKeyEvent(ImGuiKey_Escape, pPressed); break;

	case IMGN_KEY_LEFT_CONTROL:    pIO.AddKeyEvent(ImGuiKey_LeftCtrl, pPressed); break;
	case IMGN_KEY_LEFT_SHIFT:      pIO.AddKeyEvent(ImGuiKey_LeftShift, pPressed); break;
	case IMGN_KEY_LEFT_ALT:        pIO.AddKeyEvent(ImGuiKey_LeftAlt, pPressed); break;
	case IMGN_KEY_LEFT_SUPER:      pIO.AddKeyEvent(ImGuiKey_LeftSuper, pPressed); break;
	case IMGN_KEY_RIGHT_CONTROL:   pIO.AddKeyEvent(ImGuiKey_RightCtrl, pPressed); break;
	case IMGN_KEY_RIGHT_SHIFT:     pIO.AddKeyEvent(ImGuiKey_RightShift, pPressed); break;
	case IMGN_KEY_RIGHT_ALT:       pIO.AddKeyEvent(ImGuiKey_RightAlt, pPressed); break;
	case IMGN_KEY_RIGHT_SUPER:     pIO.AddKeyEvent(ImGuiKey_RightSuper, pPressed); break;
	case IMGN_KEY_MENU:            pIO.AddKeyEvent(ImGuiKey_Menu, pPressed); break;

	case IMGN_KEY_0:               pIO.AddKeyEvent(ImGuiKey_0, pPressed); break;
	case IMGN_KEY_1:               pIO.AddKeyEvent(ImGuiKey_1, pPressed); break;
	case IMGN_KEY_2:               pIO.AddKeyEvent(ImGuiKey_2, pPressed); break;
	case IMGN_KEY_3:               pIO.AddKeyEvent(ImGuiKey_3, pPressed); break;
	case IMGN_KEY_4:               pIO.AddKeyEvent(ImGuiKey_4, pPressed); break;
	case IMGN_KEY_5:               pIO.AddKeyEvent(ImGuiKey_5, pPressed); break;
	case IMGN_KEY_6:               pIO.AddKeyEvent(ImGuiKey_6, pPressed); break;
	case IMGN_KEY_7:               pIO.AddKeyEvent(ImGuiKey_7, pPressed); break;
	case IMGN_KEY_8:               pIO.AddKeyEvent(ImGuiKey_8, pPressed); break;
	case IMGN_KEY_9:               pIO.AddKeyEvent(ImGuiKey_9, pPressed); break;

	case IMGN_KEY_A:               pIO.AddKeyEvent(ImGuiKey_A, pPressed); break;
	case IMGN_KEY_B:               pIO.AddKeyEvent(ImGuiKey_B, pPressed); break;
	case IMGN_KEY_C:               pIO.AddKeyEvent(ImGuiKey_C, pPressed); break;
	case IMGN_KEY_D:               pIO.AddKeyEvent(ImGuiKey_D, pPressed); break;
	case IMGN_KEY_E:               pIO.AddKeyEvent(ImGuiKey_E, pPressed); break;
	case IMGN_KEY_F:               pIO.AddKeyEvent(ImGuiKey_F, pPressed); break;
	case IMGN_KEY_G:               pIO.AddKeyEvent(ImGuiKey_G, pPressed); break;
	case IMGN_KEY_H:               pIO.AddKeyEvent(ImGuiKey_H, pPressed); break;
	case IMGN_KEY_I:               pIO.AddKeyEvent(ImGuiKey_I, pPressed); break;
	case IMGN_KEY_J:               pIO.AddKeyEvent(ImGuiKey_J, pPressed); break;
	case IMGN_KEY_K:               pIO.AddKeyEvent(ImGuiKey_K, pPressed); break;
	case IMGN_KEY_L:               pIO.AddKeyEvent(ImGuiKey_L, pPressed); break;
	case IMGN_KEY_M:               pIO.AddKeyEvent(ImGuiKey_M, pPressed); break;
	case IMGN_KEY_N:               pIO.AddKeyEvent(ImGuiKey_N, pPressed); break;
	case IMGN_KEY_O:               pIO.AddKeyEvent(ImGuiKey_O, pPressed); break;
	case IMGN_KEY_P:               pIO.AddKeyEvent(ImGuiKey_P, pPressed); break;
	case IMGN_KEY_Q:               pIO.AddKeyEvent(ImGuiKey_Q, pPressed); break;
	case IMGN_KEY_R:               pIO.AddKeyEvent(ImGuiKey_R, pPressed); break;
	case IMGN_KEY_S:               pIO.AddKeyEvent(ImGuiKey_S, pPressed); break;
	case IMGN_KEY_T:               pIO.AddKeyEvent(ImGuiKey_T, pPressed); break;
	case IMGN_KEY_U:               pIO.AddKeyEvent(ImGuiKey_U, pPressed); break;
	case IMGN_KEY_V:               pIO.AddKeyEvent(ImGuiKey_V, pPressed); break;
	case IMGN_KEY_W:               pIO.AddKeyEvent(ImGuiKey_W, pPressed); break;
	case IMGN_KEY_X:               pIO.AddKeyEvent(ImGuiKey_X, pPressed); break;
	case IMGN_KEY_Y:               pIO.AddKeyEvent(ImGuiKey_Y, pPressed); break;
	case IMGN_KEY_Z:               pIO.AddKeyEvent(ImGuiKey_Z, pPressed); break;

	case IMGN_KEY_F1:              pIO.AddKeyEvent(ImGuiKey_F1, pPressed); break;
	case IMGN_KEY_F2:              pIO.AddKeyEvent(ImGuiKey_F2, pPressed); break;
	case IMGN_KEY_F3:              pIO.AddKeyEvent(ImGuiKey_F3, pPressed); break;
	case IMGN_KEY_F4:              pIO.AddKeyEvent(ImGuiKey_F4, pPressed); break;
	case IMGN_KEY_F5:              pIO.AddKeyEvent(ImGuiKey_F5, pPressed); break;
	case IMGN_KEY_F6:              pIO.AddKeyEvent(ImGuiKey_F6, pPressed); break;
	case IMGN_KEY_F7:              pIO.AddKeyEvent(ImGuiKey_F7, pPressed); break;
	case IMGN_KEY_F8:              pIO.AddKeyEvent(ImGuiKey_F8, pPressed); break;
	case IMGN_KEY_F9:              pIO.AddKeyEvent(ImGuiKey_F9, pPressed); break;
	case IMGN_KEY_F10:             pIO.AddKeyEvent(ImGuiKey_F10, pPressed); break;
	case IMGN_KEY_F11:             pIO.AddKeyEvent(ImGuiKey_F11, pPressed); break;
	case IMGN_KEY_F12:             pIO.AddKeyEvent(ImGuiKey_F12, pPressed); break;

	case IMGN_KEY_APOSTROPHE:      pIO.AddKeyEvent(ImGuiKey_Apostrophe, pPressed); break;
	case IMGN_KEY_COMMA:           pIO.AddKeyEvent(ImGuiKey_Comma, pPressed); break;
	case IMGN_KEY_MINUS:           pIO.AddKeyEvent(ImGuiKey_Minus, pPressed); break;
	case IMGN_KEY_PERIOD:          pIO.AddKeyEvent(ImGuiKey_Period, pPressed); break;
	case IMGN_KEY_SLASH:           pIO.AddKeyEvent(ImGuiKey_Slash, pPressed); break;
	case IMGN_KEY_SEMICOLON:       pIO.AddKeyEvent(ImGuiKey_Semicolon, pPressed); break;
	case IMGN_KEY_EQUAL:           pIO.AddKeyEvent(ImGuiKey_Equal, pPressed); break;
	case IMGN_KEY_LEFT_BRACKET:    pIO.AddKeyEvent(ImGuiKey_LeftBracket, pPressed); break;
	case IMGN_KEY_BACKSLASH:       pIO.AddKeyEvent(ImGuiKey_Backslash, pPressed); break;
	case IMGN_KEY_RIGHT_BRACKET:   pIO.AddKeyEvent(ImGuiKey_RightBracket, pPressed); break;
	case IMGN_KEY_GRAVE_ACCENT:    pIO.AddKeyEvent(ImGuiKey_GraveAccent, pPressed); break;

	case IMGN_KEY_CAPS_LOCK:       pIO.AddKeyEvent(ImGuiKey_CapsLock, pPressed); break;
	case IMGN_KEY_SCROLL_LOCK:     pIO.AddKeyEvent(ImGuiKey_ScrollLock, pPressed); break;
	case IMGN_KEY_NUM_LOCK:        pIO.AddKeyEvent(ImGuiKey_NumLock, pPressed); break;
	case IMGN_KEY_PRINT_SCREEN:    pIO.AddKeyEvent(ImGuiKey_PrintScreen, pPressed); break;
	case IMGN_KEY_PAUSE:           pIO.AddKeyEvent(ImGuiKey_Pause, pPressed); break;

	case IMGN_KEY_KP_0:            pIO.AddKeyEvent(ImGuiKey_Keypad0, pPressed); break;
	case IMGN_KEY_KP_1:            pIO.AddKeyEvent(ImGuiKey_Keypad1, pPressed); break;
	case IMGN_KEY_KP_2:            pIO.AddKeyEvent(ImGuiKey_Keypad2, pPressed); break;
	case IMGN_KEY_KP_3:            pIO.AddKeyEvent(ImGuiKey_Keypad3, pPressed); break;
	case IMGN_KEY_KP_4:            pIO.AddKeyEvent(ImGuiKey_Keypad4, pPressed); break;
	case IMGN_KEY_KP_5:            pIO.AddKeyEvent(ImGuiKey_Keypad5, pPressed); break;
	case IMGN_KEY_KP_6:            pIO.AddKeyEvent(ImGuiKey_Keypad6, pPressed); break;
	case IMGN_KEY_KP_7:            pIO.AddKeyEvent(ImGuiKey_Keypad7, pPressed); break;
	case IMGN_KEY_KP_8:            pIO.AddKeyEvent(ImGuiKey_Keypad8, pPressed); break;
	case IMGN_KEY_KP_9:            pIO.AddKeyEvent(ImGuiKey_Keypad9, pPressed); break;
	case IMGN_KEY_KP_DECIMAL:      pIO.AddKeyEvent(ImGuiKey_KeypadDecimal, pPressed); break;
	case IMGN_KEY_KP_DIVIDE:       pIO.AddKeyEvent(ImGuiKey_KeypadDivide, pPressed); break;
	case IMGN_KEY_KP_MULTIPLY:     pIO.AddKeyEvent(ImGuiKey_KeypadMultiply, pPressed); break;
	case IMGN_KEY_KP_SUBTRACT:     pIO.AddKeyEvent(ImGuiKey_KeypadSubtract, pPressed); break;
	case IMGN_KEY_KP_ADD:          pIO.AddKeyEvent(ImGuiKey_KeypadAdd, pPressed); break;
	case IMGN_KEY_KP_ENTER:        pIO.AddKeyEvent(ImGuiKey_KeypadEnter, pPressed); break;
	case IMGN_KEY_KP_EQUAL:        pIO.AddKeyEvent(ImGuiKey_KeypadEqual, pPressed); break;

	default:
		break;
	}
}

int ImgnGui::ImGui_ImplWin32_CreateVkSurface(ImGuiViewport* pViewport, ImU64 pVkInstance, const void* pVkAllocator, ImU64* pOutSurface)
{
	VkWin32SurfaceCreateInfoKHR createInfo
	{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hinstance = GetModuleHandle(nullptr),
		.hwnd = (HWND)pViewport->PlatformHandleRaw
	};

	return (int)vkCreateWin32SurfaceKHR((VkInstance)pVkInstance, &createInfo, (const VkAllocationCallbacks*)pVkAllocator, (VkSurfaceKHR*)pOutSurface);
}

void ImgnGui::Init(ImgnWindow& pWin, float pWidth, float pHeight)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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
				.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT
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
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
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
	ImGui::GetPlatformIO().Platform_CreateVkSurface = ImGui_ImplWin32_CreateVkSurface;

	VkFormat format = VK_FORMAT_B8G8R8A8_SRGB;

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

	dispatcher.Dispatch<MouseScrolledEvent>([&](MouseScrolledEvent& e)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.MouseWheelH += e.GetXOffset();
			io.MouseWheel += e.GetYOffset();

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
