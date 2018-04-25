/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Device, DepthStencilState.h)

namespace usg {

static const GLenum g_depthTestMap[] =
{
	GL_NEVER,		// DEPTH_TEST_NEVER = 0,
	GL_ALWAYS,		// DEPTH_TEST_ALWAYS = 1,
	GL_EQUAL,		// DEPTH_TEST_EQUAL = 2,
	GL_NOTEQUAL,	// DEPTH_TEST_NOTEQUAL = 3,
	GL_LESS,		// DEPTH_TEST_LESS = 4,
	GL_LEQUAL,		// DEPTH_TEST_LEQUAL = 5,
	GL_GREATER,		// DEPTH_TEST_GREATER = 6,
	GL_GEQUAL		// DEPTH_TEST_GEQUAL = 7
};



static const GLenum g_stencilTestMap[] =
{
	GL_NEVER,		// STENCIL_TEST_NEVER = 0,
	GL_ALWAYS,		// STENCIL_TEST_ALWAYS = 1,
	GL_EQUAL,		// STENCIL_TEST_EQUAL = 2,
	GL_NOTEQUAL,	// STENCIL_TEST_NOTEQUAL = 3,
	GL_LESS,		// STENCIL_TEST_LESS = 4,
	GL_LEQUAL,		// STENCIL_TEST_LEQUAL = 5,
	GL_GREATER,		// STENCIL_TEST_GREATER = 6,
	GL_GEQUAL		// STENCIL_TEST_GEQUAL = 7
};

static const GLenum g_stencilOpMap[] =
{
	GL_KEEP,		// STENCIL_OP_KEEP = 0,
	GL_ZERO,		// STENCIL_OP_ZERO = 1,
	GL_REPLACE,		// STENCIL_OP_REPLACE = 2,
	GL_INCR,		// STENCIL_OP_INCR = 3,
	GL_DECR,		// STENCIL_OP_DECR = 4,
	GL_INVERT,		// STENCIL_OP_INVERT = 5,
	GL_INCR_WRAP,	// STENCIL_OP_INCR_WRAP = 6,
	GL_DECR_WRAP,	// STENCIL_OP_DECR_WRAP = 7
};



void DepthStencilState::Init(GFXDevice* pDevice, const DepthStencilStateDecl &decl, uint32 uId)
{
	m_depthFunc			= g_depthTestMap[decl.eDepthFunc];
	m_depthEnable		= decl.bDepthEnable;
	m_depthEnableWrite	= decl.bDepthWrite;

	m_stencilEnable		= decl.bStencilEnable;
	m_stencilFunc		= g_stencilTestMap[decl.eStencilTest];
	for (uint32 i = 0; i < SOP_TYPE_COUNT; i++)
	{
		m_stencilOps[i] = g_stencilOpMap[decl.eStencilOps[i]];
	}

	for(int i=0; i<STENCIL_REF_TYPE; i++)
	{
		m_uMask[i] = decl.uMask[i];
	}

//	ASSERT(!m_stencilEnable);	// Not got the stencil values sorted yet
}

void DepthStencilState::Apply() const
{
	glDepthFunc(m_depthFunc);
	if(m_depthEnable)
	{
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}
	glDepthMask(m_depthEnableWrite);

	if(m_stencilEnable)
	{
		glEnable(GL_STENCIL_TEST);

		glStencilFuncSeparate(GL_FRONT, m_stencilFunc, m_uMask[STENCIL_REF], m_uMask[STENCIL_CMP_MASK]);
		glStencilFuncSeparate(GL_BACK,	m_stencilFunc, m_uMask[STENCIL_REF_BACK], m_uMask[STENCIL_CMP_MASK_BACK]);

		glStencilMaskSeparate(GL_FRONT, m_uMask[STENCIL_WRITE_MASK]);
		glStencilMaskSeparate(GL_BACK,	m_uMask[STENCIL_WRITE_MASK_BACK]);

		glStencilOpSeparate(GL_FRONT,	m_stencilOps[SOP_TYPE_STENCIL_FAIL], m_stencilOps[SOP_TYPE_DEPTH_FAIL], m_stencilOps[SOP_TYPE_PASS]);
		glStencilOpSeparate(GL_BACK, m_stencilOps[SOP_TYPE_STENCIL_FAIL_BACK], m_stencilOps[SOP_TYPE_DEPTH_FAIL_BACK], m_stencilOps[SOP_TYPE_PASS_BACK]);

		//glStencilFunc(m_stencilFunc, m_uMask[STENCIL_REF], m_uMask[STENCIL_CMP_MASK]);

		//glStencilMask(m_uMask[STENCIL_WRITE_MASK]);

		//glStencilOp(m_stencilOpFail, m_stencilOpZFail, m_stencilOpZPass);

	}
	else
	{
		glDisable(GL_STENCIL_TEST);
	}
}

}
