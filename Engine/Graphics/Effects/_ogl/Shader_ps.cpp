/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Core/Utility.h"
#include "Engine/Resource/PakFile.h"
#include API_HEADER(Engine/Graphics/Effects, Shader_ps.h)
#include <stdlib.h>
#include <string.h>

GLenum g_shaderTypes[] = 
{
	GL_VERTEX_SHADER, // ShaderType::VS
	GL_FRAGMENT_SHADER, // ShaderType::PS
	GL_GEOMETRY_SHADER  // ShaderType::GS
};

namespace usg {

const int MAX_STRING_ARRAY = 30;


Shader_ps::Shader_ps()
{
	m_shader = GL_INVALID_INDEX;
}

Shader_ps::~Shader_ps()
{

}

void Shader_ps::CleanUp(GFXDevice* pDevice)
{
	if (m_shader != GL_INVALID_INDEX)
	{
		glDeleteShader(m_shader);
		m_shader = GL_INVALID_INDEX;
	}	
}

void Shader_ps::PrintErrors(const char* compiler_log, const char** szStrings, uint32 uArrayCount)
{
	// Horribly nasty code for my convenience. Ok as the game will fall over anyway if we reached here
	const char* log = compiler_log;
	
	log = str::Find(log, "(") + 1;
	U8String numberString;
	uint32 i = 0;
	while (log[i] != ')' && log[i] != '\0')
	{
		i++;
	}

	numberString.CopyLength(log, i);
	uint32 uLine = atoi(numberString.CStr());
	uint32 uReadLine = 0;
	const char* szLine = nullptr;
	for (uint32 i = 0; i < uArrayCount; i++)
	{
		const char* szString = szStrings[i];
		while (*szString != '\0')
		{
			if ( (*szString) == '\n')
			{
				uReadLine++;
				if (uReadLine == uLine)
				{
					szLine = (szString + 1);
					break;
				}
			}
			szString++;
		}
	}
		
	if(szLine)
	{
		U8String tmp;
		tmp.CopySingleLine(szLine);
		tmp.ParseString("Line %d: %s\n", uLine, tmp.CStr());
		DEBUG_PRINT("%s", tmp.CStr());
	}
}

void Shader_ps::HandleError(const char* szName, const char** szStrings, uint32 uArrayCount)
{
	GLint blen = 0;	
	GLsizei slen = 0;

	glGetShaderiv(m_shader, GL_INFO_LOG_LENGTH , &blen);       
	if (blen > 1)
	{
		GLchar* compiler_log = (GLchar*)malloc(blen);
		glGetShaderInfoLog(m_shader, blen, &slen, compiler_log);
		if(szName)
		{
			DEBUG_PRINT("\nGLSL error in %s: %s", szName, compiler_log);
		}
		else
		{
			DEBUG_PRINT("\nGLSL error %s", compiler_log);
		}
#ifdef DEBUG_BUILD
		// Note this may fall over on different vendor hardware
		PrintErrors(compiler_log, szStrings, uArrayCount);
#endif
		free (compiler_log);
	}
	ASSERT(false);
}


bool Shader_ps::Init(const char** szStrings, uint32 uCount, GLenum shaderType, const char* szName)
{
	
   // for(int i=0; i<uCount; i++)
     //   printf("Line %d: %s\n",i+1,szStrings[i]);
	

	
    
    // Now the platform independent code beings
	m_shader = glCreateShader(shaderType);
	glShaderSource(m_shader, uCount, (const GLchar**)szStrings, NULL);
	glCompileShader(m_shader);

	GLint iStatus;
	glGetShaderiv(m_shader, GL_COMPILE_STATUS, &iStatus);
	if(iStatus==GL_FALSE)
	{
		HandleError(szName, (const char**)szStrings, uCount);
		return false;
	}

	return true;
}



bool Shader_ps::Init(GFXDevice* pDevice, PakFile* pakFile, const PakFileDecl::FileInfo* pFileHeader, const void* pData, uint32 uDataSize)
{
	const PakFileDecl::ShaderEntry* pShaderHdr = PakFileDecl::GetCustomHeader<PakFileDecl::ShaderEntry>(pFileHeader);

	m_shaderType = g_shaderTypes[(uint32)pShaderHdr->eShaderType];
	const char* shaderCode = (const char*)pData;
	return Init(&shaderCode, 1, m_shaderType, pFileHeader->szName);
}

bool Shader_ps::Init(const U8String& effectName, GLenum shaderType, const char* szDefines)
{    
	ASSERT(false);

	return false;
}

}