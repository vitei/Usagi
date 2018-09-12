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
	}
	else
	{
		return false;
	}


	AddDependency(szFileName);
	return true;
}


void FileFactory::AddDependency(const char* szFileName)
{
	std::string intermediateDep = szFileName;
	if (std::find(m_referencedFiles.begin(), m_referencedFiles.end(), intermediateDep) == m_referencedFiles.end())
	{
		m_referencedFiles.push_back(intermediateDep);
	}
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
	std::string depFileName = tempFileName + ".d";
	std::replace(tempFileName.begin(), tempFileName.end(), '/', '\\');
	command << "Usagi\\Tools\\bin\\ayataka.exe -a16 -o" << tempFileName.c_str() << " -d" << depFileName.c_str() << " -h" << skeletonName << " -sk" << animDir.c_str() << " " << szFileName;
	system( (std::string("mkdir ") + RemoveFileName(tempFileName)).c_str());

	system(command.str().c_str());

	ModelEntry* pModel = new ModelEntry;
	pModel->srcName = szFileName;
	pModel->name = relativeNameNoExt + ".vmdl";

	FILE* pFileOut = nullptr;

	fopen_s(&pFileOut, tempFileName.c_str(), "rb");
	if (!pFileOut)
	{
		delete pModel;
		return false;
	}

	fseek(pFileOut, 0, SEEK_END);
	pModel->binarySize = ftell(pFileOut);
	fseek(pFileOut, 0, SEEK_SET);
	pModel->binary = new uint8[pModel->binarySize];
	fread(pModel->binary, 1, pModel->binarySize, pFileOut);

	AddDependenciesFromDepFile(depFileName.c_str(), pModel);

	DeleteFile(tempFileName.c_str());
	DeleteFile(depFileName.c_str());

	m_resources.push_back(pModel);
	return true;
}

void FileFactory::AddDependenciesFromDepFile(const char* szDepFileName, ResourceEntry* pEntry)
{
	std::ifstream depFile(szDepFileName);
	char szCurrentDir[256];
	GetCurrentDirectory(256, szCurrentDir);
	std::string dirLower = szCurrentDir + std::string("/");
	std::replace(dirLower.begin(), dirLower.end(), '\\', '/');
	std::transform(dirLower.begin(), dirLower.end(), dirLower.begin(), ::tolower);
	std::string intermediateDep;
	std::getline(depFile, intermediateDep, ':');
	while (depFile >> intermediateDep)
	{
		if (intermediateDep == "\\")
			continue;
		std::string depLower;
		std::replace(intermediateDep.begin(), intermediateDep.end(), '\\', '/');
		if (pEntry)
		{
			// Full path version for the build system
			pEntry->dependencies.push_back(intermediateDep);
		}
		
		// Simplified case correct version relative directory for the builds purposes (need to map src to dest later)
		depLower.resize(intermediateDep.size());
		std::transform(intermediateDep.begin(), intermediateDep.end(), depLower.begin(), ::tolower);
		if (depLower.find(dirLower) == 0)
		{
			intermediateDep = intermediateDep.substr(dirLower.size());
		}
		AddDependency(intermediateDep.c_str());
	}
}

void FileFactory::ExportResources(const char* szFileName)
{
	ResourcePakExporter::Export(szFileName, m_resources);
}

void FileFactory::WriteDependencies(const char* szFileName)
{
	std::stringstream dependencies;
	
	// FIXME: Probably going to want an input file dictating what goes into the pak file and that will be the root dependency
	std::string formatted;	// m_inputFile;
	std::replace(formatted.begin(), formatted.end(), '\\', '/');
	dependencies << formatted << ": ";

	for (uint32 i = 0; i < m_referencedFiles.size(); i++)
	{
		dependencies << m_referencedFiles[i] << " ";
	}
	// Spit out the dependencies
	std::ofstream depFile(szFileName, std::ofstream::binary);
	depFile.clear();
	depFile << dependencies.str();
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