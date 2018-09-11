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

void FileFactory::Init(const char* rootPath, const char* tempDir)
{
	m_rootDir = rootPath;
	m_tempDir = tempDir;
}

bool FileFactory::LoadFile(const char* szFileName)
{
	if (HasExtension(szFileName, "fbx"))
	{
		// Process the fbx file
		LoadModel(szFileName);
		return true;
	}

	
	return false;
}

bool FileFactory::LoadModel(const char* szFileName)
{
	std::stringstream command;
	std::string relativePath = std::string(szFileName).substr(m_rootDir.size()+1);
	std::string relativeNameNoExt = RemoveExtension(relativePath);
	relativePath = RemoveFileName(relativePath) + "/";
	std::string tempFileName = m_tempDir + "pakgen/" + relativeNameNoExt + ".vmdl";
	std::string skeletonName = m_tempDir + "skel/" + relativeNameNoExt + ".xml";
	std::string animDir = m_tempDir + "pakgen/" + relativePath;
	std::replace(tempFileName.begin(), tempFileName.end(), '/', '\\');
	command << "ayataka.exe -a16 -o" << tempFileName.c_str() << " -h" << skeletonName << " -sk" << animDir.c_str() << " " << szFileName;
	system( (std::string("mkdir ") + RemoveFileName(tempFileName)).c_str());

	system(command.str().c_str());

	DeleteFile(tempFileName.c_str());
	return true;
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

const char* FileFactory::GetExtension(const char* szFileName)
{
	const char* chr = strrchr(szFileName, '.');
	if (chr)
		szFileName = chr + 1;
	while (chr)
	{
		chr = strchr(szFileName, '.');
		if (chr)
			szFileName = chr + 1;
	}
	return szFileName;
}

bool FileFactory::HasExtension(const char* szFileName, const char* szExt)
{
	if (_stricmp(GetExtension(szFileName), szExt) == 0)
	{
		return true;
	}
	return false;
}

std::string FileFactory::RemoveExtension(const std::string& fileName)
{
	std::string out = fileName.substr(0, fileName.find_last_of("."));
	return out;
}

std::string FileFactory::RemovePath(const std::string& fileName)
{
	std::string out = fileName.substr(fileName.find_last_of("\\/")+1);
	return out;
}

std::string FileFactory::RemoveFileName(const std::string& fileName)
{
	std::string out = fileName.substr(0, fileName.find_last_of("\\/"));
	return out;
}