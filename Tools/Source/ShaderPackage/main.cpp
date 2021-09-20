#include "Engine/Common/Common.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Textures/TGAFile.h"
#include "gli/gli.hpp"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Resource/PakDecl.h"
#include "Engine/Core/Utility.h"
#include "../ResourceLib/MaterialDefinition/MaterialDefinitionExporter.h"
#include "Engine/Layout/Fonts/TextStructs.pb.h"
#include "ResourcePakExporter.h"
#include <yaml-cpp/yaml.h>
#include <sstream>
#include <algorithm>
#include <fstream>
#include "VulkanShaderCompiler.h"
#include "OpenGLShaderCompiler.h"
#include <pb.h>




const char* g_szExtensions[] =
{
	".vert",
	".frag",
	".geom",
	".tesc",
	".tese"
};

const char* g_szUsageStrings[] =
{
	"vertex_shader",
	"fragment_shader",
	"geometry_shader",
	"tessellation_control_shader",
	"tessellation_evaluation_shader"
};


	
struct EffectEntry : public ResourceEntry
{
	virtual const void* GetData() override { return nullptr; }
	virtual uint32 GetDataSize() override { return 0; };
	virtual const void* GetCustomHeader() { return nullptr; }
	virtual uint32 GetCustomHeaderSize() { return 0; }

};



struct CustomFXEntry : public ResourceEntry
{
	virtual const void* GetData() override { return materialDef.GetBinary(); }
	virtual uint32 GetDataSize() override { return materialDef.GetBinarySize(); };
	virtual const void* GetCustomHeader() { return &materialDef.GetHeader(); }
	virtual uint32 GetCustomHeaderSize() { return materialDef.GetHeaderSize();  }
	// We keep the data as it is, just fix up the pointers
	virtual bool KeepDataAfterLoading() { return true; }

	MaterialDefinitionExporter	materialDef;
	uint64						definitionCRC;	// Rather than getting overly clever we just check for duplicates

};


struct DefineSets
{
	std::string name;
	std::string defines;
	std::string defineSetName;
	std::string definesAsCRC;
	std::string customFXName;
	uint32		CRC[(uint32)usg::ShaderType::COUNT];
	uint64		CustomFXCRC;
};

struct EffectDefinition
{
	std::string name;
	std::string customFXName;
	std::string prog[(uint32)usg::ShaderType::COUNT];

	std::vector<DefineSets> sets;

};





bool CheckArgument(std::string& target, const std::string& argument)
{
	if (strncmp(target.c_str(), argument.c_str(), argument.length()) == 0)
	{
		target.erase(0, argument.length());
		return true;
	}
	else
	{
		return false;
	}
}

IShaderCompiler* pCompiler = nullptr;

