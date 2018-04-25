/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include API_HEADER(Engine/Graphics/Textures, Sampler.h)

namespace usg {

struct FilterMap
{
	GLuint filter;
	GLuint filterMip;
};

static const FilterMap g_MinFilterMap[] =
{
	GL_NEAREST, GL_NEAREST,					// TF_POINT = 0,
	GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR,		// TF_LINEAR,
};

static const FilterMap g_MaxFilterMap[] =
{
	GL_NEAREST, GL_NEAREST,			// TF_POINT = 0,
	GL_LINEAR, GL_LINEAR,			// TF_LINEAR,
};

static const GLuint g_textureWrapMap[] =
{
	GL_REPEAT,  		// SC_WRAP = 0,
	GL_MIRRORED_REPEAT, // SC_MIRROR = 1,
	GL_CLAMP_TO_EDGE 	// SC_CLAMP = 2,
};


const GLenum  g_cmpFuncMap[] =
{
	GL_NEVER,	 		// CF_TEST_NEVER
	GL_ALWAYS,			// CF_TEST_ALWAYS
	GL_EQUAL,			// CF_TEST_EQUAL,
	GL_NOTEQUAL,		// CF_TEST_NOTEQUAL
	GL_LESS,			// CF_TEST_LESS
	GL_LEQUAL,			// CF_TEST_LEQUAL
	GL_GREATER,			// CF_TEST_GREATER
	GL_GEQUAL			// CF_TEST_GEQUAL
};


static GLfloat g_anisoLevel[] =
{
 	1.f,
    2.f,
    4.f,
    8.f,
    16.f
};


Sampler::Sampler()
{

}

Sampler::~Sampler()
{

}

void Sampler::Init(GFXDevice* pDevice, const SamplerDecl &decl, uint32 uId)
{
	// TODO: We need a share set of state mappings
	m_wrapU		= g_textureWrapMap[decl.eClampU];
	m_wrapV		= g_textureWrapMap[decl.eClampV];
	m_minFilter	= g_MinFilterMap[decl.eFilterMin].filter;
	m_magFilter	= g_MaxFilterMap[decl.eFilterMag].filter;
	m_minFilterMip	= g_MinFilterMap[decl.eFilterMin].filterMip;
	m_magFilterMip	= g_MaxFilterMap[decl.eFilterMag].filterMip;
	m_cmpFunc = decl.bEnableCmp ? g_cmpFuncMap[decl.eCmpFnc] : GL_ALWAYS;
	m_cmpMode = decl.bEnableCmp ? GL_COMPARE_R_TO_TEXTURE : GL_NONE;
	m_fAnisoLevel = g_anisoLevel[decl.eAnisoLevel];
	m_fLODBias = decl.LodBias;
	m_sMinLOD = decl.LodMinLevel;


	/*glGenSamplers(1, &m_sampler);
	glGenSamplers(1, &m_samplerMip);

	glSamplerParameteri(m_sampler , GL_TEXTURE_WRAP_S, m_wrapU);  
	glSamplerParameteri(m_sampler , GL_TEXTURE_WRAP_T, m_wrapV);  
	glSamplerParameteri(m_sampler , GL_TEXTURE_MIN_FILTER , m_minFilter);  
	glSamplerParameteri(m_sampler , GL_TEXTURE_MAG_FILTER , m_magFilter);  

	glSamplerParameteri(m_samplerMip , GL_TEXTURE_WRAP_S, m_wrapU);  
	glSamplerParameteri(m_samplerMip , GL_TEXTURE_WRAP_T, m_wrapV);  
	glSamplerParameteri(m_samplerMip , GL_TEXTURE_MIN_FILTER, m_minFilterMip);  
	glSamplerParameteri(m_samplerMip , GL_TEXTURE_MAG_FILTER, m_magFilterMip);  

	glSamplerParameterf(m_samplerMip, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);*/

	/*m_states.lodMin		= 0;

	m_states.borderColor[0] = 0.0f;
	m_states.borderColor[1] = 0.0f;
	m_states.borderColor[2] = 0.0f;
	m_states.borderColor[3] = 0.0f;*/
}


void Sampler::Apply(uint32 uUnit, GLenum eTextureType, bool bHasMipMap) const
{
	/*if(bHasMipMap)
	{
		glBindSampler(uUnit, m_samplerMip);  
	}
	else
	{
		glBindSampler(uUnit, m_sampler);  
	}*/
	if(bHasMipMap)
	{
		glTexParameterf(eTextureType, GL_TEXTURE_MAX_ANISOTROPY_EXT, m_fAnisoLevel );
	}

	glTexParameterf(eTextureType, GL_TEXTURE_LOD_BIAS, m_fLODBias);
	glTexParameteri(eTextureType, GL_TEXTURE_MIN_LOD, m_sMinLOD);

	glTexParameteri(eTextureType, GL_TEXTURE_WRAP_S, m_wrapU);
	glTexParameteri(eTextureType, GL_TEXTURE_WRAP_T, m_wrapV);

	if(bHasMipMap)
	{
		glTexParameteri(eTextureType, GL_TEXTURE_MIN_FILTER, m_minFilterMip);
		glTexParameteri(eTextureType, GL_TEXTURE_MAG_FILTER, m_magFilterMip);
	}
	else
	{
		glTexParameteri(eTextureType, GL_TEXTURE_MIN_FILTER, m_minFilter);
		glTexParameteri(eTextureType, GL_TEXTURE_MAG_FILTER, m_magFilter);
	}

	glTexParameteri(eTextureType, GL_TEXTURE_COMPARE_MODE, m_cmpMode);
	glTexParameteri(eTextureType, GL_TEXTURE_COMPARE_FUNC, m_cmpFunc);
}

}

