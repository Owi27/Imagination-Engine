#include "D:/GitHub/Imagination-Engine/Refactor/Imagination - Current/build/CMakeFiles/Imagination.dir/Debug/cmake_pch.hxx"
#include "RenderGraph.h"

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
	std::vector<std::vector<size_t>> dependencies(_passes.size());  // What each pass depends on
	std::vector<std::vector<size_t>> dependents(_passes.size());    // What depends on each pass

	// Track which pass produces each resource (write-after-write dependencies)
	std::unordered_map<std::string, size_t> resourceWriters;

	// Dependency Discovery Through Resource Usage Analysis
	// Analyze each pass to determine data flow relationships

	for (int i = 0; auto& pass : _passes)
	{
		for (const auto& output : pass.outputs)
		{
			resourceWriters[output] = i; //set the idx of the pass that writes the output
		}

		i++;
	}

	for (int i = 0; auto& pass : _passes)
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

		i++;
	}

	//for (int8_t i = 0; auto& pass : _passes)
	//{
	//    for (const auto& output : pass.outputs)
	//    {
	//        resourceWriters[output] = i++; //set the idx of the pass that writes the output
	//    }
	//}


	//for (size_t i = 0; i < _passes.size(); i++)
	//{
	//    const auto& pass = _passes[i];

	//    for (const auto& output : pass.outputs)
	//    {
	//        resourceWriters[output] = i; //set the idx of the pass that writes the output
	//    }
	//}

	//for (size_t i = 0; i < _passes.size(); ++i)
	//{
	//    const auto& pass = _passes[i];

	//    // Process input dependencies - this pass must wait for producers
	//    for (const auto& input : pass.inputs)
	//    {
	//        auto it = resourceWriters.find(input);
	//        if (it != resourceWriters.end())
	//        {
	//            // Found the pass that produces this input - create dependency link
	//            dependencies[i].push_back(it->second);      // This pass depends on the producer
	//            dependents[it->second].push_back(i);        // Producer has this as dependent
	//        }
	//    }

	//    // Register output production - subsequent passes may depend on these
	//    for (const auto& output : pass.outputs)
	//    {
	//        resourceWriters[output] = i;                    // Record this pass as producer
	//    }
	//}

	// Topological Sort for Optimal Execution Order
	// Use depth-first search to compute valid execution sequence while detecting cycles
	std::vector<bool> visited(_passes.size(), false);       // Track completed nodes
	std::vector<bool> inStack(_passes.size(), false);       // Track current recursion path

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
	for (size_t i = 0; i < _passes.size(); ++i)
	{
		if (!visited[i])
		{
			visit(i);
		}
	}

	auto& device = Device::Inst().GetDevice();
	// Automatic Synchronization Object Creation
	   // Generate semaphores for all dependencies identified during analysis
	for (size_t i = 0; i < _passes.size(); ++i)
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
}

