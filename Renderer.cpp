#include "pch.h"
#include "Renderer.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"
#include "OperatorOverloads.h"

void Renderer::InitDXC()
{
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler));
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils));
	_utils->CreateDefaultIncludeHandler(&_includeHandler);

	std::filesystem::create_directories("Shaders/SPV");
	CompileShaders();
}

void Renderer::CompileShaders()
{
	// Convert shader code to a DXC blob
	DxcBuffer sourceBuffer;
	std::string shaderCode;
	std::wstring hlsl, out;

	//compute shaders
	for (auto& shader : std::filesystem::directory_iterator("Shaders/Compute"))
	{
		//convert shader to dxc buffer
		shaderCode = ShaderAsString(shader.path().string().c_str());
		sourceBuffer.Ptr = shaderCode.c_str();
		sourceBuffer.Size = shaderCode.size();
		sourceBuffer.Encoding = DXC_CP_ACP;

		//define arguments
		std::vector<LPCWSTR> arguments;
		arguments.push_back(L"-spirv");
		arguments.push_back(L"-T");
		arguments.push_back(L"cs_6_6");
		arguments.push_back(L"-E");
		arguments.push_back(L"main");
		hlsl = shader.path().filename().wstring();
		arguments.push_back(hlsl.c_str());
		arguments.push_back(L"-Fo");
		out = L"Shaders/SPV/" + shader.path().filename().stem().wstring() + L".spv";
		arguments.push_back(out.c_str());

		ComPtr<IDxcResult> result;
		_compiler->Compile(&sourceBuffer, arguments.data(), arguments.size(), _includeHandler.Get(), IID_PPV_ARGS(&result));

		// Check for compilation errors
		ComPtr<IDxcBlobUtf8> errors;
		if (SUCCEEDED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr)) && errors && errors->GetStringLength() > 0) {
			std::cout << "Shader compilation errors: " << errors->GetStringPointer() << "\n";
			return;
		}

		//write compilation to spv
		ComPtr<IDxcBlob> shaderBlob;
		if (SUCCEEDED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr)))
		{
			// Write the compiled shader to file
			std::ofstream outFile(L"Shaders/SPV/" + shader.path().filename().stem().wstring() + L".spv", std::ios::binary);
			outFile.write(static_cast<const char*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize());
			outFile.close();
		}
	}

	//vertex shaders
	for (auto& shader : std::filesystem::directory_iterator("Shaders/Vertex"))
	{
		//convert shader to dxc buffer
		shaderCode = ShaderAsString(shader.path().string().c_str());
		sourceBuffer.Ptr = shaderCode.c_str();
		sourceBuffer.Size = shaderCode.size();
		sourceBuffer.Encoding = DXC_CP_ACP;

		//define arguments
		std::vector<LPCWSTR> arguments;
		arguments.push_back(L"-spirv");
		arguments.push_back(L"-T");
		arguments.push_back(L"vs_6_6");
		arguments.push_back(L"-E");
		arguments.push_back(L"main");
		hlsl = shader.path().filename().wstring();
		arguments.push_back(hlsl.c_str());
		arguments.push_back(L"-Fo");
		out = L"Shaders/SPV/" + shader.path().filename().stem().wstring() + L".spv";
		arguments.push_back(out.c_str());

		ComPtr<IDxcResult> result;
		_compiler->Compile(&sourceBuffer, arguments.data(), arguments.size(), _includeHandler.Get(), IID_PPV_ARGS(&result));

		// Check for compilation errors
		ComPtr<IDxcBlobUtf8> errors;
		if (SUCCEEDED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr)) && errors && errors->GetStringLength() > 0) {
			std::cout << "Shader compilation errors: " << errors->GetStringPointer() << "\n";
			return;
		}

		//write compilation to spv
		ComPtr<IDxcBlob> shaderBlob;
		if (SUCCEEDED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr)))
		{
			// Write the compiled shader to file
			std::ofstream outFile(L"Shaders/SPV/" + shader.path().filename().stem().wstring() + L".spv", std::ios::binary);
			outFile.write(static_cast<const char*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize());
			outFile.close();
		}
	}

	//pixelshaders
	for (auto& shader : std::filesystem::directory_iterator("Shaders/Fragment"))
	{
		//convert shader to dxc buffer
		shaderCode = ShaderAsString(shader.path().string().c_str());
		sourceBuffer.Ptr = shaderCode.c_str();
		sourceBuffer.Size = shaderCode.size();
		sourceBuffer.Encoding = DXC_CP_ACP;

		//define arguments
		std::vector<LPCWSTR> arguments;
		arguments.push_back(L"-spirv");
		arguments.push_back(L"-T");
		arguments.push_back(L"ps_6_6");
		arguments.push_back(L"-E");
		arguments.push_back(L"main");
		hlsl = shader.path().filename().wstring();
		arguments.push_back(hlsl.c_str());
		arguments.push_back(L"-Fo");
		out = L"Shaders/SPV/" + shader.path().filename().stem().wstring() + L".spv";
		arguments.push_back(out.c_str());

		ComPtr<IDxcResult> result;
		_compiler->Compile(&sourceBuffer, arguments.data(), arguments.size(), _includeHandler.Get(), IID_PPV_ARGS(&result));

		// Check for compilation errors
		ComPtr<IDxcBlobUtf8> errors;
		if (SUCCEEDED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr)) && errors && errors->GetStringLength() > 0) {
			std::cout << "Shader compilation errors: " << errors->GetStringPointer() << "\n";
			return;
		}

		//write compilation to spv
		ComPtr<IDxcBlob> shaderBlob;
		if (SUCCEEDED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr)))
		{
			// Write the compiled shader to file
			std::ofstream outFile(L"Shaders/SPV/" + shader.path().filename().stem().wstring() + L".spv", std::ios::binary);
			outFile.write(static_cast<const char*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize());
			outFile.close();
		}
	}

	VkPipelineShaderStageCreateInfo pssci;
	GvkHelper::create_shader(_device, "Shaders/SPV/FragmentShader.spv", "main", VK_SHADER_STAGE_FRAGMENT_BIT, &_pixelShaderModule, &pssci);
	GvkHelper::create_shader(_device, "Shaders/SPV/VertexShader.spv", "main", VK_SHADER_STAGE_VERTEX_BIT, &_vertexShaderModule, &pssci);
}

