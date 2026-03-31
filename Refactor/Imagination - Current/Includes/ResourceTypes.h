#pragma once

#include <wrl/client.h>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
using namespace Microsoft::WRL;

#include "ResourceManager.h"

#include "gltf/stb_image.h"
#include "gltf/tiny_gltf.h"

struct Vertex
{
	Math::vec3<float> pos;
	Math::vec3<float> nrm;
	Math::vec2<float> uv0;
	Math::vec4<float> tan;
	Math::vec3<float> col;

	static vk::VertexInputBindingDescription GetBindingDescription() { return { 0, sizeof(Vertex), vk::VertexInputRate::eVertex }; }
	static std::array<vk::VertexInputAttributeDescription, 3> GetAttributeDescriptions()
	{
		return
		{
			vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos)),
			vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, col)),
			vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv0)),
		};
	}

	bool operator==(const Vertex& other) const
	{
		return pos == other.pos && nrm == other.nrm && uv0 == other.uv0 && tan == other.tan && col == other.col;
	}
};

namespace std
{
	template<> struct hash<Vertex>
	{
		uint64_t operator()(const Vertex& vertex) const noexcept
		{
			uint64_t seed = 0;
			std::hash<float> hasher;

			seed ^= hasher(vertex.pos.x) + hasher(vertex.pos.y) + hasher(vertex.pos.z);
			seed ^= hasher(vertex.nrm.x) + hasher(vertex.nrm.y) + hasher(vertex.nrm.z);
			seed ^= hasher(vertex.uv0.x) + hasher(vertex.uv0.y);
			seed ^= hasher(vertex.tan.x) + hasher(vertex.tan.y) + hasher(vertex.tan.z);

			return seed;
		}
	};
}

class Texture : public Resource
{
	vk::Image _image;
	vk::DeviceMemory _memory;
	vk::DeviceSize _offset;
	vk::ImageView _imageView;
	vk::Sampler _sampler;

	int width = 0, height = 0, channels = 0;

	void CreateImage(uint8_t* pData);
	uint32_t FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps);
	void TransitionImageLayout(vk::ImageLayout pOldLayout, vk::ImageLayout pNewLayout);

protected:
	virtual bool DoLoad() override { return true; }
	virtual bool DoUnload() override { return true; }

public:
	explicit Texture(const std::string& pID) : Resource(pID)
	{
	}

	~Texture() override
	{
		Unload();
	}

	bool Load() override
	{
		std::string filePath = "Textures/" + GetID() + ".png";

		stbi_set_flip_vertically_on_load(true);
		uint8_t* data = stbi_load(filePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

		if (!data) return false;

		CreateImage(data);

		return Resource::Load();
	}

	void Unload() override
	{
		if (IsLoaded())
		{
			auto& device = *Device::Inst().GetDevice();
			device.destroySampler(_sampler);
			device.destroyImageView(_imageView);
			device.destroyImage(_image);
			device.freeMemory(_memory);

			Resource::Unload();
		}
	}

	vk::Image GetImage() const { return _image; }
	vk::ImageView GetImageView() const { return _imageView; }
	vk::Sampler GetSampler() const { return _sampler; }
};

class Mesh : public Resource
{
	tinygltf::Model model;

	vk::Buffer _vertexBuffer;
	vk::DeviceMemory _vertexBufferMemory;
	vk::DeviceSize _vertexBufferOffset;

	vk::Buffer _indexBuffer;
	vk::DeviceMemory _indexBufferMemory;
	vk::DeviceSize _indexBufferOffset;

	uint32_t _vertexCount = 0, _indexCount = 0;

	void LoadModel(const std::string& pFile, std::vector<Vertex>& pVertices, std::vector<uint32_t>& pIndices);
	void CreateBuffers(std::vector<Vertex>& pVertices, std::vector<uint32_t>& pIndices);
	uint32_t FindMemoryType(uint32_t pTypeFilter, vk::MemoryPropertyFlags pProps);
	void CopyBuffer(vk::Buffer pSrc, vk::Buffer pDst, vk::DeviceSize pSize);

protected:
	virtual bool DoLoad() override { return true; }
	virtual bool DoUnload() override { return true; }

public:
	explicit Mesh(const std::string& pID) : Resource(pID)
	{
	}

	~Mesh() override
	{
		Unload();
	}

