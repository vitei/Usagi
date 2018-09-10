#include "Engine/Common/Common.h"
#include "../ResourcePak/ResourcePakExporter.h"
#include <algorithm>
#include <fstream>
#include "FileFactory.h"


FileFactory::FileFactory()
{

}

FileFactory::~FileFactory()
{

}

void FileFactory::ExportResources(const char* szFileName)
{
	ResourcePakExporter::Export(szFileName, m_resources);
}

void FileFactory::WriteDependencies(const char* szFileName)
{
	// Spit out the dependencies
	std::ofstream depFile(szFileName, std::ofstream::binary);
	depFile.clear();
	depFile << m_dependencies.str();
}

