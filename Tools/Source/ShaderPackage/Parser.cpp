#include "Engine/Common/Common.h"
#include "gli/gli.hpp"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Resource/PakDecl.h"
#include "Engine/Core/Utility.h"
#include "ResourcePakExporter.h"
#include <yaml-cpp/yaml.h>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <pb.h>


bool LocatePragma(char* szProgram, char*& szPragmaLine, char*& szNextLine)
{
	szPragmaLine = strstr(szProgram, "#include");
	if (!szPragmaLine)
	{
		return false;
	}

	// Terminate the previous string
	szPragmaLine[0] = '\0';
	szPragmaLine++;

	szNextLine = szPragmaLine;
	while (*szNextLine != '\n' && *szNextLine != '\0')
	{
		szNextLine++;
	}

	ASSERT(*szNextLine == '\n');
	*szNextLine = '\0';	// Terminate the pragma line
	szNextLine++;

	return true;
}

bool ParseManually(const char* szFileName, const char* szDefines, const std::string& includes, std::string& fileOut, std::vector<std::string>& referencedFiles)
{
	const int MAX_STRING_ARRAY = 30;
	FILE* pShaderFile;
	fopen_s(&pShaderFile, szFileName, "rb");

	char* buffer[MAX_STRING_ARRAY];
	uint32 uBufferCount = 0;

	if (!pShaderFile)
		return false;

	// Add this file to the set of references
	std::string fullName = szFileName;
	char fullPath[256];
	_fullpath(fullPath, fullName.c_str(), 256);
	referencedFiles.push_back(fullPath);

	char defines[256];
	char* szDefineLoc = defines;
	strcpy_s(szDefineLoc, 256, "#version 450\n");
	szDefineLoc += 13;
	if (szDefines)
	{
		while (*szDefines != '\0')
		{
			strcpy_s(szDefineLoc, &defines[256] - szDefineLoc, "#define ");
			szDefineLoc += 8;
			while (*szDefines == ' ')
			{
				szDefines++;
			} 

			do
			{
				if (*szDefines == '=')
				{
					*szDefineLoc++ = ' ';
				}
				else
				{
					*szDefineLoc++ = *szDefines;
				}
				
				szDefines++;
			} while (*szDefines != ' ' && *szDefines != '\0');
			*szDefineLoc++ = '\n';
		}
		*szDefineLoc++ = '\0';
	}
	else
	{
		defines[0] = '\0';
	}
	char* stringArray[MAX_STRING_ARRAY];
	// File size
	fseek(pShaderFile, 0, SEEK_END);
	memsize uFileSize = ftell(pShaderFile);
	fseek(pShaderFile, 0, SEEK_SET);

	buffer[0] = (char*)malloc(sizeof(char)*uFileSize + 1);
	uBufferCount++;
	ASSERT(buffer[0] != NULL);

	if (!buffer[0])
		return false;

	memsize uRead = fread(buffer[0], 1, uFileSize, pShaderFile);
	buffer[0][uRead] = '\0';
	fclose(pShaderFile);


	stringArray[0] = defines;
	stringArray[1] = buffer[0];

	uint32 uArrayIndex = 1;
	bool bFound = true;
	char* szPragma = NULL;
	while (bFound)
	{
		bFound = LocatePragma(stringArray[uArrayIndex], szPragma, stringArray[uArrayIndex + 1]);

		if (!bFound)
		{
			// No more pragmas
			break;
		}

		while (*szPragma != '\"')
			szPragma++;

		szPragma++;

		char* search = szPragma;
		while (*search != '\0' && *search != '\"')
			search++;

		ASSERT(*search == '\"');
		*search = '\0';

		FILE* pIncludeFile;
		std::string includeName = szFileName;
		includeName = includeName.substr(0, includeName.find_last_of("\\/"));
		includeName += "/";
		includeName += szPragma;
		std::string fullIncludeName = includeName;
		fopen_s(&pIncludeFile, includeName.c_str(), "rb");
		if (!pIncludeFile)
		{
			std::string nextInclude = includes;
			while (nextInclude.find_first_of("-I") != std::string::npos)
			{
				nextInclude = nextInclude.substr(nextInclude.find_first_of("-I") + 2);
				std::string includePath = nextInclude;
				if (includePath.find_first_of(" ") != std::string::npos)
				{
					includePath = includePath.substr(0, includePath.find_first_of(" "));
				}
				// Check in the include paths
				fullIncludeName = includePath + "/" + szPragma;
				fopen_s(&pIncludeFile, fullIncludeName.c_str(), "rb");
			}
		}

		if (!pIncludeFile)
		{
			printf("Could not find include file %s in %s\n", includeName.c_str(), szFileName);
			return false;
		}

		_fullpath(fullPath, fullIncludeName.c_str(), 256);
		referencedFiles.push_back(fullPath);

		fseek(pIncludeFile, 0, SEEK_END);
		memsize uFileSize = ftell(pIncludeFile);
		fseek(pIncludeFile, 0, SEEK_SET);

		buffer[uBufferCount] = (char*)malloc(sizeof(char)*uFileSize + 1);

		uRead = fread(buffer[uBufferCount], 1, uFileSize, pIncludeFile);
		buffer[uBufferCount][uRead] = '\0';
		fclose(pIncludeFile);

		stringArray[uArrayIndex + 2] = stringArray[uArrayIndex + 1];
		stringArray[uArrayIndex + 1] = buffer[uBufferCount];

		uBufferCount++;
		uArrayIndex += 2;
	}

	fileOut = "";
	for (uint32 i = 0; i < uArrayIndex+1; i++)
	{
		fileOut += stringArray[i];
	}

	for (uint32 i = 0; i < uBufferCount; i++)
	{
		free(buffer[i]);
	}

	return true;
}



