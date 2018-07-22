#include "Engine/Common/Common.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Graphics/Textures/TGAFile.h"
#include "gli/gli.hpp"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Resource/EffectPakDecl.h"
#include "Engine/Core/Utility.h"
#include "Engine/Layout/Fonts/TextStructs.pb.h"
#include "../ResourcePak/ResourcePakExporter.h"
#include <yaml-cpp/yaml.h>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <ShaderLang.h>
#include <pb.h>


const char* g_szExtensions[] =
{
	".vert",
	".frag",
	".geom"
};

struct EffectEntry : public ResourceEntry
{
	virtual void* GetData() override { return nullptr; }
	virtual uint32 GetDataSize() override { return 0; };
	virtual void* GetCustomHeader() { return &entry; }
	virtual uint32 GetCustomHeaderSize() { return sizeof(entry); }

	usg::PakFileDecl::EffectEntry entry;
};

struct ShaderEntry : public ResourceEntry
{
	virtual void* GetData() override { return binary; }
	virtual uint32 GetDataSize() override { return binarySize; };
	virtual void* GetCustomHeader() { return nullptr; }
	virtual uint32 GetCustomHeaderSize() { return 0; }

	void* binary;
	uint32 binarySize;
};


struct DefineSets
{
	std::string name;
	std::string defines;
	uint32		CRC[(uint32)usg::ShaderType::COUNT];
};

struct EffectDefinition
{
	std::string name;
	std::string prog[(uint32)usg::ShaderType::COUNT];

	std::vector<DefineSets> sets;
};


