#include "pch.h"
#include "Renderer.h"
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tinygltf/tiny_gltf.h"
#include "MathOverloads.h"
#include "DEBUG.h"

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
				if (node.name.find("Cone") != std::string::npos) _geometryData.tangents.push_back(vec4{ 0, 0, 0, 0 });
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

void VulkanRenderer::CreateFrameGraphNodes()
{
	FrameGraphNode offscreenBuffers;
	{
		offscreenBuffers.name = "Offscreen Buffers";
		offscreenBuffers.outputResources = { "Vertex Buffers", "Index Buffer", "Offscreen UB" };
		offscreenBuffers.Setup = [&](FrameGraphNode& node)
			{
				FrameGraphBufferResource<Vertex> vertexBuffers;
				{
					vertexBuffers.parent = node.name;
					vertexBuffers.name = node.outputResources[0];
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
				vertexBuffers.prepared = true;
				_frameGraph->AddBufferResource(vertexBuffers.name, vertexBuffers);

				FrameGraphBufferResource<unsigned int> indexBuffer;
				{
					indexBuffer.parent = node.name;
					indexBuffer.name = node.outputResources[1];
					indexBuffer.buffers.resize(1);
					GvkHelper::create_buffer(_physicalDevice, _device, sizeof(unsigned int) * _geometryData.indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &indexBuffer.buffers[0].buffer, &indexBuffer.buffers[0].memory);
					GvkHelper::write_to_buffer(_device, indexBuffer.buffers[0].memory, _geometryData.indices.data(), sizeof(unsigned int) * _geometryData.indices.size());
				}
				indexBuffer.prepared = true;
				_frameGraph->AddBufferResource(indexBuffer.name, indexBuffer);

				FrameGraphBufferResource<UniformBufferOffscreen> offscreenUniformBuffer;
				{
					offscreenUniformBuffer.parent = node.name;
					offscreenUniformBuffer.name = "Offscreen UB";
					offscreenUniformBuffer.buffers.resize(1);
					_vlk.GetAspectRatio(_aspect);
					UniformBufferOffscreen data;
					{
						data.world = GW::MATH::GIdentityMatrixF;
						GMatrix::LookAtLHF(vec4{ 7.f, 4.f, -10.f }, vec4{ 0.f, 0.f, 0.f }, vec4{ 0, 1, 0 }, data.view);
						GMatrix::ProjectionVulkanLHF(G_DEGREE_TO_RADIAN(65), _aspect, .1f, 256.f, data.proj);
					}

					offscreenUniformBuffer.data.push_back(data);

					GvkHelper::create_buffer(_physicalDevice, _device, sizeof(UniformBufferOffscreen), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &offscreenUniformBuffer.buffers[0].buffer, &offscreenUniformBuffer.buffers[0].memory);
					GvkHelper::write_to_buffer(_device, offscreenUniformBuffer.buffers[0].memory, &data, sizeof(UniformBufferOffscreen));
				}
				offscreenUniformBuffer.prepared = true;
				_frameGraph->AddBufferResource(offscreenUniformBuffer.name, offscreenUniformBuffer);

				node.isSetupComplete = true;
			};
		offscreenBuffers.Execute = [&](VkCommandBuffer& commandBuffer, FrameGraphNode& node)
			{

			};
		offscreenBuffers.shouldExecute = true;
	}
	_frameGraph->AddNode(offscreenBuffers);

	FrameGraphNode offscreenPass;
	{
		offscreenPass.name = "Offscreen Pass";
		offscreenPass.inputResources = { "Vertex Buffers", "Index Buffer", "Offscreen UB" };
		offscreenPass.outputResources = { "GBuffer: Position", "GBuffer: Normal", "GBuffer: Albedo", "Depth Buffer" };
		offscreenPass.Setup = [&](FrameGraphNode& node)
			{
				//assert that input resources are prepared
				//Debug::CheckInputResources(*_frameGraph, offscreenPass);

				FrameGraphBufferResource<UniformBufferOffscreen>& offscreenUB = _frameGraph->GetBufferResource<UniformBufferOffscreen>(node.inputResources[2]);

				//FRAMEBUFFER
				{
					FrameGraphImageResource gBufferPos, gBufferNrm, gBufferAlb, depth;
					{
						//Gbuffer Position
						gBufferPos.name = node.outputResources[0];
						gBufferPos.parent = node.name;
						gBufferPos.extent = { _width, _height, 1 };
						gBufferPos.format = VK_FORMAT_R16G16B16A16_SFLOAT;
						GvkHelper::create_image(_physicalDevice, _device, gBufferPos.extent, 1, VK_SAMPLE_COUNT_1_BIT, gBufferPos.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr, &gBufferPos.image.image, &gBufferPos.image.memory);
						GvkHelper::create_image_view(_device, gBufferPos.image.image, gBufferPos.format, VK_IMAGE_ASPECT_COLOR_BIT, 1, nullptr, &gBufferPos.image.imageView);

						//Gbuffer Normal
						gBufferNrm.name = node.outputResources[1];
						gBufferNrm.parent = node.name;
						gBufferNrm.extent = { _width, _height, 1 };
						gBufferNrm.format = VK_FORMAT_R16G16B16A16_SFLOAT;
						GvkHelper::create_image(_physicalDevice, _device, gBufferNrm.extent, 1, VK_SAMPLE_COUNT_1_BIT, gBufferNrm.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr, &gBufferNrm.image.image, &gBufferNrm.image.memory);
						GvkHelper::create_image_view(_device, gBufferNrm.image.image, gBufferNrm.format, VK_IMAGE_ASPECT_COLOR_BIT, 1, nullptr, &gBufferNrm.image.imageView);

						//Gbuffer Albedo
						gBufferAlb.name = node.outputResources[2];
						gBufferAlb.parent = node.name;
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

						depth.name = node.outputResources[3];
						depth.parent = node.name;
						depth.extent = { _width, _height, 1 };
						//set format
						GvkHelper::find_depth_format(_physicalDevice, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, formats.data(), &depth.format);
						GvkHelper::create_image(_physicalDevice, _device, depth.extent, 1, VK_SAMPLE_COUNT_1_BIT, depth.format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, nullptr, &depth.image.image, &depth.image.memory);
						GvkHelper::create_image_view(_device, depth.image.image, depth.format, VK_IMAGE_ASPECT_DEPTH_BIT, 1, nullptr, &depth.image.imageView);
					}

					gBufferPos.prepared = true;
					gBufferNrm.prepared = true;
					gBufferAlb.prepared = true;
					depth.prepared = true;
					_frameGraph->AddImageResource(gBufferPos.name, gBufferPos);
					_frameGraph->AddImageResource(gBufferNrm.name, gBufferNrm);
					_frameGraph->AddImageResource(gBufferAlb.name, gBufferAlb);
					_frameGraph->AddImageResource(depth.name, depth);

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

					FrameGraphImageResource* posResource = &_frameGraph->GetImageResource(node.outputResources[0]),
						* nrmResource = &_frameGraph->GetImageResource(node.outputResources[1]),
						* albResource = &_frameGraph->GetImageResource(node.outputResources[2]),
						* depthResource = &_frameGraph->GetImageResource(node.outputResources[3]);

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

					vkCreateRenderPass(_device, &renderPassCreateInfo, nullptr, &node.frameBuffer.renderPass);

					std::array<VkImageView, 4> imageViews =
					{
						posResource->image.imageView,
						nrmResource->image.imageView,
						albResource->image.imageView,
						depthResource->image.imageView
					};

					VkFramebufferCreateInfo frameBufferCreateInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
					frameBufferCreateInfo.renderPass = node.frameBuffer.renderPass;
					frameBufferCreateInfo.pAttachments = imageViews.data();
					frameBufferCreateInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
					frameBufferCreateInfo.width = _width;
					frameBufferCreateInfo.height = _height;
					frameBufferCreateInfo.layers = 1;

					vkCreateFramebuffer(_device, &frameBufferCreateInfo, nullptr, &node.frameBuffer.frameBuffer);

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

				//DESCRIPTOR SET
				{
					std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
					{
						{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}
					};

					VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
					descriptorPoolCreateInfo.maxSets = 1;
					descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
					descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

					vkCreateDescriptorPool(_device, &descriptorPoolCreateInfo, nullptr, &node.frameBuffer.descriptorPool);

					std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings =
					{
						{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr}
					};

					VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
					descriptorSetLayoutCreateInfo.bindingCount = descriptorSetLayoutBindings.size();
					descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

					vkCreateDescriptorSetLayout(_device, &descriptorSetLayoutCreateInfo, nullptr, &node.frameBuffer.descriptorSetLayout);

					VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
					descriptorSetAllocateInfo.descriptorPool = node.frameBuffer.descriptorPool;
					descriptorSetAllocateInfo.descriptorSetCount = 1;
					descriptorSetAllocateInfo.pSetLayouts = &node.frameBuffer.descriptorSetLayout;

					vkAllocateDescriptorSets(_device, &descriptorSetAllocateInfo, &node.frameBuffer.descriptorSet);

					VkDescriptorBufferInfo descriptorBufferInfo;
					descriptorBufferInfo.buffer = offscreenUB.buffers[0].buffer;
					descriptorBufferInfo.offset = 0;
					descriptorBufferInfo.range = sizeof(UniformBufferOffscreen);

					std::vector<VkWriteDescriptorSet> offscreenWriteDescriptorSets =
					{
						MakeWrite(node.frameBuffer.descriptorSet, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &descriptorBufferInfo)
					};

					vkUpdateDescriptorSets(_device, offscreenWriteDescriptorSets.size(), offscreenWriteDescriptorSets.data(), 0, nullptr);
				}

				//GRAPHICS PIPELINE
				{
					VkPushConstantRange pushConstantRange;
					{
						pushConstantRange.offset = 0;
						pushConstantRange.size = sizeof(PCR);
						pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
					}

					_vlk.GetRenderPass((void**)&_renderPass);
					VkPipelineShaderStageCreateInfo pssci;

					node.frameBuffer.shaderModules.resize(2);

					GvkHelper::create_shader(_device, "Shaders/SPV/OffscreenFragmentShader.spv", "main", VK_SHADER_STAGE_FRAGMENT_BIT, &node.frameBuffer.shaderModules[0], &pssci);
					GvkHelper::create_shader(_device, "Shaders/SPV/OffscreenVertexShader.spv", "main", VK_SHADER_STAGE_VERTEX_BIT, &node.frameBuffer.shaderModules[1], &pssci);

					VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[2] = { {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO}, {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO} };
					//fragment shader
					pipelineShaderStageCreateInfos[0].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
					pipelineShaderStageCreateInfos[0].module = node.frameBuffer.shaderModules[0];
					pipelineShaderStageCreateInfos[0].pName = "main";
					//vertex shader
					pipelineShaderStageCreateInfos[1].stage = VK_SHADER_STAGE_VERTEX_BIT;
					pipelineShaderStageCreateInfos[1].module = node.frameBuffer.shaderModules[1];
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
						{ 1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0 },
						{ 2, 2, VK_FORMAT_R32G32_SFLOAT, 0 },
						{ 3, 3, VK_FORMAT_R32G32B32A32_SFLOAT, 0 },
					};

					//vertex input info
					VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
					pipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = 4;
					pipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vertexInputBindingDescription;
					pipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = 4;
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
					pipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_FRONT_BIT; //offscreen
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

					std::vector<VkPipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStates = { pipelineColorBlendAttachmentState, pipelineColorBlendAttachmentState, pipelineColorBlendAttachmentState };

					//color blend state
					VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
					pipelineColorBlendStateCreateInfo.logicOpEnable = false;
					pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
					pipelineColorBlendStateCreateInfo.attachmentCount = pipelineColorBlendAttachmentStates.size();
					pipelineColorBlendStateCreateInfo.pAttachments = pipelineColorBlendAttachmentStates.data();
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
					pipelineLayoutCreateInfo.pSetLayouts = &node.frameBuffer.descriptorSetLayout;
					pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
					pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

					vkCreatePipelineLayout(_device, &pipelineLayoutCreateInfo, nullptr, &node.frameBuffer.pipelineLayout);

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
					graphicsPipelineCreateInfo.layout = node.frameBuffer.pipelineLayout;
					graphicsPipelineCreateInfo.renderPass = node.frameBuffer.renderPass;
					graphicsPipelineCreateInfo.subpass = 0;
					graphicsPipelineCreateInfo.basePipelineHandle = nullptr;

					vkCreateGraphicsPipelines(_device, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &node.frameBuffer.pipeline);
				}

				node.frameBuffer.bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				node.isSetupComplete = true;
			};
		offscreenPass.Execute = [&](VkCommandBuffer& commandBuffer, FrameGraphNode& fgNode)
			{
				FrameGraphBufferResource<Vertex>& vBuffer = _frameGraph->GetBufferResource<Vertex>(fgNode.inputResources[0]);
				FrameGraphBufferResource<unsigned int>& iBuffer = _frameGraph->GetBufferResource<unsigned int>(fgNode.inputResources[1]);
				FrameGraphBufferResource<UniformBufferOffscreen>& offscreenUB = _frameGraph->GetBufferResource<UniformBufferOffscreen>(fgNode.inputResources[2]);
				auto& data = offscreenUB.data[0];
				auto now = std::chrono::steady_clock::now();
				_deltaTime = now - _lastUpdate;
				_lastUpdate = now;

				data.deltaTime = _deltaTime.count();
				GvkHelper::write_to_buffer(_device, offscreenUB.buffers[0].memory, &data, sizeof(UniformBufferOffscreen));

				VkCommandBufferBeginInfo commandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };

				// Clear values for all attachments written in the fragment shader
				std::array<VkClearValue, 4> clearValues;
				clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
				clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
				clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
				clearValues[3].depthStencil = { 1.0f, 0 };

				VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
				renderPassBeginInfo.renderPass = fgNode.frameBuffer.renderPass;
				renderPassBeginInfo.framebuffer = fgNode.frameBuffer.frameBuffer;
				renderPassBeginInfo.renderArea.extent.width = _width;
				renderPassBeginInfo.renderArea.extent.height = _height;
				renderPassBeginInfo.clearValueCount = clearValues.size();
				renderPassBeginInfo.pClearValues = clearValues.data();

				GvkHelper::signal_command_start(_device, _commandPool, &commandBuffer);
				//vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
				vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

				VkViewport viewport = { 0, 0, static_cast<float>(_width), static_cast<float>(_height), 0, 1 };
				VkRect2D scissor = { {0, 0}, {_width, _height} };

				vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
				vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

				vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, fgNode.frameBuffer.pipeline);

				vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, fgNode.frameBuffer.pipelineLayout, 0, 1, &fgNode.frameBuffer.descriptorSet, 0, nullptr);

				std::array<VkBuffer, 4> vertexBuffers =
				{
					vBuffer.buffers[0].buffer,
					vBuffer.buffers[1].buffer,
					vBuffer.buffers[2].buffer,
					vBuffer.buffers[3].buffer,
				};

				std::vector<VkDeviceSize> offsets = { 0, 0, 0, 0 };
				vkCmdBindVertexBuffers(commandBuffer, 0, vBuffer.buffers.size(), vertexBuffers.data(), offsets.data());
				vkCmdBindIndexBuffer(commandBuffer, iBuffer.buffers[0].buffer, 0, VK_INDEX_TYPE_UINT32);

				int i = 0;
				for (auto& node : _model.nodes)
				{
					auto& mesh = _model.meshes[node.mesh];
					for (auto& prim : mesh.primitives)
					{
						vkCmdPushConstants(commandBuffer, fgNode.frameBuffer.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PCR), &_drawInfo[i].nodeWorld);
						vkCmdDrawIndexed(commandBuffer, _drawInfo[i].idxCount, 1, _drawInfo[i].firstIdx, _drawInfo[i].vertexOffset, 0);
						i++;
					}
				}

				vkCmdEndRenderPass(commandBuffer);
				vkEndCommandBuffer(commandBuffer);
				_commandBuffers[0].clear();
				_commandBuffers[0].push_back(commandBuffer);

				//Prepare(fgNode);
			};
		offscreenPass.shouldExecute = true;
	};
	_frameGraph->AddNode(offscreenPass);

	FrameGraphNode compositionBuffers;
	{
		compositionBuffers.name = "Composition Buffers";
		compositionBuffers.inputResources = { "Offscreen UB" };
		compositionBuffers.outputResources = { "Composition UB" };
		compositionBuffers.Setup = [&](FrameGraphNode& node)
			{
				//assert that input resources are prepared
				//Debug::CheckInputResources(*_frameGraph, compositionBuffers);

				FrameGraphBufferResource<UniformBufferFinal> compositionUB;
				{
					compositionUB.parent = node.name;
					compositionUB.name = node.outputResources[0];
					compositionUB.buffers.resize(1);

					UniformBufferFinal data;
					{
						FrameGraphBufferResource<UniformBufferOffscreen>& offscreenUB = _frameGraph->GetBufferResource<UniformBufferOffscreen>(node.inputResources[0]);
						auto& offscreenData = offscreenUB.data[0];

						data.view = offscreenData.view.row4;

						std::default_random_engine gen(777);
						std::uniform_real_distribution<float> distribution(0.f, 1.f);
						std::uniform_real_distribution<float> distribution2(-3.f, 3.f);

						for (size_t i = 0; i < 10; i++)
						{
							data.lights[i].pos = { distribution2(gen) , distribution2(gen) , distribution2(gen) };
							data.lights[i].col = { distribution(gen) , distribution(gen) , distribution(gen) };
							data.lights[i].radius = 5.f;
						}
					}
					compositionUB.data.push_back(data);

					GvkHelper::create_buffer(_physicalDevice, _device, sizeof(UniformBufferFinal), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &compositionUB.buffers[0].buffer, &compositionUB.buffers[0].memory);
					GvkHelper::write_to_buffer(_device, compositionUB.buffers[0].memory, &data, sizeof(UniformBufferFinal));
				}
				_frameGraph->AddBufferResource(compositionUB.name, compositionUB);

				node.isSetupComplete = true;
			};
		compositionBuffers.Execute = [&](VkCommandBuffer& commandBuffer, FrameGraphNode& fgNode)
			{
				//Prepare(compositionBuffers);
			};
		compositionBuffers.shouldExecute = true;
	}
	_frameGraph->AddNode(compositionBuffers);

	FrameGraphNode compositionPass; //todo
	{
		compositionPass.name = "Composition Pass";
		compositionPass.inputResources = { "Composition UB", "GBuffer: Position", "GBuffer: Normal", "GBuffer: Albedo" };
		compositionPass.outputResources = { "Composition Image" };
		compositionPass.Setup = [&](FrameGraphNode& node)
			{
				//assert that input resources are prepared
				//Debug::CheckInputResources(*_frameGraph, compositionPass);

				_vlk.GetRenderPass((void**)&node.frameBuffer.renderPass);
				_vlk.GetSwapchainFramebuffer(_currentFrame, (void**)&node.frameBuffer.frameBuffer);
				_vlk.GetSwapchainImageCount(_swapchainImageCount);

				FrameGraphBufferResource<UniformBufferFinal>& compositionUB = _frameGraph->GetBufferResource<UniformBufferFinal>(node.inputResources[0]);
				FrameGraphImageResource& posResource = _frameGraph->GetImageResource(node.inputResources[1]),
					& nrmResource = _frameGraph->GetImageResource(node.inputResources[2]),
					& albResource = _frameGraph->GetImageResource(node.inputResources[3]);

				//DESCRIPTOR SET
				{
					std::vector<VkDescriptorPoolSize> descriptorPoolSizes =
					{
						{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
						{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3}
					};

					VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
					descriptorPoolCreateInfo.maxSets = 1;
					descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
					descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

					vkCreateDescriptorPool(_device, &descriptorPoolCreateInfo, nullptr, &node.frameBuffer.descriptorPool);

					std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings =
					{
						{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
						{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
						{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
						{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
					};

					VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
					descriptorSetLayoutCreateInfo.bindingCount = descriptorSetLayoutBindings.size();
					descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

					vkCreateDescriptorSetLayout(_device, &descriptorSetLayoutCreateInfo, nullptr, &node.frameBuffer.descriptorSetLayout);

					VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
					descriptorSetAllocateInfo.descriptorPool = node.frameBuffer.descriptorPool;
					descriptorSetAllocateInfo.descriptorSetCount = 1;
					descriptorSetAllocateInfo.pSetLayouts = &node.frameBuffer.descriptorSetLayout;

					vkAllocateDescriptorSets(_device, &descriptorSetAllocateInfo, &node.frameBuffer.descriptorSet);

					VkDescriptorBufferInfo descriptorBufferInfo;
					descriptorBufferInfo.buffer = compositionUB.buffers[0].buffer;
					descriptorBufferInfo.offset = 0;
					descriptorBufferInfo.range = sizeof(UniformBufferFinal);

					std::vector<VkDescriptorImageInfo> descriptorImageInfos =
					{
						{_colorSampler, posResource.image.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
						{_colorSampler, nrmResource.image.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
						{_colorSampler, albResource.image.imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
					};

					std::vector<VkWriteDescriptorSet> finalWriteDescriptorSets =
					{
						MakeWrite(node.frameBuffer.descriptorSet, 0, 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, &descriptorBufferInfo),
						MakeWrite(node.frameBuffer.descriptorSet, 1, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &descriptorImageInfos[0], nullptr),
						MakeWrite(node.frameBuffer.descriptorSet, 2, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &descriptorImageInfos[1], nullptr),
						MakeWrite(node.frameBuffer.descriptorSet, 3, 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, &descriptorImageInfos[2], nullptr),
					};

					vkUpdateDescriptorSets(_device, finalWriteDescriptorSets.size(), finalWriteDescriptorSets.data(), 0, nullptr);
				}

				//GRAPHICS PIPELINE
				{
					VkPipelineShaderStageCreateInfo pssci;

					node.frameBuffer.shaderModules.resize(2);

					GvkHelper::create_shader(_device, "Shaders/SPV/FragmentShader.spv", "main", VK_SHADER_STAGE_FRAGMENT_BIT, &node.frameBuffer.shaderModules[0], &pssci);
					GvkHelper::create_shader(_device, "Shaders/SPV/VertexShader.spv", "main", VK_SHADER_STAGE_VERTEX_BIT, &node.frameBuffer.shaderModules[1], &pssci);

					VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfos[2] = { {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO}, {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO} };
					//fragment shader
					pipelineShaderStageCreateInfos[0].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
					pipelineShaderStageCreateInfos[0].module = node.frameBuffer.shaderModules[0];
					pipelineShaderStageCreateInfos[0].pName = "main";
					//vertex shader
					pipelineShaderStageCreateInfos[1].stage = VK_SHADER_STAGE_VERTEX_BIT;
					pipelineShaderStageCreateInfos[1].module = node.frameBuffer.shaderModules[1];
					pipelineShaderStageCreateInfos[1].pName = "main";


					//assembly state
					VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
					pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
					pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = false;

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
					pipelineLayoutCreateInfo.pSetLayouts = &node.frameBuffer.descriptorSetLayout;
					pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
					//pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;

					vkCreatePipelineLayout(_device, &pipelineLayoutCreateInfo, nullptr, &node.frameBuffer.pipelineLayout);

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
					graphicsPipelineCreateInfo.layout = node.frameBuffer.pipelineLayout;
					graphicsPipelineCreateInfo.renderPass = node.frameBuffer.renderPass;
					graphicsPipelineCreateInfo.subpass = 0;
					graphicsPipelineCreateInfo.basePipelineHandle = nullptr;

					vkCreateGraphicsPipelines(_device, nullptr, 1, &graphicsPipelineCreateInfo, nullptr, &node.frameBuffer.pipeline);
				}
				node.frameBuffer.bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

				node.isSetupComplete = true;
			};
		compositionPass.Execute = [&](VkCommandBuffer& commandBuffer, FrameGraphNode& fgNode)
			{
				VkClearValue clearValues[2];
				clearValues[0].color = { { 0.2f, 0.0f, 0.0f, 0.0f } };
				clearValues[1].depthStencil = { 1.0f, 0 };

				VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
				renderPassBeginInfo.renderPass = fgNode.frameBuffer.renderPass;
				renderPassBeginInfo.framebuffer = fgNode.frameBuffer.frameBuffer;
				renderPassBeginInfo.renderArea.offset.x = 0;
				renderPassBeginInfo.renderArea.offset.y = 0;
				renderPassBeginInfo.renderArea.extent.width = _width;
				renderPassBeginInfo.renderArea.extent.height = _height;
				renderPassBeginInfo.clearValueCount = 2;
				renderPassBeginInfo.pClearValues = clearValues;

				VkCommandBufferBeginInfo commandBufferBeginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
				_commandBuffers[1].clear();
				for (size_t i = 0; i < _swapchainImageCount; i++)
				{
					_vlk.GetSwapchainFramebuffer(i, (void**)&renderPassBeginInfo.framebuffer);

					//_vlk.GetCommandBuffer(i, (void**)&commandBuffer);
					//vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
					GvkHelper::signal_command_start(_device, _commandPool, &commandBuffer);
					vkCmdBeginRenderPass(commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

					VkViewport viewport = { 0, 0, static_cast<float>(_width), static_cast<float>(_height), 0, 1 };
					VkRect2D scissor = { {0, 0}, {_width, _height} };

					vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
					vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
					vkCmdBindDescriptorSets(commandBuffer, fgNode.frameBuffer.bindPoint, fgNode.frameBuffer.pipelineLayout, 0, 1, &fgNode.frameBuffer.descriptorSet, 0, nullptr);
					vkCmdBindPipeline(commandBuffer, fgNode.frameBuffer.bindPoint, fgNode.frameBuffer.pipeline);
					vkCmdDraw(commandBuffer, 3, 1, 0, 0);

					vkCmdEndRenderPass(commandBuffer);
					vkEndCommandBuffer(commandBuffer);
					_commandBuffers[1].push_back(commandBuffer);
					//GvkHelper::signal_command_end(_device, _queue, _commandPool, &commandBuffer);
				}

				//Prepare(fgNode);
			};
		compositionPass.shouldExecute = true;
	}
	_frameGraph->AddNode(compositionPass);
}

void VulkanRenderer::CleanUp()
{
	vkDeviceWaitIdle(_device);


}

void VulkanRenderer::Prepare(FrameGraphNode node)
{
	for (auto out : node.outputResources)
	{
		//_frameGraph->GetResource(out)->prepared = true;
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
	LoadModel("Models/Sponza/glTF/Sponza.gltf");
	CreateFrameGraphNodes();

	VkSemaphoreCreateInfo semaphoreCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	for (size_t i = 0; i < MAX_FRAMES; i++)
		vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_presentCompleteSemaphore[i]);
	vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_offscreenSemaphore);
	vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_compositionSemaphore);
	vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_postProcessSemaphore);

	_submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	_submitInfo.pWaitDstStageMask = &_submitPipelineStages;
	_submitInfo.waitSemaphoreCount = 1;
	_submitInfo.pWaitSemaphores = &_presentCompleteSemaphore[0];
	_submitInfo.signalSemaphoreCount = 1;
	_submitInfo.pSignalSemaphores = &_compositionSemaphore;

	_fences.resize(MAX_FRAMES);
	VkFenceCreateInfo fenceCreateInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Optional: Start in signaled state
	for (size_t i = 0; i < MAX_FRAMES; i++)
		vkCreateFence(_device, &fenceCreateInfo, nullptr, &_fences[i]);


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
	vkWaitForFences(_device, 1, &_fences[_currentFrame], true, UINT64_MAX);
	vkResetFences(_device, 1, &_fences[_currentFrame]);

	VkCommandBuffer commandBuffer;
	_frameGraph->Execute(commandBuffer);

	unsigned int frameIdx = 0;
	vkAcquireNextImageKHR(_device, _swapchain, 0, _presentCompleteSemaphore[_currentFrame], nullptr, &frameIdx);

	_submitInfo.pWaitSemaphores = &_presentCompleteSemaphore[_currentFrame];
	_submitInfo.pSignalSemaphores = &_offscreenSemaphore;
	_submitInfo.commandBufferCount = _commandBuffers[0].size();
	_submitInfo.pCommandBuffers = _commandBuffers[0].data();

	vkQueueSubmit(_queue, 1, &_submitInfo, nullptr);

	_submitInfo.pWaitSemaphores = &_offscreenSemaphore;
	_submitInfo.pSignalSemaphores = &_compositionSemaphore;
	_submitInfo.commandBufferCount = 1;
	_submitInfo.pCommandBuffers = &_commandBuffers[1][_currentFrame];

	vkQueueSubmit(_queue, 1, &_submitInfo, _fences[_currentFrame]);

	//_submitInfo.pWaitSemaphores = &_compositionSemaphore;
	//_submitInfo.pSignalSemaphores = &_postProcessSemaphore;
	//_submitInfo.pCommandBuffers = &_commandBuffers[2].data();
	//
	//vkQueueSubmit(_queue, 1, &_submitInfo, VK_NULL_HANDLE);

	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &_swapchain;
	presentInfo.pImageIndices = &frameIdx;
	presentInfo.pWaitSemaphores = &_compositionSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	vkQueuePresentKHR(_queue, &presentInfo);
	_currentFrame = (_currentFrame + 1) % MAX_FRAMES;
}

void VulkanRenderer::UpdateCamera()
{
	_win.IsFocus(_isFocused);

	if (!_isFocused) return;

	mat4 cam = GW::MATH::GIdentityMatrixF;

	FrameGraphBufferResource<UniformBufferOffscreen>& offscreenUB = _frameGraph->GetBufferResource<UniformBufferOffscreen>("Offscreen UB");
	auto& offscreenData = offscreenUB.data[0];

	GMatrix::InverseF(offscreenData.view, cam);

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

	GMatrix::InverseF(cam, offscreenData.view);
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