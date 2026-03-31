#pragma once

class Device
{
    std::optional<vk::raii::Device> _device;
    std::optional<vk::raii::PhysicalDevice> _physicalDevice;
    std::optional<vk::raii::CommandPool> _commandPool;
    std::optional<vk::raii::Queue> _queue;
    EventBus _eventBus;

    static inline std::unique_ptr<Device> _instance = nullptr;

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

	vk::CommandBuffer BeginSingleTimeCommand()
	{
		vk::CommandBufferAllocateInfo allocInfo
		{
			.commandPool = *_commandPool,
			.level = vk::CommandBufferLevel::ePrimary,
			.commandBufferCount = 1
		};

		auto commandBuffer = std::move(_device->allocateCommandBuffers(allocInfo).front());

		vk::CommandBufferBeginInfo beginInfo
		{
			.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit
		};

		commandBuffer.begin(beginInfo);

		return commandBuffer;
	}

	void EndSingleTimeCommand(vk::CommandBuffer& pCommandBuffer)
	{
		pCommandBuffer.end();

		vk::SubmitInfo submitInfo
		{
			.commandBufferCount = 1,
			.pCommandBuffers = &pCommandBuffer
		};

		_queue->submit(submitInfo, nullptr);
		_queue->waitIdle();
	}
};
