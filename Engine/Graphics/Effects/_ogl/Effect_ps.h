/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_EFFECT_PS_
#define _USG_GRAPHICS_PC_EFFECT_PS_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Primitives/VertexDeclaration.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

namespace usg {

class GLSLShader;
class Texture;
class U8String;
class GFXDevice;
typedef struct _EffectPak EffectPak;

class Effect_ps
{
public:
	Effect_ps();
	~Effect_ps();

	void Init(GFXDevice* pDevice, const char* szEffectName);
	bool Init(GFXDevice* pDevice, const EffectPak& pak, const void* pData, const char* szPackPath) { ASSERT(false); return false; }	// Not yet implemented on PC
	void CleanUp(GFXDevice* pDevice) {}
	GLuint GetProgram() const { return m_programObject; }

	void Apply() const;

private:
	PRIVATIZE_COPY(Effect_ps)
	
	void Init(const GLSLShader* pVertex, const GLSLShader* pFrag, const GLSLShader* pGeom);
	void HandleError();
	void GetShaderNames(const char* effectName, U8String &vsOut, U8String &psOut, U8String &gsOut, U8String &defines);

	enum
	{
		MAX_TEXTURES = 8
	};

	GLuint				m_programObject;
	static GLuint		m_activeProgram;

};

}

#endif