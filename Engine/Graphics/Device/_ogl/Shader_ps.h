/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_GLSLSHADER
#define _USG_GRAPHICS_PC_GLSLSHADER
#include "Engine/Common/Common.h"
#include "Engine/Core/String/U8String.h"
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

namespace usg {

class Shader_ps
{
public:
	Shader_ps();
	~Shader_ps();

	bool Init(const U8String& effectName, GLenum shaderType, const char* szDefines);
	bool Init(char** szStrings, uint32 uCount, GLenum shaderType, const char* szName = NULL);

	GLuint GetHandle() const { return m_shader; }
	const U8String &GetName() { return m_name; }
	GLenum GetShaderType() { return m_shaderType; }
	uint32 GetDefinesCRC() { return m_uDefinesCRC; }

private:
	PRIVATIZE_COPY(GLSLShader)

	void HandleError(const char* szName, const char** szStrings, uint32 uArrayCount);
	void PrintErrors(const char* compiler_log, const char** szStrings, uint32 uArrayCount);
	bool LocatePragma(char* szProgram, char*& szPragmaLine, char*& szNextLine);

	GLuint		m_shader;
	U8String	m_name;
	GLenum		m_shaderType;
	uint32		m_uDefinesCRC;
};

}

#endif
