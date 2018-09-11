#pragma once
#include "Engine/Common/Common.h"
#include "Engine/Core/Utility.h"
#include "../ResourcePak/ResourcePakExporter.h"
#include <sstream>
#include <algorithm>

class FileFactory 
{
public:
	FileFactory();
	virtual ~FileFactory();

	void Init(const char* rootPath, const char* tempDir);
	virtual bool LoadFile(const char* szFileName);
	void ExportResources(const char* szFileName);
	void WriteDependencies(const char* szFileName);

protected:
	const char* GetExtension(const char* szFileName);
	bool HasExtension(const char* szFileName, const char* szExt);
	bool LoadModel(const char* szFileName);
	std::string RemoveExtension(const std::string& fileName);
	std::string RemovePath(const std::string& fileName);
	std::string RemoveFileName(const std::string& fileName);

	std::string m_tempDir;
	std::string m_rootDir;
	std::vector<ResourceEntry*> m_resources;
	std::stringstream m_dependencies;
};

