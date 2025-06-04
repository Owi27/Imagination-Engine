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

void VulkanRenderer::OffscreenTest()
{
	//auto& model = _model;

	//VkPipelineLayout pipelineLayout;
	//offscreen.SetPipeline(_vk.CreateGraphicsPipeline(offscreen., pipelineLayout));

	/*std::unique_ptr<Texture>
		pos = std::make_unique<Texture>(_vkContext),
		nrm = std::make_unique<Texture>(_vkContext),
		alb = std::make_unique<Texture>(_vkContext),
		depth = std::make_unique<Texture>(_vkContext);*/

		//pos->CreateImage({ _width, _height, 1 }, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		//pos->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		//nrm->CreateImage({ _width, _height, 1 }, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		//nrm->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);
		//alb->CreateImage({ _width, _height, 1 }, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		//alb->CreateImageView(VK_IMAGE_ASPECT_COLOR_BIT);

		//VkFormat depthFormat;

		//std::vector<VkFormat> formats =
		//{
		//	VK_FORMAT_D32_SFLOAT_S8_UINT,
		//	VK_FORMAT_D32_SFLOAT,
		//	VK_FORMAT_D24_UNORM_S8_UINT,
		//	VK_FORMAT_D16_UNORM_S8_UINT,
		//	VK_FORMAT_D16_UNORM
		//};

		//GvkHelper::find_depth_format(_physicalDevice, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, formats.data(), &depthFormat);
		//depth->CreateImage({ _width, _height, 1 }, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		//depth->CreateImageView(VK_IMAGE_ASPECT_DEPTH_BIT);



		//std::unique_ptr<Buffer>
		//	pBuffer = std::make_unique<Buffer>(_vkContext),
		//	nBuffer = std::make_unique<Buffer>(_vkContext),
		//	uBuffer = std::make_unique<Buffer>(_vkContext),
		//	tBuffer = std::make_unique<Buffer>(_vkContext),
		//	iBuffer = std::make_unique<Buffer>(_vkContext);

		//pBuffer->CreateBuffer(sizeof(vec3) * _geometryData.positions.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		//pBuffer->WriteToBuffer(_geometryData.positions.data());
		//nBuffer->CreateBuffer(sizeof(vec3) * _geometryData.normals.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		//nBuffer->WriteToBuffer(_geometryData.normals.data());
		//uBuffer->CreateBuffer(sizeof(vec2) * _geometryData.texCoords.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		//uBuffer->WriteToBuffer(_geometryData.texCoords.data());
		//tBuffer->CreateBuffer(sizeof(vec4) * _geometryData.tangents.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		//tBuffer->WriteToBuffer(_geometryData.tangents.data());
		//iBuffer->CreateBuffer(sizeof(unsigned) * _geometryData.indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		//iBuffer->WriteToBuffer(_geometryData.indices.data());

		//PipelineDescription pipelineDescription
		//{
		//	.vertexInput = POSITION | NORMAL | TEXCOORD | TANGENT,
		//	.vertexShader = std::make_shared<Shader>(_vkContext, "OffscreenVertexShader", ShaderType::VERTEX_SHADER),
		//	.fragmentShader = std::make_shared<Shader>(_vkContext, "OffscreenFragmentShader", ShaderType::PIXEL_SHADER),
		//	.cullMode = FRONT,
		//	.pipelineLayoutCreateInfo
		//	{
		//		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		//		.setLayoutCount = 0,
		//		.pSetLayouts = nullptr,
		//		.pushConstantRangeCount = 0
		//	}
		//};

		//VkPipelineLayout pipelineLayout;

		//VkCommandBuffer cb;

		//_vk.CreateGraphicsPipeline(pipelineDescription, pipelineLayout);

		//std::vector<Texture> renderTex = { *pos, *nrm, *alb };

		//_vk.Render(renderTex, *depth, [&](VkCommandBuffer& commandBuffer)
		//	{
		//		VkViewport viewport = { 0, 0, static_cast<float>(_width), static_cast<float>(_height), 0, 1 };
		//		VkRect2D scissor = { {0, 0}, {_width, _height} };

		//		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		//		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		//		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, );

		//		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, fgNode.frameBuffer.pipelineLayout, 0, 1, &fgNode.frameBuffer.descriptorSet, 0, nullptr);

		//		std::array<VkBuffer, 4> vertexBuffers =
		//		{
		//			vBuffer.buffers[0].GetVkBuffer(),
		//			vBuffer.buffers[1].GetVkBuffer(),
		//			vBuffer.buffers[2].GetVkBuffer(),
		//			vBuffer.buffers[3].GetVkBuffer(),
		//		};

		//		std::vector<VkDeviceSize> offsets = { 0, 0, 0, 0 };
		//		vkCmdBindVertexBuffers(commandBuffer, 0, vBuffer.buffers.size(), vertexBuffers.data(), offsets.data());
		//		vkCmdBindIndexBuffer(commandBuffer, iBuffer.buffers[0].GetVkBuffer(), 0, VK_INDEX_TYPE_UINT32);

		//		int i = 0;
		//		for (auto& node : _model.nodes)
		//		{
		//			auto& mesh = _model.meshes[node.mesh];
		//			for (auto& prim : mesh.primitives)
		//			{
		//				vkCmdPushConstants(commandBuffer, fgNode.frameBuffer.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PCR), &_drawInfo[i].nodeWorld);
		//				vkCmdDrawIndexed(commandBuffer, _drawInfo[i].idxCount, 1, _drawInfo[i].firstIdx, _drawInfo[i].vertexOffset, 0);
		//				i++;
		//			}
		//		}


		//	})

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

void VulkanRenderer::LoadModel(std::string filename, ModelID id)
{
	tinygltf::TinyGLTF gltfLoader;
	std::string error;
	std::string warning;

	unsigned long long pos = filename.find_last_of('/');
	std::string path = filename.substr(0, pos);

	bool fileLoaded = gltfLoader.LoadASCIIFromFile(&_models[id], &error, &warning, filename);
	tinygltf::Model& model = _models[id];

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

	CreateGeometryData(id);

	////load textures
	//if (model.images.size() > 0)
	//{
	//	_textures.resize(model.images.size());
	//	int i = 0;
	//	for (auto& image : model.images)
	//	{
	//		_textures.push_back(Texture(image));
	//	}
	//}
}

void VulkanRenderer::CreateGeometryData(ModelID id)
{
	int vCount = 0, iCount = 0, firstIdx = 0, vertexOffset = 0;
	Renderable r;
	auto& geoData = _renderables[id].first;
	auto& model = _models[id];

	for (auto& node : model.nodes)
	{
		if (node.mesh >-1)
		{
			auto& mesh = model.meshes[node.mesh];
			for (auto prim : mesh.primitives)
			{
				r.firstIdx = geoData.indices.size();
				r.vertexOffset = geoData.positions.size();
				r.world = GetLocalMatrix(node);

				//position
				tinygltf::Accessor& accessor = model.accessors[prim.attributes.find("POSITION")->second];
				tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
				vCount = accessor.count;
				{
					auto pos = (const float*)&buffer.data[bufferView.byteOffset + accessor.byteOffset];

					if (pos)
					{
						for (size_t i = 0; i < accessor.count; i++)
						{
							geoData.positions.push_back(vec3{ pos[i * 3 + 0], pos[i * 3 + 1], pos[i * 3 + 2] });
						}

						//_offscreenData.min = { (float)accessor.minValues[0], (float)accessor.minValues[1] , (float)accessor.minValues[2] };
					}
				}

				//normals
				accessor = model.accessors[prim.attributes.find("NORMAL")->second];
				bufferView = model.bufferViews[accessor.bufferView];
				buffer = model.buffers[bufferView.buffer];
				{
					auto nrm = (const float*)&buffer.data[bufferView.byteOffset + accessor.byteOffset];

					if (nrm)
					{
						for (size_t i = 0; i < accessor.count; i++)
						{
							geoData.normals.push_back(vec3{ nrm[i * 3 + 0], nrm[i * 3 + 1], nrm[i * 3 + 2] });
						}
					}
				}

				//texCoord
				accessor = model.accessors[prim.attributes.find("TEXCOORD_0")->second];
				bufferView = model.bufferViews[accessor.bufferView];
				buffer = model.buffers[bufferView.buffer];
				{
					auto uv0 = (const float*)&buffer.data[bufferView.byteOffset + accessor.byteOffset];

					if (uv0)
					{
						for (size_t i = 0; i < accessor.count; i++)
						{
							geoData.texCoords.push_back(vec2{ uv0[i * 2 + 0], uv0[i * 2 + 1] });
						}
					}
				}

				//index buffer
				accessor = model.accessors[prim.indices];
				bufferView = model.bufferViews[accessor.bufferView];
				buffer = model.buffers[bufferView.buffer];
				iCount = accessor.count;
				r.idxCount = (unsigned int)accessor.count;
				{
					switch (accessor.componentType)
					{
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
					{
						std::vector<unsigned int> uIntPrims;
						uIntPrims.resize(accessor.count);
						memcpy(uIntPrims.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(unsigned int));
						geoData.indices.insert(geoData.indices.end(), uIntPrims.begin(), uIntPrims.end());
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
					{
						std::vector<unsigned short> uShortPrims;
						uShortPrims.resize(accessor.count);
						memcpy(uShortPrims.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(unsigned short));
						geoData.indices.insert(geoData.indices.end(), uShortPrims.begin(), uShortPrims.end());
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
					{
						std::vector<unsigned char> uCharPrims;
						uCharPrims.resize(accessor.count);
						memcpy(uCharPrims.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(unsigned char));
						geoData.indices.insert(geoData.indices.end(), uCharPrims.begin(), uCharPrims.end());
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
					accessor = model.accessors[prim.attributes.find("TANGENT")->second];
					bufferView = model.bufferViews[accessor.bufferView];
					buffer = model.buffers[bufferView.buffer];
					{
						auto tan = (const float*)&buffer.data[bufferView.byteOffset + accessor.byteOffset];

						if (tan)
						{
							for (size_t i = 0; i < accessor.count; i++)
							{
								geoData.tangents.push_back(vec4{ tan[i * 4 + 0], tan[i * 4 + 1], tan[i * 4 + 2], tan[i * 4 + 3] });
							}
						}
					}
				}
				else
				{
					if (node.name.find("Cone") != std::string::npos) geoData.tangents.push_back(vec4{ 0, 0, 0, 0 });
					else
					{
						std::vector<vec3> tangent(vCount);
						std::vector<vec3> biTangent(vCount);

						for (size_t i = 0; i < iCount; i += 3)
						{
							//local index
							unsigned int i0 = geoData.indices[firstIdx + i + 0];
							unsigned int i1 = geoData.indices[firstIdx + i + 1];
							unsigned int i2 = geoData.indices[firstIdx + i + 2];
							assert(i0 < vCount);
							assert(i1 < vCount);
							assert(i2 < vCount);

							//global index
							unsigned int gi0 = i0 + vertexOffset;
							unsigned int gi1 = i1 + vertexOffset;
							unsigned int gi2 = i2 + vertexOffset;

							const auto& p0 = geoData.positions[gi0];
							const auto& p1 = geoData.positions[gi1];
							const auto& p2 = geoData.positions[gi2];

							const auto& uv0 = geoData.texCoords[gi0];
							const auto& uv1 = geoData.texCoords[gi1];
							const auto& uv2 = geoData.texCoords[gi2];

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
							const auto& n = geoData.normals[vertexOffset + a];

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

							geoData.tangents.emplace_back(vec4{ oTangent.x, oTangent.y, oTangent.z, handedness });
						}
					}
				}

				//material
				{
					auto& glMaterial = model.materials[prim.material];
					Material material
					{
						.baseColorFactor = {(float)glMaterial.pbrMetallicRoughness.baseColorFactor[0], (float)glMaterial.pbrMetallicRoughness.baseColorFactor[1], (float)glMaterial.pbrMetallicRoughness.baseColorFactor[2], (float)glMaterial.pbrMetallicRoughness.baseColorFactor[3]},
						.baseColorTexture = glMaterial.pbrMetallicRoughness.baseColorTexture.index,
						.metallicFactor = (float)glMaterial.pbrMetallicRoughness.metallicFactor,
						.roughnessFactor = (float)glMaterial.pbrMetallicRoughness.roughnessFactor,
						.metallicRoughnessTexture = glMaterial.pbrMetallicRoughness.metallicRoughnessTexture.index,
						.emissiveTexture = glMaterial.emissiveTexture.index,
						.emissiveFactor = { (float)glMaterial.emissiveFactor[0], (float)glMaterial.emissiveFactor[1], (float)glMaterial.emissiveFactor[2]},
						.alphaMode = 0,
						.alphaCutoff = (float)glMaterial.alphaCutoff,
						.doubleSided = 0,
						.normalTexture = glMaterial.normalTexture.index,
						.normalTextureScale = (float)glMaterial.normalTexture.scale,
						.occlusionTexture = glMaterial.occlusionTexture.index,
						.occlusionTextureStrength = (float)glMaterial.occlusionTexture.strength
					};

					_materialInfo.push_back(material);
				}
				_renderables[id].second.push_back(std::move(r));

			}
		}
	}
}

void VulkanRenderer::CleanUp()
{
	vkDeviceWaitIdle(_device);


}

void VulkanRenderer::Prepare()
{

}

void VulkanRenderer::UploadTextureToGPU(tinygltf::Image& image, Texture* texture)
{
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
	vec4 scale = { 1, 1, 1, 0 };
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

VulkanRenderer::VulkanRenderer(GWindow win) : Renderer(win), _vk(*VulkanContext::GetInst(_win))
{
	LoadModel("Models/Sponza/glTF/Sponza.gltf", MODEL);
	//LoadModel("Models/VR/Untitled.gltf", MODEL);
	LoadModel("Models/Cube/Cube.gltf", CUBE);

	//skybox test
	{
		RenderPass& skybox = _graph.AddPass("skybox", FRAMEGRAPH_GRAPHICS_BIT);
		skybox.AddUB("skybox uniform", new SkyboxUniform
			{
				.proj = GW::MATH::GIdentityMatrixF,
				.model = GW::MATH::GIdentityMatrixF
			}, sizeof(SkyboxUniform));
		std::vector<std::string> skyboxPaths =
		{
			"skybox/front.png", "skybox/back.png",
			"skybox/top.png", "skybox/bottom.png",
			"skybox/right.png", "skybox/left.png"
		};
		skybox.AddCubeMap("skybox cubemap", skyboxPaths);
		skybox.AddDescriptorPoolSize({ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1 });
		skybox.AddDescriptorPoolSize({ .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1 });
		skybox.AddDescriptorSetLayoutBinding({ .binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT });
		skybox.AddDescriptorSetLayoutBinding({ .binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT });
		skybox.SetVertexInput(POSITION | TEXCOORD);
		skybox.SetShaders("SkyBox");
		skybox.AddVBOutput("cube position data", _renderables[CUBE].first.positions.data(), sizeof(vec3) * _renderables[CUBE].first.positions.size());
		skybox.AddVBOutput("cube texcoord data", _renderables[CUBE].first.texCoords.data(), sizeof(vec2) * _renderables[CUBE].first.texCoords.size());
		skybox.AddIBOutput("cube indices", _renderables[CUBE].first.indices.data(), sizeof(unsigned) * _renderables[CUBE].first.indices.size());
		skybox.AddTOutput("sky", VK_FORMAT_R16G16B16A16_SFLOAT);
		skybox.SetRenderables(_renderables[CUBE].second);
		skybox.SetDrawCalls([&skybox](VkCommandBuffer& commandBuffer)
			{
				for (auto renderable : skybox.GetRenderables())
				{
					vkCmdDrawIndexed(commandBuffer, renderable.idxCount, 1, renderable.firstIdx, renderable.vertexOffset, 0);
				}
			});
	}


	//offscreen
	{
		RenderPass& offscreen = _graph.AddPass("offscreen", FRAMEGRAPH_GRAPHICS_BIT);
		offscreen.AddVBOutput("position data", _renderables[MODEL].first.positions.data(), sizeof(vec3) * _renderables[MODEL].first.positions.size());
		offscreen.AddVBOutput("normal data", _renderables[MODEL].first.normals.data(), sizeof(vec3) * _renderables[MODEL].first.normals.size());
		offscreen.AddVBOutput("texcoord data", _renderables[MODEL].first.texCoords.data(), sizeof(vec2) * _renderables[MODEL].first.texCoords.size());
		offscreen.AddVBOutput("tangent data", _renderables[MODEL].first.tangents.data(), sizeof(vec4) * _renderables[MODEL].first.tangents.size());
		offscreen.AddIBOutput("indices", _renderables[MODEL].first.indices.data(), sizeof(unsigned) * _renderables[MODEL].first.indices.size());
		offscreen.AddUB("offscreen uniform", new UniformBufferOffscreen
		{
			.world = GW::MATH::GIdentityMatrixF,
			.deltaTime = 0.f
		}, sizeof(UniformBufferOffscreen));
		auto& oub = *static_cast<UniformBufferOffscreen*>(_graph._blackboard.Get<void*>("offscreen uniform"));
		GMatrix::LookAtLHF(vec4{ 0.f, 0.f, 0.f }, vec4{ 0.f, 0.f, 0.f }, vec4{ 0, 1, 0 }, oub.view);
		GMatrix::ProjectionVulkanLHF(G_DEGREE_TO_RADIAN(65), _vk.GetAspectRatio(), .1f, 256.f, oub.proj);
		auto& proj = static_cast<SkyboxUniform*>(_graph._blackboard.Get<void*>("skybox uniform"))->proj;
		proj = oub.proj;

		offscreen.AddTOutput("gbuffer position", VK_FORMAT_R16G16B16A16_SFLOAT);
		offscreen.AddTOutput("gbuffer normal", VK_FORMAT_R16G16B16A16_SFLOAT);
		offscreen.AddTOutput("gbuffer albedo", VK_FORMAT_R8G8B8A8_UNORM);
		offscreen.AddTOutput("gbuffer mProp", VK_FORMAT_R8G8B8A8_UNORM);
		//offscreen.AddTOutput("gbuffer emissive", VK_FORMAT_R8G8B8A8_UNORM);
		offscreen.AddDOutput("depth");

		offscreen.SetVertexInput(POSITION | NORMAL | TEXCOORD | TANGENT);
		offscreen.SetCullMode(VK_CULL_MODE_FRONT_BIT);
		offscreen.SetShaders("Offscreen");
		offscreen.SetPushConstantRange({ .stageFlags = VK_SHADER_STAGE_VERTEX_BIT, .offset = 0, .size = sizeof(PCR) });
		offscreen.AddDescriptorPoolSize({ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1 });
		offscreen.AddDescriptorPoolSize({ .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1 }); 
		offscreen.AddDescriptorSetLayoutBinding({ .binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_VERTEX_BIT });
		offscreen.AddDescriptorSetLayoutBinding({ .binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT });
		offscreen.SetRenderables(_renderables[MODEL].second);
		offscreen.AddSB("offscreen storage", _materialInfo.data(), _materialInfo.size() * sizeof(Material));
		offscreen.SetModelTextures(_models[MODEL].textures, _models[MODEL].images);
		offscreen.SetDrawCalls([this, &offscreen](VkCommandBuffer& commandBuffer)
			{
				unsigned i = 0;
				for (auto& renderable : offscreen.GetRenderables())
				{
					mat4 nMatrix;
					GMatrix::InverseF(renderable.world, nMatrix);
					GMatrix::TransposeF(nMatrix, nMatrix);
					PCR pcr
					{
						.model = renderable.world,
						.normal = nMatrix
					};
					vkCmdPushConstants(commandBuffer, offscreen.GetPipelineLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PCR), &pcr);
					vkCmdDrawIndexed(commandBuffer, renderable.idxCount, 1, renderable.firstIdx, renderable.vertexOffset, i++);
				}
				_vk.MB(commandBuffer);
			});
	}

	//lighting
	{
		RenderPass& lighting = _graph.AddPass("lighting", FRAMEGRAPH_GRAPHICS_BIT);
		lighting.SetCullMode(VK_CULL_MODE_FRONT_BIT);
		lighting.SetShaders("Lighting");
		lighting.AddTInput("gbuffer position");
		lighting.AddTInput("gbuffer normal");
		lighting.AddTInput("gbuffer albedo");
		lighting.AddTInput("gbuffer mProp");
		lighting.AddTInput("depth");
		lighting.AddTInput("sky");

		std::default_random_engine gen(777);
		std::uniform_real_distribution<float> distribution(0.f, 1.f);
		std::uniform_real_distribution<float> distribution2(-3.f, 3.f);

		/*for (size_t i = 0; i < 10; i++)
		{
			lub.lights[i].pos = { distribution2(gen) , distribution2(gen) , distribution2(gen) };
			lub.lights[i].col = { distribution(gen) , distribution(gen) , distribution(gen) };
			lub.lights[i].radius = 5.f;
		}*/
		lighting.AddUB("lighting uniform", new UniformBufferFinal
			{
				.view = static_cast<UniformBufferOffscreen*>(_graph._blackboard.Get<void*>("offscreen uniform"))->view.row4
			}, sizeof(UniformBufferFinal));

		auto& lub = *static_cast<UniformBufferFinal*>(_graph._blackboard.Get<void*>("lighting uniform"));
		for (size_t i = 0; i < 10; i++)
		{
			lub.lights[i].pos = { distribution2(gen) , distribution2(gen) , distribution2(gen) };
			lub.lights[i].col = { distribution(gen) , distribution(gen) , distribution(gen) };
			lub.lights[i].radius = 5.f;
		}

		lighting.SetModelTextures(_models[MODEL].textures, _models[MODEL].images);
		lighting.AddDescriptorPoolSize({ .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1 });
		lighting.AddDescriptorPoolSize({ .type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = 5 });
		lighting.AddDescriptorSetLayoutBinding({ .binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT });
		lighting.AddDescriptorSetLayoutBinding({ .binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT });
		lighting.AddDescriptorSetLayoutBinding({ .binding = 2, .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT });
		lighting.AddDescriptorSetLayoutBinding({ .binding = 3, .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT });
		lighting.AddDescriptorSetLayoutBinding({ .binding = 4, .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT });
		lighting.AddDescriptorSetLayoutBinding({ .binding = 5, .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT });
		lighting.AddDescriptorSetLayoutBinding({ .binding = 6, .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT });
		//lighting.AddDescriptorSetLayoutBinding({ .binding = 6, .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT });
		lighting.AddTOutput("lighting", VK_FORMAT_R16G16B16A16_SFLOAT);
		lighting.SetDrawCalls([](VkCommandBuffer& commandBuffer)
			{
				vkCmdDraw(commandBuffer, 3, 1, 0, 0);
			});
	}

	//swapchain
	{
		RenderPass& swapchain = _graph.AddPass("swapchain", FRAMEGRAPH_GRAPHICS_BIT);
		swapchain.AddTInput("lighting");
		swapchain.AddTOutput("swapchain");
		swapchain.SetShaders();
		swapchain.AddDescriptorPoolSize({ .type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = 1 });
		swapchain.AddDescriptorSetLayoutBinding({ .binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT });
		swapchain.SetDrawCalls([](VkCommandBuffer& commandBuffer)
			{
				vkCmdDraw(commandBuffer, 3, 1, 0, 0);
			});
	}

	_graph.BuildCommandBuffers();

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
	_vk.StartFrame();

	//_imGuiContext.Render();
	_vk.ReadImGuiInputs(_gInput);
	_graph.Execute();
	//_vk.RenderImGui();

	_vk.EndFrame();
}

void VulkanRenderer::UpdateCamera()
{
	_win.IsFocus(_isFocused);

	if (!_isFocused) return;

	mat4 cam = GW::MATH::GIdentityMatrixF;

	auto& view = static_cast<UniformBufferOffscreen*>(_graph._blackboard.Get<void*>("offscreen uniform"))->view;
	auto& deltaTime = static_cast<UniformBufferOffscreen*>(_graph._blackboard.Get<void*>("offscreen uniform"))->deltaTime;

	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<float> dt = _lastUpdate - now;
	deltaTime = -dt.count();
	_lastUpdate = now;

	GMatrix::InverseF(view, cam);

	float y = 0.0f;

	float totalY = 0.0f;
	float totalZ = 0.0f;
	float totalX = 0.0f;

	const float cameraSpeed = 5.f;
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

	cam.row4.y += totalY * cameraSpeed * deltaTime;

	perFrameSpeed = cameraSpeed * deltaTime;

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
		float totalYaw = G_DEGREE_TO_RADIAN(65) * _vk.GetAspectRatio() * mouseDeltaX / screenWidth + rightStickXaxis * thumbSpeed;
		mat4 yawMatrix = GW::MATH::GIdentityMatrixF;
		vec4 camSave = cam.row4;
		cam.row4 = { 0,0,0,1 };
		GMatrix::RotateYGlobalF(cam, totalYaw, cam);
		cam.row4 = camSave;
	}

	auto& camPos = static_cast<UniformBufferFinal*>(_graph._blackboard.Get<void*>("lighting uniform"))->view;
	camPos = cam.row4;
	GMatrix::InverseF(cam, view);
	auto& skyView = static_cast<SkyboxUniform*>(_graph._blackboard.Get<void*>("skybox uniform"))->model;
	skyView = view;
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