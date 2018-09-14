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

	struct PureBinaryEntry : public ResourceEntry
	{
		PureBinaryEntry();
		virtual ~PureBinaryEntry();

		virtual void* GetData() override { return binary; }
		virtual uint32 GetDataSize() override { return binarySize; };
		virtual void* GetCustomHeader() { return nullptr; }
		virtual uint32 GetCustomHeaderSize() { return 0; }


		void* binary;
		uint32 binarySize;
	};

	const char* GetExtension(const char* szFileName);
	bool HasExtension(const char* szFileName, const char* szExt);
	bool LoadModel(const char* szFileName);
	void AddDependency(const char* szFileName);
	void AddDependenciesFromDepFile(const char* szDepFileName, ResourceEntry* pEntry);
	std::string RemoveExtension(const std::string& fileName);
	std::string RemovePath(const std::string& fileName);
	std::string RemoveFileName(const std::string& fileName);

	std::string m_tempDir;
	std::string m_rootDir;
	std::vector<ResourceEntry*> m_resources;
	std::vector<std::string> m_referencedFiles;
};

