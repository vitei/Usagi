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


bool ProcessFile(const std::string& fileName)
{
	return g_pFileFactory->LoadFile(fileName.c_str());
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
	std::string tempDir;
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
		else if (CheckArgument(arg, "-t"))
		{
			tempDir = arg;
		}
	}

	if (inputDir.empty() || outputFile.empty())
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

	g_pFileFactory->Init(inputDir.c_str(), tempDir.c_str());


	if (!ProcessFiles(inputDir))
		return -1;
	

	// Write out the file
	g_pFileFactory->ExportResources(outputFile.c_str());
	g_pFileFactory->WriteDependencies(dependencyFile.c_str());


	delete g_pFileFactory;

	return 0;
}