void Renderer::LoadModel(std::string filename, float scale)
{
	tinygltf::TinyGLTF gltfLoader;
	std::string error;
	std::string warning;

	unsigned long long pos = filename.find_last_of('/');
	std::string path = filename.substr(0, pos);

	bool fileLoaded = gltfLoader.LoadASCIIFromFile(&_model, &error, &warning, filename);

	if (!warning.empty())
	{
		std::cout << "Warning: " << warning << '\n';
	}

	if (!error.empty())
	{
		std::cout << "Error: " << error << '\n';
	}

	if (!fileLoaded)
	{
		std::cout << "Failed to parse model\n";
	}

	_gltfScene.ImportMaterials(_model);
	_gltfScene.ImportDrawableNodes(_model);

	CreateGeometryBuffer();
}

void Renderer::CreateTextureImages(VkCommandBuffer& commandBuffer)
{
	VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;

	auto AddDefaultTexture = [this, commandBuffer, format]()
		{
			unsigned char white[4] = { 255, 255, 255, 255 };

			Texture outTex;
			GvkHelper::create_image(_physicalDevice, _device, { 1, 1, 1 }, 1, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr, &outTex.texImage, &outTex.texBuffer.memory);
			GvkHelper::create_image_view(_device, outTex.texImage, format, VK_IMAGE_ASPECT_COLOR_BIT, 1, nullptr, &outTex.texImageView);
			CreateSampler(&outTex);

			_textures.push_back(outTex);
		};

	if (_model.images.empty())
	{
		AddDefaultTexture();
		return;
	}

	_textures.reserve(_model.images.size());
	for (size_t i = 0; i < _model.images.size(); i++)
	{
		Texture tex;
		UploadTextureToGPU(&_model.images[i], &tex);
		CreateSampler(&tex);
		_textures.push_back(tex);
	}
}