void RenderGraph::Execute(vk::raii::CommandBuffer& pCommandBuffer, vk::Queue pQueue)
{
	// Execution state management for dynamic synchronization
	std::vector<vk::CommandBuffer> cmdBuffers;           // Command buffer storage
	std::vector<vk::Semaphore> waitSemaphores;           // Synchronization dependencies for current pass
	std::vector<vk::PipelineStageFlags> waitStages;      // Pipeline stages to wait on
	std::vector<vk::Semaphore> signalSemaphores;         // Semaphores to signal after current pass

	std::unordered_map<std::string, vk::ImageLayout> currentLayouts;
	for (auto& [handle, resource] : _resources)
	{
		currentLayouts[handle] = resource.initLayout;
	}

	pCommandBuffer.reset();
	pCommandBuffer.begin({});

	for (auto passIdx : _executionOrder)
	{
		const auto& pass = _passes[passIdx];

		for (auto& input : pass.inputs)
		{
			auto& resource = _resources[input];
			auto oldLayout = currentLayouts[input];
			auto newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

			if (oldLayout != newLayout)
			{
				TransitionImageLayout(pCommandBuffer, resource.image, oldLayout, newLayout, vk::AccessFlagBits2::eMemoryWrite, vk::AccessFlagBits2::eShaderRead, vk::PipelineStageFlagBits2::eAllCommands, vk::PipelineStageFlagBits2::eFragmentShader, resource.aspect);
				currentLayouts[input] = newLayout;
			}
		}

		for (auto& output : pass.outputs)
		{
			auto& resource = _resources[output];
			const vk::ImageLayout oldLayout = currentLayouts[output];
			const vk::ImageLayout newLayout = (resource.aspect & vk::ImageAspectFlagBits::eColor) == vk::ImageAspectFlagBits::eColor ? vk::ImageLayout::eColorAttachmentOptimal : vk::ImageLayout::eDepthStencilAttachmentOptimal;

			if (oldLayout != newLayout)
			{
				TransitionImageLayout(pCommandBuffer, resource.image, oldLayout, newLayout, vk::AccessFlagBits2::eMemoryRead, vk::AccessFlagBits2::eColorAttachmentWrite, vk::PipelineStageFlagBits2::eAllCommands, vk::PipelineStageFlagBits2::eColorAttachmentOutput, resource.aspect);
				currentLayouts[output] = newLayout;
			}
		}

		pass.Execute(pCommandBuffer);

		for (auto& output : pass.outputs)
		{
			auto& resource = _resources[output];
			const vk::ImageLayout oldLayout = currentLayouts[output];
			const vk::ImageLayout newLayout = resource.finalLayout;

			if (oldLayout != newLayout)
			{
				TransitionImageLayout(pCommandBuffer, resource.image, oldLayout, newLayout, vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2::eMemoryRead, vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eAllCommands, resource.aspect);
				currentLayouts[output] = newLayout;
			}

		}

	}

	pCommandBuffer.end();










































	// Ordered Pass Execution with Automatic Dependency Management
	   // Execute each pass in the computed dependency-safe order
	for (auto passIdx : _executionOrder)
	{
		const auto& pass = _passes[passIdx];

		// Synchronization Setup - Collect Dependencies for Current Pass
		// Determine what this pass must wait for before executing
		waitSemaphores.clear();
		waitStages.clear();

		for (size_t i = 0; i < _semaphoreSignalWaitPairs.size(); ++i)
		{
			if (_semaphoreSignalWaitPairs[i].second == passIdx)
			{
				// This pass depends on the completion of another pass
				waitSemaphores.push_back(_semaphores[i]);                           // Wait for dependency completion
				waitStages.push_back(vk::PipelineStageFlagBits::eColorAttachmentOutput);  // Wait at output stage
			}
		}

		// Collect semaphores that this pass will signal for dependent passes
		signalSemaphores.clear();
		for (size_t i = 0; i < _semaphoreSignalWaitPairs.size(); ++i)
		{
			if (_semaphoreSignalWaitPairs[i].first == passIdx)
			{
				// Other passes depend on this pass's completion
				signalSemaphores.push_back(_semaphores[i]);                         // Signal completion for dependents
			}
		}

		// Command Buffer Preparation and Resource Layout Transitions
		// Set up command recording and transition resources to appropriate layouts
		pCommandBuffer.begin({});                                                   // Begin command recording

		// Transition input resources to shader-readable layouts
		for (const auto& input : pass.inputs)
		{
			auto& resource = _resources[input];

			vk::ImageMemoryBarrier barrier;
			barrier.setOldLayout(resource.initLayout)                           // Current resource layout
				.setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)          // Target layout for reading
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)                // No queue family transfer
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setImage(resource.image)                                      // Target image
				.setSubresourceRange({ resource.aspect, 0, 1, 0, 1 })  // Full image range
				.setSrcAccessMask(vk::AccessFlagBits::eMemoryWrite)             // Previous write access
				.setDstAccessMask(vk::AccessFlagBits::eShaderRead);             // Required read access

			pCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eFragmentShader, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, barrier);

			// Insert pipeline barrier for safe layout transition
			//pCommandBuffer.pipelineBarrier(
			//    vk::PipelineStageFlagBits::eAllCommands,                           // Wait for all previous work
			//    vk::PipelineStageFlagBits::eFragmentShader,                        // Enable fragment shader access
			//    vk::DependencyFlagBits::eByRegion,                                 // Region-local dependency
			//    0, nullptr, 0, nullptr, 1, &barrier                               // Image barrier only
			//);
		}

		// Transition output resources to render target layouts
		for (const auto& output : pass.outputs)
		{
			auto& resource = _resources[output];

			vk::ImageMemoryBarrier barrier;
			barrier.setOldLayout(resource.initLayout)                           // Current layout state
				.setNewLayout(vk::ImageLayout::eColorAttachmentOptimal)         // Optimal for color output
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setImage(resource.image)
				.setSubresourceRange({ resource.aspect, 0, 1, 0, 1 })
				.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)              // Previous read access
				.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);   // Required write access

			// Insert barrier for safe transition to writable state
			pCommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::DependencyFlagBits::eByRegion, nullptr, nullptr, barrier);
		}

		// Pass Execution - Execute the Actual Rendering Logic
		// Call the user-provided rendering function with prepared command buffer
		pass.Execute(pCommandBuffer);                                           // Execute pass-specific rendering

		// Final Layout Transitions - Prepare Resources for Subsequent Use
		// Transition output resources to their final required layouts
		for (const auto& output : pass.outputs)
		{
			auto& resource = _resources[output];

			vk::ImageMemoryBarrier barrier;
			barrier.setOldLayout(vk::ImageLayout::eColorAttachmentOptimal)         // Current writable layout
				.setNewLayout(resource.finalLayout)                             // Required final layout
				.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
				.setImage(resource.image)
				.setSubresourceRange({ resource.aspect, 0, 1, 0, 1 })
				.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)    // Previous write operations
				.setDstAccessMask(vk::AccessFlagBits::eMemoryRead);             // Enable subsequent reads

			// Insert final barrier for layout transition
			pCommandBuffer.pipelineBarrier(
				vk::PipelineStageFlagBits::eColorAttachmentOutput,                 // After color writes complete
				vk::PipelineStageFlagBits::eAllCommands,                           // Before any subsequent work
				vk::DependencyFlagBits::eByRegion,
				nullptr, nullptr, barrier
			);
		}

		// Command Submission with Synchronization
		// Submit command buffer with appropriate dependency and signaling semaphores
		pCommandBuffer.end();                                                       // Finalize command recording

		vk::SubmitInfo submitInfo;
		submitInfo.setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))      // Dependencies to wait for
			.setPWaitSemaphores(waitSemaphores.data())                                 // Dependency semaphores
			.setPWaitDstStageMask(waitStages.data())                                   // Pipeline stages to wait at
			.setCommandBufferCount(1)                                                  // Single command buffer
			.setPCommandBuffers(&*pCommandBuffer)                                      // Command buffer to execute
			.setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()))  // Semaphores to signal
			.setPSignalSemaphores(signalSemaphores.data());                           // Signal semaphores

		pQueue.submit(1, &submitInfo, nullptr);                                              // Submit to GPU queue
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

	_resources[pName] = std::move(imageResource);

}

void RenderGraph::AddPass(const std::string& pName, const std::vector<std::string>& pInputs, const std::vector<std::string>& pOutputs, std::function<void(vk::raii::CommandBuffer&)> pExecute)
{
	_passes.push_back(Pass
		{
			.name = pName,
			.inputs = pInputs,
			.outputs = pOutputs,
			.Execute = pExecute
		});
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
