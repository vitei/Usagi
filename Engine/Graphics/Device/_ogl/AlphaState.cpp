/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include API_HEADER(Engine/Graphics/Device, AlphaState.h)

namespace usg {

const GLenum  g_blendFuncMap[] =
{
	GL_ZERO,						// BLEND_FUNC_ZERO = 0
	GL_ONE,							// BLEND_FUNC_ONE = 1
	GL_SRC_COLOR,					// BLEND_FUNC_SRC_COLOR = 2
	GL_ONE_MINUS_SRC_COLOR,			// BLEND_FUNC_ONE_MINUS_SRC_COLOR = 3
	GL_DST_COLOR,					// BLEND_FUNC_DST_COLOR = 4
	GL_ONE_MINUS_DST_COLOR,			// BLEND_FUNC_ONE_MINUS_DST_COLOR = 5
	GL_SRC_ALPHA,					// BLEND_FUNC_SRC_ALPHA = 6
	GL_ONE_MINUS_SRC_ALPHA,			// BLEND_FUNC_ONE_MINUS_SRC_ALPHA = 7
	GL_DST_ALPHA,					// BLEND_FUNC_DST_ALPHA = 8
	GL_ONE_MINUS_DST_ALPHA,			// BLEND_FUNC_ONE_MINUS_DST_ALPHA = 9
	GL_CONSTANT_COLOR,				// BLEND_FUNC_CONSTANT_COLOR = 10
	GL_ONE_MINUS_CONSTANT_COLOR,	// BLEND_FUNC_ONE_MINUS_CONSTANT_COLOR = 11
	GL_CONSTANT_ALPHA,				// BLEND_FUNC_CONSTANT_ALPHA = 12
	GL_ONE_MINUS_CONSTANT_ALPHA,	// BLEND_FUNC_ONE_MINUS_CONSTANT_ALPHA = 13
	GL_SRC_ALPHA_SATURATE,			// BLEND_FUNC_SRC_ALPHA_SATURATE = 14
};


const GLenum  g_blendEqMap[] =
{
	GL_FUNC_ADD,				// BLEND_EQUATION_ADD = 0
	GL_FUNC_SUBTRACT,			// BLEND_EQUATION_SUBTRACT = 1
	GL_FUNC_REVERSE_SUBTRACT,	// BLEND_EQUATION_REVERSE_SUBTRACT = 2
	GL_MIN,						// BLEND_EQUATION_MIN = 3
	GL_MAX						// BLEND_EQUATION_MAX = 4
};

const GLenum  g_alphaTestMap[] =
{
	GL_NEVER,	 		// ALPHA_TEST_NEVER
	GL_ALWAYS,			// ALPHA_TEST_ALWAYS
	GL_EQUAL,			// ALPHA_TEST_EQUAL,
	GL_NOTEQUAL,		// ALPHA_TEST_NOTEQUAL
	GL_LESS,			// ALPHA_TEST_LESS
	GL_LEQUAL,			// ALPHA_TEST_LEQUAL
	GL_GREATER,			// ALPHA_TEST_GREATER
	GL_GEQUAL			// ALPHA_TEST_GEQUAL
};



void AlphaState::Init(GFXDevice* pDevice, const AlphaStateDecl &decl, uint32 uId)
{
	m_bEnable		= decl.bBlendEnable;

	m_eqRgb			= g_blendEqMap[decl.blendEq];
	m_eqAlpha		= g_blendEqMap[decl.blendEqAlpha];
	m_srcRgb		= g_blendFuncMap[decl.srcBlend];
	m_srcAlpha		= g_blendFuncMap[decl.srcBlendAlpha];
	m_dstRgb		= g_blendFuncMap[decl.dstBlend];
	m_dstAlpha		= g_blendFuncMap[decl.dstBlendAlpha];

	m_eAlphaTest = g_alphaTestMap[decl.eAlphaTest];
	m_fAlphaRef = ((float)decl.uAlphaRef)/255.f;

	for(int i=0; i<MAX_COLOR_TARGETS; i++)
	{
		m_uColorMask[i]	= decl.uColorMask[i];
	}
}

void AlphaState::Apply() const
{
	if(m_bEnable)
	{
		glEnable(GL_BLEND);
		glBlendEquationSeparate(m_eqRgb, m_eqAlpha);
		glBlendFuncSeparate(m_srcRgb, m_dstRgb, m_srcAlpha, m_dstAlpha);
	}
	else
	{
		glDisable(GL_BLEND);
	}
    /*
	if( m_eAlphaTest == GL_ALWAYS ) {
		glDisable( GL_ALPHA_TEST );
	}
	else {
		glEnable( GL_ALPHA_TEST );
	}
     */

	for(int i=0; i<MAX_COLOR_TARGETS; i++)
	{
	//	glColorMaski(i, m_uColorMask[i]&RT_MASK_RED, m_uColorMask[i]&RT_MASK_GREEN, m_uColorMask[i]&RT_MASK_BLUE, m_uColorMask[i]&RT_MASK_ALPHA);
		glColorMaski(i, (m_uColorMask[i] & RT_MASK_RED) != 0, (m_uColorMask[i] & RT_MASK_GREEN) != 0, (m_uColorMask[i] & RT_MASK_BLUE) != 0, (m_uColorMask[i] & RT_MASK_ALPHA) != 0);
	}

//	glAlphaFunc(m_eAlphaTest, m_fAlphaRef);
    
    
    CHECK_OGL_ERROR();   
}

}

