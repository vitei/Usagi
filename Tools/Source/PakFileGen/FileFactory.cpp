#include "Engine/Common/Common.h"
#include "../ResourceLib/ResourcePakExporter.h"
#include "Engine/Scene/Model/Model.pb.h"
#include "Engine/Graphics/Materials/Material.pb.h"
#include "Engine/Audio/AudioBank.pb.h"
#include "../Ayataka/common/ModelConverterBase.h"
#include "../Ayataka/fbx/FbxConverter.h"
#include <algorithm>
#include <fstream>
#include <filesystem>
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

void FileFactory::Init(const char* rootPath, const char* tempDir, const char* pakName)
{
	m_rootDir = rootPath;
	m_tempDir = tempDir;
	m_pakName = pakName;
	m_pakName = RemovePath(m_pakName);
	m_pakName = RemoveExtension(m_pakName);
}

std::string FileFactory::LoadWavFile(const char* szFileName)
{
	return LoadRawFile(szFileName);
}


FileFactory::TextureSettings FileFactory::GetTextureSettings(const YAML::Node& node)
{
	TextureSettings ret;
	if (node)
	{
		const YAML::Node format = node["format"];
		const YAML::Node mips = node["mips"];
		if (format)
		{
			ret.format = format.as<std::string>().c_str();
			ret.bConvert = true;
		}
		if (mips)
		{
			ret.bGenMips = mips.as<bool>();
		}
	}
	return ret;
}

std::string FileFactory::LoadFile(const char* szFileName, YAML::Node node)
{
	// Note we don't have .wav files here as the sound bank adds them
	// For build times we don't want the pack file to be passed the raw files, but they are handled for testing, exception is Audio yaml as it's one file per pack and we need to parse
	std::string outName;
	if (HasExtension(szFileName, "fbx"))
	{
		// Process the fbx file
		outName = LoadModel(szFileName, node);
	}
	else if (HasExtension(szFileName, "vpb"))
	{
		outName = LoadRawFile(szFileName);
	}
	else if (HasExtension(szFileName, "yml"))
	{
		switch(GetYmlType(szFileName))
		{
		case YML_VPB:
			outName = LoadYMLVPBFile(szFileName);
			break;
		case YML_ENTITY:
			outName = LoadYMLEntityFile(szFileName);
			break;
		case YML_AUDIO:
			// Implicitly packages all wav files used by this sound bank
			outName = LoadYMLAudioFile(szFileName);
			break;
		default:
			ASSERT(false);
		}

	}
	else
	{
		return "";
	}


	if (outName.size() > 0)
	{
		AddDependency(szFileName);
	}
	return outName;
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


std::string FileFactory::LoadRawFile(const char* szFileName)
{
	std::string relativePath = std::string(szFileName).substr(m_rootDir.size());

	PureBinaryEntry* pFileEntry = new PureBinaryEntry;
	pFileEntry->srcName = szFileName;
	pFileEntry->SetName(relativePath.c_str(), usg::ResourceType::UNDEFINED);

	FILE* pFileOut = nullptr;

	fopen_s(&pFileOut, szFileName, "rb");
	if (!pFileOut)
	{
		delete pFileEntry;
		return "";
	}

	fseek(pFileOut, 0, SEEK_END);
	pFileEntry->binarySize = ftell(pFileOut);
	fseek(pFileOut, 0, SEEK_SET);
	pFileEntry->binary = new uint8[pFileEntry->binarySize];
	fread(pFileEntry->binary, 1, pFileEntry->binarySize, pFileOut);
	fclose(pFileOut);

	m_resources.push_back(pFileEntry);
	return pFileEntry->GetName();
}


void FileFactory::AddDependency(const char* szFileName)
{
	std::string intermediateDep = szFileName;
	if (std::find(m_referencedFiles.begin(), m_referencedFiles.end(), intermediateDep) == m_referencedFiles.end())
	{
		m_referencedFiles.push_back(intermediateDep);
	}
}



std::string FileFactory::LoadModel(const char* szFileName, const YAML::Node& node)
{
	std::stringstream command;
	std::string relativePath = std::string(szFileName).substr(m_rootDir.size());
	std::string fileNameBase;

	if (node && node["NameInPak"])
	{
		std::string nameOverride = node["NameInPak"].as<std::string>();
		fileNameBase = RemoveFileName(relativePath) + "/" + nameOverride;
	}
	else
	{
		fileNameBase = RemoveExtension(relativePath);
	}

	relativePath = RemoveFileName(fileNameBase) + "/";
	std::string skelDir = m_tempDir + "skel/";
	std::string skelName = szFileName;
	memsize pos = skelName.find_first_of("Models");
	if (pos != std::string::npos)
	{
		skelName = skelName.substr(pos + 7);
	}
	skelName = skelDir + skelName;

	bool bAsCollision = false;
	if (node && node["IsCollision"])
	{
		bAsCollision = node["IsCollision"].as<bool>();
	}
	
	DependencyTracker dependencies;
	ModelConverterBase* pConverter = vnew(usg::ALLOC_OBJECT) FbxConverter;
	int ret = pConverter->Load(szFileName, bAsCollision, false, &dependencies, &node);
	if (ret != 0) {
		FATAL_RELEASE(false, "Failed to find model %s", szFileName);
		vdelete pConverter;
		return false;
	}

	AddDependenciesFromDepTracker(dependencies);

	if (bAsCollision) {
		pConverter->CalculatePolygonNormal();
	}

	pConverter->ReverseCoordinate();

	pConverter->Process();

	// FIXME: Endian options
	if (bAsCollision) {
		pConverter->StoreCollisionBinary(false);
	}
	else {
		pConverter->Store(16U, false);
	}

	PureBinaryEntry* pModel = new PureBinaryEntry;
	pModel->srcName = szFileName;
	pModel->SetName(fileNameBase + ".vmdl", usg::ResourceType::MODEL);
	pModel->binarySize = (uint32)pConverter->GetBinarySize();
	pModel->binary = new uint8[pModel->binarySize];


	pConverter->ExportStoredBinary(pModel->binary, (memsize)pModel->binarySize);

	for (uint32 i = 0; i < pConverter->GetAnimationCount(); i++)
	{
		PureBinaryEntry* pAnim = new PureBinaryEntry;
		pAnim->srcName = szFileName;
		pAnim->SetName(fileNameBase + ".vskla", usg::ResourceType::SKEL_ANIM);
		pAnim->binarySize = (uint32)pConverter->GetAnimBinarySize(i);
		pAnim->binary = new uint8[pModel->binarySize];
		pConverter->ExportAnimation(i, pAnim->binary, (size_t)pAnim->binarySize);
	}

	pConverter->ExportBoneHierarchy(skelDir.c_str());


	std::vector<std::string> textureList = pConverter->GetTextureNames();
	for (auto itr : textureList)
	{
		YAML::Node out;
		out.force_insert("format", pConverter->GetTextureFormat(itr));
		out.force_insert("mips", true);

		FILE* pFile = nullptr;
		std::string fullDir = m_rootDir + relativePath + itr;
		std::string fileName = fullDir + ".tga";

		// Disabling tga for now, gli can't compress. Alternative needed
		fopen_s(&pFile, fileName.c_str(), "rb");
		if(!pFile)
		{
			fileName = fullDir + ".dds";
			fopen_s(&pFile, fileName.c_str(), "rb");
		}

		if (pFile)
		{
			fclose(pFile);
			std::string outName = LoadFile(fileName.c_str(), node);

			if (outName.size())
			{
				pModel->AddDependency(outName.c_str(), "Texture");
			}
		}
	} 


	m_resources.push_back(pModel); 


	return pModel->GetName();
}

bool FileFactory::HasSrcResource(std::string srcName)
{
	for (auto itr : m_resources)
	{
		if (itr->srcName == srcName)
		{
			return true;
		}
	}
	return false;
}

bool FileFactory::HasDestResource(std::string dstName)
{
	for (auto itr : m_resources)
	{
		if (itr->GetName() == dstName)
		{
			return true;
		}
	}
	return false;
}


std::string FileFactory::LoadYMLEntityFile(const char* szFileName)
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

	return pFileEntry->GetName();
}