int main(int argc, char *argv[])
{
	std::string inputFile;
	std::string outBinary;
	std::string shaderDir;
	std::string tempDir;
	std::string dependencyFile;
	std::string api;
	std::string intFileName;
	std::string packageName;
	std::string includeDirs;
	std::string arg;

	for (int i = 1; i < argc; i++)
	{
		arg = argv[i];

		if (arg.at(0) != '-')
		{
			inputFile = arg;
		}
		else if (CheckArgument(arg, "-a"))
		{
			api = arg;
		}
		else if (CheckArgument(arg, "-o"))
		{
			outBinary = arg;
		}
		else if (CheckArgument(arg, "-t"))
		{
			tempDir = arg;
		}
		else if (CheckArgument(arg, "-s"))
		{
			shaderDir = arg;
		}
		else if (CheckArgument(arg, "-i"))
		{
			if (!includeDirs.empty())
				includeDirs += " ";
			includeDirs += "-I" + arg;
		}
	}

	if (api == "vulkan")
	{
		pCompiler = new VulkanShaderCompiler;
	}
	else if (api == "ogl")
	{
		pCompiler = new OpenGLShaderCompiler;
	}
	else
	{
		printf("Invalid API");
		return -1;
	}
	pCompiler->Init();

	//dependencyFile = argv[6];
	dependencyFile = outBinary + ".d";
	
	intFileName = inputFile.substr(inputFile.find_last_of("\\/") + 1, inputFile.size());
	intFileName = intFileName.substr(0, intFileName.find_last_of("."));

	printf("Converting %s\n", inputFile.c_str());

	
	YAML::Node mainNode = YAML::LoadFile(inputFile.c_str());
	YAML::Node yamlEffect = mainNode["Effects"];
	YAML::Node customFX = mainNode["CustomEffects"];
	std::map<uint32, ShaderEntry> requiredShaders[(uint32)usg::ShaderType::COUNT];
	std::vector<EffectDefinition> effects;
	std::vector<std::string> referencedFiles;
	std::stringstream effectDependencies;

	{
		std::string formatted = inputFile;
		std::replace(formatted.begin(), formatted.end(), '\\', '/');
		effectDependencies << formatted << ": ";
	}

	std::vector<CustomFXEntry> customFXEntries;
	
	for (YAML::const_iterator it = yamlEffect.begin(); it != yamlEffect.end(); ++it)
	{
		EffectDefinition def;
		def.name = (*it)["name"].as<std::string>();
		def.prog[(uint32)usg::ShaderType::VS] = (*it)["vert"].as<std::string>();
		def.prog[(uint32)usg::ShaderType::PS] = (*it)["frag"].as<std::string>();
		def.customFXName = (*it)["custom_effect"] ? (*it)["custom_effect"].as<std::string>() : "";
		{
			bool bHasDefault = true;
			if ((*it)["has_default"])
			{
				bHasDefault = (*it)["has_default"].as<bool>();
			}

			DefineSets set;
			if (bHasDefault)
			{
				set.name = intFileName + "." + def.name + ".fx";
				set.defineSetName = "";
				set.definesAsCRC = "";
				set.defines = "";
				set.customFXName = def.customFXName;
				if ((*it)["geom"])
				{
					def.prog[(uint32)usg::ShaderType::GS] = (*it)["geom"].as<std::string>();
				}
				if ((*it)["tesc"])
				{
					def.prog[(uint32)usg::ShaderType::TC] = (*it)["tesc"].as<std::string>();
				}
				if ((*it)["tese"])
				{
					def.prog[(uint32)usg::ShaderType::TE] = (*it)["tese"].as<std::string>();
				}
				def.sets.push_back(set);
			}

			if ((*it)["define_sets"])
			{
				YAML::Node defineSets = (*it)["define_sets"];
				for (YAML::const_iterator defineIt = defineSets.begin(); defineIt != defineSets.end(); ++defineIt)
				{
					// Package.Effect.DefineSet.fx
					set.defineSetName = std::string(".") + (*defineIt)["name"].as<std::string>();
					set.customFXName = def.customFXName;
					set.name = intFileName + "." + def.name + set.defineSetName + ".fx";
					set.defines = (*defineIt)["defines"].as<std::string>();
					set.definesAsCRC = std::string(".") + std::to_string(utl::CRC32(set.defines.c_str()));
					def.sets.push_back(set);
				}
			}

			uint32 uStandardSets = (uint32)def.sets.size();

			// Anything in a "global" define set is a combination of defines appended to every other shader
			if ((*it)["global_sets"])
			{
				YAML::Node globalSets = (*it)["global_sets"];
				for (YAML::const_iterator globalIt = globalSets.begin(); globalIt != globalSets.end(); ++globalIt)
				{
					// Package.Effect.DefineSet.fx
					std::string name = (*globalIt)["name"].as<std::string>();
					std::string defines = (*globalIt)["defines"].as<std::string>();

					for (uint32 i = 0; i < uStandardSets; i++)
					{
						set = def.sets[i];
						if (set.defines.length() > 0)
						{
							set.defines = set.defines + " " + defines;
						}
						else
						{
							set.defines = defines;
						}
						set.definesAsCRC = std::string(".") + std::to_string(utl::CRC32(set.defines.c_str()));
						set.defineSetName = set.defineSetName + std::string(".") + (*globalIt)["name"].as<std::string>();
						set.name = intFileName + "." + def.name + set.defineSetName + ".fx";
						def.sets.push_back(set);
					}
				}
			}
		}

		for (uint32 i = 0; i < def.sets.size(); i++)
		{
			def.sets[i].CustomFXCRC = 0;
			MaterialDefinitionExporter* pDef = nullptr;
			if (def.customFXName.size() > 0)
			{
				CustomFXEntry entry;
				entry.materialDef.Load(customFX[def.customFXName], def.sets[i].defines);
				entry.materialDef.InitBinaryData();
				entry.materialDef.InitAutomatedCode();
				uint64 uCustomFXCRC = entry.materialDef.GetCRC();
				bool bFound = false;
				for (int i = 0; i < customFXEntries.size(); i++)
				{
					if (customFXEntries[i].definitionCRC == uCustomFXCRC)
					{
						pDef = &customFXEntries[i].materialDef;
						bFound = true;
					}
				}
				if (!bFound)
				{
					std::string customFXName = intFileName + "." + def.customFXName + "." + std::to_string(customFXEntries.size()) + ".cfx";
					entry.SetName(customFXName, usg::ResourceType::CUSTOM_EFFECT);
					entry.definitionCRC = uCustomFXCRC;
					customFXEntries.push_back(entry);
					pDef = &customFXEntries.back().materialDef;
				}
				def.sets[i].CustomFXCRC = uCustomFXCRC;
			}
			for (uint32 j = 0; j < (uint32)usg::ShaderType::COUNT; j++)
			{
				std::string progName = intFileName + "." + def.prog[j] + def.sets[i].customFXName + def.sets[i].definesAsCRC + g_szExtensions[j] + ".SPV";
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
						std::string inputFileName = def.prog[j] + g_szExtensions[j];
						inputFileName = shaderDir + "/" + inputFileName;
						ShaderEntry shader;
						shader.SetName(progName, usg::ResourceType::SHADER);
						shader.entry.eShaderType = (usg::ShaderType)(j);
						bool bSuccess = false;
						std::string tempFileName = intFileName + ".SPV";
						tempFileName = tempDir + "/" + tempFileName;
						bSuccess = pCompiler->Compile(inputFileName, def.sets[i].defines, tempFileName, includeDirs, shader, pDef, referencedFiles, (usg::ShaderType)j);
						if (!bSuccess)
						{
							return -1;
						}
						requiredShaders[j][def.sets[i].CRC[j]] = shader;
					}
				}
			}
		}
		effects.push_back(def);
	}

	for (uint32 i = 0; i < referencedFiles.size(); i++)
	{
		effectDependencies << referencedFiles[i] << " ";
	}

	std::vector<ResourceEntry*> resources;
	std::vector<EffectEntry> effectEntries;

	for (auto& effectItr : effects)
	{
		for (auto& setItr : effectItr.sets)
		{
			EffectEntry effect;
			effect.SetName(setItr.name, usg::ResourceType::EFFECT);

			for (uint32 i = 0; i < (uint32)usg::ShaderType::COUNT; i++)
			{
				if (setItr.CRC[i] != 0)
				{
					auto shaderEntry = requiredShaders[i].find(setItr.CRC[i]);
					if (shaderEntry != requiredShaders[i].end())
					{
						effect.AddDependency((*shaderEntry).second.GetName(), g_szUsageStrings[i]);
					}
				}
			}

			if (setItr.CustomFXCRC != 0)
			{
				for (auto& customFX : customFXEntries)
				{
					if (customFX.definitionCRC == setItr.CustomFXCRC)
					{
						effect.AddDependency(customFX.GetName(), "CustomFX");
						break;
					}
				}
			}
			effectEntries.push_back(effect);
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

	for (auto& effectItr : effectEntries)
	{
		resources.push_back(&effectItr);
	}

	for (auto& customFX : customFXEntries)
	{
		resources.push_back(&customFX);
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

	// Spit out the dependencies
	std::ofstream depFile(dependencyFile.c_str(), std::ofstream::binary);
	depFile.clear();
	depFile << effectDependencies.str();

	pCompiler->Cleanup();
	delete pCompiler;

	return 0;
}