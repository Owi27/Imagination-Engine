#pragma once

enum ShaderType
{
	PIXEL_SHADER,
	VERTEX_SHADER,
	COMPUTE_SHADER
};

class Shader
{
	VkShaderModule _shaderModule;
	VkShaderStageFlagBits _shaderStageFlagBits;
	VkPipelineShaderStageCreateInfo _pssci;
	VulkanContext* _vk = VulkanContext::GetInst();

	std::string ShaderAsString(const char* shaderFilePath);

public:
	Shader(std::string filename, ShaderType shaderType);
	~Shader();

	VkShaderModule GetVkShaderModule() const { return _shaderModule; }
	VkShaderStageFlagBits GetVkShaderStageFlagBits() const { return _shaderStageFlagBits; }
};