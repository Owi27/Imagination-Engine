#include "pch.h"
#include "Renderer.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"
#include "MathOverloads.h"

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
#ifndef NDEBUG
		arguments.push_back(L"-Zi");
		arguments.push_back(L"-Qembed_debug");
#endif // NDEBUG


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
}

void VulkanRenderer::CreateGeometryData()
{
	int vCount = 0, iCount = 0, firstIdx = 0, vertexOffset = 0;
	DrawInfo di;

	for (auto& node : _model.nodes)
	{
		auto& mesh = _model.meshes[node.mesh];
		for (auto prim : mesh.primitives)
		{
			di.firstIdx = _geometryData.indices.size();
			di.vertexOffset = _geometryData.positions.size();
			di.nodeWorld = GetLocalMatrix(node);

			//position
			tinygltf::Accessor& accessor = _model.accessors[prim.attributes.find("POSITION")->second];
			tinygltf::BufferView& bufferView = _model.bufferViews[accessor.bufferView];
			tinygltf::Buffer& buffer = _model.buffers[bufferView.buffer];
			vCount = accessor.count;
			{
				auto pos = (const float*)&buffer.data[bufferView.byteOffset + accessor.byteOffset];

				if (pos)
				{
					for (size_t i = 0; i < accessor.count; i++)
					{
						_geometryData.positions.push_back(vec3{ pos[i * 3 + 0], pos[i * 3 + 1], pos[i * 3 + 2] });
					}

					//_offscreenData.min = { (float)accessor.minValues[0], (float)accessor.minValues[1] , (float)accessor.minValues[2] };
				}
			}

			//normals
			accessor = _model.accessors[prim.attributes.find("NORMAL")->second];
			bufferView = _model.bufferViews[accessor.bufferView];
			buffer = _model.buffers[bufferView.buffer];
			{
				auto nrm = (const float*)&buffer.data[bufferView.byteOffset + accessor.byteOffset];

				if (nrm)
				{
					for (size_t i = 0; i < accessor.count; i++)
					{
						_geometryData.normals.push_back(vec3{ nrm[i * 3 + 0], nrm[i * 3 + 1], nrm[i * 3 + 2] });
					}
				}
			}

			//texCoord
			accessor = _model.accessors[prim.attributes.find("TEXCOORD_0")->second];
			bufferView = _model.bufferViews[accessor.bufferView];
			buffer = _model.buffers[bufferView.buffer];
			{
				auto uv0 = (const float*)&buffer.data[bufferView.byteOffset + accessor.byteOffset];

				if (uv0)
				{
					for (size_t i = 0; i < accessor.count; i++)
					{
						_geometryData.texCoords.push_back(vec2{ uv0[i * 2 + 0], uv0[i * 2 + 1] });
					}
				}
			}

			//index buffer
			accessor = _model.accessors[prim.indices];
			bufferView = _model.bufferViews[accessor.bufferView];
			buffer = _model.buffers[bufferView.buffer];
			iCount = accessor.count;
			di.idxCount = (unsigned int)accessor.count;
			{
				switch (accessor.componentType)
				{
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
				{
					std::vector<unsigned int> uIntPrims;
					uIntPrims.resize(accessor.count);
					memcpy(uIntPrims.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(unsigned int));
					_geometryData.indices.insert(_geometryData.indices.end(), uIntPrims.begin(), uIntPrims.end());
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
				{
					std::vector<unsigned short> uShortPrims;
					uShortPrims.resize(accessor.count);
					memcpy(uShortPrims.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(unsigned short));
					_geometryData.indices.insert(_geometryData.indices.end(), uShortPrims.begin(), uShortPrims.end());
					break;
				}
				case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
				{
					std::vector<unsigned char> uCharPrims;
					uCharPrims.resize(accessor.count);
					memcpy(uCharPrims.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(unsigned char));
					_geometryData.indices.insert(_geometryData.indices.end(), uCharPrims.begin(), uCharPrims.end());
					break;
				}
				default:
					std::cout << "Index component type " << accessor.componentType << " not supported!\n";
					return;
				}
			}

			//tangent
			if (prim.attributes.contains("TANGENT"))
			{
				accessor = _model.accessors[prim.attributes.find("TANGENT")->second];
				bufferView = _model.bufferViews[accessor.bufferView];
				buffer = _model.buffers[bufferView.buffer];
				{
					auto tan = (const float*)&buffer.data[bufferView.byteOffset + accessor.byteOffset];

					if (tan)
					{
						for (size_t i = 0; i < accessor.count; i++)
						{
							_geometryData.tangents.push_back(vec4{ tan[i * 4 + 0], tan[i * 4 + 1], tan[i * 4 + 2], tan[i * 4 + 3] });
						}
					}
				}
			}
			else
			{
				if (node.name.find("Cone") != std::string::npos) _geometryData.tangents.push_back(vec4{ 0, 0, 0, 1 });
				else
				{

					std::vector<vec3> tangent(vCount);
					std::vector<vec3> biTangent(vCount);

					for (size_t i = 0; i < iCount; i += 3)
					{
						//local index
						unsigned int i0 = _geometryData.indices[firstIdx + i + 0];
						unsigned int i1 = _geometryData.indices[firstIdx + i + 1];
						unsigned int i2 = _geometryData.indices[firstIdx + i + 2];
						assert(i0 < vCount);
						assert(i1 < vCount);
						assert(i2 < vCount);

						//global index
						unsigned int gi0 = i0 + vertexOffset;
						unsigned int gi1 = i1 + vertexOffset;
						unsigned int gi2 = i2 + vertexOffset;

						const auto& p0 = _geometryData.positions[gi0];
						const auto& p1 = _geometryData.positions[gi1];
						const auto& p2 = _geometryData.positions[gi2];

						const auto& uv0 = _geometryData.texCoords[gi0];
						const auto& uv1 = _geometryData.texCoords[gi1];
						const auto& uv2 = _geometryData.texCoords[gi2];

						vec3 e1, e2;
						{
							GVector2D::Subtract3F(p1, p0, e1);
							GVector2D::Subtract3F(p2, p0, e2);
						}

						vec2 duvE1, duvE2;
						{
							GVector2D::Subtract2F(uv1, uv0, duvE1);
							GVector2D::Subtract2F(uv2, uv0, duvE2);
						}

						float r = 1.f;
						float a = duvE1.x * duvE2.y - duvE2.x * duvE1.y;

						if (fabs(a) > 0) //catch degenerated UVs
						{
							r = 1.f / a;
						}

						vec3 t, b;
						{
							vec3 v[3];

							//t
							GVector2D::Scale3F(e1, duvE2.y, v[0]);
							GVector2D::Scale3F(e2, duvE1.y, v[1]);
							GVector2D::Subtract3F(v[0], v[1], v[2]);
							GVector2D::Scale3F(v[2], r, t);

							//b
							GVector2D::Scale3F(e2, duvE1.x, v[0]);
							GVector2D::Scale3F(e1, duvE2.x, v[1]);
							GVector2D::Subtract3F(v[0], v[1], v[2]);
							GVector2D::Scale3F(v[2], r, b);
						}

						GVector2D::Add3F(tangent[i0], t, tangent[i0]);
						GVector2D::Add3F(tangent[i1], t, tangent[i1]);
						GVector2D::Add3F(tangent[i2], t, tangent[i2]);

						GVector2D::Add3F(biTangent[i0], b, biTangent[i0]);
						GVector2D::Add3F(biTangent[i1], b, biTangent[i1]);
						GVector2D::Add3F(biTangent[i2], b, biTangent[i2]);
					}

					for (unsigned int a = 0; a < vCount; a++)
					{
						const auto& t = tangent[a];
						const auto& b = biTangent[a];
						const auto& n = _geometryData.normals[vertexOffset + a];

						vec3 oTangent;
						{
							vec3 v;
							float d;
							GVector2D::Dot3F(n, t, d);
							GVector2D::Scale3F(n, d, v);
							GVector2D::Subtract3F(t, v, v);
							GVector2D::Normalize3F(v, oTangent);
						}

						if (oTangent.x == 0 && oTangent.y == 0 && oTangent.z == 0) //if tangent invalid
						{
							if (fabsf(n.x) > fabsf(n.y))
								GVector2D::Scale3F(vec3{ n.z, 0, -n.x }, 1 / sqrtf(n.x * n.x + n.z * n.z), oTangent);
							else
								GVector2D::Scale3F(vec3{ 0, -n.z, n.y }, 1 / sqrtf(n.y * n.y + n.z * n.z), oTangent);
						}

						//calculate handedness
						float handedness;
						{
							float f;
							vec3 v;
							GVector2D::Cross3F(n, t, v);
							GVector2D::Dot3F(v, b, f);
							handedness = f < 0.f ? 1.f : -1.f;
						}

						_geometryData.tangents.emplace_back(vec4{ oTangent.x, oTangent.y, oTangent.z, handedness });
					}
				}
			}

			_drawInfo.push_back(di);
		}
	}
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

void VulkanRenderer::CreateForwardRenderer()
{
}

void VulkanRenderer::CreateUniformBuffers()
{

	//final
	std::default_random_engine gen(777);
	std::uniform_real_distribution<float> distribution(0.f, 1.f);
	std::uniform_real_distribution<float> distribution2(-3.f, 3.f);

	//lights
	{

		for (size_t i = 0; i < 10; i++)
		{
			_finalData.lights[i].pos = { distribution2(gen) , distribution2(gen) , distribution2(gen) };
			_finalData.lights[i].col = { distribution(gen) , distribution(gen) , distribution(gen) };
			_finalData.lights[i].radius = 5.f;
		}
	}

	_finalData.view = _offscreenData.view.row4;
	GvkHelper::create_buffer(_physicalDevice, _device, sizeof(UniformBufferFinal), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &_finalUniformBuffer.buffer, &_finalUniformBuffer.memory);
	GvkHelper::write_to_buffer(_device, _finalUniformBuffer.memory, &_finalData, sizeof(UniformBufferFinal));
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
		{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
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
	//offscreen
	VkDescriptorBufferInfo descriptorBufferInfo;
	descriptorBufferInfo.buffer = _offscreenUniformBuffer.buffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(UniformBufferOffscreen);

	std::vector<VkWriteDescriptorSet> offscreenWriteDescriptorSets =
	{
		MakeWrite(_offscreenDescriptorSet, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &descriptorBufferInfo),
	};

	vkUpdateDescriptorSets(_device, offscreenWriteDescriptorSets.size(), offscreenWriteDescriptorSets.data(), 0, nullptr);

	//final
	descriptorBufferInfo.buffer = _finalUniformBuffer.buffer;
	descriptorBufferInfo.offset = 0;
	descriptorBufferInfo.range = sizeof(UniformBufferFinal);

	std::vector<VkDescriptorImageInfo> descriptorImageInfos =
	{
		{_colorSampler, _offscreenFrameBuffer.position.texture.texImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
		{_colorSampler, _offscreenFrameBuffer.normal.texture.texImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
		{_colorSampler, _offscreenFrameBuffer.albedo.texture.texImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
	};

	std::vector<VkWriteDescriptorSet> finalWriteDescriptorSets =
	{
		MakeWrite(_descriptorSet, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &descriptorBufferInfo),
		MakeWrite(_descriptorSet, 1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &descriptorImageInfos[0], nullptr),
		MakeWrite(_descriptorSet, 2, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &descriptorImageInfos[1], nullptr),
		MakeWrite(_descriptorSet, 3, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &descriptorImageInfos[2], nullptr),
	};

	vkUpdateDescriptorSets(_device, finalWriteDescriptorSets.size(), finalWriteDescriptorSets.data(), 0, nullptr);
}

void VulkanRenderer::CreateGraphicsPipelines()
{
	VkPushConstantRange pushConstantRange;
	{
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PCR);
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	}

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
	VkVertexInputBindingDescription vertexInputBindingDescription[4] =
	{
		{0, sizeof(vec3), VK_VERTEX_INPUT_RATE_VERTEX}, //pos
		{1, sizeof(vec3), VK_VERTEX_INPUT_RATE_VERTEX}, //nrm
		{2, sizeof(vec2), VK_VERTEX_INPUT_RATE_VERTEX}, //uv
		{3, sizeof(vec4), VK_VERTEX_INPUT_RATE_VERTEX}, //tan
	};

	//vertex attributes
	VkVertexInputAttributeDescription vertexInputAttributeDescription[4] =
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
		{3, 3, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
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
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

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
	pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 4;
	pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescription;
	pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 4;
	pipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vertexInputAttributeDescription;

	pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT; //offscreen

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

	std::vector<VkBuffer> vertexBufferss =
	{
		_vertexBuffer[0].buffer,
		_vertexBuffer[1].buffer,
		_vertexBuffer[2].buffer,
		_vertexBuffer[3].buffer,
	};

	std::vector<VkDeviceSize> offsets = { 0, 0, 0, 0 };
	vkCmdBindVertexBuffers(_offscreenCommandBuffer, 0, vertexBufferss.size(), vertexBufferss.data(), offsets.data());
	vkCmdBindIndexBuffer(_offscreenCommandBuffer, _indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

	int i = 0;
	for (auto& node : _model.nodes)
	{
		auto& mesh = _model.meshes[node.mesh];
		for (auto& prim : mesh.primitives)
		{
			vkCmdPushConstants(_offscreenCommandBuffer, _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PCR), &_drawInfo[i].nodeWorld);
			vkCmdDrawIndexed(_offscreenCommandBuffer, _drawInfo[i].idxCount, 1, _drawInfo[i].firstIdx, _drawInfo[i].vertexOffset, 0);
			i++;
		}
	}

	vkCmdEndRenderPass(_offscreenCommandBuffer);
	vkEndCommandBuffer(_offscreenCommandBuffer);
}

void VulkanRenderer::CreateFrameGraphNodes()
{
	FrameGraphNode offscreenBuffers;
	{
		offscreenBuffers.name = "Offscreen Buffers";
		offscreenBuffers.outputResources = { "Vertex Buffers", "Index Buffer" };
		offscreenBuffers.Setup = [&]()
			{
				FrameGraphBufferResource vertexBuffers;
				{
					vertexBuffers.parent = offscreenBuffers.name;
					vertexBuffers.name = offscreenBuffers.outputResources[0];
					CreateGeometryData();
					vertexBuffers.buffers.resize(4);
					//position
					GvkHelper::create_buffer(_physicalDevice, _device, sizeof(vec3) * _geometryData.positions.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexBuffers.buffers[0].buffer, &vertexBuffers.buffers[0].memory);
					GvkHelper::write_to_buffer(_device, vertexBuffers.buffers[0].memory, _geometryData.positions.data(), sizeof(vec3) * _geometryData.positions.size());
					//normal
					GvkHelper::create_buffer(_physicalDevice, _device, sizeof(vec3) * _geometryData.normals.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexBuffers.buffers[1].buffer, &vertexBuffers.buffers[1].memory);
					GvkHelper::write_to_buffer(_device, vertexBuffers.buffers[1].memory, _geometryData.normals.data(), sizeof(vec3) * _geometryData.normals.size());
					//texcoord
					GvkHelper::create_buffer(_physicalDevice, _device, sizeof(vec2) * _geometryData.texCoords.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexBuffers.buffers[2].buffer, &vertexBuffers.buffers[2].memory);
					GvkHelper::write_to_buffer(_device, vertexBuffers.buffers[2].memory, _geometryData.texCoords.data(), sizeof(vec2) * _geometryData.texCoords.size());
					//tangent
					GvkHelper::create_buffer(_physicalDevice, _device, sizeof(vec4) * _geometryData.tangents.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vertexBuffers.buffers[3].buffer, &vertexBuffers.buffers[3].memory);
					GvkHelper::write_to_buffer(_device, vertexBuffers.buffers[3].memory, _geometryData.tangents.data(), sizeof(vec4) * _geometryData.tangents.size());
				}
				_frameGraph->AddResource(vertexBuffers.name, &vertexBuffers);

				FrameGraphBufferResource indexBuffer;
				{
					indexBuffer.parent = offscreenBuffers.name;
					indexBuffer.name = offscreenBuffers.outputResources[1];
					indexBuffer.buffers.resize(1);
					GvkHelper::create_buffer(_physicalDevice, _device, sizeof(unsigned int) * _geometryData.indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &_indexBuffer.buffer, &_indexBuffer.memory);
					GvkHelper::write_to_buffer(_device, _indexBuffer.memory, _geometryData.indices.data(), sizeof(unsigned int) * _geometryData.indices.size());
				}
				_frameGraph->AddResource(indexBuffer.name, &indexBuffer);

				FrameGraphBufferResource offscreenUniformBuffer;
				{
					offscreenUniformBuffer.parent = offscreenBuffers.name;
					offscreenUniformBuffer.name = "Offscreen UB";
					offscreenUniformBuffer.buffers.resize(1);
					_vlk.GetAspectRatio(_aspect);
					UniformBufferOffscreen data;
					{
						data.world = GW::MATH::GIdentityMatrixF;
						GMatrix::LookAtLHF(vec4{ 7.f, 4.f, -10.f }, vec4{ 0.f, 0.f, 0.f }, vec4{ 0, 1, 0 }, data.view);
						GMatrix::ProjectionVulkanLHF(G_DEGREE_TO_RADIAN(65), _aspect, .1f, 256.f, data.proj);
					}

					offscreenUniformBuffer.data.push_back(static_cast<void*>(&data));

					GvkHelper::create_buffer(_physicalDevice, _device, sizeof(UniformBufferOffscreen), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &offscreenUniformBuffer.buffers[0].buffer, &offscreenUniformBuffer.buffers[0].memory);
					GvkHelper::write_to_buffer(_device, offscreenUniformBuffer.buffers[0].memory, &data, sizeof(UniformBufferOffscreen));
				}
				_frameGraph->AddResource(offscreenUniformBuffer.name, &offscreenUniformBuffer);

				offscreenBuffers.isSetupComplete = true;
			};
		offscreenBuffers.Execute = [&](VkCommandBuffer commandBuffer)
			{
				Prepare(offscreenBuffers);
			};
		offscreenBuffers.shouldExecute = true;
	}
	_frameGraph->AddNode(offscreenBuffers);

	FrameGraphNode offscreenPass;
	{
		offscreenPass.name = "Offscreen Pass";
		offscreenPass.inputResources = { "Vertex Buffers", "Index Buffer", "Offscreen UB" };
		offscreenPass.outputResources = { "GBuffer: Position", "GBuffer: Normal", "GBuffer: Albedo", "Depth Buffer" };
		offscreenPass.Setup = [&]()
			{
				//assert that input resources are prepared
				Debug::CheckInputResources(*_frameGraph, offscreenPass);

				//FRAMEBUFFER
				{
					FrameGraphImageResource gBufferPos, gBufferNrm, gBufferAlb, depth;
					{
						//Gbuffer Position
						gBufferPos.name = offscreenPass.outputResources[0];
						gBufferPos.parent = offscreenPass.name;
						gBufferPos.extent = { _width, _height, 1 };
						gBufferPos.format = VK_FORMAT_R16G16B16A16_SFLOAT;
						GvkHelper::create_image(_physicalDevice, _device, gBufferPos.extent, 1, VK_SAMPLE_COUNT_1_BIT, gBufferPos.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr, &gBufferPos.image.image, &gBufferPos.image.memory);
						GvkHelper::create_image_view(_device, gBufferPos.image.image, gBufferPos.format, VK_IMAGE_ASPECT_COLOR_BIT, 1, nullptr, &gBufferPos.image.imageView);

						//Gbuffer Normal
						gBufferNrm.name = offscreenPass.outputResources[1];
						gBufferNrm.parent = offscreenPass.name;
						gBufferNrm.extent = { _width, _height, 1 };
						gBufferNrm.format = VK_FORMAT_R16G16B16A16_SFLOAT;
						GvkHelper::create_image(_physicalDevice, _device, gBufferNrm.extent, 1, VK_SAMPLE_COUNT_1_BIT, gBufferNrm.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr, &gBufferNrm.image.image, &gBufferNrm.image.memory);
						GvkHelper::create_image_view(_device, gBufferNrm.image.image, gBufferNrm.format, VK_IMAGE_ASPECT_COLOR_BIT, 1, nullptr, &gBufferNrm.image.imageView);

						//Gbuffer Albedo
						gBufferAlb.name = offscreenPass.outputResources[2];
						gBufferAlb.parent = offscreenPass.name;
						gBufferAlb.extent = { _width, _height, 1 };
						gBufferAlb.format = VK_FORMAT_R8G8B8A8_UNORM;
						GvkHelper::create_image(_physicalDevice, _device, gBufferAlb.extent, 1, VK_SAMPLE_COUNT_1_BIT, gBufferAlb.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr, &gBufferAlb.image.image, &gBufferAlb.image.memory);
						GvkHelper::create_image_view(_device, gBufferAlb.image.image, gBufferAlb.format, VK_IMAGE_ASPECT_COLOR_BIT, 1, nullptr, &gBufferAlb.image.imageView);

						std::vector<VkFormat> formats =
						{
							VK_FORMAT_D32_SFLOAT_S8_UINT,
							VK_FORMAT_D32_SFLOAT,
							VK_FORMAT_D24_UNORM_S8_UINT,
							VK_FORMAT_D16_UNORM_S8_UINT,
							VK_FORMAT_D16_UNORM
						};

						depth.name = offscreenPass.outputResources[3];
						depth.parent = offscreenPass.name;
						depth.extent = { _width, _height, 1 };
						//set format
						GvkHelper::find_depth_format(_physicalDevice, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, formats.data(), &depth.format);
						GvkHelper::create_image(_physicalDevice, _device, depth.extent, 1, VK_SAMPLE_COUNT_1_BIT, depth.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr, &depth.image.image, &depth.image.memory);
						GvkHelper::create_image_view(_device, depth.image.image, depth.format, VK_IMAGE_ASPECT_COLOR_BIT, 1, nullptr, &depth.image.imageView);
					}

					_frameGraph->AddResource(gBufferPos.name, &gBufferPos);
					_frameGraph->AddResource(gBufferNrm.name, &gBufferNrm);
					_frameGraph->AddResource(gBufferAlb.name, &gBufferAlb);
					_frameGraph->AddResource(depth.name, &depth);

					//FrameGraphImageResource* depthResource = dynamic_cast<FrameGraphImageResource*>(_frameGraph->GetResource(offscreenPass.inputResources[0]));

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

					FrameGraphImageResource* posResource = dynamic_cast<FrameGraphImageResource*>(_frameGraph->GetResource(offscreenPass.outputResources[0])),
						* nrmResource = dynamic_cast<FrameGraphImageResource*>(_frameGraph->GetResource(offscreenPass.outputResources[1])),
						* albResource = dynamic_cast<FrameGraphImageResource*>(_frameGraph->GetResource(offscreenPass.outputResources[2])),
						* depthResource = dynamic_cast<FrameGraphImageResource*>(_frameGraph->GetResource(offscreenPass.outputResources[3]));

					//formats
					attachmentDescription[0].format = posResource->format;
					attachmentDescription[1].format = nrmResource->format;
					attachmentDescription[2].format = albResource->format;
					attachmentDescription[3].format = depthResource->format;

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

					vkCreateRenderPass(_device, &renderPassCreateInfo, nullptr, &offscreenPass.frameBuffer.renderPass);

					std::array<VkImageView, 4> imageViews =
					{
						posResource->image.imageView,
						nrmResource->image.imageView,
						albResource->image.imageView,
						depthResource->image.imageView
					};

					VkFramebufferCreateInfo frameBufferCreateInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
					frameBufferCreateInfo.renderPass = offscreenPass.frameBuffer.renderPass;
					frameBufferCreateInfo.pAttachments = imageViews.data();
					frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
					frameBufferCreateInfo.width = _width;
					frameBufferCreateInfo.height = _height;
					frameBufferCreateInfo.layers = 1;

					vkCreateFramebuffer(_device, &frameBufferCreateInfo, nullptr, &offscreenPass.frameBuffer.frameBuffer);

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

				offscreenPass.isSetupComplete = true;
			};
		offscreenPass.Execute = [&](VkCommandBuffer commandBuffer)
			{
				FrameGraphBufferResource* vBuffer = dynamic_cast<FrameGraphBufferResource*>(_frameGraph->GetResource(offscreenPass.inputResources[0]));
				FrameGraphBufferResource* iBuffer = dynamic_cast<FrameGraphBufferResource*>(_frameGraph->GetResource(offscreenPass.inputResources[1]));
				FrameGraphBufferResource* offscreenUB = dynamic_cast<FrameGraphBufferResource*>(_frameGraph->GetResource(offscreenPass.inputResources[2]));
				auto* data = static_cast<UniformBufferOffscreen*>(offscreenUB->data[0]);
				auto now = std::chrono::steady_clock::now();
				_deltaTime = now - _lastUpdate;
				_lastUpdate = now;

				data->deltaTime = _deltaTime.count();
				GvkHelper::write_to_buffer(_device, offscreenUB->buffers[0].memory, &data, sizeof(UniformBufferOffscreen));


				VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
				if (!_offscreenSemaphore) vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_offscreenSemaphore);

				VkCommandBufferBeginInfo commandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

				// Clear values for all attachments written in the fragment shader
				std::array<VkClearValue, 4> clearValues;
				clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
				clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
				clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
				clearValues[3].depthStencil = { 1.0f, 0 };

				VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
				renderPassBeginInfo.renderPass = offscreenPass.frameBuffer.renderPass;
				renderPassBeginInfo.framebuffer = offscreenPass.frameBuffer.frameBuffer;
				renderPassBeginInfo.renderArea.extent.width = _width;
				renderPassBeginInfo.renderArea.extent.height = _height;
				renderPassBeginInfo.clearValueCount = clearValues.size();
				renderPassBeginInfo.pClearValues = clearValues.data();

				vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
				vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

				VkViewport viewport = { 0, 0, static_cast<float>(_width), static_cast<float>(_height), 0, 1 };
				VkRect2D scissor = { {0, 0}, {_width, _height} };

				vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _offscreenPipeline);

				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 1, &_offscreenDescriptorSet, 0, nullptr);

				std::array<VkBuffer, 4> vertexBuffers =
				{
					vBuffer->buffers[0].buffer,
					vBuffer->buffers[1].buffer,
					vBuffer->buffers[2].buffer,
					vBuffer->buffers[3].buffer,
				};

				std::vector<VkDeviceSize> offsets = { 0, 0, 0, 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());
				vkCmdBindIndexBuffer(commandBuffer, iBuffer->buffers[0].buffer, 0, VK_INDEX_TYPE_UINT32);

				int i = 0;
				for (auto& node : _model.nodes)
				{
					auto& mesh = _model.meshes[node.mesh];
					for (auto& prim : mesh.primitives)
					{
						vkCmdPushConstants(commandBuffer, offscreenPass.frameBuffer.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PCR), &_drawInfo[i].nodeWorld);
						vkCmdDrawIndexed(commandBuffer, _drawInfo[i].idxCount, 1, _drawInfo[i].firstIdx, _drawInfo[i].vertexOffset, 0);
						i++;
					}
				}

				vkCmdEndRenderPass(commandBuffer);
				vkEndCommandBuffer(commandBuffer);

				Prepare(offscreenPass);
			};
		offscreenPass.shouldExecute = false;
	};
	_frameGraph->AddNode(offscreenPass);
}

void VulkanRenderer::UpdateLights()
{
	_finalData.lights[0].pos.y = sin(_deltaTime.count() * 360);
	_finalData.lights[1].pos.y = sin(_deltaTime.count() * 360);
	_finalData.lights[2].pos.y = sin(_deltaTime.count() * 360);
	_finalData.lights[3].pos.y = sin(_deltaTime.count() * 360);
}


void VulkanRenderer::CleanUp()
{
	vkDeviceWaitIdle(_device);


}

void VulkanRenderer::Prepare(FrameGraphNode node)
{
	for (auto out : node.outputResources)
	{
		_frameGraph->GetResource(out)->prepared = true;
	}
}

VkWriteDescriptorSet VulkanRenderer::MakeWrite(VkDescriptorSet descriptorSet, unsigned int binding, unsigned int descriptorCount, VkDescriptorType type, const VkDescriptorImageInfo* pImageInfo, const VkDescriptorBufferInfo* pBufferInfo)
{
	VkWriteDescriptorSet writeDescriptorSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	writeDescriptorSet.descriptorCount = descriptorCount;
	writeDescriptorSet.descriptorType = type;
	writeDescriptorSet.dstBinding = binding;
	writeDescriptorSet.dstSet = descriptorSet;
	writeDescriptorSet.pBufferInfo = pBufferInfo;
	writeDescriptorSet.pImageInfo = pImageInfo;
	return writeDescriptorSet;
}

mat4 VulkanRenderer::GetLocalMatrix(const tinygltf::Node& node)
{
	//translation
	vec4 translation = { 0, 0, 0, 0 };
	if (node.translation.size() == 3)
	{
		translation = { (float)node.translation[0], (float)node.translation[1] , (float)node.translation[2] };
	}

	//rotation
	mat4 rotation = GW::MATH::GIdentityMatrixF;
	if (node.rotation.size() == 4)
	{
		GW::MATH::GQUATERNIONF quat = { (float)node.rotation[0], (float)node.rotation[1], (float)node.rotation[2], (float)node.rotation[3] };
		GW::MATH::GMatrix::ConvertQuaternionF(quat, rotation);
		//GQuaternion::SetByMatrixF(rotation, rotation);
	}

	//scale
	vec4 scale = { 1, 1, 1, 1 };
	if (node.scale.size() == 3)
	{
		scale = { (float)node.scale[0], (float)node.scale[1] , (float)node.scale[2] };
	}

	mat4 matrix = GW::MATH::GIdentityMatrixF;

	mat4 translatedMatrix;
	mat4 scaledMatrix;
	mat4 rotatedMatrix;

	GMatrix::TranslateLocalF(GW::MATH::GIdentityMatrixF, translation, translatedMatrix);
	GMatrix::MultiplyMatrixF(translatedMatrix, rotation, rotatedMatrix);
	GMatrix::ScaleLocalF(GW::MATH::GIdentityMatrixF, scale, scaledMatrix);
	GMatrix::MultiplyMatrixF(rotatedMatrix, scaledMatrix, matrix);

	return matrix;
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
	LoadModel("Models/Shapes/Shapes.gltf");
	//CreateOffscreenFrameBuffer();
	CreateUniformBuffers();
	CreateDescriptorSets();
	WriteDescriptorSets();
	CreateGraphicsPipelines();
	CreateCommandBuffers();
	CreateFrameGraphNodes();
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


	_finalData.view = _offscreenData.view.row4;
	_finalData.viewProj = GW::MATH::GIdentityMatrixF * _offscreenData.view * _offscreenData.proj;
	GvkHelper::write_to_buffer(_device, _finalUniformBuffer.memory, &_finalData, sizeof(UniformBufferFinal));

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

	mat4 cam = GW::MATH::GIdentityMatrixF;

	GMatrix::InverseF(_offscreenData.view, cam);

	float y = 0.0f;

	float totalY = 0.0f;
	float totalZ = 0.0f;
	float totalX = 0.0f;

	const float cameraSpeed = 25.f;
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