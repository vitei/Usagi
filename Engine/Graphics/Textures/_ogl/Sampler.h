/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_SAMPLER_H
#define _USG_GRAPHICS_PC_SAMPLER_H
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/RenderState.h"
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

namespace usg {

class Sampler
{
public:
	Sampler();
	~Sampler();

	void Init(GFXDevice* pDevice, const SamplerDecl &decl, uint32 uId);
	void Apply(uint32 uUnit, GLenum eTextureType, bool bHasMipMap) const;

private:
	GLuint	m_sampler;
	GLuint	m_samplerMip;
	GLfloat	m_fAnisoLevel;

	GLint m_wrapU;
	GLint m_wrapV;
	GLint m_minFilter;
	GLint m_magFilter;
	GLint m_minFilterMip;
	GLint m_magFilterMip;
	GLenum m_cmpFunc;
	GLenum m_cmpMode;
	GLfloat m_fLODBias;
	GLint m_sMinLOD;
	
	// TODO:
	//GLint lodMin;
	//GLint loadBias;

	//GLfloat borderColor[4];
};

}

#endif