std::string FileFactory::LoadYMLVPBFile(const char* szFileName)
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

	return pFileEntry->GetName();
}


std::string FileFactory::LoadYMLAudioFile(const char* szFileName)
{
	std::stringstream command;
	std::string relativePath = std::string(szFileName).substr(m_rootDir.size());
	std::string relativeNameNoExt = RemoveExtension(relativePath);
	relativePath = RemoveFileName(relativePath) + "/";
	std::string tempFileName = m_tempDir + relativeNameNoExt + ".vpb";
	std::string depFileName = tempFileName + ".d";

	//std::filesystem::path cwd = std::filesystem::current_path();
	std::string fullPath = /*cwd.string() + std::string("\\") +*/ RemoveFileName(tempFileName);

	command << "ruby Usagi\\Tools\\ruby\\yml2vpb.rb -o " << tempFileName.c_str() << " --MF " << depFileName.c_str() << " -RUsagi/_build/ruby -R_build/ruby" " -d Data/Components/Defaults.yml " << szFileName;

	memsize pos = 0;
	do
	{
		pos = fullPath.find_first_of("\\/", pos + 1);
		CreateDirectory(fullPath.substr(0, pos).c_str(), NULL);
	} while (pos != std::string::npos);

	system(command.str().c_str());

	PureBinaryEntry* pFileEntry = new PureBinaryEntry;
	pFileEntry->srcName = szFileName;
	pFileEntry->SetName(relativeNameNoExt + std::string(".vpb"), usg::ResourceType::PROTOCOL_BUFFER);

	FILE* pFileOut = nullptr;

	fopen_s(&pFileOut, tempFileName.c_str(), "rb");
	if (!pFileOut)
	{
		delete pFileEntry;
		return "";
	}

	// FIXME: Should probably do with a protocol buffer file, but parsing the yml creates fewer dependencies
	YAML::Node mainNode = YAML::LoadFile(szFileName);
	YAML::Node soundFiles = mainNode["AudioBank"]["soundFiles"];

	std::vector< std::string > loadedFiles;
	for (YAML::const_iterator it = soundFiles.begin(); it != soundFiles.end(); ++it)
	{
		if ((*it)["filename"].IsDefined())
		{
			bool bStream = false;
			std::string fileName = "Data/Audio/" + (*it)["filename"].as<std::string>();
			fileName += ".wav";

			if (std::find(loadedFiles.begin(), loadedFiles.end(), fileName) == loadedFiles.end())
			{
				LoadWavFile(fileName.c_str());
				loadedFiles.push_back(fileName);
			}

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

	return pFileEntry->GetName();
}


void FileFactory::AddDependenciesFromDepTracker(DependencyTracker& tracker)
{
	const std::vector<std::string>& deps = tracker.GetDependencies();
	for (auto itr : deps)
	{
		AddDependency(itr.c_str());
	}
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
	std::string out = fileName.substr(fileName.find_last_of("\\/") + 1);
	return out;
}

std::string FileFactory::RemoveFileName(const std::string& fileName)
{
	std::string out = fileName.substr(0, fileName.find_last_of("\\/"));
	return out;
}