int main(int argc, char *argv[])
{
	std::string inputFile; 
	std::string outBinary;
	std::string shaderDir;
	std::string tempDir;
	std::string dependencyFile;
	std::string api;
	std::string intFileName;

	if (argc != 6)
	{
		printf("Format should be ShaderPackage <input.yml> <output> <temporary_dir> <shader_dir> <api>");
		return -1;
	}

	inputFile = argv[1];
	outBinary = argv[2];
	tempDir = argv[3];
	shaderDir = argv[4];
	api = argv[5];
	//dependencyFile = argv[6];
	
	intFileName = inputFile.substr(inputFile.find_last_of("\\/") + 1, inputFile.size());
	intFileName = intFileName.substr(0, intFileName.find_last_of("."));

	printf("Converting %s", inputFile.c_str());

	
	YAML::Node mainNode = YAML::LoadFile(inputFile.c_str());
	YAML::Node shaders = mainNode["Effects"];
	std::map<uint32, ShaderEntry> requiredShaders[(uint32)usg::ShaderType::COUNT];
	std::vector<EffectDefinition> effects;
	std::vector<std::string> referencedFiles;
	std::stringstream effectDependencies;

	{
		std::string formatted = inputFile;
		std::replace(formatted.begin(), formatted.end(), '\\', '/');
		effectDependencies << formatted << ": ";
	}
	uint32 uShaderBinarySize = 0;
	usg::EffectPakDecl::Header hdr;
	hdr.uShaderDeclOffset = sizeof(hdr);
	hdr.uShaderCount = 0;
	hdr.uEffectCount = 0;
	hdr.uShaderBinaryOffset = 0;
	
	for (YAML::const_iterator it = shaders.begin(); it != shaders.end(); ++it)
	{
		EffectDefinition def;
		def.name = (*it)["name"].as<std::string>();
		def.prog[(uint32)usg::ShaderType::VS] = (*it)["vert"].as<std::string>();
		def.prog[(uint32)usg::ShaderType::PS] = (*it)["frag"].as<std::string>();
		DefineSets set;
		set.name = def.name;
		set.defines = "";
		if ((*it)["geom"])
		{
			def.prog[(uint32)usg::ShaderType::GS] = (*it)["geom"].as<std::string>();
		}
		def.sets.push_back(set);
		if ((*it)["define_sets"])
		{
			YAML::Node defineSets = (*it)["define_sets"];
			for (YAML::const_iterator defineIt = defineSets.begin(); defineIt != defineSets.end(); ++defineIt)
			{
				set.name = def.name + "." + (*defineIt)["name"].as<std::string>();
				set.defines = (*defineIt)["defines"].as<std::string>();
				def.sets.push_back(set);
			}
		}

		effects.push_back(def);
		hdr.uEffectCount += (uint32)def.sets.size();

		for (uint32 i = 0; i < def.sets.size(); i++)
		{
			std::string defines = "-DPLATFORM_PC -DAPI_VULKAN";
			std::string defineList = set.defines;
			size_t nextDefine = std::string::npos;
			do
			{
				nextDefine = defineList.find_first_of(' ');
				if (nextDefine != std::string::npos)
				{
					defines += std::string(" -D") + defineList.substr(0, nextDefine);
					defineList = defineList.substr(nextDefine + 1);
				}

			} while (nextDefine != std::string::npos);
			for (uint32 j = 0; j < (uint32)usg::ShaderType::COUNT; j++)
			{
				std::string progName = def.prog[j] + def.sets[i].defines + g_szExtensions[j] + ".SPV";
				if (!def.prog[j].empty())
				{
					def.sets[i].CRC[j] = utl::CRC32(progName.c_str());
				}
				else
				{
					def.sets[i].CRC[j] = 0;
				}

				if (def.sets[i].CRC[j] != 0)
				{
					if (requiredShaders[j].find(def.sets[i].CRC[j]) == requiredShaders[j].end())
					{
						// Get the input file name
						std::string inputFileName = def.prog[j] + g_szExtensions[j];
						std::string outputFileName = intFileName + ".SPV";
						inputFileName = shaderDir + "\\" + inputFileName;
						outputFileName = tempDir + "\\" + outputFileName;
						ShaderEntry shader;
						shader.name = progName;
						std::stringstream command;
						std::replace(outputFileName.begin(), outputFileName.end(), '/', '\\');
						std::string outputDir = outputFileName.substr(0, outputFileName.find_last_of("\\/"));
						CreateDirectory(outputDir.c_str(), NULL);

						command << "glslc " << inputFileName.c_str() << " -o" << outputFileName.c_str() << " -MD -std=450 -Werror " << defines;
						//glslang::TShader* shader = new glslang::TShader(g_glslLangLang[j]);
						// FIXME: code for glslang natively, but for now it is cleaner to use the command line
						system(command.str().c_str());

						FILE* pFileOut = nullptr;
						fopen_s(&pFileOut, outputFileName.c_str(), "r");
						fseek(pFileOut, 0, SEEK_END);
						shader.binarySize = ftell(pFileOut);
						fseek(pFileOut, 0, SEEK_SET);
						shader.binary = new uint8[shader.binarySize];
						fread(shader.binary, 1, shader.binarySize, pFileOut);
						requiredShaders[j][def.sets[i].CRC[j]] = shader;

						std::string depFileName = outputFileName + ".d";
						std::ifstream depFile(depFileName);

						std::string intermediateDep;
						std::getline(depFile, intermediateDep, ':');
						while ( depFile >> intermediateDep )
						{
							std::replace(intermediateDep.begin(), intermediateDep.end(),'\\', '/');
							if (std::find(referencedFiles.begin(), referencedFiles.end(), intermediateDep) == referencedFiles.end())
							{
								referencedFiles.push_back(intermediateDep);
							}
						}

						fclose(pFileOut);
						depFile.close();

						uShaderBinarySize += shader.binarySize + sizeof(usg::EffectPakDecl::ShaderEntry);
						hdr.uShaderCount++;
					}
				}
			}
		}
	}

	for (uint32 i = 0; i < referencedFiles.size(); i++)
	{
		effectDependencies << referencedFiles[i] << " ";
	}

	hdr.uShaderBinaryOffset = hdr.uShaderBinaryOffset + sizeof(usg::EffectPakDecl::ShaderEntry) * hdr.uShaderCount;
	hdr.uEffectDefinitionOffset = hdr.uShaderBinaryOffset + uShaderBinarySize;
	hdr.uEffectBinaryOffset = hdr.uEffectDefinitionOffset + sizeof(usg::EffectPakDecl::EffectEntry) * hdr.uEffectCount;

	uint32 uFileSize = hdr.uShaderBinaryOffset + uShaderBinarySize;

	std::vector<ResourceEntry*> resources;
	std::vector<EffectEntry> effectEntries;
	for (auto& effectItr : effects)
	{
		for (auto& setItr : effectItr.sets)
		{
			EffectEntry effect;
			effect.name = setItr.name;
			memcpy(effect.entry.CRC, setItr.CRC, sizeof(effect.entry.CRC));
			effectEntries.push_back(effect);
			resources.push_back( &effectEntries.back() );
		}
	}

	for (uint32 i = 0; i < (uint32)usg::ShaderType::COUNT; i++)
	{
		for (auto& itr : requiredShaders[i])
		{
			ResourceEntry* entry = &itr.second;
			resources.push_back(entry);
		}
	}

	// Write out the file
	ResourcePakExporter::Export(outBinary.c_str(), resources);

	// Delete the binary data
	for (uint32 i = 0; i < (uint32)usg::ShaderType::COUNT; i++)
	{
		for (auto& itr : requiredShaders[i])
		{
			if (itr.second.binary)
			{
				delete itr.second.binary;
				itr.second.binary = nullptr;
			}

		}
	}

	return 0;
}