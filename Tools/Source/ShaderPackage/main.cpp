#include "Engine/Common/Common.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Core/String/U8String.h"
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
#include <ShaderLang.h>
#include <pb.h>


const char* g_szExtensions[] =
{
	".vert",
	".frag",
	".geom"
};

const char* g_szUsageStrings[] =
{
	"vertex_shader",
	"fragment_shader",
	"geometry_shader"
};

struct EffectEntry : public ResourceEntry
{
	virtual const void* GetData() override { return nullptr; }
	virtual uint32 GetDataSize() override { return 0; };
	virtual const void* GetCustomHeader() { return nullptr; }
	virtual uint32 GetCustomHeaderSize() { return 0; }

};

struct ShaderEntry : public ResourceEntry
{
	virtual const void* GetData() override { return binary; }
	virtual uint32 GetDataSize() override { return binarySize; };
	virtual const void* GetCustomHeader() { return &entry; }
	virtual uint32 GetCustomHeaderSize() {return sizeof(entry); }

	usg::PakFileDecl::ShaderEntry entry;

	void* binary;
	uint32 binarySize;
};

struct CustomFXEntry : public ResourceEntry
{
	virtual const void* GetData() override { return materialDef.GetBinary(); }
	virtual uint32 GetDataSize() override { return materialDef.GetBinarySize(); };
	virtual const void* GetCustomHeader() { return &materialDef.GetHeader(); }
	virtual uint32 GetCustomHeaderSize() { return materialDef.GetHeaderSize();  }

	MaterialDefinitionExporter materialDef;
};


struct DefineSets
{
	std::string name;
	std::string defines;
	std::string defineSetName;
	std::string definesAsCRC;
	uint32		CRC[(uint32)usg::ShaderType::COUNT];
};

struct EffectDefinition
{
	std::string name;
	std::string prog[(uint32)usg::ShaderType::COUNT];

	std::vector<DefineSets> sets;

};

bool ParseManually(const char* szFileName, const char* szDefines, const std::string& includes, std::string& fileOut, std::vector<std::string>& referencedFiles);


bool CompileOGLShader(const std::string& inputFileName, const std::string& setDefines, const std::string& includes,
	ShaderEntry& shader, std::vector<std::string>& referencedFiles)
{
	std::string shaderCode;
	std::string defines = setDefines;
	if(setDefines.size() > 0)
		defines += " ";
	defines += "PLATFORM_PC ";
	defines += "API_OGL";
	
	if (ParseManually(inputFileName.c_str(), defines.c_str(), includes, shaderCode, referencedFiles))
	{
		shader.binary = new uint8[shaderCode.size()+1];
		memcpy(shader.binary, shaderCode.data(), shaderCode.size()+1);
		shader.binarySize = (uint32)shaderCode.size()+1;
		return true;
	}
	return false;
}

bool CompileVulkanShader(const std::string& inputFileName, const std::string& setDefines, const std::string& tempFileName, const std::string& includes,
	ShaderEntry& shader, std::vector<std::string>& referencedFiles)
{
	// Get the input file name
	std::string outputFileName = tempFileName;

	std::string defines = "-DPLATFORM_PC -DAPI_VULKAN";
	// TODO: Handle multiple include directories
	std::string defineList = setDefines;
	size_t nextDefine = std::string::npos;
	while (!defineList.empty())
	{
		nextDefine = defineList.find_first_of(' ');
		if (nextDefine != std::string::npos)
		{
			defines += std::string(" -D") + defineList.substr(0, nextDefine);
			defineList = defineList.substr(nextDefine + 1);
		}
		else
		{
			// The last define
			defines += std::string(" -D") + defineList;
			defineList.clear();
		}

	} while (nextDefine != std::string::npos);

	std::stringstream command;
	// Delete the last instance of this file
	DeleteFile(outputFileName.c_str());
	std::replace(outputFileName.begin(), outputFileName.end(), '/', '\\');
	std::string outputDir = outputFileName.substr(0, outputFileName.find_last_of("\\/"));
	CreateDirectory(outputDir.c_str(), NULL);

	command << "glslc " << inputFileName.c_str() << " -o" << outputFileName.c_str() << " -MD -std=450 -Werror " << defines << " " << includes;
	//glslang::TShader* shader = new glslang::TShader(g_glslLangLang[j]);
	// FIXME: code for glslang natively, but for now it is cleaner to use the command line
	system(command.str().c_str()); 

	FILE* pFileOut = nullptr;
	fopen_s(&pFileOut, outputFileName.c_str(), "rb");
	// FIXME: Should do cleanup
	if (!pFileOut)
		return false;
	fseek(pFileOut, 0, SEEK_END);
	shader.binarySize = ftell(pFileOut);
	fseek(pFileOut, 0, SEEK_SET);
	shader.binary = new uint8[shader.binarySize];
	fread(shader.binary, 1, shader.binarySize, pFileOut);


	std::string depFileName = outputFileName + ".d";
	std::ifstream depFile(depFileName);

	std::string intermediateDep;
	std::getline(depFile, intermediateDep, ':');
	while (depFile >> intermediateDep)
	{
		std::replace(intermediateDep.begin(), intermediateDep.end(), '\\', '/');
		if (std::find(referencedFiles.begin(), referencedFiles.end(), intermediateDep) == referencedFiles.end())
		{
			referencedFiles.push_back(intermediateDep);
		}
	}

	fclose(pFileOut);
	depFile.close();

	return true;
}


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

	//dependencyFile = argv[6];
	dependencyFile = outBinary + ".d";
	
	intFileName = inputFile.substr(inputFile.find_last_of("\\/") + 1, inputFile.size());
	intFileName = intFileName.substr(0, intFileName.find_last_of("."));

	printf("Converting %s\n", inputFile.c_str());

	
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
	
	for (YAML::const_iterator it = shaders.begin(); it != shaders.end(); ++it)
	{
		EffectDefinition def;
		def.name = (*it)["name"].as<std::string>();
		def.prog[(uint32)usg::ShaderType::VS] = (*it)["vert"].as<std::string>();
		def.prog[(uint32)usg::ShaderType::PS] = (*it)["frag"].as<std::string>();
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
				if ((*it)["geom"])
				{
					def.prog[(uint32)usg::ShaderType::GS] = (*it)["geom"].as<std::string>();
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
			for (uint32 j = 0; j < (uint32)usg::ShaderType::COUNT; j++)
			{
				std::string progName = intFileName + "." + def.prog[j] + def.sets[i].definesAsCRC + g_szExtensions[j] + ".SPV";
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
						if (api == "vulkan")
						{
							std::string tempFileName = intFileName + ".SPV";
							tempFileName = tempDir + "/" + tempFileName;
							bSuccess = CompileVulkanShader(inputFileName, def.sets[i].defines, tempFileName, includeDirs, shader, referencedFiles);
						}
						else if (api == "ogl")
						{
							bSuccess = CompileOGLShader(inputFileName, def.sets[i].defines, includeDirs, shader, referencedFiles);
						}
						else
						{
							ASSERT(false);
						}
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


	return 0;
}