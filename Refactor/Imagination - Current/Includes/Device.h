#pragma once

#include <wrl/client.h>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
using namespace Microsoft::WRL;

class Device
{
    static inline std::unique_ptr<Device> _instance = nullptr;
    
    std::optional<vk::raii::Device> _device;
    std::optional<vk::raii::PhysicalDevice> _physicalDevice;
    std::optional<vk::raii::CommandPool> _commandPool;
    std::optional<vk::raii::Queue> _queue;
    EventBus _eventBus;

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

    vk::raii::Device& GetDevice() { return _device.value(); }
    vk::raii::PhysicalDevice& GetPhysicalDevice() { return _physicalDevice.value(); }
    vk::raii::CommandPool& GetCommandPool() { return _commandPool.value(); }
    vk::raii::Queue& GetQueue() { return _queue.value(); }
    EventBus& GetEventBus() { return _eventBus; }
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


};
