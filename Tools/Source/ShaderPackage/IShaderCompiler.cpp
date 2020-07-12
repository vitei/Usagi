#include "Engine/Common/Common.h"
#include "gli/gli.hpp"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "../ResourceLib/MaterialDefinition/MaterialDefinitionExporter.h"
#include "Engine/Resource/PakDecl.h"
#include "Engine/Core/Utility.h"
#include "ResourcePakExporter.h"
#include <yaml-cpp/yaml.h>
#include <sstream>
#include <algorithm>
#include <fstream>
#include <pb.h>
#include "IShaderCompiler.h"


IShaderCompiler::IShaderCompiler()
{
}


IShaderCompiler::~IShaderCompiler()
{
}


// FIXME: This code has survived since before Usagi even existed, originally designed for glsl emulation
// It could very obviously do with a re-write

enum EPragmaType
{
	PRAGMA_TYPE_NONE = 0,
	PRAGMA_TYPE_INCLUDE,
	PRAGMA_TYPE_AUTO_GEN
};

EPragmaType LocatePragma(char* szProgram, char*& szPragmaLine, char*& szNextLine)
{
	EPragmaType eType = PRAGMA_TYPE_NONE;
	char* include = strstr(szProgram, "#include");
	char* generated = strstr(szProgram, "// <<GENERATED_CODE>>");
	szPragmaLine = nullptr;
	if (include && (!generated || include < generated) )
	{
		szPragmaLine = include;
		eType = PRAGMA_TYPE_INCLUDE;
	}
	else if (generated && (!include || generated < include))
	{
		szPragmaLine = generated;
		eType = PRAGMA_TYPE_AUTO_GEN;

	}
	
	if (!szPragmaLine)
		return PRAGMA_TYPE_NONE;

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

	return eType;
}

bool InsertIncludeFile(const char* szFileName, char* szPragma, std::vector<char*>& buffer, const std::string& includes, std::vector<std::string>& referencedFiles)
{
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

	char fullPath[256];
	_fullpath(fullPath, fullIncludeName.c_str(), 256);
	referencedFiles.push_back(fullPath);

	fseek(pIncludeFile, 0, SEEK_END);
	memsize uFileSize = ftell(pIncludeFile);
	fseek(pIncludeFile, 0, SEEK_SET);

	buffer.push_back((char*)malloc(sizeof(char)*uFileSize + 1));

	size_t uRead = fread(buffer.back(), 1, uFileSize, pIncludeFile);
	buffer.back()[uRead] = '\0';
	fclose(pIncludeFile);

	return true;

}

bool ParseManually(const char* szFileName, const char* szDefines, const class MaterialDefinitionExporter* pMaterialDef, const std::string& includes,
	std::string& fileOut, std::vector<std::string>& referencedFiles, usg::ShaderType eShaderType)
{
	FILE* pShaderFile;
	fopen_s(&pShaderFile, szFileName, "rb");

	std::vector<char*> buffer;

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
	std::vector<char*> stringArray;
	// File size
	fseek(pShaderFile, 0, SEEK_END);
	memsize uFileSize = ftell(pShaderFile);
	fseek(pShaderFile, 0, SEEK_SET);

	buffer.push_back( (char*)malloc(sizeof(char)*uFileSize + 1) );

	memsize uRead = fread(buffer.back(), 1, uFileSize, pShaderFile);
	buffer.back()[uRead] = '\0';
	fclose(pShaderFile);


	stringArray.push_back(defines);
	stringArray.push_back(buffer.back());

	bool bFound = true;
	char* szPragma = NULL;
	while (bFound)
	{
		char* remainder;
		EPragmaType eType = LocatePragma(stringArray.back(), szPragma, remainder);
		bFound = eType != PRAGMA_TYPE_NONE;

		if (!bFound)
		{
			// No more pragmas
			break;
		}

		if (eType == PRAGMA_TYPE_INCLUDE)
		{
			if (InsertIncludeFile(szFileName, szPragma, buffer, includes, referencedFiles))
			{
				stringArray.push_back(buffer.back());
			}
			else
			{
				// FIXME: Mem leak
				FATAL_RELEASE(false, "Missing include file %s\n", szFileName);
				return false;
			}
		}
		else
		{
			if (pMaterialDef)
			{
				const std::string automatedCode = pMaterialDef->GetAutomatedCode(eShaderType);
				buffer.push_back((char*)malloc(automatedCode.size() + 1));
				strcpy_s(buffer.back(), automatedCode.size()+1, automatedCode.c_str());
				buffer.back()[automatedCode.size()] = '\0';
				stringArray.push_back(buffer.back());
			}
			else
			{
				ASSERT(false);
				return false;
			}
		}

		stringArray.push_back(remainder);
	}

	fileOut = "";
	for (size_t i = 0; i < stringArray.size(); i++)
	{
		fileOut += stringArray[i];
	}

	for (uint32 i = 0; i < buffer.size(); i++)
	{
		free(buffer[i]);
	}

	return true;
}
