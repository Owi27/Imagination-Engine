#include "pch.cpp"
#include "ResourceTypes.h"

void Texture::CreateImage(uint8_t* pData)
{
	auto& device = *Device::Inst().GetDevice();

	vk::Format format;

	switch (channels)
	{
	case 1:
		format = vk::Format::eR8Srgb;
		break;
	case 2:
		format = vk::Format::eR8G8Srgb;
		break;
	case 3:
		channels = 4;
	case 4:
		format = vk::Format::eR8G8B8A8Srgb;
		break;
	}

	vk::DeviceSize imageSize = width * height * channels;

	vk::Buffer stagingBuffer;
	vk::DeviceMemory stagingMemory;

	//staging
	{
		vk::BufferCreateInfo stagingCreateInfo
		{
			.size = imageSize,
			.usage = vk::BufferUsageFlagBits::eTransferSrc,
			.sharingMode = vk::SharingMode::eExclusive
		};

		stagingBuffer = device.createBuffer(stagingCreateInfo);

		vk::MemoryRequirements memReqs = device.getBufferMemoryRequirements(stagingBuffer);// .getMemoryRequirements();// .getBufferMemoryRequirements(stagingBuffer);
		vk::MemoryAllocateInfo stagingAllocateInfo
		{
			.allocationSize = memReqs.size,
			.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
		};

		stagingMemory = device.allocateMemory(stagingAllocateInfo);
		device.bindBufferMemory(stagingBuffer, stagingMemory, 0);
		//device.bindBufferMemory(stagingBuffer, stagingMemory, 0);

		
		void* data = device.mapMemory(stagingMemory, 0, imageSize);  //stagingMemory.mapMemory(0, imageSize);
		memcpy(data, pData, imageSize);
		device.unmapMemory(stagingMemory);
		//stagingMemory.unmapMemory();
	}

	stbi_image_free(pData);

	vk::ImageCreateInfo imageCreateInfo
	{
		.imageType = vk::ImageType::e2D,
		.format = format,
		.extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = vk::SampleCountFlagBits::e1,
		.tiling = vk::ImageTiling::eOptimal,
		.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		.sharingMode = vk::SharingMode::eExclusive
	};

	_image = device.createImage(imageCreateInfo);
	
	vk::MemoryRequirements memReqs = device.getImageMemoryRequirements(_image);
	vk::MemoryAllocateInfo memoryAllocateInfo
	{
		.allocationSize = memReqs.size,
		.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
	};

	_memory = device.allocateMemory(memoryAllocateInfo);
	device.bindImageMemory(_image, _memory, 0);

	TransitionImageLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	//copy buffer to image
	{
		auto commandBuffer = Device::Inst().BeginSingleTimeCommand();

		vk::BufferImageCopy region
		{
			.bufferOffset = 0,
			.bufferRowLength = 0,
			.bufferImageHeight = 0,
			.imageSubresource = { vk::ImageAspectFlagBits::eColor, 0, 0, 1 },
			.imageOffset = {0, 0, 0},
			.imageExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1}
		};

		commandBuffer.copyBufferToImage(stagingBuffer, _image, vk::ImageLayout::eTransferDstOptimal, { region });

		Device::Inst().EndSingleTimeCommand(commandBuffer);
	}
	//
	// CopyBufferToImage(staging.buffer, _texture.image, static_cast<uint32_t>(width), static_cast<uint32_t>(height));
	TransitionImageLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	//sampler
	vk::PhysicalDeviceProperties properties = Device::Inst().GetPhysicalDevice().getProperties();

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

	_sampler = device.createSampler(samplerInfo);
}