	bool Load() override
	{
		std::string filepath = "../Models/" + GetID() + "glTF" + GetID() + ".gltf";

		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		LoadModel(filepath, vertices, indices);

		CreateBuffers(vertices, indices);

		_vertexCount = static_cast<uint32_t>(vertices.size());
		_indexCount = static_cast<uint32_t>(indices.size());

		return Resource::Load();
	}

	void Unload() override
	{
		if (IsLoaded())
		{
			auto& device = *Device::Inst().GetDevice();

			device.destroyBuffer(_indexBuffer);
			device.freeMemory(_indexBufferMemory);

			device.destroyBuffer(_vertexBuffer);
			device.freeMemory(_vertexBufferMemory);

			Resource::Unload();
		}
	}

	vk::Buffer GetVertexBuffer() const { return _vertexBuffer; }
	vk::Buffer GetIndexBuffer() const { return _indexBuffer; }
	uint32_t GetVertexCount() const { return _vertexCount; }
	uint32_t GetIndexCount() const { return _indexCount; }
};

class Shader : public Resource
{
	ComPtr<IDxcCompiler3> _compiler;
	ComPtr<IDxcUtils> _utils;
	ComPtr<IDxcIncludeHandler> _includeHandler;

	vk::ShaderModule _shaderModule;
	vk::ShaderStageFlagBits _stage;
	std::string _shader;

	std::unordered_map<vk::ShaderStageFlagBits, std::wstring> _sTarget =
	{
		std::make_pair(vk::ShaderStageFlagBits::eFragment, L"ps_6_6"),
		std::make_pair(vk::ShaderStageFlagBits::eVertex, L"vs_6_6"),
		std::make_pair(vk::ShaderStageFlagBits::eCompute, L"cs_6_6")
	};

protected:
	virtual bool DoLoad() override { return true; }
	virtual bool DoUnload() override { return true; }

public:
	Shader(const std::string& pID, vk::ShaderStageFlagBits pShaderStage, const std::string& pShader) : Resource(pID)
	{
		_stage = pShaderStage;
		_shader = pShader;
	}

	~Shader() override
	{
		Unload();
	}

	bool Load() override
	{
		std::vector<uint32_t> spv;

		DxcBuffer sourceBuffer;
		sourceBuffer.Ptr = _shader.c_str();
		sourceBuffer.Size = _shader.size();
		sourceBuffer.Encoding = DXC_CP_ACP;

		std::vector<LPCWSTR> args
		{
			L"-spirv",
			L"-T",
			_sTarget[_stage].c_str(),
			L"-E",
			L"main",
	#ifndef NDEBUG
			L"-Zi",
			L"-Qembed_debug"
	#endif // NDEBUG
		};

		ComPtr<IDxcResult> result;
		_compiler->Compile(&sourceBuffer, args.data(), static_cast<uint32_t>(args.size()), _includeHandler.Get(), IID_PPV_ARGS(&result));

		//check for compilation errors
		ComPtr<IDxcBlobUtf8> errors;
		if (SUCCEEDED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr)) && errors && errors->GetStringLength() > 0)
		{
			std::stringstream ss;
			ss << "Shader compilation errors : " << errors->GetStringPointer();
			throw std::runtime_error(ss.str());
		}

		//write compilation to spv
		ComPtr<IDxcBlob> shaderBlob;
		if (SUCCEEDED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr)))
		{
			const uint64_t byteCount = shaderBlob->GetBufferSize();

			spv.resize(byteCount * 0.25f);
			std::memcpy(spv.data(), shaderBlob->GetBufferPointer(), byteCount);
		};

		vk::ShaderModuleCreateInfo createInfo
		{
			.codeSize = spv.size() * sizeof(uint32_t),
			.pCode = spv.data()
		};

		_shaderModule = Device::Inst().GetDevice().createShaderModule(createInfo);

		return Resource::Load();
	}

	void Unload() override
	{
		if (IsLoaded())
		{
			auto& device = *Device::Inst().GetDevice();
			device.destroyShaderModule(_shaderModule);

			Resource::Unload();
		}
	}

	vk::ShaderModule GetShaderModule() const { return _shaderModule; }
	vk::ShaderStageFlagBits GetStage() const { return _stage; }
};

//class Material : public Resource
//{
//public:
//	Material 
//	{
//	}
//
//	~Material : public Resource()
//	{
//	}
//
//private:
//
//};
