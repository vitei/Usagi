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
#include <ShaderLang.h>
#include <pb.h>


const char* g_szExtensions[] =
{
	".vert",
	".geom",
	".frag"
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
	uint32 uShaderBinaryOffset;
	uint32 uShaderCount;
	uint32 uEffectBinaryOffset;
	uint32 uEffectCount;
};

int main(int argc, char *argv[])
{
	std::string outputstub;
	std::string inputFile; 
	std::string outBinary;
	std::string shaderDir;
	std::string tempDir;

	if (argc != 5)
	{
		printf("Format should be ShaderPackage <input.yml> <temporary_dir> <output_dir> <shader_dir>");
		return -1;
	}

	inputFile = argv[1];
	tempDir = argv[2];
	outputstub = argv[3];
	shaderDir = argv[4];
	
	outBinary = outputstub + ".vsh";


	printf("Converting %s", inputFile.c_str());

	
	YAML::Node mainNode = YAML::LoadFile(inputFile.c_str());
	YAML::Node shaders = mainNode["Shaders"];
	std::map<uint32, Shader> requiredShaders[(uint32)usg::ShaderType::COUNT];
	std::vector<EffectDefinition> effects;

	Header hdr;
	hdr.uEffectBinaryOffset = sizeof(Header);
	hdr.uShaderCount = 0;
	hdr.uEffectCount = 0;
	hdr.uShaderBinaryOffset = 0;
	uint32 uShaderBinarySize = 0;
	
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
				defines += std::string(" -D") + defineList.substr(0, nextDefine);
				if (nextDefine != std::string::npos)
				{
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
						std::string outputFileName = inputFileName + ".spv";
						inputFileName = shaderDir + "\\" + inputFileName;
						outputFileName = tempDir + "\\" + outputFileName;
						Shader shader;
						shader.CRC32 = def.sets[i].CRC[j];
						shader.name = def.prog[i];
						std::stringstream command;
						std::string outputDir = outputFileName.substr(0, outputFileName.find_last_of("\\/"));
						CreateDirectory(outputDir.c_str(), NULL);
						command << "glslc " << inputFileName.c_str() << " -o" << outputFileName.c_str() << " -std=450 -Werror " << defines;
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

						uShaderBinarySize += shader.binarySize + sizeof(ShaderEntry);
						hdr.uShaderCount++;
					}
				}
			}
		}
	}

	hdr.uShaderBinaryOffset = hdr.uEffectBinaryOffset + (sizeof(EffectDefinition) * hdr.uEffectCount);
	uint32 uFileSize = hdr.uShaderBinaryOffset + uShaderBinarySize;

	FILE* pFileOut;
	CreateDirectory(outBinary.substr(0, outBinary.find_last_of("\\/")).c_str(), NULL);
	fopen_s(&pFileOut, outBinary.c_str(), "w");

	fwrite(&hdr, sizeof(Header), 1, pFileOut);
	fwrite(effects.data(), sizeof(EffectDefinition), effects.size(), pFileOut);

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

	return 0;
}