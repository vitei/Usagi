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

	void ExportResources(const char* szFileName);
	void WriteDependencies(const char* szFileName);

protected:
	std::vector<ResourceEntry*> m_resources;
	std::stringstream m_dependencies;
};

