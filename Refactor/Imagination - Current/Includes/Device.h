#pragma once

#include <wrl/client.h>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
using namespace Microsoft::WRL;

struct Buffer
{
	vk::raii::Buffer buffer = nullptr;
	vk::raii::DeviceMemory memory = nullptr;
};

struct Image
{
	vk::raii::Image image = nullptr;
	vk::raii::ImageView imageView = nullptr;
	vk::raii::DeviceMemory memory = nullptr;
};

class Device
{
    static inline std::unique_ptr<Device> _instance = nullptr;
    
    std::optional<vk::raii::Instance> _vkInstance;
    std::optional<vk::raii::Device> _device;
    std::optional<vk::raii::PhysicalDevice> _physicalDevice;
    std::optional<vk::raii::CommandPool> _commandPool;
    std::optional<vk::raii::Queue> _queue;
    //EventBus _eventBus;

    //dxc
    ComPtr<IDxcCompiler3> _compiler;
    ComPtr<IDxcUtils> _utils;
    ComPtr<IDxcIncludeHandler> _includeHandler;

    Device() = default;

public:
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    static Device& Inst()
    {
        if (!_instance)
            _instance.reset(new Device());

        return *_instance;
    }

    vk::raii::Instance& GetVkInstance() { return _vkInstance.value(); }
    vk::raii::Device& GetDevice() { return _device.value(); }
    vk::raii::PhysicalDevice& GetPhysicalDevice() { return _physicalDevice.value(); }
    vk::raii::CommandPool& GetCommandPool() { return _commandPool.value(); }
    vk::raii::Queue& GetQueue() { return _queue.value(); }
    //EventBus& GetEventBus() { return _eventBus; }
    const vk::raii::Instance& GetVkInstance() const { return _vkInstance.value(); }
    const vk::raii::Device& GetDevice() const { return _device.value(); }
    const vk::raii::PhysicalDevice& GetPhysicalDevice() const { return _physicalDevice.value(); }
    const vk::raii::CommandPool& GetCommandPool() const { return _commandPool.value(); }
    const vk::raii::Queue& GetQueue() const { return _queue.value(); }

    void InitDXC()
    {
        DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler));
        DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils));
        _utils->CreateDefaultIncludeHandler(&_includeHandler);
    }

    std::vector<uint32_t> MakeSPV(const std::string& pShader, const std::wstring& pTarget, const std::wstring& pEntryPoint = L"main")
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
		_compiler->Compile(&sourceBuffer, args.data(), static_cast<uint32_t>(args.size()), _includeHandler.Get(), IID_PPV_ARGS(&result));

		//check for compilation errors
		ComPtr<IDxcBlobUtf8> errors;
		if (SUCCEEDED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr)) && errors && errors->GetStringLength() > 0)
		{
			std::stringstream ss;
			ss << "Shader compilation errors : " << errors->GetStringPointer();
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


    void SetInstance(vk::raii::Instance&& instance)
    {
        _vkInstance.emplace(std::move(instance));
    }

	void SetDevice(vk::raii::Device&& device)
    {
        _device.emplace(std::move(device));
    }

    void SetPhysicalDevice(vk::raii::PhysicalDevice&& physicalDevice)
    {
        _physicalDevice.emplace(std::move(physicalDevice));
    }

    void SetCommandPool(vk::raii::CommandPool&& commandPool)
    {
        _commandPool.emplace(std::move(commandPool));
    }

    void SetQueue(vk::raii::Queue&& queue)
    {
        _queue.emplace(std::move(queue));
    }

	vk::raii::CommandBuffer BeginSingleTimeCommand()
	{
		vk::CommandBufferAllocateInfo allocInfo
		{
			.commandPool = *_commandPool,
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = 1
		};

		auto commandBuffer= std::move(_device->allocateCommandBuffers(allocInfo).front());

		vk::CommandBufferBeginInfo beginInfo
		{
			.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
		};

		commandBuffer.begin(beginInfo);

		return commandBuffer;
	}

	void EndSingleTimeCommand(vk::raii::CommandBuffer& pCommandBuffer)
	{
		pCommandBuffer.end();

		vk::SubmitInfo submitInfo
		{
			.commandBufferCount = 1,
			.pCommandBuffers = &*pCommandBuffer
		};

		_queue->submit(submitInfo, nullptr);
		_queue->waitIdle();
	}

	uint32_t FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps)
	{
		vk::PhysicalDeviceMemoryProperties memProperties = _physicalDevice->getMemoryProperties();

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((pTypeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & pProps) == pProps)
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	void CreateImage(uint32_t pWidth, uint32_t pHeight, vk::Format pFormat, vk::ImageTiling pTiling, vk::ImageUsageFlags pUsage, vk::MemoryPropertyFlags pProps, Image& pImage)
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

		pImage.image = vk::raii::Image(*_device, imageCreateInfo);

		vk::MemoryRequirements memRequirements = pImage.image.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo
		{
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, pProps)
		};

		pImage.memory = vk::raii::DeviceMemory(*_device, allocInfo);
		pImage.image.bindMemory(pImage.memory, 0);
	}

	void CreateImageView(Image& pImage, vk::Format pFormat, vk::ImageAspectFlags pAspectFlags)
	{
		vk::ImageViewCreateInfo imageViewCreateInfo
		{
			.image = pImage.image,
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

		pImage.imageView = vk::raii::ImageView(*_device, imageViewCreateInfo);
	}

	void CreateBuffer(vk::DeviceSize pSize, vk::BufferUsageFlags pUsage, vk::MemoryPropertyFlags pProps, Buffer& pBuffer)
	{
		vk::BufferCreateInfo bufferCreateInfo
		{
			.size = pSize,
			.usage = pUsage,
			.sharingMode = vk::SharingMode::eExclusive
		};

		pBuffer.buffer = vk::raii::Buffer(*_device, bufferCreateInfo);

		vk::MemoryRequirements memReqs = pBuffer.buffer.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo
		{
			.allocationSize = memReqs.size,
			.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, pProps)
		};

		pBuffer.memory = vk::raii::DeviceMemory(*_device, allocInfo);

		pBuffer.buffer.bindMemory(*pBuffer.memory, 0);
	}

	void CopyBufferToImage(const vk::raii::Buffer& pBuffer, vk::raii::Image& pImage, uint32_t pWidth, uint32_t pHeight)
	{
		vk::raii::CommandBuffer commandBuffer = BeginSingleTimeCommand();

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

		EndSingleTimeCommand(commandBuffer);
	}

	void CopyBuffer(vk::raii::Buffer& pSrc, vk::raii::Buffer& pDst, vk::DeviceSize pSize)
	{
		vk::raii::CommandBuffer commandCopyBuffer = BeginSingleTimeCommand();

		commandCopyBuffer.copyBuffer(pSrc, pDst, vk::BufferCopy(0, 0, pSize));

		EndSingleTimeCommand(commandCopyBuffer);
	}

	void TransitionImageLayout(const vk::raii::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout)
	{
		vk::raii::CommandBuffer commandBuffer = BeginSingleTimeCommand();

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
		
		EndSingleTimeCommand(commandBuffer);
	}

};