void Renderer::CreateGeometryBuffer()
{
	VkCommandBuffer commandBuffer;
	GvkHelper::signal_command_start(_device, _commandPool, &commandBuffer);

	GvkHelper::create_buffer(_physicalDevice, _device, sizeof(vec3) * _gltfScene.positions.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, &_vertexBuffer.buffer, &_vertexBuffer.memory);
	GvkHelper::write_to_buffer(_device, _vertexBuffer.memory, _gltfScene.positions.data(), sizeof(vec3) * _gltfScene.positions.size());

	GvkHelper::create_buffer(_physicalDevice, _device, sizeof(unsigned int) * _gltfScene.indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, &_idxBuffer.buffer, &_idxBuffer.memory);
	GvkHelper::write_to_buffer(_device, _idxBuffer.memory, _gltfScene.indices.data(), sizeof(unsigned int) * _gltfScene.indices.size());

	GvkHelper::create_buffer(_physicalDevice, _device, sizeof(vec3) * _gltfScene.normals.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, &_normalBuffer.buffer, &_normalBuffer.memory);
	GvkHelper::write_to_buffer(_device, _normalBuffer.memory, _gltfScene.normals.data(), sizeof(vec3) * _gltfScene.normals.size());

	GvkHelper::create_buffer(_physicalDevice, _device, sizeof(vec2) * _gltfScene.texCoords0.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, &_texCoordBuffer.buffer, &_texCoordBuffer.memory);
	GvkHelper::write_to_buffer(_device, _texCoordBuffer.memory, _gltfScene.texCoords0.data(), sizeof(vec2) * _gltfScene.texCoords0.size());

	std::vector<GltfShaderMaterial> shaderMaterials;
	for (auto& m : _gltfScene.materials)
	{
		GltfShaderMaterial smat;
		smat.pbrBaseColorFactor = m.baseColorFactor;
		smat.pbrBaseColorTexture = m.baseColorTexture;
		smat.pbrMetallicFactor = m.metallicFactor;
		smat.pbrRoughnessFactor = m.roughnessFactor;
		smat.pbrMetallicRoughnessTexture = m.metallicRoughnessTexture;
		smat.emissiveTexture = m.emissiveTexture;
		smat.emissiveFactor = m.emissiveFactor;
		smat.alphaMode = m.alphaMode;
		smat.alphaCutoff = m.alphaCutoff;
		smat.normalTexture = m.normalTexture;
		smat.normalTextureScale = m.normalTextureScale;
		smat.uvTransform = m.textureTransform.uvTransform;
		smat.unlit = m.unlit.active;
		smat.transmissionFactor = m.transmission.factor;
		smat.transmissionTexture = m.transmission.texture;
		smat.anisotropyTexture = m.anisotropy.texture;
		smat.anisotropyStrength = m.anisotropy.factor;
		smat.anisotropyRotation = m.anisotropy.factor;
		smat.ior = m.ior.ior;
		smat.attenuationColor = m.volume.attenuationColor;
		smat.thicknessFactor = m.volume.thicknessFactor;
		smat.thicknessTexture = m.volume.thicknessTexture;
		smat.attenuationDistance = m.volume.attenuationDistance;
		smat.clearcoatFactor = m.clearcoat.factor;
		smat.clearcoatRoughness = m.clearcoat.roughnessFactor;
		smat.clearcoatTexture = m.clearcoat.texture;
		smat.clearcoatRoughnessTexture = m.clearcoat.roughnessTexture;
		smat.sheenColorFactor = m.sheen.colorFactor;
		smat.sheenColorTexture = m.sheen.colorTexture;
		smat.sheenRoughnessFactor = m.sheen.roughnessFactor;
		smat.sheenRoughnessTexture = m.sheen.roughnessTexture;

		shaderMaterials.emplace_back(smat);
	}
	GvkHelper::create_buffer(_physicalDevice, _device, sizeof(GltfShaderMaterial) * shaderMaterials.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, &_materialBuffer.buffer, &_materialBuffer.memory);
	GvkHelper::write_to_buffer(_device, _materialBuffer.memory, shaderMaterials.data(), sizeof(GltfShaderMaterial) * shaderMaterials.size());

	std::vector<PrimMeshInfo> primLookup;
	for (auto& primMesh : _gltfScene.primMeshes)
	{
		primLookup.push_back({ primMesh.firstIndex, primMesh.vertexOffset, primMesh.materialIndex });
	}
	GvkHelper::create_buffer(_physicalDevice, _device, sizeof(PrimMeshInfo) * primLookup.size(), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, &_primInfo.buffer, &_primInfo.memory);
	GvkHelper::write_to_buffer(_device, _primInfo.memory, primLookup.data(), sizeof(PrimMeshInfo) * primLookup.size());


	//SceneDescription sceneDesc;
	//sceneDesc.vertexAddress = GetBufferDeviceAddress(_vertexBuffer.buffer);
	//sceneDesc.idxAddress = GetBufferDeviceAddress(_idxBuffer.buffer);
	//sceneDesc.normalAddress = GetBufferDeviceAddress(_normalBuffer.buffer);
	//sceneDesc.uvAddress = GetBufferDeviceAddress(_texCoordBuffer.buffer);
	//sceneDesc.materialAddress = GetBufferDeviceAddress(_materialBuffer.buffer);
	//sceneDesc.primInfoAddress = GetBufferDeviceAddress(_primInfo.buffer);

	//GvkHelper::create_buffer(_physicalDevice, _device, sizeof(SceneDescription), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT, &_sceneDesc.buffer, &_sceneDesc.memory);
	//GvkHelper::write_to_buffer(_device, _sceneDesc.memory, &sceneDesc, sizeof(SceneDescription));

	CreateTextureImages(commandBuffer);

	GvkHelper::signal_command_end(_device, _graphicsQueue, _commandPool, &commandBuffer);
}

