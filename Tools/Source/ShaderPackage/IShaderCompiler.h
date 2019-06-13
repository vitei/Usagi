#pragma once
#include <string>
#include <vector>
#include "ResourcePakExporter.h"

bool ParseManually(const char* szFileName, const char* szDefines, const std::string& includes, std::string& fileOut, std::vector<std::string>& referencedFiles);

struct ShaderEntry : public ResourceEntry
{
	virtual const void* GetData() override { return binary; }
	virtual uint32 GetDataSize() override { return binarySize; };
	virtual const void* GetCustomHeader() { return &entry; }
	virtual uint32 GetCustomHeaderSize() { return sizeof(entry); }

	usg::PakFileDecl::ShaderEntry entry;

	void* binary;
	uint32 binarySize;
};

class IShaderCompiler
{
public:
	IShaderCompiler();
	virtual ~IShaderCompiler();

	virtual void Init() {}

	virtual bool Compile(const std::string& inputFileName, const std::string& setDefines, const std::string& tempFileName, const std::string& includes,
		ShaderEntry& shader, std::vector<std::string>& referencedFiles) = 0;

	virtual void CleanUp() {}

};

