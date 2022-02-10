#include "Engine/Common/Common.h"
#include "../ResourceLib/ResourcePakExporter.h"
#include "Engine/Scene/Model/Model.pb.h"
#include "Engine/Graphics/Materials/Material.pb.h"
#include "Engine/Audio/AudioBank.pb.h"
#include <algorithm>
#include <fstream>
#include <yaml-cpp/yaml.h>
#include "FileFactory.h"


FileFactory::PureBinaryEntry::PureBinaryEntry()
{
	binary = nullptr;
	binarySize = 0;
}
FileFactory::PureBinaryEntry::~PureBinaryEntry()
{
	if (binary)
	{
		delete binary;
		binary = nullptr;
	}
}

FileFactory::FileFactory()
{
	
}

FileFactory::~FileFactory()
{
	for (auto itr : m_resources)
	{
		if (itr)
		{
			delete itr;
		}
	}
}

void FileFactory::Init(const char* rootPath, const char* tempDir)
{
	m_rootDir = rootPath;
	m_tempDir = tempDir;
}

bool FileFactory::LoadWavFile(const char* szFileName)
{
	return LoadRawFile(szFileName);
}


bool FileFactory::LoadFile(const char* szFileName)
{
	// Note we don't have .wav files here as the sound bank adds them
	// For build times we don't want the pack file to be passed the raw files, but they are handled for testing, exception is Audio yaml as it's one file per pack and we need to parse
	if (HasExtension(szFileName, "fbx"))
	{
		// Process the fbx file
		LoadModel(szFileName);
	}
	else if (HasExtension(szFileName, "vpb"))
	{
		LoadRawFile(szFileName);
	}
	else if (HasExtension(szFileName, "yml"))
	{
		switch(GetYmlType(szFileName))
		{
		case YML_VPB:
			LoadYMLVPBFile(szFileName);
			break;
		case YML_ENTITY:
			LoadYMLEntityFile(szFileName);
			break;
		case YML_AUDIO:
			// Implicitly packages all wav files used by this sound bank
			LoadYMLAudioFile(szFileName);
			break;
		default:
			ASSERT(false);
		}

	}
	else
	{
		return false;
	}


	AddDependency(szFileName);
	return true;
}

FileFactory::YmlType FileFactory::GetYmlType(const char* szFileName)
{
	std::string path = RemoveFileName(szFileName);
	while( path.size() > 0)
	{
		memsize lastPath = path.find_last_of("\\/");
		if (lastPath == std::string::npos)
		{
			return YML_VPB;
		}
		std::string cmpPath = path.substr(lastPath + 1);
		if (cmpPath == "Entities")
		{
			return YML_ENTITY;
		}
		else if(cmpPath == "Audio")
		{
			return YML_AUDIO;
		}

		lastPath = path.find_last_of("\\/");
		if(lastPath == std::string::npos )
		{
			return YML_VPB;
		}

		path = path.substr(0, lastPath);
	}

	return YML_VPB;
}


