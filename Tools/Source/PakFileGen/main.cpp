#include "Engine/Common/Common.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Graphics/Textures/TGAFile.h"
#include "gli/gli.hpp"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Resource/PakDecl.h"
#include "Engine/Core/Utility.h"
#include "Engine/Layout/Fonts/TextStructs.pb.h"
#include "../ResourcePak/ResourcePakExporter.h"
#include <yaml-cpp/yaml.h>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <ShaderLang.h>
#include <pb.h>


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


bool ProcessFile(const std::string& fileName)
{
	return true;
}

bool ProcessFiles(const std::string& inputDir)
{
	std::string pathName = inputDir + "/*";
	WIN32_FIND_DATA findFileData;
	HANDLE hFind;
	hFind = FindFirstFile(pathName.c_str(), &findFileData);

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
			if (!ProcessFile(inputDir + "/" + findFileData.cFileName))
				return false;
		}

	} while (FindNextFile(hFind, &findFileData) != 0);

	return true;
}


// TODO: This should also handle shader file packs
int main(int argc, char *argv[])
{
	std::string platform = "win";
	std::string inputDir;
	std::string outputFile;
	std::string dependencyFile;
	std::string arg;

	for (int i = 1; i < argc; i++)
	{
		arg = argv[i];

		if (arg.at(0) != '-')
		{
			inputDir = arg;
		}
		else if (CheckArgument(arg, "-p"))
		{
			platform = arg;
		}
		else if (CheckArgument(arg, "-o"))
		{
			outputFile = arg;
		}
		else if (CheckArgument(arg, "-d"))
		{
			dependencyFile = arg;
		}
	}

	if (inputDir.empty() || outputFile.empty())
	{
		printf("Invalid arguments\nProper usage PakFileGen <<inputdir>> -o<<outputfile>>");
		return -1;
	}

	if (dependencyFile.empty())
	{
		dependencyFile = outputFile + ".d";
	}


	if (!ProcessFiles(inputDir))
		return -1;
	

	// Write out the file
	std::stringstream dependencies;
	std::vector<ResourceEntry*> resources;
	ResourcePakExporter::Export(outputFile.c_str(), resources);


	// Spit out the dependencies
	std::ofstream depFile(dependencyFile.c_str(), std::ofstream::binary);
	depFile.clear();
	depFile << dependencies.str();


	return 0;
}