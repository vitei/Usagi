/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_GLSLSHADER
#define _USG_GRAPHICS_PC_GLSLSHADER
#include "Engine/Common/Common.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Resource/PakDecl.h"
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

namespace usg {

class PakFile;

class Shader_ps
{
public:
	Shader_ps();
	~Shader_ps();

	bool Init(const U8String& effectName, GLenum shaderType, const char* szDefines);
	bool Init(GFXDevice* pDevice, PakFile* pakFile, const PakFileDecl::FileInfo* pFileHeader, const void* pData, uint32 uDataSize);
	void CleanUp(GFXDevice* pDevice);

	GLuint GetHandle() const { return m_shader; }
	const U8String &GetName() { return m_name; }
	GLenum GetShaderType() { return m_shaderType; }
	uint32 GetDefinesCRC() { return m_uDefinesCRC; }

private:
	PRIVATIZE_COPY(Shader_ps)

	bool Init(const char** szStrings, uint32 uCount, GLenum shaderType, const char* szName = NULL);
	void HandleError(const char* szName, const char** szStrings, uint32 uArrayCount);
	void PrintErrors(const char* compiler_log, const char** szStrings, uint32 uArrayCount);

	GLuint		m_shader;
	U8String	m_name;
	GLenum		m_shaderType;
	uint32		m_uDefinesCRC;
};

}

#endif
