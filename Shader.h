#pragma once
#include "VulkanContext.h"

enum ShaderType : unsigned char
{
	FRAGMENT_SHADER,
	VERTEX_SHADER,
	COMPUTE_SHADER
};

class Shader
{
	VulkanContext& _vk;
	VkShaderModule _shaderModule;
	VkShaderStageFlagBits _shaderStageFlagBits;
	VkPipelineShaderStageCreateInfo _pssci;
	std::string _entryPointName = "main";

	std::string ShaderAsString(const char* shaderFilePath);

public:
	Shader(std::string filename, ShaderType shaderType) : _vk(*VulkanContext::GetInst())
	{
		switch (shaderType)
		{
		case FRAGMENT_SHADER:
			_shaderStageFlagBits = VK_SHADER_STAGE_FRAGMENT_BIT;
			break;
		case VERTEX_SHADER:
			_shaderStageFlagBits = VK_SHADER_STAGE_VERTEX_BIT;
			break;
		case COMPUTE_SHADER:
			_shaderStageFlagBits = VK_SHADER_STAGE_COMPUTE_BIT;
			break;
		default:
			break;
		}

		// Convert shader code to a DXC blob
		DxcBuffer sourceBuffer;
		std::string shaderCode, full;
		std::wstring hlsl, out;

		//convert shader to dxc buffer
		full = "Shaders/" + filename + ".hlsl";
		shaderCode = ShaderAsString(full.c_str());
		sourceBuffer.Ptr = shaderCode.c_str();
		sourceBuffer.Size = shaderCode.size();
		sourceBuffer.Encoding = DXC_CP_ACP;

		std::wstring tWstring(filename.begin(), filename.end());

		//define arguments
		std::vector<LPCWSTR> arguments;
		arguments.push_back(L"-spirv");
		arguments.push_back(L"-T");

		switch (shaderType)
		{
		case FRAGMENT_SHADER:
			arguments.push_back(L"ps_6_6");
			break;
		case VERTEX_SHADER:
			arguments.push_back(L"vs_6_6");
			break;
		case COMPUTE_SHADER:
			arguments.push_back(L"cs_6_6");
			break;
		default:
			break;
		}

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
		_vk.GetCompiler()->Compile(&sourceBuffer, arguments.data(), arguments.size(), _vk.GetIncludeHandler().Get(), IID_PPV_ARGS(&result));

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

			std::string spvPath = "Shaders/SPV/" + filename + ".spv";
			GvkHelper::create_shader(_vk.GetDevice(), spvPath.c_str(), "main", _shaderStageFlagBits, &_shaderModule, &_pssci);
		}
	}

	~Shader()
	{
		vkDestroyShaderModule(_vk.GetDevice(), _shaderModule, nullptr);
	}

	VkShaderModule GetShaderModule() const { return _shaderModule; }
	VkShaderStageFlagBits GetShaderStageFlagBits() const { return _shaderStageFlagBits; }
	std::string GetEntryPointName() const { return _entryPointName; }

	void SetEntryPointName(const std::string& entryPointName) { _entryPointName = entryPointName; }
};