void Renderer::CreateUniformBuffers()
{
	_vlk.GetSwapchainImageCount(_frames);

	_descriptorSets.resize(_frames);
	_uniformBuffers.resize(_frames);

	//view
	GMatrix::LookAtLHF(vec4{ 0.f, .125f, -1.f }, vec4{ 0, 0, 0 }, vec4{ 0, 1, 0 }, view);

	//proj
	_vlk.GetAspectRatio(_aspect);
	GMatrix::ProjectionVulkanLHF(G_DEGREE_TO_RADIAN(65), _aspect, .1f, 100.f, proj);

	_matrices.prevWorldViewProjection = world * view * proj;
	_matrices.currWorldViewProjection = world * view * proj;
	_matrices.deltaTime = 0.f;

	for (size_t i = 0; i < _frames; i++)
	{
		GvkHelper::create_buffer(_physicalDevice, _device, sizeof(Matrices), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &_uniformBuffers[i].buffer, &_uniformBuffers[i].memory);
		GvkHelper::write_to_buffer(_device, _uniformBuffers[i].memory, &_matrices, sizeof(Matrices));
	}
}

void Renderer::CreateDescriptorSets()
{
	VkDescriptorSetLayoutBinding descriptorSetLayoutBinding;
	descriptorSetLayoutBinding.binding = 0;
	descriptorSetLayoutBinding.descriptorCount = 1;
	descriptorSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	descriptorSetLayoutCreateInfo.bindingCount = 1;
	descriptorSetLayoutCreateInfo.pBindings = &descriptorSetLayoutBinding;

	vkCreateDescriptorSetLayout(_device, &descriptorSetLayoutCreateInfo, nullptr, &_descriptorSetLayout);

	VkDescriptorPoolSize descriptorPoolSize[1];
	descriptorPoolSize[0].descriptorCount = _frames;
	descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	int maxSets = 0;
	for (size_t i = 0; i < ARRAYSIZE(descriptorPoolSize); i++)
	{
		maxSets += descriptorPoolSize[i].descriptorCount;
	}

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	descriptorPoolCreateInfo.poolSizeCount = 1;
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSize;
	descriptorPoolCreateInfo.maxSets = maxSets;

	vkCreateDescriptorPool(_device, &descriptorPoolCreateInfo, nullptr, &_descriptorPool);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	descriptorSetAllocateInfo.descriptorPool = _descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &_descriptorSetLayout;

	for (size_t i = 0; i < _frames; i++)
	{
		vkAllocateDescriptorSets(_device, &descriptorSetAllocateInfo, &_descriptorSets[i]);
	}
}

