#pragma once
class FileIO
{
	std::fstream _file;
	std::string _output;
	std::string _filePath;

public:
	FileIO(const std::string& filepath)
	{
		_filePath = filepath;
		_file.open(_filePath, std::ios::out | std::ios::trunc);
	}

	~FileIO()
	{

	}

	RETURN(size_t) GetFileSize()
	{

	}
};

