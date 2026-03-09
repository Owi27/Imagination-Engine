#include "pch.h"
#include "Shader.h"
#include "VulkanCtx.h"
#include <fstream>

std::string Shader::ShaderAsString(const std::string& pShaderFilePath)
{
	std::string out;
	unsigned stringLength = 0;

	std::ifstream file(pShaderFilePath, std::ios::in | std::ios::binary | std::ios::ate);
	stringLength = static_cast<uint64_t>(file.tellg());
	file.seekg(0);
	out.resize(stringLength, '\0');
	file.read(out.data(), stringLength);
	file.close();

	return out;
}

void Shader::ReadSPVFile(const std::string& pSPVFilePath)
{
	std::ifstream file(pSPVFilePath, std::ios::in | std::ios::ate | std::ios::binary);

	_spvSize = static_cast<uint64_t>(file.tellg());
	_spv.resize(_spvSize);
	file.seekg(0);
	file.read(_spv.data(), _spvSize);
	file.close();
}

void Shader::Compile(const std::string& pFilename, ShaderType pShaderType)
{
	DxcBuffer sourceBuffer;
	std::string full;
	std::wstring hlsl, out;

	//convert shader to dxc buffer
	full = "shaders/" + pFilename + ".hlsl";
	_shaderString = ShaderAsString(full.c_str());
	sourceBuffer.Ptr = _shaderString.c_str();
	sourceBuffer.Size = _shaderString.size();
	sourceBuffer.Encoding = DXC_CP_ACP;

	std::wstring tWstring(pFilename.begin(), pFilename.end());

	//define arguments
	std::vector<LPCWSTR> arguments;
	arguments.push_back(L"-spirv");
	arguments.push_back(L"-T");

	switch ((ShaderType)_shaderStageFlagBits)
	{
	case ShaderType::FRAGMENT:
		arguments.push_back(L"ps_6_6");
		break;
	case ShaderType::VERTEX:
		arguments.push_back(L"vs_6_6");
		break;
	case ShaderType::COMPUTE:
		arguments.push_back(L"cs_6_6");
		break;
	default:
		break;
	}

	arguments.push_back(L"-E");
	arguments.push_back(L"main");
	hlsl = L"shaders/" + tWstring + L".hlsl";
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
	if (SUCCEEDED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr)) && errors && errors->GetStringLength() > 0)
	{
		std::cout << "Shader compilation errors: " << errors->GetStringPointer() << "\n";
		return;
	}

	//write compilation to spv
	ComPtr<IDxcBlob> shaderBlob;
	if (SUCCEEDED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr)))
	{
		// Write the compiled shader to file
		std::ofstream outFile(L"shaders/SPV/" + out, std::ios::binary);
		outFile.write(static_cast<const char*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize());
		outFile.close();

		_spvPath = "shaders/SPV/" + pFilename + ".spv";
		ReadSPVFile(_spvPath);
		CreateShaderModule();
	}
}

bool Shader::Reload()
{
	if (ShaderAsString(_shader) == _shaderString) return false;

	Compile(_shader, (ShaderType)_shaderStageFlagBits);

	return true;
}

void Shader::CreateShaderModule()
{
	VkShaderModuleCreateInfo shaderModuleCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		//.pNext = ,
		//.flags = ,
		.codeSize = _spvSize,
		.pCode = (const unsigned*)_spv.c_str(),
	};

	vkCreateShaderModule(VkCtx::Instance().device, &shaderModuleCreateInfo, nullptr, &_shaderModule);
}

VkPipelineShaderStageCreateInfo Shader::GetPipelineShaderStageCreateInfo() const
{
	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		//.pNext = ,
		//.flags = ,
		.stage = _shaderStageFlagBits,
		.module = _shaderModule,
		.pName = _entryPointName.c_str(),
		//.pSpecializationInfo = ,
	};

	return pipelineShaderStageCreateInfo;
}