void Renderer::CreateGraphicsPipeline()
{
	_vlk.GetRenderPass((void**)&_renderPass);

	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[2] = { {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO}, {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO} };
	//vertex shader
	pipelineShaderStageCreateInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	pipelineShaderStageCreateInfos[0].module = _vertexShaderModule;
	pipelineShaderStageCreateInfos[0].pName = "main";
	//pixel shader
	pipelineShaderStageCreateInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pipelineShaderStageCreateInfos[1].module = _pixelShaderModule;
	pipelineShaderStageCreateInfos[1].pName = "main";

	//assembly state
	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = false;

	//vertex input state
	VkVertexInputBindingDescription vertexInputBindingDescription[3] =
	{
		{0, sizeof(vec3), VK_VERTEX_INPUT_RATE_VERTEX}, //pos
		{1, sizeof(vec3), VK_VERTEX_INPUT_RATE_VERTEX}, //nrm
		{2, sizeof(vec2), VK_VERTEX_INPUT_RATE_VERTEX}, //uv
		//{3, sizeof(float) * 4, VK_VERTEX_INPUT_RATE_VERTEX}, //tan
	};

	//vertex attributes
	VkVertexInputAttributeDescription vertexInputAttributeDescription[3] =
	{
		//location, binding, format, offset
		/*for my own memory:
		float: VK_FORMAT_R32_SFLOAT
		vec2: VK_FORMAT_R32G32_SFLOAT
		vec3: VK_FORMAT_R32G32B32_SFLOAT
		vec4: VK_FORMAT_R32G32B32A32_SFLOAT
		*/
		{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}, //pos
		{1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0}, //nrm
		{2, 2, VK_FORMAT_R32G32_SFLOAT, 0}, //texcoord0
		//{3, 3, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
	};

	//vertex input info
	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 3;
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescription;
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 3;
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescription;

	//viewport state
	VkViewport viewport = { 0, 0, static_cast<float>(_width), static_cast<float>(_height), 0, 1 };
	VkRect2D scissor = { {0, 0}, {_width, _height} };

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	pipelineViewportStateCreateInfo.viewportCount = 1;
	pipelineViewportStateCreateInfo.pViewports = &viewport;
	pipelineViewportStateCreateInfo.scissorCount = 1;
	pipelineViewportStateCreateInfo.pScissors = &scissor;

	//rasterizer state
	VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = false;
	pipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; //TODO: switch render modes on key press;
	pipelineRasterizationStateCreateInfo.lineWidth = 1.f;
	pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	pipelineRasterizationStateCreateInfo.depthClampEnable = false;
	pipelineRasterizationStateCreateInfo.depthBiasEnable = false;
	pipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f;
	pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
	pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

	//multisampling state
	VkPipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	pipelineMultisampleStateCreateInfo.sampleShadingEnable = false;
	pipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pipelineMultisampleStateCreateInfo.minSampleShading = 1.0f;
	pipelineMultisampleStateCreateInfo.pSampleMask = nullptr;
	pipelineMultisampleStateCreateInfo.alphaToCoverageEnable = false;
	pipelineMultisampleStateCreateInfo.alphaToOneEnable = false;

	//depth stencil state
	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	pipelineDepthStencilStateCreateInfo.depthTestEnable = true;
	pipelineDepthStencilStateCreateInfo.depthWriteEnable = true;
	pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = false;
	pipelineDepthStencilStateCreateInfo.minDepthBounds = 0.0f;
	pipelineDepthStencilStateCreateInfo.maxDepthBounds = 1.0f;
	pipelineDepthStencilStateCreateInfo.stencilTestEnable = false;

	//color blend attachment state
	VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {};
	pipelineColorBlendAttachmentState.colorWriteMask = 0xF;
	pipelineColorBlendAttachmentState.blendEnable = false;
	pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
	pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
	pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
	pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	//color blend state
	VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	pipelineColorBlendStateCreateInfo.logicOpEnable = false;
	pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
	pipelineColorBlendStateCreateInfo.attachmentCount = 1;
	pipelineColorBlendStateCreateInfo.pAttachments = &pipelineColorBlendAttachmentState;
	pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
	pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;

	//dynamic state
	VkDynamicState dynamicState[2] =
	{
		// By setting these we do not need to re-create the pipeline on Resize
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	pipelineDynamicStateCreateInfo.dynamicStateCount = 2;
	pipelineDynamicStateCreateInfo.pDynamicStates = dynamicState;

	//descriptor pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &_descriptorSetLayout;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	//pipelineLayoutCreateInfo.setLayoutCount = 1;
	//pipelineLayoutCreateInfo.pSetLayouts = &_graphicsPipelineLayout;

	vkCreatePipelineLayout(_device, &pipelineLayoutCreateInfo, nullptr, &_graphicsPipelineLayout);

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos;
	graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &pipelineMultisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
	graphicsPipelineCreateInfo.layout = _graphicsPipelineLayout;
	graphicsPipelineCreateInfo.renderPass = _renderPass;
	graphicsPipelineCreateInfo.subpass = 0;
	graphicsPipelineCreateInfo.basePipelineHandle = nullptr;

	vkCreateGraphicsPipelines(_device, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &_graphicsPipeline);
}

void Renderer::WriteDescriptorSets()
{
	for (size_t i = 0; i < _frames; i++)
	{
		VkDescriptorBufferInfo descriptorBufferInfo;
		descriptorBufferInfo.buffer = _uniformBuffers[i].buffer;
		descriptorBufferInfo.offset = 0;
		descriptorBufferInfo.range = sizeof(Matrices);

		VkWriteDescriptorSet writeDescriptorSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		writeDescriptorSet.descriptorCount = 1;
		writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
		writeDescriptorSet.dstSet = _descriptorSets[i];
		writeDescriptorSet.dstBinding = 0;

		vkUpdateDescriptorSets(_device, 1, &writeDescriptorSet, 0, nullptr);
	}
}

void Renderer::UploadTextureToGPU(const char* filepath, Texture* texture)
{
	int width, height, component;
	auto data = stbi_load(filepath, &width, &height, &component, STBI_rgb_alpha);
	component = 4;

	//if (mainTex)
	//{
	//	_win.ResizeWindow(width, height);
	//	_win.GetClientWidth(_width);
	//	_win.GetClientHeight(_height);
	//}

	VkDeviceSize size = width * height * component;

	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

	//staging buffer
	Buffer staging;
	VkDeviceMemory transient;

	GvkHelper::create_buffer(_physicalDevice, _device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging.buffer, &staging.memory);
	GvkHelper::write_to_buffer(_device, staging.memory, data, static_cast<unsigned int>(size));

	//create the new buffer
	GvkHelper::create_buffer(_physicalDevice, _device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->texBuffer.buffer, &transient);

	//copy staging
	GvkHelper::copy_buffer(_device, _commandPool, _graphicsQueue, staging.buffer, texture->texBuffer.buffer, size);

	VkExtent3D tempExtent = { width, height, 1 };
	_mipLevels = static_cast<unsigned int>(floor(log2(std::max(width, height))) + 1);
	GvkHelper::create_image(_physicalDevice, _device, tempExtent, _mipLevels, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr, &texture->texImage, &texture->texBuffer.memory);
	//VK_IMAGE_USAGE_STORAGE_BIT
	//transition
	GvkHelper::transition_image_layout(_device, _commandPool, _graphicsQueue, _mipLevels, texture->texImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	GvkHelper::copy_buffer_to_image(_device, _commandPool, _graphicsQueue, staging.buffer, texture->texImage, tempExtent);

	//create mip maps
	GvkHelper::create_mipmaps(_device, _commandPool, _graphicsQueue, texture->texImage, width, height, _mipLevels);

	GvkHelper::create_image_view(_device, texture->texImage, format, VK_IMAGE_ASPECT_COLOR_BIT, _mipLevels, nullptr, &texture->texImageView);

	vkDestroyBuffer(_device, staging.buffer, nullptr);
	vkFreeMemory(_device, staging.memory, nullptr);
	vkFreeMemory(_device, transient, nullptr);
	stbi_image_free(data);
}

void Renderer::UploadTextureToGPU(tinygltf::Image* img, Texture* texture)
{
	VkDeviceSize size = img->width * img->height * img->component;

	VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

	//staging buffer
	Buffer staging;
	VkDeviceMemory transient;

	GvkHelper::create_buffer(_physicalDevice, _device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging.buffer, &staging.memory);
	GvkHelper::write_to_buffer(_device, staging.memory, img->image.data(), static_cast<unsigned int>(size));

	//create the new buffer
	GvkHelper::create_buffer(_physicalDevice, _device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &texture->texBuffer.buffer, &transient);

	//copy staging
	GvkHelper::copy_buffer(_device, _commandPool, _graphicsQueue, staging.buffer, texture->texBuffer.buffer, size);

	VkExtent3D tempExtent = { img->width, img->height, 1 };
	_mipLevels = static_cast<unsigned int>(floor(log2(std::max(img->width, img->height))) + 1);
	GvkHelper::create_image(_physicalDevice, _device, tempExtent, _mipLevels, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr, &texture->texImage, &texture->texBuffer.memory);
	//VK_IMAGE_USAGE_STORAGE_BIT
	//transition
	GvkHelper::transition_image_layout(_device, _commandPool, _graphicsQueue, _mipLevels, texture->texImage, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	GvkHelper::copy_buffer_to_image(_device, _commandPool, _graphicsQueue, staging.buffer, texture->texImage, tempExtent);

	//create mip maps
	GvkHelper::create_mipmaps(_device, _commandPool, _graphicsQueue, texture->texImage, img->width, img->height, _mipLevels);

	GvkHelper::create_image_view(_device, texture->texImage, format, VK_IMAGE_ASPECT_COLOR_BIT, _mipLevels, nullptr, &texture->texImageView);

	vkDestroyBuffer(_device, staging.buffer, nullptr);
	vkFreeMemory(_device, staging.memory, nullptr);
	vkFreeMemory(_device, transient, nullptr);

}

void Renderer::CreateSampler(Texture* texture)
{
	VkSamplerCreateInfo samplerCreateInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.anisotropyEnable = true;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerCreateInfo.compareEnable = false;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_LESS;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.maxAnisotropy = 4.f;
	samplerCreateInfo.maxLod = VK_LOD_CLAMP_NONE;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minLod = 0;
	samplerCreateInfo.mipLodBias = 0;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	vkCreateSampler(_device, &samplerCreateInfo, nullptr, &texture->texSampler);
}

void Renderer::CleanUp()
{
	vkDeviceWaitIdle(_device);

	// Clean up shader modules
	vkDestroyShaderModule(_device, _pixelShaderModule, nullptr);
	vkDestroyShaderModule(_device, _vertexShaderModule, nullptr);

	//destroy buffers
	vkDestroyBuffer(_device, _vertexBuffer.buffer, nullptr);
	vkFreeMemory(_device, _vertexBuffer.memory, nullptr);

	//destroy pipelines
	vkDestroyPipelineLayout(_device, _graphicsPipelineLayout, nullptr);
	vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
}

std::string Renderer::ShaderAsString(const char* shaderFilePath)
{
	std::string output;
	unsigned int stringLength = 0;
	GW::SYSTEM::GFile file;
	file.Create();
	file.GetFileSize(shaderFilePath, stringLength);

	if (stringLength && +file.OpenBinaryRead(shaderFilePath))
	{
		output.resize(stringLength);
		file.Read(&output[0], stringLength);
	}

	return output;
}

Renderer::Renderer(GWindow win, GVulkanSurface vlk)
{
	_win = win;
	_vlk = vlk;
	_win.GetClientWidth(_width);
	_win.GetClientHeight(_height);

	_vlk.GetDevice((void**)&_device);
	_vlk.GetPhysicalDevice((void**)&_physicalDevice);
	_vlk.GetGraphicsQueue((void**)&_graphicsQueue);
	_vlk.GetCommandPool((void**)&_commandPool);

	//create proxies
	_gInput.Create(_win);
	_gController.Create();

	InitDXC();
	LoadModel("Models/Shapes/Shapes.gltf");
	CreateUniformBuffers();
	CreateDescriptorSets();
	WriteDescriptorSets();
	CreateGraphicsPipeline();

	//clean up/shutdown
	_shutdown.Create(vlk, [&]()
		{
			if (+_shutdown.Find(GW::GRAPHICS::GVulkanSurface::Events::RELEASE_RESOURCES, true))
			{
				CleanUp(); // unlike D3D we must be careful about destroy timing
			}
		});

}

Renderer::~Renderer()
{
}

void Renderer::Render()
{
	auto now = std::chrono::steady_clock::now();
	_deltaTime = now - _lastUpdate;
	_lastUpdate = now;

	//grab current command buffer
	unsigned int currentBuffer;
	VkCommandBuffer commandBuffer;
	_vlk.GetSwapchainCurrentImage(currentBuffer);
	_vlk.GetCommandBuffer(currentBuffer, (void**)&commandBuffer);

	//current dimensions
	_win.GetClientWidth(_width);
	_win.GetClientHeight(_height);

	GMatrix::RotateYGlobalF(world, G_DEGREE_TO_RADIAN(250) * _deltaTime.count(), world);
	_matrices.prevWorldViewProjection = _matrices.currWorldViewProjection;
	_matrices.currWorldViewProjection = world * view * proj;
	_matrices.deltaTime = _deltaTime.count();
	GvkHelper::write_to_buffer(_device, _uniformBuffers[currentBuffer].memory, &_matrices, sizeof(Matrices));

	//setup pipelines dynamic settings
	VkViewport viewport = { 0, 0, static_cast<float>(_width), static_cast<float>(_height), 0, 1 };
	VkRect2D scissor = { {0, 0}, {_width, _height} };
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipelineLayout, 0, 1, &_descriptorSets[currentBuffer], 0, nullptr);
	std::vector<VkBuffer> vertexBuffers = { _vertexBuffer.buffer, _normalBuffer.buffer, _texCoordBuffer.buffer };
	std::vector<VkDeviceSize> offsets = { 0, 0, 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, (unsigned int)vertexBuffers.size(), vertexBuffers.data(), offsets.data());
	vkCmdBindIndexBuffer(commandBuffer, _idxBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

	for (auto& node : _gltfScene.nodes)
	{
		auto& prim = _gltfScene.primMeshes[node.primMesh];

		vkCmdDrawIndexed(commandBuffer, prim.indexCount, 1, prim.firstIndex, prim.vertexOffset, 0);
	}
}

void Renderer::UpdateCamera()
{
	_win.IsFocus(_isFocused);

	if (!_isFocused) return;

	GW::MATH::GMATRIXF cam = GW::MATH::GIdentityMatrixF;

	GMatrix::InverseF(view, cam);

	float y = 0.0f;

	float totalY = 0.0f;
	float totalZ = 0.0f;
	float totalX = 0.0f;

	const float cameraSpeed = 7.5f;
	float spaceKeyState = 0.0f;
	float leftShiftState = 0.0f;
	float rightTriggerState = 0.0f;
	float leftTriggerState = 0.0f;

	float arrowRight = 0.0f;
	float arrowLeft = 0.0f;

	float wKeyState = 0.0f;
	float sKeyState = 0.0f;
	float aKeyState = 0.0f;
	float dKeyState = 0.0f;
	float leftStickX = 0.0f;
	float leftStickY = 0.0f;
	unsigned int screenHeight = 0.0f;
	_win.GetHeight(screenHeight);
	unsigned int screenWidth = 0.0f;
	_win.GetWidth(screenWidth);
	float mouseDeltaX = 0.0f;
	float mouseDeltaY = 0.0f;
	//GW::GReturn result = ;
	float rightStickYaxis = 0.0f;
	_gController.GetState(0, G_RY_AXIS, rightStickYaxis);
	float rightStickXaxis = 0.0f;
	_gController.GetState(0, G_RX_AXIS, rightStickXaxis);

	float perFrameSpeed = 0.0f;

	_gInput.GetState(G_KEY_RIGHT, arrowRight);
	_gInput.GetState(G_KEY_LEFT, arrowLeft);

	if (arrowRight != 0)
	{
		cam.row4 = { 0.0f, 50.0f, 0.0f, 1 };

	}
	if (arrowLeft != 0)
	{
		cam.row4 = { 5.75f, 5.25f, -30.5f, 1 };
	}

	if (+_gInput.GetState(G_KEY_SPACE, spaceKeyState) && spaceKeyState != 0 || +_gInput.GetState(G_KEY_LEFTSHIFT, leftShiftState) && leftShiftState != 0 || +_gController.GetState(0, G_RIGHT_TRIGGER_AXIS, rightTriggerState) && rightTriggerState != 0 || +_gController.GetState(0, G_LEFT_TRIGGER_AXIS, leftTriggerState) && leftTriggerState != 0)
	{
		totalY = spaceKeyState - leftShiftState + rightTriggerState - leftTriggerState;
	}

	cam.row4.y += totalY * cameraSpeed * _deltaTime.count();

	perFrameSpeed = cameraSpeed * _deltaTime.count();

	if (+_gInput.GetState(G_KEY_W, wKeyState) && wKeyState != 0 || +_gInput.GetState(G_KEY_A, aKeyState) && aKeyState != 0 || +_gInput.GetState(G_KEY_S, sKeyState) && sKeyState != 0 || +_gInput.GetState(G_KEY_D, dKeyState) && dKeyState != 0 || +_gController.GetState(0, G_LX_AXIS, leftStickX) && leftStickX != 0 || +_gController.GetState(0, G_LY_AXIS, leftStickY) && leftStickY != 0)
	{
		totalZ = wKeyState - sKeyState + leftStickY;
		totalX = dKeyState - aKeyState + leftStickX;
	}

	mat4 translation = GW::MATH::GIdentityMatrixF;
	vec4 vec = { totalX * perFrameSpeed, 0, totalZ * perFrameSpeed };
	GMatrix::TranslateLocalF(translation, vec, translation);
	GMatrix::MultiplyMatrixF(translation, cam, cam);

	float thumbSpeed = 3.14 * perFrameSpeed;
	auto r = _gInput.GetMouseDelta(mouseDeltaX, mouseDeltaY);
	if (G_PASS(r) && r != GW::GReturn::REDUNDANT)
	{
		float totalPitch = G_DEGREE_TO_RADIAN(65) * mouseDeltaY / screenHeight + rightStickYaxis * -thumbSpeed;
		GMatrix::RotateXLocalF(cam, totalPitch, cam);
		float totalYaw = G_DEGREE_TO_RADIAN(65) * _aspect * mouseDeltaX / screenWidth + rightStickXaxis * thumbSpeed;
		mat4 yawMatrix = GW::MATH::GIdentityMatrixF;
		vec4 camSave = cam.row4;
		cam.row4 = { 0,0,0,1 };
		GMatrix::RotateYGlobalF(cam, totalYaw, cam);
		cam.row4 = camSave;
	}

	GMatrix::InverseF(cam, view);
}
