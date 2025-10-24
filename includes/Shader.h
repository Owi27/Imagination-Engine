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
	std::string _entryPointName = "main", _shaderString, _spvPath, _shader;
	unsigned long long _shaderSize;
	//std::string _shaderString, _spvPath, _newShader;
	char* data;

	//dxc
	ComPtr<IDxcCompiler3> _compiler;
	ComPtr<IDxcUtils> _utils;
	ComPtr<IDxcIncludeHandler> _includeHandler;

	RETURN(std::string) ShaderAsString(const char* shaderFilePath);
	//RETURN(char*) ReadSPVFile();
	//RETURN(void) CreateShader();
	RETURN(void) Compile(const std::string& filename, ShaderType shaderType);
	RETURN(bool) Reload();
	RETURN(void) CreateShaderModule();

public:
	Shader(VkDevice device, const std::string& filename, ShaderType shaderType)
	{
		_device = device;
		_shaderStageFlagBits = (VkShaderStageFlagBits)shaderType;

		ATTEMPT(Compile(filename, shaderType));
	}

	~Shader()
	{
		vkDestroyShaderModule(_device, _shaderModule, nullptr);
		delete[] data;
	}

	VkShaderModule GetShaderModule() const { return _shaderModule; }
	VkShaderStageFlagBits GetShaderStageFlagBits() const { return _shaderStageFlagBits; }
	std::string GetEntryPointName() const { return _entryPointName; }
	RETURN(VkPipelineShaderStageCreateInfo) GetPipelineShaderStageCreateInfo() const;

	void SetEntryPointName(const std::string& entryPointName) { _entryPointName = entryPointName; }
};