bool FileFactory::LoadRawFile(const char* szFileName)
{
	std::string relativePath = std::string(szFileName).substr(m_rootDir.size() + 1);

	PureBinaryEntry* pFileEntry = new PureBinaryEntry;
	pFileEntry->srcName = szFileName;
	pFileEntry->SetName(relativePath.c_str(), usg::ResourceType::UNDEFINED);

	FILE* pFileOut = nullptr;

	fopen_s(&pFileOut, szFileName, "rb");
	if (!pFileOut)
	{
		delete pFileEntry;
		return false;
	}

	fseek(pFileOut, 0, SEEK_END);
	pFileEntry->binarySize = ftell(pFileOut);
	fseek(pFileOut, 0, SEEK_SET);
	pFileEntry->binary = new uint8[pFileEntry->binarySize];
	fread(pFileEntry->binary, 1, pFileEntry->binarySize, pFileOut);
	fclose(pFileOut);

	m_resources.push_back(pFileEntry);
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


bool FileFactory::LoadModelVMDL(const char* szFileName)
{
	std::string relativePath = std::string(szFileName).substr(m_rootDir.size() + 1);

	PureBinaryEntry* pFileEntry = new PureBinaryEntry;
	pFileEntry->srcName = szFileName;
	pFileEntry->SetName(relativePath.c_str(), usg::ResourceType::UNDEFINED);

	FILE* pFileOut = nullptr;

	fopen_s(&pFileOut, szFileName, "rb");
	if (!pFileOut)
	{
		delete pFileEntry;
		return false;
	}

	fseek(pFileOut, 0, SEEK_END);
	pFileEntry->binarySize = ftell(pFileOut);
	fseek(pFileOut, 0, SEEK_SET);
	pFileEntry->binary = new uint8[pFileEntry->binarySize];
	fread(pFileEntry->binary, 1, pFileEntry->binarySize, pFileOut);
	fclose(pFileOut);

	usg::exchange::ModelHeader* pHeader = reinterpret_cast<usg::exchange::ModelHeader*>(pFileEntry->binary);
	uint8* pT = reinterpret_cast<uint8*>(pHeader);
	usg::exchange::Material* pInitialMaterial = reinterpret_cast<usg::exchange::Material*>(pT + pHeader->materialOffset);

	const char* szPassNames[usg::exchange::_Material_RenderPass_count]
	{
		"forward",
		"deferred",
		"translucent",
		"depth",
		"omni_depth"
	};


	for(uint32 i=0; i<pHeader->materialNum; i++)
	{
		usg::exchange::Material* pMaterial = &pInitialMaterial[i];
		for(uint32 j=0; j< usg::exchange::Material::textures_max_count; j++)
		{
			if (pMaterial->textures[j].textureName[0] != '\0')
			{
				pFileEntry->AddDependency( RemoveFileName(szFileName) + pMaterial->textures[j].textureName, pMaterial->textures[j].textureHint );
			}
		}

		for (uint32 i = 0; i < usg::exchange::_Material_RenderPass_count; i++)
		{
			std::string fileName = pMaterial->renderPasses[i].effectName + std::string(".fx");
			pFileEntry->AddDependency(fileName.c_str(), szPassNames[i]);
		}
	}

	m_resources.push_back(pFileEntry);
	return true;
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

	PureBinaryEntry* pModel = new PureBinaryEntry;
	pModel->srcName = szFileName;
	pModel->SetName(relativeNameNoExt + ".vmdl", usg::ResourceType::MODEL);

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
	fclose(pFileOut);
	AddDependenciesFromDepFile(depFileName.c_str(), pModel);

	std::replace(tempFileName.begin(), tempFileName.end(), '\\', '/');
	DeleteFile(tempFileName.c_str());
	DeleteFile(depFileName.c_str());

	m_resources.push_back(pModel);

	std::string pathName = animDir + "/*";
	WIN32_FIND_DATA findFileData;
	HANDLE hFind;
	hFind = FindFirstFile(pathName.c_str(), &findFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				continue;
			}
			else
			{
				// Load the animations which were located in the model
				std::string baseName = m_tempDir + "pakgen/" + relativeNameNoExt;
				if (HasExtension(findFileData.cFileName, ".vskla") && strncmp(findFileData.cFileName, baseName.c_str(), baseName.size()) == 0)
				{
					PureBinaryEntry* pAnim = new PureBinaryEntry;
					pModel->SetName(relativePath + findFileData.cFileName, usg::ResourceType::SKEL_ANIM);

					fopen_s(&pFileOut, tempFileName.c_str(), "rb");
					if (!pFileOut)
					{
						delete pAnim;
						return false;
					}

					fseek(pFileOut, 0, SEEK_END);
					pAnim->binarySize = ftell(pFileOut);
					fseek(pFileOut, 0, SEEK_SET);
					pAnim->binary = new uint8[pModel->binarySize];
					fread(pAnim->binary, 1, pAnim->binarySize, pFileOut);
					fclose(pFileOut);
					DeleteFile(tempFileName.c_str());
					m_resources.push_back(pAnim);


				}
			}

		} while (FindNextFile(hFind, &findFileData) != 0);
	}


	return true;
}

bool FileFactory::LoadYMLEntityFile(const char* szFileName)
{
	std::stringstream command;
	std::string relativePath = std::string(szFileName).substr(m_rootDir.size() + 1);
	std::string relativeNameNoExt = RemoveExtension(relativePath);
	relativePath = RemoveFileName(relativePath) + "/";
	std::string tempFileName = m_tempDir + relativeNameNoExt + ".vpb";
	std::string depFileName = tempFileName + ".d";

	command << "Usagi\\Tools\\ruby\\process_hierarchy.rb" 
		<< " -IData/Entities" 
		<< " -RUsagi/_build/ruby -R_build/ruby"
		<< " -d Data/Components/Defaults.yml " 
		<< "-o " << tempFileName.c_str()
		<< " -m _romfiles/win/Models " 
		<< "--MF " << depFileName.c_str() << " "
		<< szFileName;
	CreateDirectory(RemoveFileName(tempFileName).c_str(), 0);

	system(command.str().c_str());

	PureBinaryEntry* pFileEntry = new PureBinaryEntry;
	pFileEntry->srcName = szFileName;
	pFileEntry->SetName(relativePath, usg::ResourceType::PROTOCOL_BUFFER);

	FILE* pFileOut = nullptr;

	fopen_s(&pFileOut, tempFileName.c_str(), "rb");
	if (!pFileOut)
	{
		delete pFileEntry;
		return false;
	}

	fseek(pFileOut, 0, SEEK_END);
	pFileEntry->binarySize = ftell(pFileOut);
	fseek(pFileOut, 0, SEEK_SET);
	pFileEntry->binary = new uint8[pFileEntry->binarySize];
	fread(pFileEntry->binary, 1, pFileEntry->binarySize, pFileOut);
	fclose(pFileOut);

	m_resources.push_back(pFileEntry);

	AddDependenciesFromDepFile(depFileName.c_str(), pFileEntry);

	return true;
}

