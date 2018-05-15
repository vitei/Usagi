/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Core/Utility.h"
#include API_HEADER(Engine/Graphics/Device, GLSLShader.h)
#include <stdlib.h>
#include <string.h>

namespace usg {

const int MAX_STRING_ARRAY = 30;


GLSLShader::GLSLShader()
{
	m_shader = GL_INVALID_INDEX;
}

GLSLShader::~GLSLShader()
{
	if (m_shader != GL_INVALID_INDEX)
	{
		glDeleteShader(m_shader);
		m_shader = GL_INVALID_INDEX;
	}
}

void GLSLShader::PrintErrors(const char* compiler_log, const char** szStrings, uint32 uArrayCount)
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

void GLSLShader::HandleError(const char* szName, const char** szStrings, uint32 uArrayCount)
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


bool GLSLShader::Init(char** szStrings, uint32 uCount, GLenum shaderType, const char* szName)
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

bool GLSLShader::LocatePragma(char* szProgram, char*& szPragmaLine, char*& szNextLine)
{
	szPragmaLine = strstr(szProgram, "#include");
	if(!szPragmaLine)
	{
		return false;
	}

	// Terminate the previous string
	szPragmaLine[0] = '\0';
	szPragmaLine++;

	szNextLine = szPragmaLine;
	while(*szNextLine!='\n' && *szNextLine!='\0')
	{
		szNextLine++;
	}
	
	ASSERT(*szNextLine == '\n');
	*szNextLine = '\0';	// Terminate the pragma line
	szNextLine++;

	return true;
}

bool GLSLShader::Init(const U8String& effectName, GLenum shaderType, const char* szDefines)
{    
	if (szDefines)
	{
		m_uDefinesCRC = utl::CRC32(szDefines);
	}
	// We can't pre-compile these (on PC anyway) as the vendor is responsible for compiling the raw strings
	U8String ext;
	switch(shaderType)
	{
		case GL_VERTEX_SHADER:
			ext = ".vert";
			break;
		case GL_FRAGMENT_SHADER:
			ext = ".frag";
			break;
		case GL_GEOMETRY_SHADER:
			ext = ".geom";
			break;
		default:
			 ASSERT(false);
	}
	U8String filePath = effectName + ext;
	File shaderFile(filePath.CStr(), FILE_ACCESS_READ);
	
	GLchar* buffer[MAX_STRING_ARRAY];
	uint32 uBufferCount = 0;

	if(!shaderFile.IsOpen())
		return false;

	char defines[256];
	char* szDefineLoc = defines;
	strcpy_s(szDefineLoc, 256, "#version 430\n");
	szDefineLoc += 13;
	if (szDefines)
	{		
		while (*szDefines != '\0')
		{
			strcpy_s(szDefineLoc, &defines[256] - szDefineLoc, "#define ");
			szDefineLoc += 8;
			do
			{
				*szDefineLoc++ = *szDefines;
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
  	memsize uFileSize = shaderFile.GetSize();

  	// allocate memory to contain the whole file:
  	// TODO: Ugly, but as we don't have new/delete available....
  	// FIXME: Is one of the heaps a scratch heap?
  	buffer[0] = (char*) malloc (sizeof(char)*uFileSize+1);
	uBufferCount++;
  	ASSERT(buffer[0] != NULL);

	if(!buffer[0])
		return false;

	memsize uRead = shaderFile.Read(uFileSize, buffer[0]);
	buffer[0][uRead] = '\0';
	shaderFile.Close();

    
	stringArray[0] = defines;
#ifdef PLATFORM_OSX
	stringArray[1] = "#define PLATFORM_OSX 1\n";
#else
	stringArray[1] = "#define PLATFORM_PC 1\n";
#endif
	stringArray[2] = buffer[0];
	
	uint32 uArrayIndex = 2;
	bool bFound = true;
	char* szPragma = NULL;
	while(bFound)
	{
		bFound = LocatePragma(stringArray[uArrayIndex], szPragma, stringArray[uArrayIndex+1]);

		if(!bFound)
		{
			// No more pragmas
			break;
		}
			
		while( *szPragma != '\"' )
			szPragma++;

		szPragma++;

		char* search = szPragma;
		while( *search != '\0' && *search!='\"' )
			search++;

		ASSERT(*search == '\"');
		*search = '\0';
		
		File includeFile;
		U8String includeName = filePath;
		includeName.TruncateToPath();
		includeName += U8String(szPragma);
		includeFile.Open( includeName.CStr(), FILE_ACCESS_READ );
		ASSERT( includeFile.IsOpen() );
		uFileSize = includeFile.GetSize();

		buffer[uBufferCount] = (char*) malloc (sizeof(char)*uFileSize+1);
		
		uRead = includeFile.Read(uFileSize, buffer[uBufferCount]);
		buffer[uBufferCount][uRead] = '\0';
		includeFile.Close();

		stringArray[uArrayIndex+2] = stringArray[uArrayIndex+1];
		stringArray[uArrayIndex+1] = buffer[uBufferCount];

		uBufferCount++;
		uArrayIndex+= 2;
	}



	Init(stringArray, uArrayIndex+1, shaderType, effectName.CStr());

	for(uint32 i=0; i<uBufferCount; i++)
	{
		free(buffer[i]);
	}

	m_name = effectName;
	m_shaderType = shaderType;

	return true;
}

}