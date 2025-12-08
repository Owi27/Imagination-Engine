#pragma once
#include <vulkan/vulkan.h>
#include <Windows.h>
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
	VkShaderModule _shaderModule;
	VkShaderStageFlagBits _shaderStageFlagBits;
	VkPipelineShaderStageCreateInfo _pssci;
	std::string _entryPointName = "main", _shaderString, _spvPath, _shader, _spv;
	uint64_t _spvSize;

	//dxc
	ComPtr<IDxcCompiler3> _compiler;
	ComPtr<IDxcUtils> _utils;
	ComPtr<IDxcIncludeHandler> _includeHandler;

	std::string ShaderAsString(const std::string& shaderFilePath);
	void ReadSPVFile(const std::string& pSPVFilePath);
	void Compile(const std::string& pFilename, ShaderType pShaderType);
	bool Reload();
	void CreateShaderModule();


public:
	Shader() = default;

	Shader(const std::string& pFileName, ShaderType pShaderType)
	{
		_shaderStageFlagBits = (VkShaderStageFlagBits)pShaderType;

		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&_compiler));
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&_utils));
		_utils->CreateDefaultIncludeHandler(&_includeHandler);

		Compile(pFileName, pShaderType);
	}

	VkPipelineShaderStageCreateInfo GetPipelineShaderStageCreateInfo() const;
};

