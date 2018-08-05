/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Core/File/File.h"
#include "Engine/Graphics/Effects/Shader.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Core/String/String_Util.h"
#include API_HEADER(Engine/Graphics/Device, GLSLShader.h)
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include API_HEADER(Engine/Graphics/Effects, Effect_ps.h)
#include <stdlib.h>

namespace usg {

GLuint Effect_ps::m_activeProgram = 0;


Effect_ps::Effect_ps()
{
	m_programObject = GL_INVALID_INDEX;
}

Effect_ps::~Effect_ps()
{
	if (m_programObject != GL_INVALID_INDEX)
	{
		glDeleteProgram(m_programObject);
		m_programObject = GL_INVALID_INDEX;
	}
}

void Effect_ps::HandleError()
{
	GLint blen = 0;	
	GLsizei slen = 0;
	
	glGetProgramiv(m_programObject, GL_INFO_LOG_LENGTH , &blen);       
	if (blen > 1)
	{
		GLchar* compiler_log = (GLchar*)malloc(blen);
		glGetProgramInfoLog(m_programObject, blen, &slen, compiler_log);
		DEBUG_PRINT("GLSL Shader compile error: %s\n", compiler_log);
		free (compiler_log);
	}
	ASSERT(false);
}


// We could use some proper file parsing
void Effect_ps::GetShaderNames(const char* effectName, U8String &vsOut, U8String &psOut, U8String &gsOut, U8String &defines)
{
	U8String sourceFX = effectName;
	U8String sourcePath("shaders/");
	sourceFX += ".fx";

	File defFile(sourceFX.CStr());

	uint32 uPos = 0;
	char* szText;
	ScratchObj<char> scratchText(szText, (uint32)defFile.GetSize()+1, FILE_READ_ALIGN);

	defFile.Read(defFile.GetSize(), szText);
	szText[defFile.GetSize()] = '\0';

	while (uPos < defFile.GetSize())
	{
		if (str::CompareLen(&szText[uPos], "PS", 2))
		{
			uPos += 3;
			psOut.CopySingleLine(&szText[uPos]);
			uPos += psOut.Length();
			psOut = sourcePath + psOut;
			continue;
		}

		if (str::CompareLen(&szText[uPos], "VS", 2))
		{
			uPos += 3;
			vsOut.CopySingleLine(&szText[uPos]);
			uPos += vsOut.Length();
			vsOut = sourcePath + vsOut;
			continue;
		}

		if (str::CompareLen(&szText[uPos], "GS", 2))
		{
			uPos += 3;
			gsOut.CopySingleLine(&szText[uPos]);
			uPos += gsOut.Length();
			gsOut = sourcePath + gsOut;
			continue;
		}

		if (str::CompareLen(&szText[uPos], "DEFINES", 7))
		{
			uPos += 7;
			defines.CopySingleLine(&szText[uPos]);
			uPos += defines.Length();
		}

		uPos++;
	}
}


bool Effect_ps::Init(GFXDevice* pDevice, PakFile* pakFile, const PakFileDecl::FileInfo* pFileHeader, const void* pData, uint32 uDataSize)
{
	const PakFileDecl::EffectEntry* pEffectHdr = PakFileDecl::GetCustomHeader<PakFileDecl::EffectEntry>(pFileHeader);

	Shader* pShaders[(uint32)ShaderType::COUNT] = {};

	for (uint32 i = 0; i < (uint32)ShaderType::COUNT; i++)
	{
		if (pEffectHdr->CRC[i] != 0)
		{
			ResourceBase* pResourceBase = pakFile->GetResource(pEffectHdr->CRC[i]);
			ASSERT(pResourceBase && pResourceBase->GetResourceType() == ResourceType::SHADER);
			if (pResourceBase)
			{
				pShaders[i] = (Shader*)pResourceBase;
			}
			else
			{
				return false;
			}
		}
	}
	Init(pShaders[(int)ShaderType::VS], pShaders[(int)ShaderType::PS], pShaders[(int)ShaderType::GS]);

	return true;
}

void Effect_ps::Init(GFXDevice* pDevice, const char* szEffectName)
{
#if 0
	GLSLShader* pVertex		= NULL;
	GLSLShader* pFragment	= NULL;
	GLSLShader* pGeometry	= NULL;


	U8String vertexName;
	U8String pixelName;
	U8String geomName;
	U8String defines;
	
	GetShaderNames(szEffectName, vertexName, pixelName, geomName, defines);
	
	pVertex		= pDevice->GetPlatform().GetShaderFromStock(vertexName.CStr(), GL_VERTEX_SHADER, defines.CStr());
	pFragment	= pDevice->GetPlatform().GetShaderFromStock(pixelName.CStr(), GL_FRAGMENT_SHADER, defines.CStr());

    
	if(geomName.Length() > 0)
	{
		pGeometry = pDevice->GetPlatform().GetShaderFromStock(geomName.CStr(), GL_GEOMETRY_SHADER, defines.CStr());
		if(!pGeometry)
		{
			ASSERT(false);
			return;
		}
	}

	if(!pVertex || !pFragment)
	{
		ASSERT(false);
		return;
	}

	Init(pVertex, pFragment, pGeometry);
#endif
	ASSERT(false);
}


void Effect_ps::Init(const Shader* pVertex, const Shader* pFrag, const Shader* pGeom)
{
	m_programObject =  glCreateProgram();
	glAttachShader(m_programObject, pVertex->GetPlatform().GetHandle());
	glAttachShader(m_programObject, pFrag->GetPlatform().GetHandle());
	if(pGeom)
	{
		glAttachShader(m_programObject, pGeom->GetPlatform().GetHandle());
	}

	glLinkProgram(m_programObject);

	GLint bLinked;
	glGetProgramiv(m_programObject, GL_LINK_STATUS, &bLinked);
	if(bLinked==GL_FALSE)
	{
		HandleError();
	}

	glUseProgram(m_programObject);

}


void Effect_ps::Apply() const
{
	glUseProgram(m_programObject);
}

}

