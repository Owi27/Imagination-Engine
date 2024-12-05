#include "pch.h"
#include "Renderer.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"

Renderer::Renderer(GWindow win)
{
	_win = win;
	_win.GetClientWidth(_width);
	_win.GetClientHeight(_height);
	//create proxies
	_gInput.Create(_win);
	_gController.Create();

}

Renderer::~Renderer()
{
}

void VulkanRenderer::CompileShaders()
{
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler));
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils));
	_utils->CreateDefaultIncludeHandler(&_includeHandler);

	std::filesystem::create_directories("Shaders/SPV");

	// Convert shader code to a DXC blob
	DxcBuffer sourceBuffer;
	std::string shaderCode, full;
	std::wstring hlsl, out;

	//0 - fragment, 1 - vertex, 2 - compute | dont include extension
	std::vector<std::pair<int, std::string>> shaders =
	{
		{0, "FragmentShader"},
		{0, "OffscreenFragmentShader"},
		{1, "VertexShader"},
		{1, "OffscreenVertexShader"},
	};

	for (size_t i = 0; i < shaders.size(); i++)
	{
		//convert shader to dxc buffer
		full = "Shaders/" + shaders[i].second + ".hlsl";
		shaderCode = ShaderAsString(full.c_str());
		sourceBuffer.Ptr = shaderCode.c_str();
		sourceBuffer.Size = shaderCode.size();
		sourceBuffer.Encoding = DXC_CP_ACP;

		std::wstring tWstring(shaders[i].second.begin(), shaders[i].second.end());

		//define arguments
		std::vector<LPCWSTR> arguments;
		arguments.push_back(L"-spirv");
		arguments.push_back(L"-T");
		shaders[i].first == 0 ? arguments.push_back(L"ps_6_6") : shaders[i].first == 1 ? arguments.push_back(L"vs_6_6") : arguments.push_back(L"cs_6_6");
		arguments.push_back(L"-E");
		arguments.push_back(L"main");
		hlsl = L"Shaders/" + tWstring + L".hlsl";
		arguments.push_back(hlsl.c_str());
		arguments.push_back(L"-Fo");
		out = tWstring + L".spv";
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
			std::ofstream outFile(L"Shaders/SPV/" + out, std::ios::binary);
			outFile.write(static_cast<const char*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize());
			outFile.close();
		}
	}
}

void VulkanRenderer::LoadModel(std::string filename)
{
	tinygltf::TinyGLTF gltfLoader;
	std::string error;
	std::string warning;

	unsigned long long pos = filename.find_last_of('/');
	std::string path = filename.substr(0, pos);

	bool fileLoaded = gltfLoader.LoadASCIIFromFile(&_model, &error, &warning, filename);
	//bool fileLoaded = gltfLoader.LoadBinaryFromFile(&_model, &error, &warning, filename);

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

	CreateGeometryBuffer();
}

void VulkanRenderer::CreateGeometryBuffer()
{
	void* data = _model.buffers[0].data.data();
	int size = _model.buffers[0].data.size() * sizeof(unsigned char);

	GvkHelper::create_buffer(_physicalDevice, _device, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &_geometryBuffer.buffer, &_geometryBuffer.memory);
	GvkHelper::write_to_buffer(_device, _geometryBuffer.memory, data, size);
}

