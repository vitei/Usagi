#include "Engine/Common/Common.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Graphics/Textures/TGAFile.h"
#include "gli/gli.hpp"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Core/Utility.h"
#include "Engine/Layout/Fonts/TextStructs.pb.h"
#include <yaml-cpp/yaml.h>
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

struct ShaderDefinition
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

	if (argc != 3)
	{
		printf("Format should be ShaderPackage <input.yml> <output_dir>");
		return -1;
	}

	inputFile = argv[1];
	outputstub = argv[2];
	outBinary = outputstub + ".vsh";


	printf("Converting %s", inputFile.c_str());

	
	YAML::Node mainNode = YAML::LoadFile(inputFile.c_str());
	YAML::Node shaders = mainNode["Shaders"];
	std::map<uint32, Shader> requiredShaders[(uint32)usg::ShaderType::COUNT];
	
	for (YAML::const_iterator it = shaders.begin(); it != shaders.end(); ++it)
	{
		ShaderDefinition def;
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
			YAML::Node defineSets;
			for (YAML::const_iterator defineIt = defineSets.begin(); it != defineSets.end(); ++defineIt)
			{
				set.name = def.name + (*defineIt)["name"].as<std::string>();
				set.defines = (*defineIt)["defines"].as<std::string>();
				def.sets.push_back(set);
			}
		}

		for (uint32 i = 0; i < def.sets.size(); i++)
		{
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
						Shader shader;
						shader.CRC32 = def.sets[i].CRC[j];
						shader.name = def.prog[i];
					}
				}
			}

		}
	}

	return 0;
}