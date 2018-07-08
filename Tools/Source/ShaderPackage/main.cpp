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


struct DefineSets
{
	std::string name;
	std::string defines;
	uint32		CRC[SHADER_TYPE_COUNT];
};

struct ShaderDefinition
{
	std::string name;
	std::string vert;
	std::string geom;
	std::string frag;

	std::vector<DefineSets> sets;
};

struct Shader
{
	uint32 CRC32;
	usg::ShaderType eType;
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
	usg::mem::InitialiseDefault();
	usg::mem::setConventionalMemManagement(true);
	usg::U8String::InitPool();

	std::string outputstub;
	std::string inputFile; 
	std::string outBinary;

	if (argc != 3)
	{
		printf("Format should be FontCreator <input.yml> <output_dir>");
		return -1;
	}

	inputFile = argv[1];
	outputstub = argv[2];
	outBinary = outputstub + ".vsh";


	printf("Converting %s", inputFile.c_str());

	
	YAML::Node mainNode = YAML::LoadFile(inputFile.c_str());
	YAML::Node shaders = mainNode["Shaders"];
	std::map<uint32, Shader> requiredShaders;
	
	for (YAML::const_iterator it = shaders.begin(); it != shaders.end(); ++it)
	{
		ShaderDefinition def;
		def.name = (*it)["name"].as<std::string>();
		def.vert = (*it)["vert"].as<std::string>();
		def.frag = (*it)["frag"].as<std::string>();
		DefineSets set;
		set.name = def.name;
		set.defines = "";
		if ((*it)["geom"])
		{
			def.geom = (*it)["geom"].as<std::string>();
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
			def.sets[i].CRC[usg::ShaderType::SHADER_TYPE_VS] = utl::CRC32((def.vert + def.sets[i].defines + ".vert").c_str());
			def.sets[i].CRC[usg::ShaderType::SHADER_TYPE_PS] = utl::CRC32((def.frag + def.sets[i].defines + ".frag").c_str());
			if (!def.geom.empty())
			{
				def.sets[i].CRC[usg::ShaderType::SHADER_TYPE_GS] = utl::CRC32((def.geom + def.sets[i].defines + ".geom").c_str());
			}
			else
			{
				def.sets[i].CRC[usg::ShaderType::SHADER_TYPE_GS] = 0;
			}
		}
	}

	usg::U8String::CleanupPool();

	return 0;
}