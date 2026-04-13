#include "D:/GitHub/Imagination-Engine/Refactor/Imagination - Current/build/CMakeFiles/Imagination.dir/Debug/cmake_pch.hxx"
#include "RenderGraph.h"

void RenderGraph::CreateImageResource(ImageResource& pResource)
{
	vk::ImageCreateInfo imageCreateInfo
	{
		.imageType = vk::ImageType::e2D,
		.format = pResource.format,
		.extent = {pResource.extent.width, pResource.extent.height, 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = vk::SampleCountFlagBits::e1,
		.tiling = vk::ImageTiling::eOptimal,
		.usage = pResource.usage,
		.sharingMode = vk::SharingMode::eExclusive
	};

	pResource.image = vk::raii::Image(Device::Inst().GetDevice(), imageCreateInfo);

	vk::MemoryRequirements memRequirements = pResource.image.getMemoryRequirements();
	vk::MemoryAllocateInfo allocInfo
	{
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
	};

	pResource.memory = vk::raii::DeviceMemory(Device::Inst().GetDevice(), allocInfo);
	pResource.image.bindMemory(pResource.memory, 0);

	vk::ImageViewCreateInfo imageViewCreateInfo
	{
		.image = pResource.image,
		.viewType = vk::ImageViewType::e2D,
		.format = pResource.format,
		.components
		{
			.r = vk::ComponentSwizzle::eIdentity,
			.g = vk::ComponentSwizzle::eIdentity,
			.b = vk::ComponentSwizzle::eIdentity,
			.a = vk::ComponentSwizzle::eIdentity
		},
		.subresourceRange
		{
			.aspectMask = pResource.aspect,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	pResource.view = vk::raii::ImageView(Device::Inst().GetDevice(), imageViewCreateInfo);
}

void RenderGraph::CreateBufferResource(BufferResource& pResource)
{
	vk::BufferCreateInfo bufferInfo
	{
		.size = pResource.size,
		.usage = pResource.usage | vk::BufferUsageFlagBits::eTransferDst,
		.sharingMode = vk::SharingMode::eExclusive
	};

	pResource.buffer = vk::raii::Buffer(Device::Inst().GetDevice(), bufferInfo);

	vk::MemoryRequirements memReqs = pResource.buffer.getMemoryRequirements();
	vk::MemoryAllocateInfo memoryAllocateInfo{ .allocationSize = memReqs.size, .memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, (pResource.usage & vk::BufferUsageFlagBits::eUniformBuffer) ? vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent : vk::MemoryPropertyFlagBits::eDeviceLocal)};

	pResource.memory = vk::raii::DeviceMemory(Device::Inst().GetDevice(), memoryAllocateInfo);

	pResource.buffer.bindMemory(*pResource.memory, 0);

	// Initialize sync state
	pResource.currentAccess = vk::AccessFlagBits2::eNone;
	pResource.currentStage = vk::PipelineStageFlagBits2::eTopOfPipe;
}

uint32_t RenderGraph::FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps)
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

void RenderGraph::TransitionImageLayout(vk::raii::CommandBuffer& pCommandBuffer, const vk::raii::Image& pImage, vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout, vk::AccessFlags2 pSrcAccessMask, vk::AccessFlags2 pDstAccessMask, vk::PipelineStageFlags2 pSrcStageMask, vk::PipelineStageFlags2 pDstStageMask, vk::ImageAspectFlags pImageAspectFlags)
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

	pCommandBuffer.pipelineBarrier2(dependencyInfo);
}