uint32_t Texture::FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps)
{
	return 0;
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

void Texture::TransitionImageLayout(vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout)
{
	vk::raii::CommandBuffer commandBuffer = Device::Inst().BeginSingleTimeCommand();

	vk::ImageMemoryBarrier barrier
	{
		.oldLayout = pOldLayout,
		.newLayout = pNewLayout,
		.image = _image,
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

	Device::Inst().EndSingleTimeCommand(commandBuffer);
}

void Mesh::LoadModel(const std::string& pFile, std::vector<Vertex>& pVertices, std::vector<uint32_t>& pIndices)
{
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, pFile);
	//bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename); // for binary glTF(.glb)

	if (!warn.empty()) std::format("Warn: {}\n", warn);

	if (!err.empty()) std::format("Err: {}\n", err);

	if (!ret) std::format("Failed to parse glTF: {}\n", pFile);

	std::unordered_map<Vertex, uint32_t> uniqueVertices;

	for (auto& mesh : model.meshes)
	{
		for (auto& primitive : mesh.primitives)
		{
			//pos
			const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
			const tinygltf::BufferView& posBufferView = model.bufferViews[posAccessor.bufferView];
			const tinygltf::Buffer& posBuffer = model.buffers[posBufferView.buffer];
			const uint64_t posStride = posAccessor.ByteStride(posBufferView);


			const tinygltf::Accessor* nrmAcc = primitive.attributes.contains("NORMAL") ? &model.accessors[primitive.attributes.at("NORMAL")] : nullptr;
			const tinygltf::Accessor* uvAcc = primitive.attributes.contains("TEXCOORD_0") ? &model.accessors[primitive.attributes.at("TEXCOORD_0")] : nullptr;
			const tinygltf::Accessor* tanAcc = primitive.attributes.contains("TANGENT") ? &model.accessors[primitive.attributes.at("TANGENT")] : nullptr;

			std::vector<uint32_t> primitiveToIndexMap;

			for (size_t i = 0; i < posAccessor.count; i++)
			{
				Vertex v{};

				//pos
				const float* p = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset + (i * posStride)]);
				v.pos = { p[0], p[1], p[2] };

				//nrm
				if (nrmAcc)
				{
					const auto& view = model.bufferViews[nrmAcc->bufferView];
					const float* n = reinterpret_cast<const float*>(&model.buffers[view.buffer].data[view.byteOffset + nrmAcc->byteOffset + (i * nrmAcc->ByteStride(view))]);
					v.nrm = { n[0], n[1], n[2] };
				}

				//uv
				if (uvAcc)
				{
					const auto& view = model.bufferViews[uvAcc->bufferView];
					const float* u = reinterpret_cast<const float*>(&model.buffers[view.buffer].data[view.byteOffset + uvAcc->byteOffset + (i * uvAcc->ByteStride(view))]);
					v.uv0 = { u[0], u[1] };
				}

				//tan
				if (tanAcc)
				{
					const auto& view = model.bufferViews[tanAcc->bufferView];
					const float* t = reinterpret_cast<const float*>(&model.buffers[view.buffer].data[view.byteOffset + tanAcc->byteOffset + (i * tanAcc->ByteStride(view))]);
					v.tan = { t[0], t[1], t[2], t[3]};
				}

				v.col = { 1.f, 1.f, 1.f };

				// Deduplication
				if (!uniqueVertices.contains(v))
				{
					uniqueVertices[v] = static_cast<uint32_t>(pVertices.size());
					pVertices.push_back(v);
				}
				primitiveToIndexMap.push_back(uniqueVertices[v]);
			}

			// Process indices
			const auto& idxAcc = model.accessors[primitive.indices];
			const auto& idxView = model.bufferViews[idxAcc.bufferView];
			const auto& idxBuf = model.buffers[idxView.buffer];
			const unsigned char* data = &idxBuf.data[idxView.byteOffset + idxAcc.byteOffset];

			for (size_t i = 0; i < idxAcc.count; i++)
			{
				uint32_t localIdx = 0;
				if (idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
					localIdx = reinterpret_cast<const uint16_t*>(data)[i];
				else if (idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
					localIdx = reinterpret_cast<const uint32_t*>(data)[i];
				else if (idxAcc.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
					localIdx = reinterpret_cast<const uint8_t*>(data)[i];

				// The Magic Fix: Map the local index to the global vertex array
				pIndices.push_back(primitiveToIndexMap[localIdx]);
			}
		}
	}
}

void Mesh::CreateBuffers(std::vector<Vertex>& pVertices, std::vector<uint32_t>& pIndices)
{
	auto& device = *Device::Inst().GetDevice();

	//vertex buffer
	{
		vk::DeviceSize size = sizeof(Vertex) * pVertices.size();

		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingMemory;

		{
			vk::BufferCreateInfo stagingCreateInfo
			{
				.size = size,
				.usage = vk::BufferUsageFlagBits::eTransferSrc,
				.sharingMode = vk::SharingMode::eExclusive
			};

			stagingBuffer = device.createBuffer(stagingCreateInfo);

			vk::MemoryRequirements memReqs = device.getBufferMemoryRequirements(stagingBuffer);
			vk::MemoryAllocateInfo stagingAllocateInfo
			{
				.allocationSize = memReqs.size,
				.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
			};

			stagingMemory = device.allocateMemory(stagingAllocateInfo);
			device.bindBufferMemory(stagingBuffer, stagingMemory, 0);

			void* data = device.mapMemory(stagingMemory, 0, size);
			memcpy(data, pVertices.data(), size);
			device.unmapMemory(stagingMemory);
		}

		vk::BufferCreateInfo bufferCreateInfo
		{
			.size = size,
			.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			.sharingMode = vk::SharingMode::eExclusive
		};

		_vertexBuffer = device.createBuffer(bufferCreateInfo);

		vk::MemoryRequirements memReqs = device.getBufferMemoryRequirements(_vertexBuffer);
		vk::MemoryAllocateInfo bufferAllocateInfo
		{
			.allocationSize = memReqs.size,
			.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
		};

		_vertexBufferMemory = device.allocateMemory(bufferAllocateInfo);
		device.bindBufferMemory(_vertexBuffer, _vertexBufferMemory, _vertexBufferOffset);

		CopyBuffer(stagingBuffer, _vertexBuffer, size);

		device.destroyBuffer(stagingBuffer);
		device.freeMemory(stagingMemory);
	}

	//index buffer
	{
		vk::DeviceSize size = sizeof(uint32_t) * pIndices.size();

		vk::Buffer stagingBuffer;
		vk::DeviceMemory stagingMemory;

		{
			vk::BufferCreateInfo stagingCreateInfo
			{
				.size = size,
				.usage = vk::BufferUsageFlagBits::eTransferSrc,
				.sharingMode = vk::SharingMode::eExclusive
			};

			stagingBuffer = device.createBuffer(stagingCreateInfo);

			vk::MemoryRequirements memReqs = device.getBufferMemoryRequirements(stagingBuffer);
			vk::MemoryAllocateInfo stagingAllocateInfo
			{
				.allocationSize = memReqs.size,
				.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
			};

			stagingMemory = device.allocateMemory(stagingAllocateInfo);
			device.bindBufferMemory(stagingBuffer, stagingMemory, 0);

			void* data = device.mapMemory(stagingMemory, 0, size);
			memcpy(data, pIndices.data(), size);
			device.unmapMemory(stagingMemory);
		}

		vk::BufferCreateInfo bufferCreateInfo
		{
			.size = size,
			.usage = vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
			.sharingMode = vk::SharingMode::eExclusive
		};

		_indexBuffer = device.createBuffer(bufferCreateInfo);

		vk::MemoryRequirements memReqs = device.getBufferMemoryRequirements(_indexBuffer);
		vk::MemoryAllocateInfo stagingAllocateInfo
		{
			.allocationSize = memReqs.size,
			.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
		};

		_indexBufferMemory = device.allocateMemory(stagingAllocateInfo);
		device.bindBufferMemory(_indexBuffer, _indexBufferMemory, _indexBufferOffset);

		CopyBuffer(stagingBuffer, _indexBuffer, size);

		device.destroyBuffer(stagingBuffer);
		device.freeMemory(stagingMemory);
	}
}

uint32_t Mesh::FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps)
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

void Mesh::CopyBuffer(vk::Buffer pSrc, vk::Buffer pDst, vk::DeviceSize pSize)
{
	auto commandBuffer = Device::Inst().BeginSingleTimeCommand();

	vk::BufferCopy bufferCopy
	{
		.srcOffset = 0,
		.dstOffset = 0,
		.size = pSize
	};

	commandBuffer.copyBuffer(pSrc, pDst, bufferCopy);

	Device::Inst().EndSingleTimeCommand(commandBuffer);
}