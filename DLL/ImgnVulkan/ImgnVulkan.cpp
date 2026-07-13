#include "pch.hpp"
#include "CTX.h"
#include "ImgnVulkan.hpp"
#include "Shaders.h"

namespace ImgnVulkan
{
	static VKAPI_ATTR vk::Bool32 VKAPI_CALL DebugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void*)
	{
		if (severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eError || severity == vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
		{
			std::cout << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;
		}

		return vk::False;
	}

	bool Ctx::IsDeviceSuitable(vk::raii::PhysicalDevice const& pPhysicalDevice)
	{
		return false;
	}

	void Ctx::CleanupSwapchain()
	{
		ImgnVulkan::swapchainImages.clear();
		ImgnVulkan::swapchain = nullptr;
	}

	void Ctx::RecreateSwapchain()
	{
		ImgnVulkan::device.waitIdle();

		CleanupSwapchain();
		CreateSwapchain();
		CreateImageViews();
		CreateDepthResources();
	}

	void Ctx::SetupDebugMessenger()
	{
		if (!_enableValidationLayers) return;

		vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);
		vk::DebugUtilsMessageTypeFlagsEXT     messageTypeFlags(
			vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);
		vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT
		{
			.messageSeverity = severityFlags,
			.messageType = messageTypeFlags,
			.pfnUserCallback = &DebugCallback
		};

		ImgnVulkan::debugMessenger = ImgnVulkan::instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
	}

	vk::Format Ctx::FindDepthFormat()
	{
		return FindSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint }, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
	}

	void Ctx::PickPhysicalDevice()
	{
		auto physicalDevices = vk::raii::PhysicalDevices(ImgnVulkan::instance);
		if (physicalDevices.empty()) throw std::runtime_error("failed to find GPUs with Vulkan support!");

		// Use an ordered map to automatically sort candidates by increasing score
		std::multimap<int, vk::raii::PhysicalDevice> candidates;

		for (const auto& physicalDevice : physicalDevices)
		{
			auto deviceProperties = physicalDevice.getProperties();
			auto deviceFeatures = physicalDevice.getFeatures();
			uint32_t score = 0;

			// Discrete GPUs have a significant performance advantage
			if (deviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) score += 1000;

			// Maximum possible size of textures affects graphics quality
			score += deviceProperties.limits.maxImageDimension2D;

			// Application can't function without geometry shaders
			if (!deviceFeatures.geometryShader) continue;

			candidates.insert(std::make_pair(score, physicalDevice));
		}

		// Check if the best candidate is suitable at all
		if (!candidates.empty() && candidates.rbegin()->first > 0) ImgnVulkan::physicalDevice = candidates.rbegin()->second;
		else throw std::runtime_error("failed to find a suitable GPU!");
	}

	void Ctx::RecordCommandBuffer(uint32_t pImageIdx)
	{
		auto& commandBuffer = ImgnVulkan::commandBuffers[ImgnVulkan::frameIdx];
		commandBuffer.begin({});

		TransitionImageLayout(ImgnVulkan::swapchainImages[pImageIdx], vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal, {}, vk::AccessFlagBits2::eColorAttachmentWrite, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::ImageAspectFlagBits::eColor);
		TransitionImageLayout(_graph.GetImageResource("Depth")->image.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthAttachmentOptimal, vk::AccessFlagBits2::eDepthStencilAttachmentWrite, vk::AccessFlagBits2::eDepthStencilAttachmentWrite, vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests, vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests, vk::ImageAspectFlagBits::eDepth);

		vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
		vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.f, 0);

		vk::RenderingAttachmentInfo attachmentInfo
		{
			.imageView = ImgnVulkan::swapchainImageViews[pImageIdx],
			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eStore,
			.clearValue = clearColor
		};

		vk::RenderingAttachmentInfo depthAttachmentInfo
		{
			.imageView = _graph.GetImageResource("Depth")->image.view,
			.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
			.loadOp = vk::AttachmentLoadOp::eClear,
			.storeOp = vk::AttachmentStoreOp::eDontCare,
			.clearValue = clearDepth
		};

		vk::RenderingInfo renderingInfo
		{
			.renderArea
			{
				.offset = { 0, 0 },
				.extent = ImgnVulkan::swapchainExtent
			},
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &attachmentInfo,
			.pDepthAttachment = &depthAttachmentInfo
		};

		commandBuffer.beginRendering(renderingInfo);

		////commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
		commandBuffer.setViewport(0, vk::Viewport(0.0f, 0, static_cast<float>(ImgnVulkan::swapchainExtent.width), static_cast<float>(ImgnVulkan::swapchainExtent.height), 0.0f, 1.0f));
		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), ImgnVulkan::swapchainExtent));

		////commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelineLayout, 0, *ImgnVulkan::descriptorSets[ImgnVulkan::frameIdx], nullptr);
		//commandBuffer.bindVertexBuffers(0, *_vertexBuffer.buffer, { 0 });
		//commandBuffer.bindIndexBuffer(*_indexBuffer.buffer, 0, vk::IndexType::eUint16);

		//commandBuffer.drawIndexed(indices.size(), 1, 0, 0, 0);

		commandBuffer.endRendering();

		vk::RenderingInfo guiRenderingInfo
		{
			.renderArea
			{
				.offset = { 0, 0 },
				.extent = ImgnVulkan::swapchainExtent
			},
			.layerCount = 1,
			.colorAttachmentCount = 1,
			.pColorAttachments = &attachmentInfo,
		};

		commandBuffer.beginRendering(guiRenderingInfo);
		//_gui.DrawFrame(commandBuffer);
		commandBuffer.endRendering();

		TransitionImageLayout(ImgnVulkan::swapchainImages[pImageIdx], vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR, vk::AccessFlagBits2::eColorAttachmentWrite, {}, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe, vk::ImageAspectFlagBits::eColor);

		commandBuffer.end();
	}

	bool Ctx::HasStencilComponent(vk::Format pFormat)
	{
		return pFormat == vk::Format::eD32SfloatS8Uint || pFormat == vk::Format::eD24UnormS8Uint;
	}

	uint32_t Ctx::FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps)
	{
		vk::PhysicalDeviceMemoryProperties memProperties = ImgnVulkan::physicalDevice.getMemoryProperties();

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((pTypeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & pProps) == pProps)
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	vk::Extent2D Ctx::ChooseSwapExtent(vk::SurfaceCapabilitiesKHR const& pCapabilities)
	{
		if (pCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return pCapabilities.currentExtent;

		return
		{
			std::clamp<uint32_t>(_width, pCapabilities.minImageExtent.width, pCapabilities.maxImageExtent.width),
			std::clamp<uint32_t>(_height, pCapabilities.minImageExtent.height, pCapabilities.maxImageExtent.height)
		};
	}

	uint32_t Ctx::ChooseSwapMinImageCount(vk::SurfaceCapabilitiesKHR const& pCapabilities)
	{
		auto minImageCount = std::max(3u, pCapabilities.minImageCount);
		if ((0 < pCapabilities.maxImageCount) && (pCapabilities.maxImageCount < minImageCount)) minImageCount = pCapabilities.maxImageCount;

		return minImageCount;
	}

	vk::SurfaceFormatKHR Ctx::ChooseSwapSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& pAvailablePresentModes)
	{
		const auto formatIter = std::ranges::find_if(pAvailablePresentModes, [](const auto& format)
			{
				return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
			});

		return formatIter != pAvailablePresentModes.end() ? *formatIter : pAvailablePresentModes[0];
	}

	std::vector<uint32_t> Ctx::GetSPV(const std::string& pShader, const std::wstring& pTarget, const std::wstring& pEntryPoint)
	{
		std::vector<uint32_t> spv;

		DxcBuffer sourceBuffer;
		sourceBuffer.Ptr = pShader.c_str();
		sourceBuffer.Size = pShader.size();
		sourceBuffer.Encoding = DXC_CP_ACP;

		std::vector<LPCWSTR> args
		{
			L"-spirv",
			L"-T",
			pTarget.c_str(),
			L"-E",
			pEntryPoint.c_str(),
	#ifndef NDEBUG
			L"-Zi",
			L"-Qembed_debug"
	#endif // NDEBUG
		};

		ComPtr<IDxcResult> result;
		ImgnVulkan::compiler->Compile(&sourceBuffer, args.data(), static_cast<uint32_t>(args.size()), ImgnVulkan::includeHandler.Get(), IID_PPV_ARGS(&result));

		//check for compilation errors
		ComPtr<IDxcBlobUtf8> errors;
		if (SUCCEEDED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr)) && errors && errors->GetStringLength() > 0)
		{
			std::stringstream ss;
			ss << "Shader compilation errors : " << errors->GetStringPointer();
			std::cout << ss.str() << '\n';
			throw std::runtime_error(ss.str());
		}

		//write compilation to spv
		ComPtr<IDxcBlob> shaderBlob;
		if (SUCCEEDED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr)))
		{
			const uint64_t byteCount = shaderBlob->GetBufferSize();

			spv.resize(byteCount * 0.25f);
			std::memcpy(spv.data(), shaderBlob->GetBufferPointer(), byteCount);
		};

		return spv;
	}

	vk::Format Ctx::FindSupportedFormat(const std::vector<vk::Format>& pCandidates, vk::ImageTiling pTiling, vk::FormatFeatureFlags pFeatures)
	{
		for (const auto format : pCandidates)
		{
			vk::FormatProperties props = ImgnVulkan::physicalDevice.getFormatProperties(format);

			if (pTiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & pFeatures) == pFeatures)
			{
				return format;
			}
			if (pTiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & pFeatures) == pFeatures)
			{
				return format;
			}
		}

		throw std::runtime_error("failed to find supported format!");
	}

	void Ctx::TransitionImageLayout(const vk::raii::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout)
	{
		vk::raii::CommandBuffer commandBuffer = BeginSingleCommand();

		vk::ImageMemoryBarrier barrier
		{
			.oldLayout = pOldLayout,
			.newLayout = pNewLayout,
			.image = pImage,
			.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
		};

		vk::PipelineStageFlags sourceStage;
		vk::PipelineStageFlags destinationStage;

		if (pOldLayout == vk::ImageLayout::eUndefined && pNewLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (pOldLayout == vk::ImageLayout::eTransferDstOptimal && pNewLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
		}

		commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);
		EndSingleCommand(commandBuffer);
	}

	void Ctx::TransitionImageLayout(const vk::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout)
	{
		vk::raii::CommandBuffer commandBuffer = BeginSingleCommand();

		vk::ImageMemoryBarrier barrier
		{
			.oldLayout = pOldLayout,
			.newLayout = pNewLayout,
			.image = pImage,
			.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
		};

		vk::PipelineStageFlags sourceStage;
		vk::PipelineStageFlags destinationStage;

		if (pOldLayout == vk::ImageLayout::eUndefined && pNewLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (pOldLayout == vk::ImageLayout::eTransferDstOptimal && pNewLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		// 3. Color Attachment -> Transfer Source (Preparing your Lighting-Output for the Blit)
		else if (pOldLayout == vk::ImageLayout::eColorAttachmentOptimal && pNewLayout == vk::ImageLayout::eTransferSrcOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			sourceStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		// 4. Undefined/Present -> Color Attachment (Preparing Swapchain for rendering)
		else if ((pOldLayout == vk::ImageLayout::eUndefined || pOldLayout == vk::ImageLayout::ePresentSrcKHR) &&
			pNewLayout == vk::ImageLayout::eColorAttachmentOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eNone;
			barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		}
		// 5. Color Attachment -> Present (Sending Swapchain to monitor)
		else if (pOldLayout == vk::ImageLayout::eColorAttachmentOptimal && pNewLayout == vk::ImageLayout::ePresentSrcKHR)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eNone;
			sourceStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe;
		}
		// 6. Transfer Dest -> Present (After Blit to Swapchain)
		else if (pOldLayout == vk::ImageLayout::eTransferDstOptimal && pNewLayout == vk::ImageLayout::ePresentSrcKHR)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eNone;
			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe;
		}
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
		}

		commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);
		EndSingleCommand(commandBuffer);
	}

	void Ctx::TransitionImageLayout(vk::raii::CommandBuffer& pCommandBuffer, const vk::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout)
	{
		vk::ImageMemoryBarrier barrier
		{
			.oldLayout = pOldLayout,
			.newLayout = pNewLayout,
			.image = pImage,
			.subresourceRange = { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
		};

		vk::PipelineStageFlags sourceStage;
		vk::PipelineStageFlags destinationStage;

		if (pOldLayout == vk::ImageLayout::eUndefined && pNewLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (pOldLayout == vk::ImageLayout::eTransferDstOptimal && pNewLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		// 3. Color Attachment -> Transfer Source (Preparing your Lighting-Output for the Blit)
		else if (pOldLayout == vk::ImageLayout::eColorAttachmentOptimal && pNewLayout == vk::ImageLayout::eTransferSrcOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
			sourceStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		// 4. Undefined/Present -> Color Attachment (Preparing Swapchain for rendering)
		else if ((pOldLayout == vk::ImageLayout::eUndefined || pOldLayout == vk::ImageLayout::ePresentSrcKHR) &&
			pNewLayout == vk::ImageLayout::eColorAttachmentOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eNone;
			barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		}
		// 5. Color Attachment -> Present (Sending Swapchain to monitor)
		else if (pOldLayout == vk::ImageLayout::eColorAttachmentOptimal && pNewLayout == vk::ImageLayout::ePresentSrcKHR)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eNone;
			sourceStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
			destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe;
		}
		// 6. Transfer Dest -> Present (After Blit to Swapchain)
		else if (pOldLayout == vk::ImageLayout::eTransferDstOptimal && pNewLayout == vk::ImageLayout::ePresentSrcKHR)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eNone;
			sourceStage = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe;
		}
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
		}

		pCommandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);
	}

	void Ctx::TransitionImageLayout(const vk::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::AccessFlags2 pSrcAccessMask, vk::AccessFlags2 pDstAccessMask, vk::PipelineStageFlags2 pSrcStageMask, vk::PipelineStageFlags2 pDstStageMask, vk::ImageAspectFlags pImageAspectFlags)
	{
		vk::ImageMemoryBarrier2 barrier
		{
			.srcStageMask = pSrcStageMask,
			.srcAccessMask = pSrcAccessMask,
			.dstStageMask = pDstStageMask,
			.dstAccessMask = pDstAccessMask,
			.oldLayout = pOldLayout,
			.newLayout = pNewLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = pImage,
			.subresourceRange
			{
				.aspectMask = pImageAspectFlags,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		vk::DependencyInfo dependencyInfo
		{
			.dependencyFlags = {},
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier
		};

		ImgnVulkan::commandBuffers[ImgnVulkan::frameIdx].pipelineBarrier2(dependencyInfo);
	}

	void Ctx::TransitionImageLayout(const vk::raii::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::AccessFlags2 pSrcAccessMask, vk::AccessFlags2 pDstAccessMask, vk::PipelineStageFlags2 pSrcStageMask, vk::PipelineStageFlags2 pDstStageMask, vk::ImageAspectFlags pImageAspectFlags)
	{
		vk::ImageMemoryBarrier2 barrier
		{
			.srcStageMask = pSrcStageMask,
			.srcAccessMask = pSrcAccessMask,
			.dstStageMask = pDstStageMask,
			.dstAccessMask = pDstAccessMask,
			.oldLayout = pOldLayout,
			.newLayout = pNewLayout,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = pImage,
			.subresourceRange
			{
				.aspectMask = pImageAspectFlags,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		vk::DependencyInfo dependencyInfo
		{
			.dependencyFlags = {},
			.imageMemoryBarrierCount = 1,
			.pImageMemoryBarriers = &barrier
		};

		ImgnVulkan::commandBuffers[ImgnVulkan::frameIdx].pipelineBarrier2(dependencyInfo);
	}

	vk::raii::ShaderModule Ctx::CreateShaderModule(const std::vector<uint32_t>& pCode) const
	{
		vk::ShaderModuleCreateInfo createInfo
		{
			.codeSize = pCode.size() * sizeof(uint32_t),
			.pCode = pCode.data()
		};

		vk::raii::ShaderModule shaderModule(ImgnVulkan::device, createInfo);

		return shaderModule;
	}

	vk::raii::CommandBuffer Ctx::BeginSingleCommand()
	{
		vk::CommandBufferAllocateInfo allocInfo
		{
			.commandPool = ImgnVulkan::commandPool,
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = 1
		};

		vk::raii::CommandBuffer commandBuffer = std::move(ImgnVulkan::device.allocateCommandBuffers(allocInfo).front());

		vk::CommandBufferBeginInfo beginInfo
		{
			.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
		};

		commandBuffer.begin(beginInfo);

		return commandBuffer;
	}

	void Ctx::EndSingleCommand(vk::raii::CommandBuffer& pCommandBuffer)
	{
		pCommandBuffer.end();

		vk::SubmitInfo submitInfo
		{
			.commandBufferCount = 1,
			.pCommandBuffers = &*pCommandBuffer
		};

		ImgnVulkan::queue.submit(submitInfo, nullptr);
		ImgnVulkan::queue.waitIdle();
	}

	vk::PresentModeKHR Ctx::ChooseSwapPresentMode(std::vector<vk::PresentModeKHR> const& pAvailablePresentModes)
	{
		assert(std::ranges::any_of(pAvailablePresentModes, [](auto presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
		return std::ranges::any_of(pAvailablePresentModes,
			[](const vk::PresentModeKHR value)
			{
				return vk::PresentModeKHR::eMailbox == value;
			}) ? vk::PresentModeKHR::eMailbox : vk::PresentModeKHR::eFifo;
	}

	void Ctx::CreateDevice()
	{
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = ImgnVulkan::physicalDevice.getQueueFamilyProperties();

		for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
		{
			if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) && ImgnVulkan::physicalDevice.getSurfaceSupportKHR(qfpIndex, *ImgnVulkan::surface))
			{
				// found a queue family that supports both graphics and present
				ImgnVulkan::queueIdx = qfpIndex;
				break;
			}
		}

		if (ImgnVulkan::queueIdx == ~0) throw std::runtime_error("Could not find a queue for graphics and present -> terminating");

		vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceDescriptorIndexingFeatures> featureChain =
		{
			{.features = {.samplerAnisotropy = true} },                               // vk::PhysicalDeviceFeatures2 (empty for now)
			{.synchronization2 = true, .dynamicRendering = true },      // Enable dynamic rendering from Vulkan 1.3
			{.extendedDynamicState = true },   // Enable extended dynamic state from the extension
			{.shaderSampledImageArrayNonUniformIndexing = true, .descriptorBindingSampledImageUpdateAfterBind = true, .descriptorBindingUpdateUnusedWhilePending = true, .descriptorBindingPartiallyBound = true, .descriptorBindingVariableDescriptorCount = true, .runtimeDescriptorArray = true}
		};

		float queuePriority = 0.5f;
		vk::DeviceQueueCreateInfo deviceQueueCreateInfo
		{
			.queueFamilyIndex = ImgnVulkan::queueIdx,
			.queueCount = 1, .pQueuePriorities =
			&queuePriority
		};

		vk::DeviceCreateInfo deviceCreateInfo
		{
			.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &deviceQueueCreateInfo,
			.enabledExtensionCount = static_cast<uint32_t>(_deviceExtensions.size()),
			.ppEnabledExtensionNames = _deviceExtensions.data()
		};

		ImgnVulkan::device = vk::raii::Device(ImgnVulkan::physicalDevice, deviceCreateInfo);
		ImgnVulkan::queue = vk::raii::Queue(ImgnVulkan::device, ImgnVulkan::queueIdx, 0);
	}

	void Ctx::CreateSurface(HWND pHWND)
	{
		VkSurfaceKHR surface;

		//vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo
		//{
		//	.hinstance = _win->GetInstance(),
		//	.hwnd = _win->GetHandle()
		//};

		//GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE handle;

		//HWND hwnd = static_cast<HWND>(_handle.window);
		HINSTANCE* hInst = reinterpret_cast<HINSTANCE*>(GetWindowLongPtr(static_cast<HWND>(pHWND), GWLP_HINSTANCE));

		vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo
		{
			.hinstance = *hInst ? *hInst : nullptr,
			.hwnd = pHWND
		};

		ImgnVulkan::surface = vk::raii::SurfaceKHR(ImgnVulkan::instance, surfaceCreateInfo);
	}

	void Ctx::CreateInstance()
	{
		constexpr vk::ApplicationInfo applicationInfo
		{
			.pApplicationName = "Imagination",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName = "Imagination Engine",
			.engineVersion = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion = vk::ApiVersion14
		};

		// Get the required layers
		std::vector<char const*> requiredLayers;
		if (_enableValidationLayers) requiredLayers.assign(_instanceLayers.begin(), _instanceLayers.end());

		// Check if the required layers are supported by the Vulkan implementation.
		auto layerProperties = ImgnVulkan::ctx.enumerateInstanceLayerProperties();
		auto unsupportedLayerIt = std::ranges::find_if(_instanceLayers,
			[&layerProperties](auto const& requiredLayer)
			{
				return std::ranges::none_of(layerProperties,
					[requiredLayer](auto const& layerProperty) { return strcmp(layerProperty.layerName, requiredLayer) == 0; });
			});

		if (unsupportedLayerIt != _instanceLayers.end()) throw std::runtime_error("Required layer not supported: " + std::string(*unsupportedLayerIt));

		// Check if the required extensions are supported by the Vulkan implementation.
		auto extensionProperties = ImgnVulkan::ctx.enumerateInstanceExtensionProperties();
		auto unsupportedPropertyIt =
			std::ranges::find_if(_instanceExtensions,
				[&extensionProperties](auto const& requiredExtension)
				{
					return std::ranges::none_of(extensionProperties,
						[requiredExtension](auto const& extensionProperty) { return strcmp(extensionProperty.extensionName, requiredExtension) == 0; });
				});
		if (unsupportedPropertyIt != _instanceExtensions.end())
		{
			throw std::runtime_error("Required extension not supported: " + std::string(*unsupportedPropertyIt));
		}

		vk::InstanceCreateInfo instanceCreateInfo
		{
			.pApplicationInfo = &applicationInfo,
			.enabledLayerCount = static_cast<uint32_t>(_instanceLayers.size()),
			.ppEnabledLayerNames = _instanceLayers.data(),
			.enabledExtensionCount = static_cast<uint32_t>(_instanceExtensions.size()),
			.ppEnabledExtensionNames = _instanceExtensions.data()
		};

		ImgnVulkan::instance = vk::raii::Instance(ImgnVulkan::ctx, instanceCreateInfo);
	}

	void Ctx::CreateSwapchain()
	{
		vk::SurfaceCapabilitiesKHR surfaceCapabilities = ImgnVulkan::physicalDevice.getSurfaceCapabilitiesKHR(*ImgnVulkan::surface);

		//vk::SurfaceCapabilitiesKHR surfaceCapabilities = ImgnVulkan::physicalDevice.getSurfaceCapabilitiesKHR(*ImgnVulkan::surface);
		ImgnVulkan::swapchainExtent = ChooseSwapExtent(surfaceCapabilities);

		uint32_t minImageCount = ChooseSwapMinImageCount(surfaceCapabilities);

		std::vector<vk::SurfaceFormatKHR> availableFormats = ImgnVulkan::physicalDevice.getSurfaceFormatsKHR(*ImgnVulkan::surface);
		ImgnVulkan::swapchainSurfaceFormat = ChooseSwapSurfaceFormat(availableFormats);

		std::vector<vk::PresentModeKHR> availablePresentModes = ImgnVulkan::physicalDevice.getSurfacePresentModesKHR(*ImgnVulkan::surface);
		vk::PresentModeKHR presentMode = ChooseSwapPresentMode(availablePresentModes);

		vk::SwapchainCreateInfoKHR swapchainCreateInfo
		{
			.surface = *ImgnVulkan::surface,
			.minImageCount = minImageCount,
			.imageFormat = ImgnVulkan::swapchainSurfaceFormat.format,
			.imageColorSpace = ImgnVulkan::swapchainSurfaceFormat.colorSpace,
			.imageExtent = ImgnVulkan::swapchainExtent,
			.imageArrayLayers = 1,
			.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
			.imageSharingMode = vk::SharingMode::eExclusive,
			.preTransform = surfaceCapabilities.currentTransform,
			.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode = ChooseSwapPresentMode(availablePresentModes),
			.clipped = true
		};

		ImgnVulkan::swapchain = vk::raii::SwapchainKHR(ImgnVulkan::device, swapchainCreateInfo);
		ImgnVulkan::swapchainImages = ImgnVulkan::swapchain.getImages();
	}
	void Ctx::CreateImageViews()
	{
		ImgnVulkan::swapchainImageViews.clear();
		ImgnVulkan::swapchainImageViews.reserve(ImgnVulkan::swapchainImages.size());

		for (auto& image : ImgnVulkan::swapchainImages)
		{
			ImgnVulkan::swapchainImageViews.emplace_back(CreateImageView(image, ImgnVulkan::swapchainSurfaceFormat.format, vk::ImageAspectFlagBits::eColor));
		}
	}

	void Ctx::CreateCommandPool()
	{
		vk::CommandPoolCreateInfo poolInfo
		{
			.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
			.queueFamilyIndex = ImgnVulkan::queueIdx
		};

		ImgnVulkan::commandPool = vk::raii::CommandPool(ImgnVulkan::device, poolInfo);
	}

	void Ctx::CreateSyncObjects()
	{
		assert(ImgnVulkan::presentCompleteSemaphores.empty() && ImgnVulkan::renderFinishedSemaphores.empty() && ImgnVulkan::inFlightFences.empty());

		for (size_t i = 0; i < ImgnVulkan::swapchainImages.size(); i++)
		{
			ImgnVulkan::renderFinishedSemaphores.emplace_back(ImgnVulkan::device, vk::SemaphoreCreateInfo());
		}

		for (size_t i = 0; i < MAXFRAMESINFLIGHT; i++)
		{
			ImgnVulkan::presentCompleteSemaphores.emplace_back(ImgnVulkan::device, vk::SemaphoreCreateInfo());
			ImgnVulkan::inFlightFences.emplace_back(ImgnVulkan::device, vk::FenceCreateInfo{ .flags = vk::FenceCreateFlagBits::eSignaled });
		}
	}

	void Ctx::CreateIndexBuffer()
	{
		//vk::DeviceSize bufferSize = sizeof(indices[0]) * indices.size();

		//Buffer stagingBuffer = {};
		//CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer);

		//void* data = stagingBuffer.memory.mapMemory(0, bufferSize);
		//memcpy(data, indices.data(), (size_t)bufferSize);
		//stagingBuffer.memory.unmapMemory();

		//CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, _indexBuffer);

		//CopyBuffer(stagingBuffer.buffer, _indexBuffer.buffer, bufferSize);
	}

	void Ctx::CreateVertexBuffer()
	{
		//vk::DeviceSize bufferSize = sizeof(Vertex) * vertices.size();

		//vk::BufferCreateInfo stagingInfo
		//{
		//	.size = bufferSize,
		//	.usage = vk::BufferUsageFlagBits::eTransferSrc,
		//	.sharingMode = vk::SharingMode::eExclusive
		//};

		//vk::raii::Buffer stagingBuffer(ImgnVulkan::device, stagingInfo);
		//vk::MemoryRequirements memRequirementsStaging = stagingBuffer.getMemoryRequirements();
		//vk::MemoryAllocateInfo memoryAllocateInfoStaging
		//{
		//	.allocationSize = memRequirementsStaging.size,
		//	.memoryTypeIndex = FindMemoryType(memRequirementsStaging.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
		//};

		//vk::raii::DeviceMemory stagingBufferMemory(ImgnVulkan::device, memoryAllocateInfoStaging);

		//stagingBuffer.bindMemory(stagingBufferMemory, 0);
		//void* dataStaging = stagingBufferMemory.mapMemory(0, stagingInfo.size);
		//memcpy(dataStaging, vertices.data(), stagingInfo.size);
		//stagingBufferMemory.unmapMemory();

		//vk::BufferCreateInfo bufferInfo{ .size = bufferSize,  .usage = vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, .sharingMode = vk::SharingMode::eExclusive };
		//_vertexBuffer.buffer = vk::raii::Buffer(ImgnVulkan::device, bufferInfo);

		//vk::MemoryRequirements memRequirements = _vertexBuffer.buffer.getMemoryRequirements();
		//vk::MemoryAllocateInfo memoryAllocateInfo
		//{
		//	.allocationSize = memRequirements.size,
		//	.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
		//};

		//_vertexBuffer.memory = vk::raii::DeviceMemory(ImgnVulkan::device, memoryAllocateInfo);

		//_vertexBuffer.buffer.bindMemory(*_vertexBuffer.memory, 0);

		//CopyBuffer(stagingBuffer, _vertexBuffer.buffer, stagingInfo.size);
	}

	void Ctx::CreateTextureImage()
	{
		//int width, height, channels;

		//stbi_set_flip_vertically_on_load(true);
		//uint8_t* pixels = stbi_load("../Textures/IKA Logo.png", &width, &height, &channels, STBI_rgb_alpha);

		//vk::DeviceSize imageSize = width * height * 4;

		//Buffer staging = {};
		//CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging);

		//void* data = staging.memory.mapMemory(0, imageSize);
		//memcpy(data, pixels, imageSize);
		//staging.memory.unmapMemory();

		//stbi_image_free(pixels);

		//CreateImage(width, height, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, _texture);

		//TransitionImageLayout(_texture.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		//CopyBufferToImage(staging.buffer, _texture.image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
		//TransitionImageLayout(_texture.image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	}

	void Ctx::CreateCommandBuffer()
	{
		ImgnVulkan::commandBuffers.clear();

		vk::CommandBufferAllocateInfo allocInfo
		{
			.commandPool = ImgnVulkan::commandPool,
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = MAXFRAMESINFLIGHT
		};

		ImgnVulkan::commandBuffers = vk::raii::CommandBuffers(ImgnVulkan::device, allocInfo);
	}

	void Ctx::CreateDepthResources()
	{
		_graph.AddResource("Depth", FindDepthFormat(), { ImgnVulkan::swapchainExtent.width, ImgnVulkan::swapchainExtent.height }, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageAspectFlagBits::eDepth);
		//vk::Format depthFormat = FindDepthFormat();

		//CreateImage(ImgnVulkan::swapchainExtent.width, ImgnVulkan::swapchainExtent.height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, _depth);
		//_depth.imageView = CreateImageView(_depth.image, depthFormat, vk::ImageAspectFlagBits::eDepth);
	}

	void Ctx::CreateUniformBuffers()
	{
		//_uniformBuffers.clear();
		//_uniformsBuffersMapped.clear();

		//for (size_t i = 0; i < MAXFRAMESINFLIGHT; i++)
		//{
		//	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
		//	Buffer buffer = {};

		//	CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, buffer);

		//	_uniformBuffers.emplace_back(std::move(buffer));
		//	_uniformsBuffersMapped.emplace_back(_uniformBuffers[i].memory.mapMemory(0, bufferSize));
		//}
	}

	void Ctx::CreateDescriptorPool()
	{
		std::array poolSize =
		{
			vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, ImgnVulkan::totalSets),
			vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, ImgnVulkan::totalSets),
			vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, ImgnVulkan::totalSets * ImgnVulkan::NumDescriptorsStreaming)
		};

		vk::DescriptorPoolCreateInfo poolInfo
		{
			.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind,
			.maxSets = ImgnVulkan::totalSets,
			.poolSizeCount = static_cast<uint32_t>(poolSize.size()),
			.pPoolSizes = poolSize.data()
		};

		ImgnVulkan::descriptorPool = vk::raii::DescriptorPool(ImgnVulkan::device, poolInfo);
	}

	void Ctx::CreateDescriptorSets()
	{
		std::vector<uint32_t> variableCounts(ImgnVulkan::totalSets, ImgnVulkan::NumDescriptorsStreaming);
		std::vector<vk::DescriptorSetLayout> layouts(ImgnVulkan::totalSets, *ImgnVulkan::descriptorSetLayout);

		vk::DescriptorSetVariableDescriptorCountAllocateInfo variableInfo
		{
			.descriptorSetCount = static_cast<uint32_t>(variableCounts.size()),
			.pDescriptorCounts = variableCounts.data()
		};

		vk::DescriptorSetAllocateInfo allocInfo
		{
			.pNext = &variableInfo,
			.descriptorPool = ImgnVulkan::descriptorPool,
			.descriptorSetCount = static_cast<uint32_t>(layouts.size()),
			.pSetLayouts = layouts.data()
		};

		ImgnVulkan::descriptorSets.clear();
		ImgnVulkan::descriptorSets = ImgnVulkan::device.allocateDescriptorSets(allocInfo);

		/*for (size_t i = 0; i < MAXFRAMESINFLIGHT; i++)
		{
			UpdateDescriptorSet(i, RenderPassIdx::GBuffer, _uniformBuffers[i].buffer, sizeof(UniformBufferObject), nullptr, 0, { *_texture.imageView }, *_textureSampler);
		}*/
		//for (size_t i = 0; i < MAXFRAMESINFLIGHT; i++)
		//{
		//	vk::DescriptorBufferInfo bufferInfo
		//	{
		//		.buffer = _uniformBuffers[i].buffer,
		//		.offset = 0,
		//		.range = sizeof(UniformBufferObject)
		//	};

		//	vk::DescriptorImageInfo imageInfo
		//	{
		//		.sampler = *_textureSampler,
		//		.imageView = _texture.imageView,
		//		.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
		//	};

		//	std::array descriptorWrites =
		//	{
		//		vk::WriteDescriptorSet
		//		{
		//			.dstSet = ImgnVulkan::descriptorSets[DescriptorSetIndex],
		//			.dstBinding = 0,
		//			.dstArrayElement = 0,
		//			.descriptorCount = 1,
		//			.descriptorType = vk::DescriptorType::eUniformBuffer,
		//			.pBufferInfo = &bufferInfo
		//		},
		//		vk::WriteDescriptorSet
		//		{
		//			.dstSet = ImgnVulkan::descriptorSets[i],
		//			.dstBinding = 2,
		//			.dstArrayElement = 0,
		//			.descriptorCount = NumDescriptorsStreaming,
		//			.descriptorType = vk::DescriptorType::eCombinedImageSampler,
		//			.pImageInfo = &imageInfo
		//		}
		//	};

		//	ImgnVulkan::device.updateDescriptorSets(descriptorWrites, {});
		//}
	}

	void Ctx::CreateDefaultSampler()
	{
		vk::PhysicalDeviceProperties properties = ImgnVulkan::physicalDevice.getProperties();

		vk::SamplerCreateInfo samplerInfo
		{
			.magFilter = vk::Filter::eLinear,
			.minFilter = vk::Filter::eLinear,
			.mipmapMode = vk::SamplerMipmapMode::eLinear,
			.addressModeU = vk::SamplerAddressMode::eRepeat,
			.addressModeV = vk::SamplerAddressMode::eRepeat,
			.addressModeW = vk::SamplerAddressMode::eRepeat,
			.mipLodBias = 0.f,
			.anisotropyEnable = vk::True,
			.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
			.compareEnable = vk::False,
			.compareOp = vk::CompareOp::eAlways,
			.minLod = 0.f,
			.maxLod = 0.f,
			.borderColor = vk::BorderColor::eIntOpaqueBlack,
			.unnormalizedCoordinates = vk::False
		};

		ImgnVulkan::defaultSampler = vk::raii::Sampler(ImgnVulkan::device, samplerInfo);
	}

	void Ctx::CreateGraphicsPipelines()
	{
		vk::GraphicsPipelineCreateInfo pipelineInfo;

		vk::raii::ShaderModule vertexSM = CreateShaderModule(GetSPV(Shaders::TriangleVertexShader, VertexTarget));
		vk::raii::ShaderModule fragmentSM = CreateShaderModule(GetSPV(Shaders::TriangleFragmentShader, FragmentTarget));

		vk::PipelineShaderStageCreateInfo vertShaderStageInfo
		{
			.stage = vk::ShaderStageFlagBits::eVertex,
			.module = vertexSM,
			.pName = "main"
		};

		vk::PipelineShaderStageCreateInfo fragShaderStageInfo
		{
			.stage = vk::ShaderStageFlagBits::eFragment,
			.module = fragmentSM,
			.pName = "main"
		};

		std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

		auto bindingDesc = Vertex::GetBindingDescription();
		auto attributeDesc = Vertex::GetAttributeDescriptions();

		vk::PipelineVertexInputStateCreateInfo vertexInputInfo
		{
			.vertexBindingDescriptionCount = 1,
			.pVertexBindingDescriptions = &bindingDesc,
			.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDesc.size()),
			.pVertexAttributeDescriptions = attributeDesc.data()
		};

		vk::PipelineInputAssemblyStateCreateInfo inputAssembly
		{
			.topology = vk::PrimitiveTopology::eTriangleList
		};

		vk::PipelineViewportStateCreateInfo viewportState
		{
			.viewportCount = 1,
			.scissorCount = 1
		};

		vk::PipelineRasterizationStateCreateInfo rasterizer
		{
			.depthClampEnable = vk::False,
			.rasterizerDiscardEnable = vk::False,
			.polygonMode = vk::PolygonMode::eFill,
			.cullMode = vk::CullModeFlagBits::eBack,
			.frontFace = vk::FrontFace::eClockwise,
			.depthBiasEnable = vk::False,
			.depthBiasSlopeFactor = 1.0f,
			.lineWidth = 1.0f
		};

		vk::PipelineMultisampleStateCreateInfo multisampling
		{
			.rasterizationSamples = vk::SampleCountFlagBits::e1,
			.sampleShadingEnable = vk::False
		};

		vk::PipelineDepthStencilStateCreateInfo depthStencil
		{
			.depthTestEnable = vk::True,
			.depthWriteEnable = vk::True,
			.depthCompareOp = vk::CompareOp::eLess,
			.depthBoundsTestEnable = vk::False,
			.stencilTestEnable = vk::False
		};

		vk::PipelineColorBlendAttachmentState colorBlendAttachment
		{
			.blendEnable = vk::False,
			.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
		};

		vk::PipelineColorBlendStateCreateInfo colorBlending
		{
			.logicOpEnable = vk::False,
			.logicOp = vk::LogicOp::eCopy,
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment
		};

		std::vector dynamicStates =
		{
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor
		};

		vk::PipelineDynamicStateCreateInfo dynamicState
		{
			.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data()
		};

		vk::PipelineLayoutCreateInfo pipelineLayoutInfo
		{
			.setLayoutCount = 1,
			.pSetLayouts = &*ImgnVulkan::descriptorSetLayout,
			.pushConstantRangeCount = 0
		};

		ImgnVulkan::pipelines.pipelineLayout = vk::raii::PipelineLayout(ImgnVulkan::device, pipelineLayoutInfo);

		vk::Format depthFormat = FindDepthFormat();

		vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo
		{
			.colorAttachmentCount = 1,
			.pColorAttachmentFormats = &ImgnVulkan::swapchainSurfaceFormat.format,
			.depthAttachmentFormat = depthFormat
		};

		pipelineInfo.pNext = &pipelineRenderingCreateInfo;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages.data();
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = &depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &dynamicState;
		pipelineInfo.layout = ImgnVulkan::pipelines.pipelineLayout;
		pipelineInfo.renderPass = nullptr;

		ImgnVulkan::pipelines.generalPipeline = vk::raii::Pipeline(ImgnVulkan::device, nullptr, pipelineInfo);

		//vk::PipelineLayoutCreateInfo pipelineLayoutInfo
		//{
		//	.setLayoutCount = 1,
		//	.pSetLayouts = &*ImgnVulkan::descriptorSetLayout,
		//	.pushConstantRangeCount = 0
		//};

		//_pipelines.pipelineLayout = vk::raii::PipelineLayout(ImgnVulkan::device, pipelineLayoutInfo);

		/* GBuffer*/
		{
			vk::raii::ShaderModule vertexSM = CreateShaderModule(GetSPV(Shaders::GBufferVertexShader, VertexTarget));
			vk::raii::ShaderModule fragmentSM = CreateShaderModule(GetSPV(Shaders::GBufferFragmentShader, FragmentTarget));

			vk::PipelineShaderStageCreateInfo vertShaderStageInfo
			{
				.stage = vk::ShaderStageFlagBits::eVertex,
				.module = vertexSM,
				.pName = "main"
			};

			vk::PipelineShaderStageCreateInfo fragShaderStageInfo
			{
				.stage = vk::ShaderStageFlagBits::eFragment,
				.module = fragmentSM,
				.pName = "main"
			};

			std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

			auto bindingDesc = ImgnVulkan::Vertex::GetBindingDescription();
			auto attributeDesc = ImgnVulkan::Vertex::GetAttributeDescriptions();

			vk::PipelineVertexInputStateCreateInfo vertexInputInfo
			{
				.vertexBindingDescriptionCount = 1,
				.pVertexBindingDescriptions = &bindingDesc,
				.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDesc.size()),
				.pVertexAttributeDescriptions = attributeDesc.data()
			};

		/*	vk::PipelineInputAssemblyStateCreateInfo inputAssembly
			{
				.topology = vk::PrimitiveTopology::eTriangleList
			};

			vk::PipelineViewportStateCreateInfo viewportState
			{
				.viewportCount = 1,
				.scissorCount = 1
			};

			vk::PipelineRasterizationStateCreateInfo rasterizer
			{
				.depthClampEnable = vk::False,
				.rasterizerDiscardEnable = vk::False,
				.polygonMode = vk::PolygonMode::eFill,
				.cullMode = vk::CullModeFlagBits::eNone,
				.frontFace = vk::FrontFace::eCounterClockwise,
				.depthBiasEnable = vk::False,
				.depthBiasSlopeFactor = 1.0f,
				.lineWidth = 1.0f
			};*/

			//vk::PipelineMultisampleStateCreateInfo multisampling
			//{
			//	.rasterizationSamples = vk::SampleCountFlagBits::e1,
			//	.sampleShadingEnable = vk::False
			//};

			//vk::PipelineDepthStencilStateCreateInfo depthStencil
			//{
			//	.depthTestEnable = vk::False,
			//	.depthWriteEnable = vk::False,
			//	.depthCompareOp = vk::CompareOp::eLess,
			//	.depthBoundsTestEnable = vk::False,
			//	.stencilTestEnable = vk::False
			//};

			std::array blendStates = { colorBlendAttachment, colorBlendAttachment, colorBlendAttachment };

			vk::PipelineColorBlendStateCreateInfo colorBlending
			{
				.logicOpEnable = vk::False,
				.logicOp = vk::LogicOp::eCopy,
				.attachmentCount = blendStates.size(),
				.pAttachments = blendStates.data()
			};

			std::array colorAttachmentFormats = { vk::Format::eR16G16B16A16Sfloat, vk::Format::eR16G16B16A16Sfloat, vk::Format::eR8G8B8A8Unorm };

			vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo
			{
				.colorAttachmentCount = colorAttachmentFormats.size(),
				.pColorAttachmentFormats = colorAttachmentFormats.data(),
				.depthAttachmentFormat = depthFormat
			};

			pipelineInfo.pNext = &pipelineRenderingCreateInfo;
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pColorBlendState = &colorBlending;

			ImgnVulkan::pipelines.gBufferPipeline = vk::raii::Pipeline(ImgnVulkan::device, nullptr, pipelineInfo);

			//pipelineInfo = gBufferPipelineInfo;
		}

		/* Lighting */
		{
			vk::raii::ShaderModule vertexSM = CreateShaderModule(GetSPV(Shaders::LightingVertexShader, VertexTarget));
			vk::raii::ShaderModule fragmentSM = CreateShaderModule(GetSPV(Shaders::LightingFragmentShader, FragmentTarget));

			vk::PipelineShaderStageCreateInfo vertShaderStageInfo
			{
				.stage = vk::ShaderStageFlagBits::eVertex,
				.module = vertexSM,
				.pName = "main"
			};

			vk::PipelineShaderStageCreateInfo fragShaderStageInfo
			{
				.stage = vk::ShaderStageFlagBits::eFragment,
				.module = fragmentSM,
				.pName = "main"
			};

			std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

			vk::PipelineVertexInputStateCreateInfo vertexInputInfo
			{
				.vertexBindingDescriptionCount = 0,
				.pVertexBindingDescriptions = nullptr,
				.vertexAttributeDescriptionCount = 0,
				.pVertexAttributeDescriptions = nullptr
			};

			vk::PipelineRasterizationStateCreateInfo rasterizer
			{
				.depthClampEnable = vk::False,
				.rasterizerDiscardEnable = vk::False,
				.polygonMode = vk::PolygonMode::eFill,
				.cullMode = vk::CullModeFlagBits::eNone,
				.frontFace = vk::FrontFace::eCounterClockwise,
				.depthBiasEnable = vk::False,
				.depthBiasSlopeFactor = 1.0f,
				.lineWidth = 1.0f
			};

			vk::PipelineColorBlendStateCreateInfo colorBlending
			{
				.logicOpEnable = vk::False,
				.logicOp = vk::LogicOp::eCopy,
				.attachmentCount = 1,
				.pAttachments = &colorBlendAttachment
			};

			std::array colorAttachment = { vk::Format::eR8G8B8A8Unorm };
			//vk::Format depthFormat = FindDepthFormat();

			vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo
			{
				.colorAttachmentCount = colorAttachment.size(),
				.pColorAttachmentFormats = colorAttachment.data(),
				.depthAttachmentFormat = depthFormat
			};

			pipelineInfo.pNext = &pipelineRenderingCreateInfo;
			pipelineInfo.pStages = shaderStages.data();
			pipelineInfo.pVertexInputState = &vertexInputInfo;
			pipelineInfo.pRasterizationState = &rasterizer;
			pipelineInfo.pColorBlendState = &colorBlending;

			ImgnVulkan::pipelines.lightingPipeline = vk::raii::Pipeline(ImgnVulkan::device, nullptr, pipelineInfo);
		}
	}

	void Ctx::CreateTextureImageView()
	{
		//_texture.imageView = CreateImageView(_texture.image, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor);
	}

	void Ctx::CreateDescriptorSetLayout()
	{
		std::array bindings =
		{
			vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, nullptr),
			vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, nullptr),
			vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eCombinedImageSampler, ImgnVulkan::NumDescriptorsStreaming, vk::ShaderStageFlagBits::eFragment, nullptr)
		};

		std::array<vk::DescriptorBindingFlags, 3> flags =
		{
			vk::DescriptorBindingFlags{}, vk::DescriptorBindingFlags{},
			vk::DescriptorBindingFlagBits::eVariableDescriptorCount | vk::DescriptorBindingFlagBits::ePartiallyBound | vk::DescriptorBindingFlagBits::eUpdateAfterBind | vk::DescriptorBindingFlagBits::eUpdateUnusedWhilePending
		};

		vk::DescriptorSetLayoutBindingFlagsCreateInfo bindingFlags
		{
			.bindingCount = static_cast<uint32_t>(flags.size()),
			.pBindingFlags = flags.data(),
		};

		vk::DescriptorSetLayoutCreateInfo layoutInfo
		{
			.pNext = &bindingFlags,
			.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool,
			.bindingCount = static_cast<uint32_t>(bindings.size()),
			.pBindings = bindings.data()
		};

		ImgnVulkan::descriptorSetLayout = vk::raii::DescriptorSetLayout(ImgnVulkan::device, layoutInfo);
	}

	/*void Ctx::UpdateCamera()
	{*/
		//bool focused;
		//_gWin->IsFocus(focused);

		//if (!focused) return;

		//mat4 cam = GW::MATH::GIdentityMatrixF;

		//auto ubo = _graph.GetBufferResource("GBuffer-UBO");

		//GBufferUBO gBufferUBO;
		//{
		//	void* data = ubo->memory.mapMemory(0, sizeof(GBufferUBO));
		//	memcpy(&gBufferUBO, data, sizeof(GBufferUBO));
		//	ubo->memory.unmapMemory();
		//}

		////auto& deltaTime = static_cast<UniformBufferOffscreen*>(_graph._blackboard.Get<void*>("offscreen uniform"))->deltaTime;

		//auto now = std::chrono::steady_clock::now();
		//std::chrono::duration<float> dt = _lastUpdate - now;
		//gBufferUBO.deltaTime = -dt.count();
		//_lastUpdate = now;

		//GMatrix::InverseF(gBufferUBO.view, cam);

		//float y = 0.0f;

		//float totalY = 0.0f;
		//float totalZ = 0.0f;
		//float totalX = 0.0f;

		//const float cameraSpeed = 75.f;
		//float spaceKeyState = 0.0f;
		//float leftShiftState = 0.0f;
		//float rightTriggerState = 0.0f;
		//float leftTriggerState = 0.0f;

		//float arrowRight = 0.0f;
		//float arrowLeft = 0.0f;

		//float wKeyState = 0.0f;
		//float sKeyState = 0.0f;
		//float aKeyState = 0.0f;
		//float dKeyState = 0.0f;
		//float leftStickX = 0.0f;
		//float leftStickY = 0.0f;
		//unsigned int screenHeight = 0.0f;
		//_gWin->GetHeight(screenHeight);
		//unsigned int screenWidth = 0.0f;
		//_gWin->GetWidth(screenWidth);
		//float mouseDeltaX = 0.0f;
		//float mouseDeltaY = 0.0f;
		////GW::GReturn result = ;
		//float rightStickYaxis = 0.0f;
		////_gController.GetState(0, G_RY_AXIS, rightStickYaxis);
		//float rightStickXaxis = 0.0f;
		////_gController.GetState(0, G_RX_AXIS, rightStickXaxis);

		//float perFrameSpeed = 0.0f;

		//_gInput.GetState(G_KEY_RIGHT, arrowRight);
		//_gInput.GetState(G_KEY_LEFT, arrowLeft);

		//if (arrowRight != 0)
		//{
		//	cam.row4 = { 0.0f, 50.0f, 0.0f, 1 };

		//}
		//if (arrowLeft != 0)
		//{
		//	cam.row4 = { 5.75f, 5.25f, -30.5f, 1 };
		//}

		//if (+_gInput.GetState(G_KEY_SPACE, spaceKeyState) && spaceKeyState != 0 || +_gInput.GetState(G_KEY_LEFTSHIFT, leftShiftState) && leftShiftState != 0)// || +_gController.GetState(0, G_RIGHT_TRIGGER_AXIS, rightTriggerState) && rightTriggerState != 0 || +_gController.GetState(0, G_LEFT_TRIGGER_AXIS, leftTriggerState) && leftTriggerState != 0)
		//{
		//	totalY = spaceKeyState - leftShiftState + rightTriggerState - leftTriggerState;
		//}

		//cam.row4.y += totalY * cameraSpeed * gBufferUBO.deltaTime;

		//perFrameSpeed = cameraSpeed * gBufferUBO.deltaTime;

		//if (+_gInput.GetState(G_KEY_W, wKeyState) && wKeyState != 0 || +_gInput.GetState(G_KEY_A, aKeyState) && aKeyState != 0 || +_gInput.GetState(G_KEY_S, sKeyState) && sKeyState != 0 || +_gInput.GetState(G_KEY_D, dKeyState) && dKeyState != 0)// || +_gController.GetState(0, G_LX_AXIS, leftStickX) && leftStickX != 0 || +_gController.GetState(0, G_LY_AXIS, leftStickY) && leftStickY != 0)
		//{
		//	totalZ = wKeyState - sKeyState + leftStickY;
		//	totalX = dKeyState - aKeyState + leftStickX;
		//}

		//mat4 translation = GW::MATH::GIdentityMatrixF;
		//vec4 vec = { totalX * perFrameSpeed, 0, totalZ * perFrameSpeed };
		//GMatrix::TranslateLocalF(translation, vec, translation);
		//GMatrix::MultiplyMatrixF(translation, cam, cam);

		//float thumbSpeed = 3.14 * perFrameSpeed;
		//auto r = _gInput.GetMouseDelta(mouseDeltaX, mouseDeltaY);
		//if (G_PASS(r) && r != GW::GReturn::REDUNDANT)
		//{
		//	float totalPitch = G_DEGREE_TO_RADIAN(65) * mouseDeltaY / screenHeight + rightStickYaxis * -thumbSpeed;
		//	GMatrix::RotateXLocalF(cam, totalPitch, cam);
		//	float totalYaw = G_DEGREE_TO_RADIAN(65) * 16 / 9 * mouseDeltaX / screenWidth + rightStickXaxis * thumbSpeed;
		//	mat4 yawMatrix = GW::MATH::GIdentityMatrixF;
		//	vec4 camSave = cam.row4;
		//	cam.row4 = { 0,0,0,1 };
		//	GMatrix::RotateYGlobalF(cam, totalYaw, cam);
		//	cam.row4 = camSave;
		//}

		////vec4 camPos = cam.row4;
		//GMatrix::InverseF(cam, gBufferUBO.view);

		//{
		//	void* data = ubo->memory.mapMemory(0, sizeof(GBufferUBO));
		//	memcpy(data, &gBufferUBO, sizeof(GBufferUBO));
		//	ubo->memory.unmapMemory();
		//}

	//}

	void Ctx::SetupDeferredRenderer()
	{
		//uint32_t width = ImgnVulkan::swapchainExtent.width, height = ImgnVulkan::swapchainExtent.height;

		////_sponza = _manager.Load<Mesh>("Sponza");

		//_graph.AddResource("GBuffer-Position", vk::Format::eR16G16B16A16Sfloat, { width, height }, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor);
		//_graph.AddResource("GBuffer-Normal", vk::Format::eR16G16B16A16Sfloat, { width, height }, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor);
		//_graph.AddResource("GBuffer-Albedo", vk::Format::eR8G8B8A8Unorm, { width, height }, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment, vk::ImageLayout::eUndefined, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageAspectFlagBits::eColor);
		////_graph.AddResource("Depth", vk::Format::eD32Sfloat, { width, height }, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eInputAttachment, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, vk::ImageAspectFlagBits::eDepth);
		//_graph.AddResource("FinalColor", vk::Format::eR8G8B8A8Unorm, { width, height }, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferSrcOptimal, vk::ImageAspectFlagBits::eColor);

		//GBufferUBO gBufferUBO
		//{
		//	.world = GW::MATH::GIdentityMatrixF,
		//	.view = GW::MATH::GIdentityMatrixF,
		//	.proj = GW::MATH::GIdentityMatrixF,
		//};

		//GMatrix::LookAtLHF({ 0.f, 0.25f, 0 }, { 1.f, 0.f, 1.f }, { 0.f, 1.f, 0.f }, gBufferUBO.view);
		//GMatrix::ProjectionVulkanLHF(45.f, 16 / 9, 0.01, 100.f, gBufferUBO.proj);

		//_graph.AddResource("GBuffer-UBO", sizeof(GBufferUBO), vk::BufferUsageFlagBits::eUniformBuffer, &gBufferUBO);

		//RenderPass gBufferPass
		//{
		//	.name = "GeometryPass",
		//	.inputs = {},
		//	.outputs = { "GBuffer-Position", "GBuffer-Normal", "GBuffer-Albedo", "Depth" },
		//	.bufferInputs = {"GBuffer-UBO"},
		//	.descriptorSetLayout = ImgnVulkan::descriptorSetLayout,
		//	.Execute = [&](vk::raii::CommandBuffer& commandBuffer)
		//	{
		//		auto ubo = _graph.GetBufferResource("GBuffer-UBO");
		//		UpdateDescriptorSet(ImgnVulkan::frameIdx, RenderPassIdx::GBuffer, ubo->buffer, ubo->size, nullptr, 0, {}, *_textureSampler);

		//		std::array<vk::RenderingAttachmentInfo, 3> colorAttachments;
		//		vk::RenderingAttachmentInfoKHR depthAttachment;
		//		vk::RenderingInfoKHR renderingInfo;

		//		colorAttachments[0].setImageView(_graph.GetImageResource("GBuffer-Position")->view).setImageLayout(vk::ImageLayout::eColorAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setClearValue(vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} });
		//		colorAttachments[1].setImageView(_graph.GetImageResource("GBuffer-Normal")->view).setImageLayout(vk::ImageLayout::eColorAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setClearValue(vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} });
		//		colorAttachments[2].setImageView(_graph.GetImageResource("GBuffer-Albedo")->view).setImageLayout(vk::ImageLayout::eColorAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setClearValue(vk::ClearColorValue{ std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f} });

		//		depthAttachment.setImageView(_graph.GetImageResource("Depth")->view).setImageLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal).setLoadOp(vk::AttachmentLoadOp::eClear).setStoreOp(vk::AttachmentStoreOp::eStore).setClearValue(vk::ClearDepthStencilValue{ 1.0f, 0 });

		//		renderingInfo.setRenderArea({ {0, 0}, {ImgnVulkan::swapchainExtent.width, ImgnVulkan::swapchainExtent.height} }).setLayerCount(1).setColorAttachmentCount(colorAttachments.size()).setPColorAttachments(colorAttachments.data()).setPDepthAttachment(&depthAttachment);

		//		commandBuffer.beginRendering(renderingInfo);

		//		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipelines.gBufferPipeline);
		//		commandBuffer.setViewport(0, vk::Viewport(0.0f, 0, static_cast<float>(ImgnVulkan::swapchainExtent.width), static_cast<float>(ImgnVulkan::swapchainExtent.height), 0.0f, 1.0f));
		//		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), ImgnVulkan::swapchainExtent));


		//		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelines.pipelineLayout, 0, *ImgnVulkan::descriptorSets[DescriptorSetIndex(ImgnVulkan::frameIdx, RenderPassIdx::GBuffer)], nullptr);
		//		commandBuffer.bindVertexBuffers(0, _sponza->GetVertexBuffer(), {0});
		//		commandBuffer.bindIndexBuffer(_sponza->GetIndexBuffer(), 0, vk::IndexType::eUint32);

		//		commandBuffer.drawIndexed(_sponza->GetIndexCount(), 1, 0, 0, 0);

		//		commandBuffer.endRendering();
		//	}
		//};

		//RenderPass lightingPass
		//{
		//	.name = "LightingPass",
		//	.inputs = { "GBuffer-Position", "GBuffer-Normal", "GBuffer-Albedo", "Depth" },
		//	.outputs = {"FinalColor"},
		//	.descriptorSetLayout = ImgnVulkan::descriptorSetLayout,
		//	.Execute = [&](vk::raii::CommandBuffer& commandBuffer)
		//	{
		//		vk::RenderingAttachmentInfo colorAttachment
		//		{
		//			.imageView = _graph.GetImageResource("FinalColor")->view,
		//			.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		//			.loadOp = vk::AttachmentLoadOp::eClear,
		//			.storeOp = vk::AttachmentStoreOp::eStore,
		//			.clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)
		//		};

		//		vk::RenderingInfoKHR renderingInfo
		//		{
		//			.renderArea = { {0, 0}, {ImgnVulkan::swapchainExtent.width, ImgnVulkan::swapchainExtent.height} },
		//			.layerCount = 1,
		//			.colorAttachmentCount = 1,
		//			.pColorAttachments = &colorAttachment,
		//		};

		//		std::vector<vk::ImageView> imageViews =
		//		{
		//			_graph.GetImageResource("GBuffer-Position")->view,
		//			_graph.GetImageResource("GBuffer-Normal")->view,
		//			_graph.GetImageResource("GBuffer-Albedo")->view,
		//			_graph.GetImageResource("Depth")->view,
		//		};
		//		UpdateDescriptorSet(ImgnVulkan::frameIdx, RenderPassIdx::Lighting, nullptr, 0, nullptr, 0, imageViews, *_textureSampler);

		//		commandBuffer.beginRendering(renderingInfo);

		//		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipelines.lightingPipeline);
		//		commandBuffer.setViewport(0, vk::Viewport(0.0f, 0, static_cast<float>(ImgnVulkan::swapchainExtent.width), static_cast<float>(ImgnVulkan::swapchainExtent.height), 0.0f, 1.0f));
		//		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), ImgnVulkan::swapchainExtent));


		//		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, _pipelines.pipelineLayout, 0, *ImgnVulkan::descriptorSets[DescriptorSetIndex(ImgnVulkan::frameIdx, RenderPassIdx::Lighting)], nullptr);
		//		//commandBuffer.bindVertexBuffers(0, _sponza->GetVertexBuffer(), {0});
		//		//commandBuffer.bindIndexBuffer(_sponza->GetIndexBuffer(), 0, vk::IndexType::eUint32);

		//		commandBuffer.draw(3, 1, 0, 0);

		//		commandBuffer.endRendering();
		//	}
		//};

		//_graph.AddPass(gBufferPass);
		//_graph.AddPass(lightingPass);

		//_graph.Compile();
	}

	//void Ctx::UpdateUniformBuffer(uint32_t pCurrImage)
	//{
	//	static auto startTime = std::chrono::high_resolution_clock::now();

	//	auto currentTime = std::chrono::high_resolution_clock::now();
	//	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	//	//UniformBufferObject ubo
	//	//{
	//	//	.model = RotateG(Identity<float>(), time * Radians(90.f), vec3<float>{ 0, 0, 1 }),
	//	//	.view = LookAtL(vec4<float>{2.f, 2.f, 2.f}, vec4<float>{0.f, 0.f, 0.f}, vec4<float>{0.f, 0.f, 1.f}),
	//	//	.proj = Projection<float>(Radians(45.f), static_cast<float>(ImgnVulkan::swapchainExtent.width) / static_cast<float>(ImgnVulkan::swapchainExtent.height), 0.1, 10.f)
	//	//};

	//	//memcpy(_uniformsBuffersMapped[pCurrImage], &ubo, sizeof(UniformBufferObject));
	//}

	vk::raii::ImageView Ctx::CreateImageView(vk::Image& pImage, vk::Format pFormat, vk::ImageAspectFlags pAspectFlags)
	{
		vk::ImageViewCreateInfo imageViewCreateInfo
		{
			.image = pImage,
			.viewType = vk::ImageViewType::e2D,
			.format = pFormat,
			.components
			{
				.r = vk::ComponentSwizzle::eIdentity,
				.g = vk::ComponentSwizzle::eIdentity,
				.b = vk::ComponentSwizzle::eIdentity,
				.a = vk::ComponentSwizzle::eIdentity
			},
			.subresourceRange
			{
				.aspectMask = pAspectFlags,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		return vk::raii::ImageView(ImgnVulkan::device, imageViewCreateInfo);
	}

	vk::raii::ImageView Ctx::CreateImageView(vk::raii::Image& pImage, vk::Format pFormat, vk::ImageAspectFlags pAspectFlags)
	{
		vk::ImageViewCreateInfo imageViewCreateInfo
		{
			.image = pImage,
			.viewType = vk::ImageViewType::e2D,
			.format = pFormat,
			.components
			{
				.r = vk::ComponentSwizzle::eIdentity,
				.g = vk::ComponentSwizzle::eIdentity,
				.b = vk::ComponentSwizzle::eIdentity,
				.a = vk::ComponentSwizzle::eIdentity
			},
			.subresourceRange
			{
				.aspectMask = pAspectFlags,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		return vk::raii::ImageView(ImgnVulkan::device, imageViewCreateInfo);
	}

	void Ctx::CopyBuffer(vk::raii::Buffer& pSrc, vk::raii::Buffer& pDst, vk::DeviceSize pSize)
	{
		vk::raii::CommandBuffer commandCopyBuffer = BeginSingleCommand();

		commandCopyBuffer.copyBuffer(pSrc, pDst, vk::BufferCopy(0, 0, pSize));

		EndSingleCommand(commandCopyBuffer);
	}

	void Ctx::CopyBufferToImage(const vk::raii::Buffer& pBuffer, vk::raii::Image& pImage, uint32_t pWidth, uint32_t pHeight)
	{
		vk::raii::CommandBuffer commandBuffer = BeginSingleCommand();

		vk::BufferImageCopy region
		{
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
			.imageOffset = {0, 0, 0},
			.imageExtent = {pWidth, pHeight, 1}
		};

		commandBuffer.copyBufferToImage(pBuffer, pImage, vk::ImageLayout::eTransferDstOptimal, { region });

		EndSingleCommand(commandBuffer);
	}

	void Ctx::CreateBuffer(vk::DeviceSize pSize, vk::BufferUsageFlags pUsage, vk::MemoryPropertyFlags pProps, Buffer& pBuffer)
	{
		vk::BufferCreateInfo bufferCreateInfo
		{
			.size = pSize,
			.usage = pUsage,
			.sharingMode = vk::SharingMode::eExclusive
		};

		pBuffer.buffer = vk::raii::Buffer(ImgnVulkan::device, bufferCreateInfo);

		vk::MemoryRequirements memReqs = pBuffer.buffer.getMemoryRequirements();
		vk::MemoryAllocateInfo memoryAllocateInfo{ .allocationSize = memReqs.size, .memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, pProps) };

		pBuffer.memory = vk::raii::DeviceMemory(ImgnVulkan::device, memoryAllocateInfo);

		pBuffer.buffer.bindMemory(*pBuffer.memory, 0);
	}

	void Ctx::CreateImage(uint32_t pWidth, uint32_t pHeight, vk::Format pFormat, vk::ImageTiling pTiling, vk::ImageUsageFlags pUsage, vk::MemoryPropertyFlags pProps, vk::ImageAspectFlags pAspect, Image& pImage)
	{
		vk::ImageCreateInfo imageCreateInfo
		{
			.imageType = vk::ImageType::e2D,
			.format = pFormat,
			.extent = {pWidth , pHeight, 1},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = vk::SampleCountFlagBits::e1,
			.tiling = pTiling,
			.usage = pUsage,
			.sharingMode = vk::SharingMode::eExclusive
		};

		pImage.image = vk::raii::Image(ImgnVulkan::device, imageCreateInfo);

		vk::MemoryRequirements memRequirements = pImage.image.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo
		{
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, pProps)
		};

		pImage.memory = vk::raii::DeviceMemory(ImgnVulkan::device, allocInfo);
		pImage.image.bindMemory(pImage.memory, 0);

		pImage.view = CreateImageView(pImage.image, pFormat, pAspect);
	}

	void Ctx::UpdateDescriptorSet(uint32_t pFrameIdx, RenderPassIdx pIdx, vk::Buffer pUniformBuffer, vk::DeviceSize pUniformBufferSize, vk::Buffer pStorageBuffer, vk::DeviceSize pStorageBufferSize, const std::vector<vk::ImageView>& pTextures, vk::Sampler pSampler)
	{
		std::vector<vk::WriteDescriptorSet> writes;

		vk::DescriptorBufferInfo uniformBufferInfo
		{
			.buffer = pUniformBuffer,
			.offset = 0,
			.range = pUniformBufferSize
		};

		vk::DescriptorBufferInfo storageBufferInfo
		{
			.buffer = pStorageBuffer,
			.offset = 0,
			.range = pStorageBufferSize
		};

		std::vector<vk::DescriptorImageInfo> imageInfos;
		imageInfos.reserve(pTextures.size());

		for (auto& texture : pTextures)
		{
			imageInfos.push_back(vk::DescriptorImageInfo
				{
					.sampler = pSampler,
					.imageView = texture,
					.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal
				});
		}

		if (pUniformBuffer)
		{
			writes.push_back(vk::WriteDescriptorSet
				{
					.dstSet = ImgnVulkan::descriptorSets[DescriptorSetIndex(pFrameIdx, pIdx)],
					.dstBinding = 0,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eUniformBuffer,
					.pBufferInfo = &uniformBufferInfo
				});
		}

		if (pStorageBuffer)
		{
			writes.push_back(vk::WriteDescriptorSet
				{
					.dstSet = ImgnVulkan::descriptorSets[DescriptorSetIndex(pFrameIdx, pIdx)],
					.dstBinding = 1,
					.dstArrayElement = 0,
					.descriptorCount = 1,
					.descriptorType = vk::DescriptorType::eStorageBuffer,
					.pBufferInfo = &storageBufferInfo
				});
		}

		if (!imageInfos.empty())
		{
			writes.push_back(vk::WriteDescriptorSet
				{
					.dstSet = ImgnVulkan::descriptorSets[DescriptorSetIndex(pFrameIdx, pIdx)],
					.dstBinding = 2,
					.dstArrayElement = 0,
					.descriptorCount = static_cast<uint32_t>(imageInfos.size()),
					.descriptorType = vk::DescriptorType::eCombinedImageSampler,
					.pImageInfo = imageInfos.data()
				});
		}

		ImgnVulkan::device.updateDescriptorSets(writes, {});
	}

	//void Ctx::Cleanup()
	//{
	//	CleanupSwapchain();
	//}

	//void Ctx::InitVulkanCtx(ImgnWindow* pWindow)
	//{
		/*_win = pWindow;

		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&ImgnVulkan::compiler));
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils));
		_utils->CreateDefaultIncludeHandler(&_includeHandler);

		Device::Inst().InitDXC();

		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();
		PickPhysicalDevice();
		CreateDevice();
		CreateSwapchain();
		CreateImageViews();
		CreateDescriptorSetLayout();
		CreateGraphicsPipelines();
		CreateCommandPool();
		CreateDepthResources();
		CreateTextureImage();
		CreateTextureImageView();
		CreateTextureSampler();
		CreateVertexBuffer();
		CreateIndexBuffer();
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSets();
		CreateCommandBuffer();
		CreateSyncObjects();

		SetupDeferredRenderer();

		_gui.Init(*pWindow, pWindow->width, pWindow->height);*/
	//}

	void Ctx::InitVulkanCtx(HWND pHWND, uint32_t pWidth, uint32_t pHeight)
	{
		_width = pWidth; _height = pHeight;
		//_gWin = pWindow;
		//_gWin->GetClientWidth(_gWinW);
		//_gWin->GetClientHeight(_gWinH);
		//_gWin->GetWindowHandle(_handle);
		//_gInput.Create(*_gWin);

		//_responder.Create([&](const GEvent& e)
		//	{
		//		GWindow::Events event;
		//		GWindow::EVENT_DATA data;

		//		if (+e.Read(event, data))
		//		{
		//			switch (event)
		//			{
		//			case GWindow::Events::RESIZE:
		//				{
		//					WindowResizedEvent wre(data.clientWidth, data.clientHeight);
		//					EventDispatcher dispatcher(wre);
		//					dispatcher.Dispatch<WindowResizedEvent>([&](WindowResizedEvent& e)
		//						{
		//							RecreateSwapchain();

		//							return true;
		//						});
		//				}
		//				break;
		//			case GWindow::Events::DISPLAY_CLOSED:
		//				{
		//					WindowClosedEvent wce;
		//					EventDispatcher dispatcher(wce);
		//					dispatcher.Dispatch<WindowClosedEvent>([&](WindowClosedEvent& e)
		//						{
		//							ImgnVulkan::device.waitIdle();

		//							return true;
		//						});

		//				}
		//				break;
		//			}
		//		}
		//	});

		//_gWin->Register(_responder);


		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&ImgnVulkan::compiler));
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&ImgnVulkan::utils));
		ImgnVulkan::utils->CreateDefaultIncludeHandler(&ImgnVulkan::includeHandler);

		CreateInstance();
		SetupDebugMessenger();
		CreateSurface(pHWND);
		PickPhysicalDevice();
		CreateDevice();
		CreateSwapchain();
		CreateImageViews();
		CreateDescriptorSetLayout();
		CreateGraphicsPipelines();
		CreateCommandPool();
		CreateDepthResources();
		CreateTextureImage();
		CreateTextureImageView();
		CreateDefaultSampler();
		CreateVertexBuffer();
		CreateIndexBuffer();
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSets();
		CreateCommandBuffer();
		CreateSyncObjects();

		SetupDeferredRenderer();

		//GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE handle;
		//_gWin->GetWindowHandle(handle);

		//_gWin->Register()


		//_gui.Init(static_cast<HWND>(handle.window), _gWinW, _gWinH);
		////ImgnInput input(*_gWin);
		//_input = new ImgnInput(*_gWin);
		//uint32_t o;
		//_input->bufferedInput.Observers(o);

		//_gui.SetInput(&_input->bufferedInput);

		//_input->bufferedInput.Observers(o);
	}

	void Ctx::UploadMesh(const Vertex* pVertices, uint64_t pVertexCount)
	{
		_graph.AddResource("VertexBuffer", sizeof(Vertex) * pVertexCount, vk::BufferUsageFlagBits::eVertexBuffer, pVertices);
	}

	void Ctx::UploadIndices(const uint32_t * pIndices, uint64_t pIndexCount)
	{
		_graph.AddResource("IndexBuffer", sizeof(uint32_t) * pIndexCount, vk::BufferUsageFlagBits::eIndexBuffer, pIndices);
	}

	Image Ctx::CreateImage(const uint8_t* pImageData, uint32_t pWidth, uint32_t pHeight)
	{
		Buffer staging;

		uint64_t size = pWidth * pHeight * 4;
		CreateBuffer(size, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, staging);
		
		void* data = staging.memory.mapMemory(0, size);
		memcpy(data, pImageData, size);
		staging.memory.unmapMemory();

		Image image;
		CreateImage(pWidth, pHeight, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor, image);

		CopyBufferToImage(staging.buffer, image.image, pWidth, pHeight);
	
		return image;
	}

	Image Ctx::CreateImage(const std::string& pName, vk::Format pFormat, vk::Extent2D pExtent, vk::ImageUsageFlags pUsage, vk::ImageLayout pInitialLayout, vk::ImageLayout pFinalLayout, vk::ImageAspectFlags pAspect)
	{
		_graph.AddResource(pName, pFormat, pExtent, pUsage, pInitialLayout, pFinalLayout, pAspect);
		return std::move(_graph.GetImageResource(pName)->image);
	}

	Buffer& Ctx::CreateBuffer(const std::string& pName, vk::DeviceSize pSize, vk::BufferUsageFlags pUsage, const void* pData)
	{
		_graph.AddResource(pName, pSize, pUsage, pData);
		return _graph.GetBufferResource(pName)->buffer;
	}

	void Ctx::CompileGraph()
	{
		_graph.Compile();
	}

	void Ctx::DrawFrame()
	{
		auto fenceResult = ImgnVulkan::device.waitForFences(*ImgnVulkan::inFlightFences[ImgnVulkan::frameIdx], vk::True, UINT64_MAX);
		if (fenceResult != vk::Result::eSuccess) throw std::runtime_error("failed to wait for fence!");

		ImgnVulkan::device.resetFences(*ImgnVulkan::inFlightFences[ImgnVulkan::frameIdx]);

		auto [result, imageIndex] = ImgnVulkan::swapchain.acquireNextImage(UINT64_MAX, *ImgnVulkan::presentCompleteSemaphores[ImgnVulkan::frameIdx], nullptr);

		if (result == vk::Result::eErrorOutOfDateKHR)
		{
			RecreateSwapchain();
			return;
		}

		ImgnVulkan::commandBuffers[ImgnVulkan::frameIdx].reset();
		//ImgnVulkan::commandBuffers[ImgnVulkan::frameIdx].begin({});
		//UpdateUniformBuffer(ImgnVulkan::frameIdx);

		//pCommandBuffer.reset();
		ImgnVulkan::commandBuffers[ImgnVulkan::frameIdx].begin({});


		_graph.Execute(ImgnVulkan::commandBuffers[ImgnVulkan::frameIdx], *ImgnVulkan::queue);

		TransitionImageLayout(ImgnVulkan::commandBuffers[ImgnVulkan::frameIdx], ImgnVulkan::swapchainImages[imageIndex], vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

		//blit final color for now
		{
			vk::ImageSubresourceLayers subresource
			{
				.aspectMask = vk::ImageAspectFlagBits::eColor,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1
			};

			vk::ImageBlit blit
			{
				.srcSubresource = subresource,
				.dstSubresource = subresource,
			};

			vk::Offset3D blitOffset0{ 0, 0, 0 };
			//vk::Offset3D blitOffsetSrc1{ (int32_t)renderWidth, (int32_t)renderHeight, 1 };
			vk::Offset3D blitOffsetDst1{ (int32_t)ImgnVulkan::swapchainExtent.width, (int32_t)ImgnVulkan::swapchainExtent.height, 1 };
			blit.srcOffsets[0] = blitOffset0;
			blit.srcOffsets[1] = blitOffsetDst1;
			blit.dstOffsets[0] = blitOffset0;
			blit.dstOffsets[1] = blitOffsetDst1;

			auto finalColor = _graph.GetImageResource("FinalColor");
			ImgnVulkan::commandBuffers[ImgnVulkan::frameIdx].blitImage(finalColor->image.image, vk::ImageLayout::eTransferSrcOptimal, ImgnVulkan::swapchainImages[imageIndex], vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

			TransitionImageLayout(ImgnVulkan::commandBuffers[ImgnVulkan::frameIdx], ImgnVulkan::swapchainImages[imageIndex], vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::ePresentSrcKHR);
		}

		//ui
	/*	{
			vk::RenderingAttachmentInfo attachmentInfo
			{
				.imageView = ImgnVulkan::swapchainImageViews[ImgnVulkan::frameIdx],
				.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
				.loadOp = vk::AttachmentLoadOp::eLoad,
				.storeOp = vk::AttachmentStoreOp::eStore,
				.clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f)
			};

			vk::RenderingInfo guiRenderingInfo
			{
				.renderArea
				{
					.offset = { 0, 0 },
					.extent = ImgnVulkan::swapchainExtent
				},
				.layerCount = 1,
				.colorAttachmentCount = 1,
				.pColorAttachments = &attachmentInfo,
			};

			_gui.renderingInfo = guiRenderingInfo;
			_gui.DrawFrame(ImgnVulkan::commandBuffers[ImgnVulkan::frameIdx]);
		}*/
		//RecordCommandBuffer(imageIndex);
		ImgnVulkan::commandBuffers[ImgnVulkan::frameIdx].end();

		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

		const vk::SubmitInfo submitInfo
		{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &*ImgnVulkan::presentCompleteSemaphores[ImgnVulkan::frameIdx],
			.pWaitDstStageMask = &waitDestinationStageMask,
			.commandBufferCount = 1,
			.pCommandBuffers = &*ImgnVulkan::commandBuffers[ImgnVulkan::frameIdx],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &*ImgnVulkan::renderFinishedSemaphores[imageIndex]
		};

		ImgnVulkan::queue.submit(submitInfo, *ImgnVulkan::inFlightFences[ImgnVulkan::frameIdx]);

		const vk::PresentInfoKHR presentInfoKHR
		{
			.waitSemaphoreCount = 1,
			.pWaitSemaphores = &*ImgnVulkan::renderFinishedSemaphores[imageIndex],
			.swapchainCount = 1,
			.pSwapchains = &*ImgnVulkan::swapchain,
			.pImageIndices = &imageIndex
		};

		//result = Ctx::queue.presentKHR(presentInfoKHR);

		try
		{
			result = ImgnVulkan::queue.presentKHR(presentInfoKHR);

			if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)// || _framebufferResized)
			{
				//_framebufferResized = false;
				RecreateSwapchain();
			}
		}
		catch (const vk::OutOfDateKHRError&)
		{
			//_framebufferResized = false;
			RecreateSwapchain();
			return;
		}

		//if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR || _framebufferResized)
		//{
		//	_framebufferResized = false;
		//	RecreateSwapchain();
		//}

		ImgnVulkan::frameIdx = (ImgnVulkan::frameIdx + 1) % MAXFRAMESINFLIGHT;
	}
}