void RenderGraph::Compile()
{
	std::vector<std::vector<size_t>> dependencies(_renderPasses.size());  // What each pass depends on
	std::vector<std::vector<size_t>> dependents(_renderPasses.size());    // What depends on each pass

	// Track which pass produces each resource (write-after-write dependencies)
	std::unordered_map<std::string, size_t> resourceWriters;

	// Dependency Discovery Through Resource Usage Analysis
	// Analyze each pass to determine data flow relationships
	for (int i = 0; auto& pass : _renderPasses)
	{
		for (const auto& output : pass.outputs)
		{
			resourceWriters[output] = i; //set the idx of the pass that writes the output
		}
		
		for (const auto& output : pass.bufferOutputs)
		{
			resourceWriters[output] = i; //set the idx of the pass that writes the output
		}

		i++;
	}

	for (int i = 0; auto& pass : _renderPasses)
	{
		for (const auto& input : pass.inputs)
		{
			auto it = resourceWriters.find(input);
			if (it != resourceWriters.end())
			{
				// Found the pass that produces this input - create dependency link
				dependencies[i].push_back(it->second);      // This pass depends on the producer
				dependents[it->second].push_back(i);        // Producer has this as dependent
			}
		}

		for (const auto& input : pass.bufferInputs)
		{
			auto it = resourceWriters.find(input);
			if (it != resourceWriters.end())
			{
				// Found the pass that produces this input - create dependency link
				dependencies[i].push_back(it->second);      // This pass depends on the producer
				dependents[it->second].push_back(i);        // Producer has this as dependent
			}
		}

		i++;
	}

	// Topological Sort for Optimal Execution Order
	// Use depth-first search to compute valid execution sequence while detecting cycles
	std::vector<bool> visited(_renderPasses.size(), false);       // Track completed nodes
	std::vector<bool> inStack(_renderPasses.size(), false);       // Track current recursion path

	std::function<void(size_t)> visit = [&](size_t node)
		{
			if (inStack[node])
			{
				// Cycle detection - circular dependency found
				throw std::runtime_error("Cycle detected in rendergraph");
			}

			if (visited[node])
			{
				return;  // Already processed this node and its dependencies
			}

			inStack[node] = true;   // Mark as currently being processed

			// Recursively process all dependent passes first (post-order traversal)
			for (auto dependency : dependencies[node])
			{
				visit(dependency);
			}

			inStack[node] = false;  // Remove from current path
			visited[node] = true;   // Mark as completely processed
			_executionOrder.push_back(node);  // Add to execution sequence
		};

	// Process all unvisited nodes to handle disconnected graph components
	for (size_t i = 0; i < _renderPasses.size(); ++i)
	{
		if (!visited[i])
		{
			visit(i);
		}
	}

	auto& device = Device::Inst().GetDevice();
	// Automatic Synchronization Object Creation
	   // Generate semaphores for all dependencies identified during analysis
	for (size_t i = 0; i < _renderPasses.size(); ++i)
	{
		for (auto dep : dependencies[i])
		{
			// Create a GPU semaphore for this dependency relationship
			// The dependent pass will wait on this semaphore before executing
			_semaphores.emplace_back(device.createSemaphore({}));
			_semaphoreSignalWaitPairs.emplace_back(dep, i);    // (producer, consumer) pair
		}
	}

	// Physical Resource Allocation and Creation
	// Transform resource descriptions into actual GPU objects
	for (auto& [name, resource] : _resources)
	{
		// Configure image creation parameters based on resource description
		vk::ImageCreateInfo imageCreateInfo
		{
			.imageType = vk::ImageType::e2D,
			.format = resource.format,
			.extent = {resource.extent.width , resource.extent.height, 1},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = vk::SampleCountFlagBits::e1,
			.tiling = vk::ImageTiling::eOptimal,
			.usage = resource.usage,
			.sharingMode = vk::SharingMode::eExclusive,
			.initialLayout = vk::ImageLayout::eUndefined
		};

		resource.image = device.createImage(imageCreateInfo);

		vk::MemoryRequirements memRequirements = resource.image.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo
		{
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
		};

		resource.memory = vk::raii::DeviceMemory(device, allocInfo);
		resource.image.bindMemory(resource.memory, 0);

		//image view
		vk::ImageViewCreateInfo imageViewCreateInfo
		{
			.image = resource.image,
			.viewType = vk::ImageViewType::e2D,
			.format = resource.format,
			.subresourceRange
			{
				.aspectMask = resource.aspect,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		resource.view = device.createImageView(imageViewCreateInfo);
	}

	for (auto& [name, resource] : _bufferResources)
	{
		if (!*resource.buffer) CreateBufferResource(resource);
	}
}

void RenderGraph::Execute(vk::raii::CommandBuffer& pCommandBuffer, vk::Queue pQueue)
{
	// Execution state management for dynamic synchronization
	std::vector<vk::CommandBuffer> cmdBuffers;           // Command buffer storage
	std::vector<vk::Semaphore> waitSemaphores;           // Synchronization dependencies for current pass
	std::vector<vk::PipelineStageFlags> waitStages;      // Pipeline stages to wait on
	std::vector<vk::Semaphore> signalSemaphores;         // Semaphores to signal after current pass

	for (auto passIdx : _executionOrder)
	{
		const auto& pass = _renderPasses[passIdx];

		for (auto& input : pass.bufferInputs)
		{
			BufferResource& resource = _bufferResources[input];

			vk::BufferMemoryBarrier2 barrier
			{
				.srcStageMask = resource.currentStage,
				.srcAccessMask = resource.currentAccess,
				.dstStageMask = vk::PipelineStageFlagBits2::eAllGraphics,
				.dstAccessMask = vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eUniformRead,
				.buffer = *resource.buffer,
				.offset = 0,
				.size = VK_WHOLE_SIZE
			};

			pCommandBuffer.pipelineBarrier2(vk::DependencyInfo{ .bufferMemoryBarrierCount = 1, .pBufferMemoryBarriers = &barrier });

			resource.currentStage = vk::PipelineStageFlagBits2::eAllGraphics;
			resource.currentAccess = vk::AccessFlagBits2::eShaderRead | vk::AccessFlagBits2::eUniformRead;
		}


		for (auto& input : pass.inputs)
		{
			auto& resource = _resources[input];
			if (resource.currentLayout != vk::ImageLayout::eShaderReadOnlyOptimal)
			{
				TransitionImageLayout(pCommandBuffer, resource.image, resource.currentLayout, vk::ImageLayout::eShaderReadOnlyOptimal, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2::eShaderRead, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eFragmentShader, resource.aspect);
				resource.currentLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
			}
		}

		for (auto& output : pass.outputs)
		{
			auto& resource = _resources[output];
			vk::ImageLayout target = (resource.aspect & vk::ImageAspectFlagBits::eColor) ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::eDepthStencilAttachmentOptimal;

			if (resource.currentLayout != target)
			{
				TransitionImageLayout(pCommandBuffer, resource.image, resource.currentLayout, target, vk::AccessFlagBits2::eNone, vk::AccessFlagBits2::eColorAttachmentWrite, vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput, resource.aspect);
				resource.currentLayout = target;
			}
		}

		pass.Execute(pCommandBuffer);

		for (auto& output : pass.bufferOutputs)
		{
			BufferResource& resource = _bufferResources[output];

			vk::BufferMemoryBarrier2 barrier
			{
				.srcStageMask = resource.currentStage,
				.srcAccessMask = resource.currentAccess,
				.dstStageMask = vk::PipelineStageFlagBits2::eAllGraphics,
				.dstAccessMask = vk::AccessFlagBits2::eShaderWrite,
				.buffer = *resource.buffer,
				.offset = 0,
				.size = VK_WHOLE_SIZE
			};

			pCommandBuffer.pipelineBarrier2(vk::DependencyInfo{ .bufferMemoryBarrierCount = 1, .pBufferMemoryBarriers = &barrier });

			resource.currentStage = vk::PipelineStageFlagBits2::eComputeShader;
			resource.currentAccess = vk::AccessFlagBits2::eShaderWrite;
		}
	}

	for (auto& [name, resource] : _resources)
	{
		if (resource.currentLayout != resource.finalLayout)
		{
			TransitionImageLayout(pCommandBuffer, resource.image, resource.currentLayout, resource.finalLayout, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2::eMemoryRead, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eAllCommands, resource.aspect);
			resource.currentLayout = resource.finalLayout;
		}
	}
}

void RenderGraph::RenderFrame(vk::Queue pGraphicsQueue, vk::Queue pPresentQueue)
{// Synchronize with previous frame completion
	// Prevent CPU from submitting work faster than GPU can process it
	//vk::Result result = device.waitForFences(1, &*inFlightFence, VK_TRUE, UINT64_MAX);

	// Reset fence for this frame's completion tracking
	// Prepare the fence to signal when this frame's GPU work completes
   // device.resetFences(1, &*inFlightFence);
}

void RenderGraph::AddResource(const std::string& pName, vk::Format pFormat, vk::Extent2D pExtent, vk::ImageUsageFlags pUsage, vk::ImageLayout pInitialLayout, vk::ImageLayout pFinalLayout, vk::ImageAspectFlags pAspect)
{
	ImageResource imageResource
	{
		.name = pName,
		.format = pFormat,
		.extent = pExtent,
		.usage = pUsage,
		.initLayout = pInitialLayout,
		.finalLayout = pFinalLayout,
		.aspect = pAspect
	};

	CreateImageResource(imageResource);

	_resources[pName] = std::move(imageResource);

}

void RenderGraph::AddResource(const std::string& pName, vk::DeviceSize pSize, vk::BufferUsageFlags pUsage, void* pData)
{
	BufferResource bufferResource
	{
		.name = pName,
		.size = pSize,
		.usage = pUsage
	};

	vk::BufferCreateInfo stagingInfo
	{
		.size = pSize,
		.usage = vk::BufferUsageFlagBits::eTransferSrc,
		.sharingMode = vk::SharingMode::eExclusive
	};

	vk::raii::Buffer stagingBuffer(Device::Inst().GetDevice(), stagingInfo);
	vk::MemoryRequirements memRequirementsStaging = stagingBuffer.getMemoryRequirements();
	vk::MemoryAllocateInfo memoryAllocateInfoStaging
	{
		.allocationSize = memRequirementsStaging.size,
		.memoryTypeIndex = FindMemoryType(memRequirementsStaging.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
	};

	vk::raii::DeviceMemory stagingBufferMemory(Device::Inst().GetDevice(), memoryAllocateInfoStaging);

	stagingBuffer.bindMemory(stagingBufferMemory, 0);
	void* dataStaging = stagingBufferMemory.mapMemory(0, stagingInfo.size);
	memcpy(dataStaging, pData, stagingInfo.size);
	stagingBufferMemory.unmapMemory();

	CreateBufferResource(bufferResource);

	vk::raii::CommandBuffer commandCopyBuffer = Device::Inst().BeginSingleTimeCommand();

	commandCopyBuffer.copyBuffer(stagingBuffer, bufferResource.buffer, vk::BufferCopy(0, 0, pSize));

	Device::Inst().EndSingleTimeCommand(commandCopyBuffer);

	bufferResource.currentStage = vk::PipelineStageFlagBits2::eTransfer;
	bufferResource.currentAccess = vk::AccessFlagBits2::eTransferWrite;

	_bufferResources[pName] = std::move(bufferResource);
}

void RenderGraph::AddPass(const std::string& pName, const std::vector<std::string>& pInputs, const std::vector<std::string>& pOutputs, std::function<void(vk::raii::CommandBuffer&)> pExecute)
{
	//_passes.push_back(Pass
	//	{
	//		.name = pName,
	//		.inputs = pInputs,
	//		.outputs = pOutputs,
	//		.Execute = pExecute
	//	});
}

//void RenderPass::BuildPipeline(const std::string& pVertex, const std::string& pFragment)
//{
//	vk::raii::ShaderModule vertexSM = CreateShaderModule(Device::Inst().MakeSPV(pVertex, VertexTarget)));
//	vk::raii::ShaderModule fragmentSM = CreateShaderModule(Device::Inst().MakeSPV(pFragment, FragmentTarget));
//
//	vk::PipelineShaderStageCreateInfo vertShaderStageInfo
//	{
//		.stage = vk::ShaderStageFlagBits::eVertex,
//		.module = vertexSM,
//		.pName = "main"
//	};
//
//	vk::PipelineShaderStageCreateInfo fragShaderStageInfo
//	{
//		.stage = vk::ShaderStageFlagBits::eFragment,
//		.module = fragmentSM,
//		.pName = "main"
//	};
//
//	std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = { vertShaderStageInfo, fragShaderStageInfo };
//
//	auto bindingDesc = Vertex::GetBindingDescription();
//	auto attributeDesc = Vertex::GetAttributeDescriptions();
//
//	vk::PipelineVertexInputStateCreateInfo vertexInputInfo
//	{
//		.vertexBindingDescriptionCount = 1,
//		.pVertexBindingDescriptions = &bindingDesc,
//		.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDesc.size()),
//		.pVertexAttributeDescriptions = attributeDesc.data()
//	};
//
//	vk::PipelineInputAssemblyStateCreateInfo inputAssembly
//	{
//		.topology = vk::PrimitiveTopology::eTriangleList
//	};
//
//	vk::PipelineViewportStateCreateInfo viewportState
//	{
//		.viewportCount = 1,
//		.scissorCount = 1
//	};
//
//	vk::PipelineRasterizationStateCreateInfo rasterizer
//	{
//		.depthClampEnable = vk::False,
//		.rasterizerDiscardEnable = vk::False,
//		.polygonMode = vk::PolygonMode::eFill,
//		.cullMode = vk::CullModeFlagBits::eBack,
//		.frontFace = vk::FrontFace::eClockwise,
//		.depthBiasEnable = vk::False,
//		.depthBiasSlopeFactor = 1.0f,
//		.lineWidth = 1.0f
//	};
//
//	vk::PipelineMultisampleStateCreateInfo multisampling
//	{
//		.rasterizationSamples = vk::SampleCountFlagBits::e1,
//		.sampleShadingEnable = vk::False
//	};
//
//	vk::PipelineDepthStencilStateCreateInfo depthStencil
//	{
//		.depthTestEnable = vk::True,
//		.depthWriteEnable = vk::True,
//		.depthCompareOp = vk::CompareOp::eLess,
//		.depthBoundsTestEnable = vk::False,
//		.stencilTestEnable = vk::False
//	};
//
//	vk::PipelineColorBlendAttachmentState colorBlendAttachment
//	{
//		.blendEnable = vk::False,
//		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
//	};
//
//	std::vector< vk::PipelineColorBlendAttachmentState> blendStates;
//	if (colorAttachments.size() > 1)
//	{
//		for (size_t i = 0; i < colorAttachments.size(); i++)
//		{
//			blendStates.push_back(colorBlendAttachment);
//		}
//	}
//
//	vk::PipelineColorBlendStateCreateInfo colorBlending
//	{
//		.logicOpEnable = vk::False,
//		.logicOp = vk::LogicOp::eCopy,
//		.attachmentCount = colorAttachments.size() > 1 ? blendStates.size() : 1,
//		.pAttachments = colorAttachments.size() > 1 ? blendStates.data() : &colorBlendAttachment
//	};
//
//	std::vector dynamicStates =
//	{
//		vk::DynamicState::eViewport,
//		vk::DynamicState::eScissor
//	};
//
//	vk::PipelineDynamicStateCreateInfo dynamicState
//	{
//		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
//		.pDynamicStates = dynamicStates.data()
//	};
//
//	vk::PipelineLayoutCreateInfo pipelineLayoutInfo
//	{
//		.setLayoutCount = 1,
//		.pSetLayouts = &descriptorSetLayout,
//		.pushConstantRangeCount = 0
//	};
//
//	pipelineLayout = vk::raii::PipelineLayout(Device::Inst().GetDevice(), pipelineLayoutInfo);
//
//	vk::Format depthFormat = vk::Format::eD32Sfloat;
//
//	vk::PipelineRenderingCreateInfo pipelineRenderingCreateInfo
//	{
//		.colorAttachmentCount = colorAttachments.size(),
//		.pColorAttachmentFormats = colorAttachments.data(),
//		.depthAttachmentFormat = depthFormat
//	};
//
//	vk::GraphicsPipelineCreateInfo pipelineInfo
//	{
//		.pNext = &pipelineRenderingCreateInfo,
//		.stageCount = 2,
//		.pStages = shaderStages.data(),
//		.pVertexInputState = &vertexInputInfo,
//		.pInputAssemblyState = &inputAssembly,
//		.pViewportState = &viewportState,
//		.pRasterizationState = &rasterizer,
//		.pMultisampleState = &multisampling,
//		.pDepthStencilState = &depthStencil,
//		.pColorBlendState = &colorBlending,
//		.pDynamicState = &dynamicState,
//		.layout = pipelineLayout,
//		.renderPass = nullptr
//	};
//
//	pipeline = vk::raii::Pipeline(Device::Inst().GetDevice(), nullptr, pipelineInfo);
//}
//
//vk::raii::ShaderModule RenderPass::CreateShaderModule(const std::vector<uint32_t>& pCode) const
//{
//	vk::ShaderModuleCreateInfo createInfo
//	{
//		.codeSize = pCode.size() * sizeof(uint32_t),
//		.pCode = pCode.data()
//	};
//
//	vk::raii::ShaderModule shaderModule(Device::Inst().GetDevice(), createInfo);
//
//	return shaderModule;
//}
