#pragma once

#include "Engine/Core/Utility.h"
#include "../ResourceLib/ResourcePakExporter.h"
#include "../Ayataka/Dependencies/DependencyTracker.h"
#include <yaml-cpp/yaml.h>
#include <sstream>
#include <algorithm>

class FileFactory 
{
public:
	FileFactory();
	virtual ~FileFactory();

	void Init(const char* rootPath, const char* tempDir, const char* pakName);
	virtual std::string LoadFile(const char* szFileName, YAML::Node node);
	// Just load the wav for most platforms but support conversion
	std::string LoadWavFile(const char* szFileName);

	std::string LoadParticleEffect(const char* szFileName);
	std::string LoadParticleEmitter(const char* szFileName);

	void ExportResources(const char* szFileName);
	void WriteDependencies(const char* szFileName);

	void AddTextureDependecy(const char* szTexName, ResourceEntry* pEntry, YAML::Node details);

	// Looks at a timestamps to determine is a rebuild is necessary, should be called for time consuming files
	// particluarly those in mega pak files
	bool FileDirty(const char* szInName, const char* szOutName, const char* szDepName);
	std::string GetBuildFileName(const char* szInName);

protected:

	struct PureBinaryEntry : public ResourceEntry
	{
		PureBinaryEntry();
		virtual ~PureBinaryEntry();

		virtual const void* GetData() override { return binary; }
		virtual uint32 GetDataSize() override { return binarySize; };
		virtual const void* GetCustomHeader() { return nullptr; }
		virtual uint32 GetCustomHeaderSize() { return 0; }


		virtual bool KeepDataAfterLoading() { return bKeepMemory; }



		void* binary;
		uint32 binarySize;
		bool bKeepMemory = false;
	};

	enum class YmlType : uint32
	{
		YML_VPB = 0,
		YML_ENTITY,
		YML_AUDIO,
		YML_LAYOUT
	};

	enum class VpbType : uint32
	{
		VPB_RAW = 0,
		VPB_EFFECT,
		VPB_EMITTER
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
	std::string LoadModel(const char* szFileName, const YAML::Node& node);
	void AddDependency(const char* szFileName);
	void AddDependenciesFromDepFile(const char* szDepFileName, ResourceEntry* pEntry);
	void AddDependenciesFromDepTracker(DependencyTracker& tracker);
	std::string LoadRawFile(const char* szFileName, usg::ResourceType eType = usg::ResourceType::UNDEFINED);
	std::string LoadYMLVPBFile(const char* szFileName);
	std::string LoadYMLEntityFile(const char* szFileName);
	std::string LoadYMLAudioFile(const char* szFileName);
	std::string LoadYMLLayout(const char* szFileName);
	YmlType GetYmlType(const char* szFileName);
	VpbType GetVpbType(const char* szFileName);


	std::string RemoveExtension(const std::string& fileName);
	std::string RemovePath(const std::string& fileName);
	std::string RemoveFileName(const std::string& fileName);
	
	TextureSettings GetTextureSettings(const YAML::Node& node);

	bool HasSrcResource(std::string srcName);
	bool HasDestResource(std::string dstName);


	std::string m_tempDir;
	std::string m_rootDir;
	std::string m_pakName;
	std::vector<ResourceEntry*> m_resources;
	std::vector<std::string> m_referencedFiles;
};

