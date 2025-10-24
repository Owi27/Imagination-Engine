#include "D:/GitHub/Imagination-Engine/build/CMakeFiles/Imagination.dir/Debug/cmake_pch.hxx"
#include "Shader.h"

RETURN(std::string) Shader::ShaderAsString(const char* shaderFilePath)
{
	//TODO
	std::string out;
	unsigned stringLength = 0;
	GW::SYSTEM::GFile file;
	file.Create();
	file.GetFileSize(shaderFilePath, stringLength);

	if (stringLength && +file.OpenBinaryRead(shaderFilePath))
	{
		out.resize(stringLength);
		file.Read(&out[0], stringLength);
	}

	return out;
}

//RETURN(void) Shader::WritetToSPVFile()
//{
//	std::ifstream spv(_spvPath, std::ios::binary | std::ios::ate);
//	
//	if (!spv) return std::unexpected("Shader.cpp | ReadShaderFile() | shader spv was null");
//
//	std::streamsize size = spv.tellg();
//	spv.seekg(0, std::ios::beg);
//
//	_shaderSize = (unsigned long long)size;
//
//	data = new char[size + 1];
//
//	if (!spv.read(data, size))
//	{
//		delete[] data;
//		data = nullptr;
//		size = 0;
//		
//		return std::unexpected("Shader.cpp | ReadShaderFile() | failed to read shader spv");
//	}
//
//	data[size] = '\0';
//}
//
//RETURN(void) Shader::CreateShader()
//{
//	ATTEMPT(ReadSPVFile());
//	ATTEMPT(CreateShaderModule());
//
//	VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo
//	{
//		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
//		//.pNext = ,
//		//.flags = ,
//		.stage = _shaderStageFlagBits,
//		.module = _shaderModule,
//		.pName = _entryPointName.c_str(),
//		//.pSpecializationInfo = ,
//	};
//
//	
//}

RETURN(void) Shader::Compile(const std::string& filename, ShaderType shaderType)
{
	// Convert shader code to a DXC blob
	DxcBuffer sourceBuffer;
	std::string full;
	std::wstring hlsl, out;

	//convert shader to dxc buffer
	full = "Shaders/" + filename + ".hlsl";
	_shaderString = *ShaderAsString(full.c_str());
	sourceBuffer.Ptr = _shaderString.c_str();
	sourceBuffer.Size = _shaderString.size();
	sourceBuffer.Encoding = DXC_CP_ACP;

	std::wstring tWstring(filename.begin(), filename.end());

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
	if (SUCCEEDED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr)) && errors && errors->GetStringLength() > 0) return std::unexpected("Shader compilation errors: " + (char)errors->GetStringPointer());

	//write compilation to spv
	ComPtr<IDxcBlob> shaderBlob;
	if (SUCCEEDED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr)))
	{
		// Write the compiled shader to file
		std::ofstream outFile(L"Shaders/SPV/" + out, std::ios::binary);
		outFile.write(static_cast<const char*>(shaderBlob->GetBufferPointer()), shaderBlob->GetBufferSize());
		outFile.close();

		//_spvPath = "Shaders/SPV/" + filename + ".spv";
		//GvkHelper::create_shader(_device, spvPath.c_str(), "main", _shaderStageFlagBits, &_shaderModule, &_pssci);

		ATTEMPT(CreateShaderModule());
	}
}

RETURN(bool) Shader::Reload()
{
	if (ShaderAsString(_shader.c_str()) == _shaderString) return false;

	VkShaderModuleCreateInfo shaderModuleCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		//.pNext = ,
		//.flags = ,
		.codeSize = _shaderSize,
		.pCode = (const unsigned*)_shaderString.c_str(),
	};

	vkDestroyShaderModule(_device, _shaderModule, nullptr);
	if (vkCreateShaderModule(_device, &shaderModuleCreateInfo, nullptr, &_shaderModule)) return std::unexpected("Shader.cpp | CreateShaderModule() | vkCreateShaderModule");
}

RETURN(void) Shader::CreateShaderModule()
{
	//Setup create info
	VkShaderModuleCreateInfo shaderModuleCreateInfo
	{
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		//.pNext = ,
		//.flags = ,
		.codeSize = _shaderSize,
		.pCode = (const unsigned*)_shaderString.c_str(),
	};

	if (vkCreateShaderModule(_device, &shaderModuleCreateInfo, nullptr, &_shaderModule)) return std::unexpected("Shader.cpp | CreateShaderModule() | vkCreateShaderModule");
}

RETURN(VkPipelineShaderStageCreateInfo) Shader::GetPipelineShaderStageCreateInfo() const
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
