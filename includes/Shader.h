#pragma once
#include <dxcapi.h>
#include <wrl/client.h>
#pragma comment(lib, "dxcompiler.lib")
using namespace Microsoft::WRL;

enum class ShaderType
{
	FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
	VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
	COMPUTE = VK_SHADER_STAGE_COMPUTE_BIT
};

class Shader
{
	VkDevice _device;
	VkShaderModule _shaderModule;
	VkShaderStageFlagBits _shaderStageFlagBits;
	VkPipelineShaderStageCreateInfo _pssci;
	std::string _entryPointName = "main", _shaderString, _spvPath, _shader, _spv;
	unsigned long long _shaderSize, _spvSize;
	//std::string _shaderString, _spvPath, _newShader;
	//char* data;

	//dxc
	ComPtr<IDxcCompiler3> _compiler;
	ComPtr<IDxcUtils> _utils;
	ComPtr<IDxcIncludeHandler> _includeHandler;

	RETURN(std::string) ShaderAsString(const char* shaderFilePath);
	RETURN(void) ReadSPVFile(const std::string& filename);
	//RETURN(void) CreateShader();
	RETURN(void) Compile(const std::string& filename, ShaderType shaderType);
	RETURN(bool) Reload();
	RETURN(void) CreateShaderModule();

public:
	Shader() = default;

	Shader(VkDevice device, const std::string& filename, ShaderType shaderType)
	{
		_device = device;
		_shaderStageFlagBits = (VkShaderStageFlagBits)shaderType;

		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler));
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils));
		_utils->CreateDefaultIncludeHandler(&_includeHandler);

		Attempt(Compile(filename, shaderType));
	}

	~Shader()
	{
		vkDestroyShaderModule(_device, _shaderModule, nullptr);
		//delete[] data;
	}

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	Shader(Shader&& o) noexcept
		: _device(o._device),
		_shaderModule(o._shaderModule),
		_shaderStageFlagBits(o._shaderStageFlagBits),
		_entryPointName(std::move(o._entryPointName)),
		_shaderString(std::move(o._shaderString)),
		_spvPath(std::move(o._spvPath)),
		_shader(std::move(o._shader)),
		_spv(std::move(o._spv)),
		_compiler(std::move(o._compiler)),
		_utils(std::move(o._utils)),
		_includeHandler(std::move(o._includeHandler))
	{
		o._device = VK_NULL_HANDLE;
		o._shaderModule = VK_NULL_HANDLE;
		o._shaderStageFlagBits = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	}

	Shader& operator=(Shader&& o) noexcept
	{
		if (this != &o)
		{
			if (_shaderModule != VK_NULL_HANDLE && _device != VK_NULL_HANDLE)
			{
				vkDestroyShaderModule(_device, _shaderModule, nullptr);
				_shaderModule = VK_NULL_HANDLE;
			}
			_device = o._device;
			_shaderModule = o._shaderModule;
			_shaderStageFlagBits = o._shaderStageFlagBits;
			_entryPointName = std::move(o._entryPointName);
			_shaderString = std::move(o._shaderString);
			_spvPath = std::move(o._spvPath);
			_shader = std::move(o._shader);
			_spv = std::move(o._spv);
			_compiler = std::move(o._compiler);
			_utils = std::move(o._utils);
			_includeHandler = std::move(o._includeHandler);

			o._device = VK_NULL_HANDLE;
			o._shaderModule = VK_NULL_HANDLE;
			o._shaderStageFlagBits = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		}

		return *this;
	}

	VkShaderModule GetShaderModule() const { return _shaderModule; }
	VkShaderStageFlagBits GetShaderStageFlagBits() const { return _shaderStageFlagBits; }
	std::string GetEntryPointName() const { return _entryPointName; }
	RETURN(VkPipelineShaderStageCreateInfo) GetPipelineShaderStageCreateInfo() const;

	void SetEntryPointName(const std::string& entryPointName) { _entryPointName = entryPointName; }
};