#pragma once

#include "Engine/Core/Utility.h"
#include "../ResourceLib/ResourcePakExporter.h"
#include <yaml-cpp/yaml.h>
#include <sstream>
#include <algorithm>

class FileFactory 
{
public:
	FileFactory();
	virtual ~FileFactory();

	void Init(const char* rootPath, const char* tempDir);
	virtual bool LoadFile(const char* szFileName, YAML::Node node);
	// Just load the wav for most platforms but support conversion
	virtual bool LoadWavFile(const char* szFileName);
	void ExportResources(const char* szFileName);
	void WriteDependencies(const char* szFileName);

protected:

	struct PureBinaryEntry : public ResourceEntry
	{
		PureBinaryEntry();
		virtual ~PureBinaryEntry();

		virtual const void* GetData() override { return binary; }
		virtual uint32 GetDataSize() override { return binarySize; };
		virtual const void* GetCustomHeader() { return nullptr; }
		virtual uint32 GetCustomHeaderSize() { return 0; }


		void* binary;
		uint32 binarySize;
	};

	enum YmlType
	{
		YML_VPB = 0,
		YML_ENTITY,
		YML_AUDIO
	};

	struct TextureSettings
	{
		bool bGenMips = true;
		bool bConvert = false;
		// BC7 RGB
		usg::string format = "BC7-srgb";
	};

	const char* GetExtension(const char* szFileName);
	bool HasExtension(const char* szFileName, const char* szExt);
	bool LoadModel(const char* szFileName);
	bool LoadModelVMDL(const char* szFileName);
	void AddDependency(const char* szFileName);
	void AddDependenciesFromDepFile(const char* szDepFileName, ResourceEntry* pEntry);
	bool LoadRawFile(const char* szFileName);
	bool LoadYMLVPBFile(const char* szFileName);
	bool LoadYMLEntityFile(const char* szFileName);
	bool LoadYMLAudioFile(const char* szFileName);
	YmlType GetYmlType(const char* szFileName);


	std::string RemoveExtension(const std::string& fileName);
	std::string RemovePath(const std::string& fileName);
	std::string RemoveFileName(const std::string& fileName);

	TextureSettings GetTextureSettings(const YAML::Node& node);

	std::string m_tempDir;
	std::string m_rootDir;
	std::vector<ResourceEntry*> m_resources;
	std::vector<std::string> m_referencedFiles;
};

