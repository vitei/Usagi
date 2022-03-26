#include "Engine/Common/Common.h"
#include "Engine/Graphics/RenderConsts.h"
#include "FileFactory.h"
#include "FileFactoryWin.h"
#include "Engine/Graphics/Textures/TGAFile.h"
#include "Engine/Resource/PakDecl.h"
#include "Engine/Core/Utility.h"
#include "../ResourceLib/ResourcePakExporter.h"
#include <yaml-cpp/yaml.h>
#include <sstream>
#include <fstream>
#include <ShaderLang.h>
#include <pb.h>

FileFactory* g_pFileFactory = nullptr;

bool CheckArgument(std::string& target, const std::string& argument)
{
	if ( strncmp(target.c_str(), argument.c_str(), argument.length() ) == 0)
	{
		target.erase(0, argument.length());
		return true;
	}
	else
	{
		return false;
	}
}


bool ProcessFile(const std::string& fileName, YAML::Node node)
{
	return g_pFileFactory->LoadFile(fileName.c_str(), node).size() > 0;
}

// We shouldn't use this, creating pak file definitions for dependency sanity
bool ProcessFiles(const std::string& inputDir)
{
	std::string pathName = inputDir + "/*";
	WIN32_FIND_DATA findFileData;
	HANDLE hFind;
	hFind = FindFirstFile(pathName.c_str(), &findFileData);

	YAML::Node fakeNode;

	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("Invalid directory\n");
		return false;
	}

	do
	{
		if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (strcmp(findFileData.cFileName, ".") != 0 
				&& strcmp(findFileData.cFileName, "..") != 0 )
			{
				if( !ProcessFiles(inputDir + "/" + findFileData.cFileName) )
					return false;
			}
		}
		else
		{
			if (!ProcessFile(inputDir + "/" + findFileData.cFileName, fakeNode))
				return false;
		}

	} while (FindNextFile(hFind, &findFileData) != 0);

	return true;
}


// TODO: This should also handle shader file packs
int main(int argc, char *argv[])
{
	std::string platform = "win";
	std::string input;
	std::string outputFile;
	std::string dependencyFile;
	std::string tempDir;
	std::string arg;
	bool bIsPakDefinition = false;

	for (int i = 1; i < argc; i++)
	{
		arg = argv[i];

		if (arg.at(0) != '-')
		{
			input = arg;
		}
		else if (CheckArgument(arg, "-p"))
		{
			platform = arg;
		}
		else if (CheckArgument(arg, "-o"))
		{
			outputFile = arg;
		}
		else if (CheckArgument(arg, "-def"))
		{
			bIsPakDefinition = true;
		}
		else if (CheckArgument(arg, "-d"))
		{
			dependencyFile = arg;
		}
		else if (CheckArgument(arg, "-t"))
		{
			tempDir = arg;
		}

	}

	if (input.empty() || outputFile.empty())
	{
		printf("Invalid arguments\nProper usage PakFileGen <<inputdir>> -o<<outputfile>>");
		return -1;
	}

	if (tempDir.empty())
	{
		tempDir = "_build/";
	}

	if (dependencyFile.empty())
	{
		dependencyFile = outputFile + ".d";
	}

	if (platform == "win")
	{
		g_pFileFactory = (FileFactory*)(new FileFactoryWin);
	}
	if (!g_pFileFactory)
	{
		printf("Failed to create file factory");
		return -1;
	}

	std::string rootDir = "Data/";
	memsize first = input.find(rootDir);
	std::string rootPath = std::string(input).substr( 0, first + rootDir.size());


	g_pFileFactory->Init(rootPath.c_str(), tempDir.c_str(), outputFile.c_str());


	// Abandoning auto directories as need meta data
	if (!bIsPakDefinition)
	{
		YAML::Node dummyNode;
		ProcessFile(input.c_str(), dummyNode);
	}
	else
	{
		std::string path = input.c_str();
		size_t pathEnd = path.find_last_of("\\/");
		if (pathEnd != std::string::npos)
		{
			path = path.substr(0, pathEnd);
		}

		YAML::Node mainNode = YAML::LoadFile(input.c_str());
		if (!mainNode)
			return -1;

		YAML::Node resources = mainNode["Resources"];
		if (!resources)
			return -1;

		for (YAML::const_iterator it = resources.begin(); it != resources.end(); ++it)
		{
			YAML::Node file = (*it)["File"];
			YAML::Node data = (*it)["Data"];

			if (file)
			{
				std::string fileName = path + "/" +  file.as<std::string>();
				ProcessFile(fileName, data);
			}
		}
	}
	
	//if (!ProcessFiles(inputDir))
	//	return -1;
	

	// Write out the file
	g_pFileFactory->ExportResources(outputFile.c_str());
	g_pFileFactory->WriteDependencies(dependencyFile.c_str());


	delete g_pFileFactory;

	return 0;
}