void VulkanRenderer::CreateOffscreenFrameBuffer()
{
	auto CreateAttachment = [this](VkFormat format, VkImageUsageFlagBits usage, FrameBufferTexture* frameBufferTexture)
		{
			VkImageAspectFlags aspectMask = 0;
			VkImageLayout imageLayout;

			frameBufferTexture->format = format;

			if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
			{
				aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}

			if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

				if (format >= VK_FORMAT_D16_UNORM_S8_UINT) aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

				imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}

			assert(aspectMask > 0);

			GvkHelper::create_image(_physicalDevice, _device, { _width, _height, 1 }, 1, VK_SAMPLE_COUNT_1_BIT, format, VK_IMAGE_TILING_OPTIMAL, usage | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr, &frameBufferTexture->texture.texImage.image, &frameBufferTexture->texture.texImage.memory);
			GvkHelper::create_image_view(_device, frameBufferTexture->texture.texImage.image, format, aspectMask, 1, nullptr, &frameBufferTexture->texture.texImageView);
		};

	CreateAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &_offscreenFrameBuffer.position);
	CreateAttachment(VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &_offscreenFrameBuffer.normal);
	CreateAttachment(VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, &_offscreenFrameBuffer.albedo);

	std::vector<VkFormat> formats =
	{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	VkFormat depthFormat;
	GvkHelper::find_depth_format(_physicalDevice, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, formats.data(), &depthFormat);
	CreateAttachment(depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, &_offscreenFrameBuffer.depth);

	std::array<VkAttachmentDescription, 4> attachmentDescription;

	for (size_t i = 0; i < 4; i++)
	{
		attachmentDescription[i].flags = 0;
		attachmentDescription[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescription[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescription[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescription[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescription[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		if (i == 3)
		{
			attachmentDescription[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescription[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			attachmentDescription[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescription[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}

	//formats
	attachmentDescription[0].format = _offscreenFrameBuffer.position.format;
	attachmentDescription[1].format = _offscreenFrameBuffer.normal.format;
	attachmentDescription[2].format = _offscreenFrameBuffer.albedo.format;
	attachmentDescription[3].format = _offscreenFrameBuffer.depth.format;

	std::vector<VkAttachmentReference> colorAttachmentReference;
	colorAttachmentReference.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorAttachmentReference.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorAttachmentReference.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	VkAttachmentReference depthAttachmentReference;
	depthAttachmentReference.attachment = 3;
	depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription;
	subpassDescription.flags = 0;
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.pColorAttachments = colorAttachmentReference.data();
	subpassDescription.colorAttachmentCount = colorAttachmentReference.size();
	subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pResolveAttachments = nullptr;

	// Use subpass dependencies for attachment layout transitions
	std::array<VkSubpassDependency, 2> subpassDependencies;

	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassCreateInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassCreateInfo.pAttachments = attachmentDescription.data();
	renderPassCreateInfo.attachmentCount = attachmentDescription.size();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	renderPassCreateInfo.dependencyCount = subpassDependencies.size();
	renderPassCreateInfo.pDependencies = subpassDependencies.data();

	vkCreateRenderPass(_device, &renderPassCreateInfo, nullptr, &_offscreenFrameBuffer.renderPass);

	std::array<VkImageView, 4> imageViews;
	imageViews[0] = _offscreenFrameBuffer.position.texture.texImageView;
	imageViews[1] = _offscreenFrameBuffer.normal.texture.texImageView;
	imageViews[2] = _offscreenFrameBuffer.albedo.texture.texImageView;
	imageViews[3] = _offscreenFrameBuffer.depth.texture.texImageView;

	VkFramebufferCreateInfo frameBufferCreateInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
	frameBufferCreateInfo.renderPass = _offscreenFrameBuffer.renderPass;
	frameBufferCreateInfo.pAttachments = imageViews.data();
	frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
	frameBufferCreateInfo.width = _width;
	frameBufferCreateInfo.height = _height;
	frameBufferCreateInfo.layers = 1;

	vkCreateFramebuffer(_device, &frameBufferCreateInfo, nullptr, &_offscreenFrameBuffer.frameBuffer);

	//create sampler
	VkSamplerCreateInfo samplerCreateInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.maxAnisotropy = 1.f;
	samplerCreateInfo.maxLod = 1.f;
	samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
	samplerCreateInfo.minLod = 0;
	samplerCreateInfo.mipLodBias = 0;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

	vkCreateSampler(_device, &samplerCreateInfo, nullptr, &_colorSampler);
}

void VulkanRenderer::CreateUniformBuffers()
{
	_vlk.GetAspectRatio(_aspect);

	_offscreenData.world = GW::MATH::GIdentityMatrixF;
	GMatrix::LookAtLHF(vec4{ 0.f, 0.f, 0.f }, vec4{ 0.f, 0.f, 0.f }, vec4{ 0, 1, 0 }, _offscreenData.view);
	GMatrix::ProjectionVulkanLHF(G_DEGREE_TO_RADIAN(65), _aspect, .1f, 256.f, _offscreenData.proj);

	GvkHelper::create_buffer(_physicalDevice, _device, sizeof(UniformBufferOffscreen), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &_uniformBuffer.buffer, &_uniformBuffer.memory);
	GvkHelper::write_to_buffer(_device, _uniformBuffer.memory, &_offscreenData, sizeof(UniformBufferOffscreen));
}

void VulkanRenderer::CreateDescriptorSets()
{
	//offscreen
	std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
	{
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6}
	};

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	descriptorPoolCreateInfo.maxSets = 2;
	descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

	vkCreateDescriptorPool(_device, &descriptorPoolCreateInfo, nullptr, &_descriptorPool);

	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings =
	{
		{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
		{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
		{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
	};

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	descriptorSetLayoutCreateInfo.bindingCount = descriptorSetLayoutBindings.size();
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	vkCreateDescriptorSetLayout(_device, &descriptorSetLayoutCreateInfo, nullptr, &_descriptorSetLayout);

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	descriptorSetAllocateInfo.descriptorPool = _descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &_descriptorSetLayout;

	vkAllocateDescriptorSets(_device, &descriptorSetAllocateInfo, &_offscreenDescriptorSet);
	vkAllocateDescriptorSets(_device, &descriptorSetAllocateInfo, &_descriptorSet);
}

void VulkanRenderer::WriteDescriptorSets()
{
	std::vector<VkWriteDescriptorSet> writeDescriptorSets;
	VkWriteDescriptorSet writeDescriptorSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };

	VkDescriptorBufferInfo descriptorBufferInfo;
	descriptorBufferInfo.buffer = _uniformBuffer.buffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(UniformBufferOffscreen);

	std::vector<VkDescriptorImageInfo> descriptorImageInfos =
	{
		{_colorSampler, _offscreenFrameBuffer.position.texture.texImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
		{_colorSampler, _offscreenFrameBuffer.normal.texture.texImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
		{_colorSampler, _offscreenFrameBuffer.albedo.texture.texImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
	};

	writeDescriptorSet.descriptorCount = 1;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSet.dstBinding = 0;
	writeDescriptorSet.dstSet = _offscreenDescriptorSet;
	writeDescriptorSet.pBufferInfo = &descriptorBufferInfo;
	writeDescriptorSets.push_back(writeDescriptorSet);

	writeDescriptorSet.dstSet = _descriptorSet;
	writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	writeDescriptorSet.dstBinding = 1;
	writeDescriptorSet.pImageInfo = &descriptorImageInfos[0];
	writeDescriptorSets.push_back(writeDescriptorSet);

	writeDescriptorSet.dstSet = _descriptorSet;
	writeDescriptorSet.dstBinding = 2;
	writeDescriptorSet.pImageInfo = &descriptorImageInfos[1];
	writeDescriptorSets.push_back(writeDescriptorSet);

	writeDescriptorSet.dstSet = _descriptorSet;
	writeDescriptorSet.dstBinding = 3;
	writeDescriptorSet.pImageInfo = &descriptorImageInfos[2];
	writeDescriptorSets.push_back(writeDescriptorSet);

	vkUpdateDescriptorSets(_device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}

void VulkanRenderer::CreateGraphicsPipelines()
{
	_vlk.GetRenderPass((void**)&_renderPass);
	VkPipelineShaderStageCreateInfo pssci;

	GvkHelper::create_shader(_device, "Shaders/SPV/FragmentShader.spv", "main", VK_SHADER_STAGE_FRAGMENT_BIT, &_fragmentShaderModule, &pssci);
	GvkHelper::create_shader(_device, "Shaders/SPV/VertexShader.spv", "main", VK_SHADER_STAGE_VERTEX_BIT, &_vertexShaderModule, &pssci);

	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[2] = { {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO}, {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO} };
	//vertex shader
	pipelineShaderStageCreateInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	pipelineShaderStageCreateInfos[0].module = _vertexShaderModule;
	pipelineShaderStageCreateInfos[0].pName = "main";
	//pixel shader
	pipelineShaderStageCreateInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pipelineShaderStageCreateInfos[1].module = _fragmentShaderModule;
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
		{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
		{1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0},
		{2, 2, VK_FORMAT_R32G32_SFLOAT, 0},
	};

	//vertex input info
	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;

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
	pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT; //final comp
	pipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
	pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
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

	vkCreatePipelineLayout(_device, &pipelineLayoutCreateInfo, nullptr, &_pipelineLayout);

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
	graphicsPipelineCreateInfo.layout = _pipelineLayout;
	graphicsPipelineCreateInfo.renderPass = _renderPass;
	graphicsPipelineCreateInfo.subpass = 0;
	graphicsPipelineCreateInfo.basePipelineHandle = nullptr;

	vkCreateGraphicsPipelines(_device, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &_graphicsPipeline);

	//offscreen
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 3;
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescription;
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 3;
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescription;

	pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT; //offscreen

	GvkHelper::create_shader(_device, "Shaders/SPV/OffscreenFragmentShader.spv", "main", VK_SHADER_STAGE_FRAGMENT_BIT, &_offscreenFragmentShaderModule, &pssci);
	GvkHelper::create_shader(_device, "Shaders/SPV/OffscreenVertexShader.spv", "main", VK_SHADER_STAGE_VERTEX_BIT, &_offscreenVertexShaderModule, &pssci);

	//vertex shader
	pipelineShaderStageCreateInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	pipelineShaderStageCreateInfos[0].module = _offscreenVertexShaderModule;
	pipelineShaderStageCreateInfos[0].pName = "main";
	//pixel shader
	pipelineShaderStageCreateInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	pipelineShaderStageCreateInfos[1].module = _offscreenFragmentShaderModule;
	pipelineShaderStageCreateInfos[1].pName = "main";

	//important
	std::vector<VkPipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStates = { pipelineColorBlendAttachmentState, pipelineColorBlendAttachmentState, pipelineColorBlendAttachmentState };

	pipelineColorBlendStateCreateInfo.attachmentCount = pipelineColorBlendAttachmentStates.size();
	pipelineColorBlendStateCreateInfo.pAttachments = pipelineColorBlendAttachmentStates.data();
	graphicsPipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pStages = pipelineShaderStageCreateInfos;
	graphicsPipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.renderPass = _offscreenFrameBuffer.renderPass;

	vkCreateGraphicsPipelines(_device, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &_offscreenPipeline);
}

void VulkanRenderer::CreateCommandBuffers()
{
	_vlk.GetSwapchainImageCount(_swapchainImageCount);
	_drawCommandBuffers.resize(_swapchainImageCount);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	commandBufferAllocateInfo.commandPool = _commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = _swapchainImageCount;

	vkAllocateCommandBuffers(_device, &commandBufferAllocateInfo, _drawCommandBuffers.data());

	VkClearValue clearValues[2];
	clearValues[0].color = { { 0.2f, 0.0f, 0.0f, 0.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassBeginInfo.renderPass = _renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = _width;
	renderPassBeginInfo.renderArea.extent.height = _height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	VkCommandBufferBeginInfo commandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

	for (size_t i = 0; i < _swapchainImageCount; i++)
	{
		_vlk.GetSwapchainFramebuffer(i, (void**)&renderPassBeginInfo.framebuffer);

		vkBeginCommandBuffer(_drawCommandBuffers[i], &commandBufferBeginInfo);
		vkCmdBeginRenderPass(_drawCommandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport = { 0, 0, static_cast<float>(_width), static_cast<float>(_height), 0, 1 };
		VkRect2D scissor = { {0, 0}, {_width, _height} };

		vkCmdSetViewport(_drawCommandBuffers[i], 0, 1, &viewport);
		vkCmdSetScissor(_drawCommandBuffers[i], 0, 1, &scissor);
		vkCmdBindDescriptorSets(_drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_descriptorSet, 0, nullptr);
		vkCmdBindPipeline(_drawCommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);
		vkCmdDraw(_drawCommandBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(_drawCommandBuffers[i]);
		vkEndCommandBuffer(_drawCommandBuffers[i]);
	}
}

void VulkanRenderer::CreateDeferredCommandBuffers()
{
	if (!_offscreenCommandBuffer)
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
		commandBufferAllocateInfo.commandPool = _commandPool;
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = 1;

		vkAllocateCommandBuffers(_device, &commandBufferAllocateInfo, &_offscreenCommandBuffer);
	}

	VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_offscreenSemaphore);

	VkCommandBufferBeginInfo commandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

	// Clear values for all attachments written in the fragment shader
	std::array<VkClearValue, 4> clearValues;
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[3].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassBeginInfo.renderPass = _offscreenFrameBuffer.renderPass;
	renderPassBeginInfo.framebuffer = _offscreenFrameBuffer.frameBuffer;
	renderPassBeginInfo.renderArea.extent.width = _width;
	renderPassBeginInfo.renderArea.extent.height = _height;
	renderPassBeginInfo.clearValueCount = clearValues.size();
	renderPassBeginInfo.pClearValues = clearValues.data();

	vkBeginCommandBuffer(_offscreenCommandBuffer, &commandBufferBeginInfo);
	vkCmdBeginRenderPass(_offscreenCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport = { 0, 0, static_cast<float>(_width), static_cast<float>(_height), 0, 1 };
	VkRect2D scissor = { {0, 0}, {_width, _height} };

	vkCmdSetViewport(_offscreenCommandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(_offscreenCommandBuffer, 0, 1, &scissor);

	vkCmdBindPipeline(_offscreenCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _offscreenPipeline);

	vkCmdBindDescriptorSets(_offscreenCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_offscreenDescriptorSet, 0, nullptr);

	for (auto& node : _model.nodes)
	{
		auto& mesh = _model.meshes[node.mesh];
		for (auto& prim : mesh.primitives)
		{
			const auto& posAccessor = _model.accessors[prim.attributes.find("POSITION")->second];
			const auto& nrmAccessor = _model.accessors[prim.attributes.find("NORMAL")->second];
			const auto& uv0Accessor = _model.accessors[prim.attributes.find("TEXCOORD_0")->second];
			const auto& idx = _model.accessors[prim.indices];

			vkCmdBindVertexBuffers(_offscreenCommandBuffer, 0, 1, &_geometryBuffer.buffer, &_model.bufferViews[posAccessor.bufferView].byteOffset + posAccessor.byteOffset);
			vkCmdBindVertexBuffers(_offscreenCommandBuffer, 1, 1, &_geometryBuffer.buffer, &_model.bufferViews[nrmAccessor.bufferView].byteOffset + nrmAccessor.byteOffset);
			vkCmdBindVertexBuffers(_offscreenCommandBuffer, 2, 1, &_geometryBuffer.buffer, &_model.bufferViews[uv0Accessor.bufferView].byteOffset + uv0Accessor.byteOffset);
			vkCmdBindIndexBuffer(_offscreenCommandBuffer, _geometryBuffer.buffer, _model.bufferViews[idx.bufferView].byteOffset + idx.byteOffset, idx.componentType == 5121 ? VK_INDEX_TYPE_UINT8_KHR : idx.componentType == 5123 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32);

			vkCmdDrawIndexed(_offscreenCommandBuffer, idx.count, 1, 0, 0, 0);
		}
	}

	vkCmdEndRenderPass(_offscreenCommandBuffer);
	vkEndCommandBuffer(_offscreenCommandBuffer);
}

void VulkanRenderer::CleanUp()
{

}

std::string VulkanRenderer::ShaderAsString(const char* shaderFilePath)
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

VulkanRenderer::VulkanRenderer(GWindow win) : Renderer(win)
{
#ifndef NDEBUG
	const char* debugLayers[] =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	if (-_vlk.Create(_win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT | GW::GRAPHICS::TRIPLE_BUFFER, sizeof(debugLayers) / sizeof(debugLayers[0]), debugLayers, 0, nullptr, 0, nullptr, false)) return;
#else
	if (-_vlk.Create(_win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT | GW::GRAPHICS::TRIPLE_BUFFER)) return; //return if creation didn't work
#endif

	_vlk.GetDevice((void**)&_device);
	_vlk.GetPhysicalDevice((void**)&_physicalDevice);
	_vlk.GetCommandPool((void**)&_commandPool);
	_vlk.GetGraphicsQueue((void**)&_queue);
	_vlk.GetSwapchain((void**)&_swapchain);

	CompileShaders();
	LoadModel("Models/Test/Test.gltf");
	CreateOffscreenFrameBuffer();
	CreateUniformBuffers();
	CreateDescriptorSets();
	WriteDescriptorSets();
	CreateGraphicsPipelines();
	CreateCommandBuffers();
	CreateDeferredCommandBuffers();

	VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_presentCompleteSemaphore);
	vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_renderCompleteSemaphore);

	_submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	_submitInfo.pWaitDstStageMask = &_submitPipelineStages;
	_submitInfo.waitSemaphoreCount = 1;
	_submitInfo.pWaitSemaphores = &_presentCompleteSemaphore;
	_submitInfo.signalSemaphoreCount = 1;
	_submitInfo.pSignalSemaphores = &_renderCompleteSemaphore;

	_shutdown.Create(_vlk, [&]()
		{
			if (+_shutdown.Find(GW::GRAPHICS::GVulkanSurface::Events::RELEASE_RESOURCES, true))
			{
				CleanUp();
			}
		});
}

VulkanRenderer::~VulkanRenderer()
{
}

void VulkanRenderer::Render()
{
	GvkHelper::write_to_buffer(_device, _uniformBuffer.memory, &_offscreenData, sizeof(UniformBufferOffscreen));

	auto now = std::chrono::steady_clock::now();
	_deltaTime = now - _lastUpdate;
	_lastUpdate = now;

	unsigned int currentBuffer;

	vkAcquireNextImageKHR(_device, _swapchain, 0, _presentCompleteSemaphore, nullptr, &currentBuffer);

	_submitInfo.pWaitSemaphores = &_presentCompleteSemaphore;
	_submitInfo.pSignalSemaphores = &_offscreenSemaphore;
	_submitInfo.commandBufferCount = 1;
	_submitInfo.pCommandBuffers = &_offscreenCommandBuffer;

	vkQueueSubmit(_queue, 1, &_submitInfo, VK_NULL_HANDLE);

	_submitInfo.pWaitSemaphores = &_offscreenSemaphore;
	_submitInfo.pSignalSemaphores = &_renderCompleteSemaphore;
	_submitInfo.pCommandBuffers = &_drawCommandBuffers[currentBuffer];

	vkQueueSubmit(_queue, 1, &_submitInfo, VK_NULL_HANDLE);

	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &_swapchain;
	presentInfo.pImageIndices = &currentBuffer;

	if (_renderCompleteSemaphore)
	{
		presentInfo.pWaitSemaphores = &_renderCompleteSemaphore;
		presentInfo.waitSemaphoreCount = 1;
	}

	vkQueuePresentKHR(_queue, &presentInfo);
	vkQueueWaitIdle(_queue);
}

void VulkanRenderer::UpdateCamera()
{
	_win.IsFocus(_isFocused);

	if (!_isFocused) return;

	GW::MATH::GMATRIXF cam = GW::MATH::GIdentityMatrixF;

	GMatrix::InverseF(_offscreenData.view, cam);

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

	GMatrix::InverseF(cam, _offscreenData.view);

}

DX12Renderer::DX12Renderer(GWindow win) : Renderer(win)
{
	if (-_dxs.Create(_win, GW::GRAPHICS::DEPTH_BUFFER_SUPPORT)) return; //return if creation didn't work
}

DX12Renderer::~DX12Renderer()
{
}

void DX12Renderer::Render()
{
}

void DX12Renderer::UpdateCamera()
{
}
