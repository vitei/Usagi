#include "Engine/Common/Common.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Graphics/Textures/TGAFile.h"
#include "gli/gli.hpp"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Core/Utility.h"
#include "Engine/Layout/Fonts/TextStructs.pb.h"
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

struct EffectEntry
{
	char		name[USG_MAX_PATH] = {};
	uint32		CRC[(uint32)usg::ShaderType::COUNT] = {};
	uint32		binarySize;	// 0 on platforms which do not compile complete effect combinations
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

struct Shader
{
	uint32 CRC32;
	std::string name;
	void* binary;
	uint32 binarySize;
};


struct ShaderEntry
{
	char			szName[256];
	usg::ShaderType eType;
	uint32			uBinarySize;
	// Binary follows directly after
};

struct Header
{
	uint32 uShaderBinaryOffset; // Not valid on platforms which do compile complete effect combinations
	uint32 uShaderCount;
	uint32 uEffectDefinitionOffset;
	uint32 uEffectBinaryOffset;	// Not valid on platforms which don't compile complete effect combinations
	uint32 uEffectCount;
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
	std::map<uint32, Shader> requiredShaders[(uint32)usg::ShaderType::COUNT];
	std::vector<EffectDefinition> effects;
	std::vector<std::string> referencedFiles;
	std::stringstream effectDependencies;

	{
		std::string formatted = inputFile;
		std::replace(formatted.begin(), formatted.end(), '\\', '/');
		effectDependencies << formatted << ": ";
	}
	uint32 uShaderBinarySize = 0;
	Header hdr;
	hdr.uEffectBinaryOffset = sizeof(Header);
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
				if (!def.prog[j].empty())
				{
					def.sets[i].CRC[j] = utl::CRC32((def.prog[j] + def.sets[i].defines).c_str());
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
						Shader shader;
						shader.CRC32 = def.sets[i].CRC[j];
						shader.name = def.prog[j];
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
						requiredShaders[j][shader.CRC32] = shader;

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

						uShaderBinarySize += shader.binarySize + sizeof(ShaderEntry);
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

	hdr.uShaderBinaryOffset = hdr.uEffectBinaryOffset + (sizeof(EffectDefinition) * hdr.uEffectCount);
	uint32 uFileSize = hdr.uShaderBinaryOffset + uShaderBinarySize;

	FILE* pFileOut;
	std::replace(outBinary.begin(), outBinary.end(), '/', '\\');
	std::string tmp = outBinary.substr(0, outBinary.find_last_of("\\/")).c_str();
	CreateDirectory(tmp.c_str(), NULL);
	fopen_s(&pFileOut, outBinary.c_str(), "w");

	fwrite(&hdr, sizeof(Header), 1, pFileOut);
	for (auto &itr : effects)
	{

		for (auto &definesItr : itr.sets)
		{
			EffectEntry effectEntry;
			effectEntry.binarySize = 0;	// Not yet supported
			memcpy(effectEntry.CRC, definesItr.CRC, sizeof(definesItr.CRC));
			sprintf_s(effectEntry.name, definesItr.name.c_str());
			fwrite(&effectEntry, sizeof(EffectEntry), 1, pFileOut);
		}
	}

	for (uint32 i = 0; i < (uint32)usg::ShaderType::COUNT; i++)
	{
		for (auto itr = requiredShaders[i].begin(); itr != requiredShaders[i].end(); ++itr)
		{
			ShaderEntry entry;
			entry.eType = (usg::ShaderType)i;
			strcpy_s(entry.szName, sizeof(entry.szName), (*itr).second.name.c_str());
			entry.uBinarySize = (*itr).second.binarySize;
			fwrite(&entry, sizeof(ShaderEntry), 1, pFileOut);
			fwrite((*itr).second.binary, 1, (*itr).second.binarySize, pFileOut);
			delete (*itr).second.binary;
		}
	}	
	fclose(pFileOut);
	pFileOut = nullptr;

	// Spit out the dependencies
	std::ofstream depFile(dependencyFile.c_str(), std::ofstream::binary);
	depFile.clear();
	depFile << effectDependencies.str();


	return 0;
}