bool FileFactory::LoadYMLVPBFile(const char* szFileName)
{
	std::stringstream command;
	std::string relativePath = std::string(szFileName).substr(m_rootDir.size() + 1);
	std::string relativeNameNoExt = RemoveExtension(relativePath);
	relativePath = RemoveFileName(relativePath) + "/";
	std::string tempFileName = m_tempDir + relativeNameNoExt + ".vpb";
	std::string depFileName = tempFileName + ".d";

	command << "Usagi\\Tools\\ruby\\yml2vpb.rb -o" << tempFileName.c_str() << " --MF " << depFileName.c_str() << " -RUsagi/_build/ruby -R_build/ruby" " -d Data/Components/Defaults.yml " << szFileName;
	CreateDirectory(RemoveFileName(tempFileName).c_str(), 0);

	system(command.str().c_str());

	PureBinaryEntry* pFileEntry = new PureBinaryEntry;
	pFileEntry->srcName = szFileName;
	pFileEntry->SetName(relativePath, usg::ResourceType::PROTOCOL_BUFFER);

	FILE* pFileOut = nullptr;

	fopen_s(&pFileOut, tempFileName.c_str(), "rb");
	if (!pFileOut)
	{
		delete pFileEntry;
		return false;
	}

	fseek(pFileOut, 0, SEEK_END);
	pFileEntry->binarySize = ftell(pFileOut);
	fseek(pFileOut, 0, SEEK_SET);
	pFileEntry->binary = new uint8[pFileEntry->binarySize];
	fread(pFileEntry->binary, 1, pFileEntry->binarySize, pFileOut);
	fclose(pFileOut);

	m_resources.push_back(pFileEntry);

	AddDependenciesFromDepFile(depFileName.c_str(), pFileEntry);

	return true;
}


bool FileFactory::LoadYMLAudioFile(const char* szFileName)
{
	std::stringstream command;
	std::string relativePath = std::string(szFileName).substr(m_rootDir.size() + 1);
	std::string relativeNameNoExt = RemoveExtension(relativePath);
	relativePath = RemoveFileName(relativePath) + "/";
	std::string tempFileName = m_tempDir + relativeNameNoExt + ".vpb";
	std::string depFileName = tempFileName + ".d";

	command << "Usagi\\Tools\\ruby\\yml2vpb.rb -o" << tempFileName.c_str() << " --MF " << depFileName.c_str() << " -RUsagi/_build/ruby -R_build/ruby" " -d Data/Components/Defaults.yml " << szFileName;
	CreateDirectory(RemoveFileName(tempFileName).c_str(), 0);

	system(command.str().c_str());

	PureBinaryEntry* pFileEntry = new PureBinaryEntry;
	pFileEntry->srcName = szFileName;
	pFileEntry->SetName(relativePath, usg::ResourceType::PROTOCOL_BUFFER);

	FILE* pFileOut = nullptr;

	fopen_s(&pFileOut, tempFileName.c_str(), "rb");
	if (!pFileOut)
	{
		delete pFileEntry;
		return false;
	}

	// FIXME: Should probably do with a protocol buffer file, but parsing the yml creates fewer dependencies
	YAML::Node mainNode = YAML::LoadFile(szFileName);
	YAML::Node soundFiles = mainNode["AudioBank"]["soundFiles"];
	for (YAML::const_iterator it = soundFiles.begin(); it != soundFiles.end(); ++it)
	{
		if ((*it)["filename"].IsDefined())
		{
			bool bStream = false;
			std::string fileName = RemoveFileName(szFileName) + (*it)["filename"].as<std::string>();
			fileName += ".wav";
			LoadWavFile(fileName.c_str());

			if ((*it)["stream"].IsDefined())
			{
				bStream = (*it)["stream"].as<bool>();
			}

			pFileEntry->AddDependency(fileName, bStream ? "stream" : "loaded");
		}
	}

	fseek(pFileOut, 0, SEEK_END);
	pFileEntry->binarySize = ftell(pFileOut);
	fseek(pFileOut, 0, SEEK_SET);
	pFileEntry->binary = new uint8[pFileEntry->binarySize];
	fread(pFileEntry->binary, 1, pFileEntry->binarySize, pFileOut);
	fclose(pFileOut);

	m_resources.push_back(pFileEntry);

	AddDependenciesFromDepFile(depFileName.c_str(), pFileEntry);

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

		// This is for assets that the source might reference
		#if 0
		if (pEntry)
		{
			// Full path version for the build system
			pEntry->AddDependency(intermediateDep, "");
		}
		#endif
		
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



