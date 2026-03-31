#include "D:/GitHub/Imagination-Engine/Refactor/Imagination - Current/build/CMakeFiles/Imagination.dir/Debug/cmake_pch.hxx"
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
	vk::CommandBuffer commandBuffer = Device::Inst().BeginSingleTimeCommand();

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
			//indices
			const tinygltf::Accessor& indexAccessor = model.accessors[primitive.indices];
			const tinygltf::BufferView& indexBufferView = model.bufferViews[indexAccessor.bufferView];
			const tinygltf::Buffer& indexBuffer = model.buffers[indexBufferView.buffer];

			//pos
			const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
			const tinygltf::BufferView& posBufferView = model.bufferViews[posAccessor.bufferView];
			const tinygltf::Buffer& posBuffer = model.buffers[posBufferView.buffer];

			bool hasNrms = primitive.attributes.find("NORMAL") != primitive.attributes.end();
			const tinygltf::Accessor* nrmAccessor = nullptr;
			const tinygltf::BufferView* nrmBufferView = nullptr;
			const tinygltf::Buffer* nrmBuffer = nullptr;

			if (hasNrms)
			{
				nrmAccessor = &model.accessors[primitive.attributes.at("NORMAL")];
				nrmBufferView = &model.bufferViews[nrmAccessor->bufferView];
				nrmBuffer = &model.buffers[nrmBufferView->buffer];
			}

			bool hasTexCoords = primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end();
			const tinygltf::Accessor* texCoordAccessor = nullptr;
			const tinygltf::BufferView* texCoordBufferView = nullptr;
			const tinygltf::Buffer* texCoordBuffer = nullptr;

			if (hasTexCoords)
			{
				texCoordAccessor = &model.accessors[primitive.attributes.at("TEXCOORD_0")];
				texCoordBufferView = &model.bufferViews[texCoordAccessor->bufferView];
				texCoordBuffer = &model.buffers[texCoordBufferView->buffer];
			}

			bool hasTans = primitive.attributes.find("TANGENT") != primitive.attributes.end();
			const tinygltf::Accessor* tanAccessor = nullptr;
			const tinygltf::BufferView* tanBufferView = nullptr;
			const tinygltf::Buffer* tanBuffer = nullptr;

			if (hasTans)
			{
				tanAccessor = &model.accessors[primitive.attributes.at("TANGENT")];
				tanBufferView = &model.bufferViews[tanAccessor->bufferView];
				tanBuffer = &model.buffers[tanBufferView->buffer];
			}

			for (size_t i = 0; i < posAccessor.count; i++)
			{
				Vertex v;

				const float* pos = reinterpret_cast<const float*>(&posBuffer.data[posBufferView.byteOffset + posAccessor.byteOffset + i * 12]);
				const float* nrm = hasNrms ? reinterpret_cast<const float*>(&nrmBuffer->data[nrmBufferView->byteOffset + nrmAccessor->byteOffset + i * 12]) : new float[3] { 0.f, 0.f, 0.f };
				const float* uv0 = hasTexCoords ? reinterpret_cast<const float*>(&texCoordBuffer->data[texCoordBufferView->byteOffset + texCoordAccessor->byteOffset + i * 8]) : new float[2] { 0.f, 0.f };
				const float* tan = hasTans ? reinterpret_cast<const float*>(&tanBuffer->data[tanBufferView->byteOffset + tanAccessor->byteOffset + i * 16]) : new float[4] { 0.f, 0.f, 0.f, 0.f };

				v.pos = { pos[0], pos[1], pos[2] };
				v.nrm = { nrm[0], nrm[1], nrm[2] };
				v.uv0 = { uv0[0], uv0[1] };
				v.tan = { tan[0], tan[1], tan[2], tan[3] };
				v.col = { 1.f, 1.f, 1.f };

				if (!uniqueVertices.contains(v))
				{
					uniqueVertices[v] = static_cast<uint32_t>(pVertices.size());
					pVertices.push_back(v);
				}
			}

			// Process indices
			const unsigned char* indexData = &indexBuffer.data[indexBufferView.byteOffset + indexAccessor.byteOffset];

			// Handle different index component types
			if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			{
				const uint16_t* indices16 = reinterpret_cast<const uint16_t*>(indexData);
				for (size_t i = 0; i < indexAccessor.count; i++)
				{
					Vertex vertex = pVertices[indices16[i]];
					pIndices.push_back(uniqueVertices[vertex]);
				}
			}
			else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
			{
				const uint32_t* indices32 = reinterpret_cast<const uint32_t*>(indexData);
				for (size_t i = 0; i < indexAccessor.count; i++)
				{
					Vertex vertex = pVertices[indices32[i]];
					pIndices.push_back(uniqueVertices[vertex]);
				}
			}
			else if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
			{
				const uint8_t* indices8 = reinterpret_cast<const uint8_t*>(indexData);
				for (size_t i = 0; i < indexAccessor.count; i++)
				{
					Vertex vertex = pVertices[indices8[i]];
					pIndices.push_back(uniqueVertices[vertex]);
				}
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
		vk::MemoryAllocateInfo stagingAllocateInfo
		{
			.allocationSize = memReqs.size,
			.memoryTypeIndex = FindMemoryType(memReqs.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
		};

		_vertexBufferMemory = device.allocateMemory(stagingAllocateInfo);
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
			memcpy(data, pVertices.data(), size);
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