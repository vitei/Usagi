/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include API_HEADER(Engine/Graphics/Device, DepthStencilState.h)

namespace usg {

static const VkCompareOp g_depthTestMap[] =
{
	VK_COMPARE_OP_NEVER,			// DEPTH_TEST_NEVER = 0,
	VK_COMPARE_OP_ALWAYS,			// DEPTH_TEST_ALWAYS = 1,
	VK_COMPARE_OP_EQUAL,			// DEPTH_TEST_EQUAL = 2,
	VK_COMPARE_OP_NOT_EQUAL,		// DEPTH_TEST_NOTEQUAL = 3,
	VK_COMPARE_OP_LESS,				// DEPTH_TEST_LESS = 4,
	VK_COMPARE_OP_LESS_OR_EQUAL,	// DEPTH_TEST_LEQUAL = 5,
	VK_COMPARE_OP_GREATER,			// DEPTH_TEST_GREATER = 6,
	VK_COMPARE_OP_GREATER_OR_EQUAL	// DEPTH_TEST_GEQUAL = 7
};

static const VkCompareOp g_stencilTestMap[] =
{
	VK_COMPARE_OP_NEVER,			// STENCIL_TEST_NEVER = 0,
	VK_COMPARE_OP_ALWAYS,			// STENCIL_TEST_ALWAYS = 1,
	VK_COMPARE_OP_EQUAL,			// STENCIL_TEST_EQUAL = 2,
	VK_COMPARE_OP_NOT_EQUAL,		// STENCIL_TEST_NOTEQUAL = 3,
	VK_COMPARE_OP_LESS,				// STENCIL_TEST_LESS = 4,
	VK_COMPARE_OP_LESS_OR_EQUAL,	// STENCIL_TEST_LEQUAL = 5,
	VK_COMPARE_OP_GREATER,			// STENCIL_TEST_GREATER = 6,
	VK_COMPARE_OP_GREATER_OR_EQUAL	// STENCIL_TEST_GEQUAL = 7
};

static const VkStencilOp g_stencilOpMap[] =
{
	VK_STENCIL_OP_KEEP,					// STENCIL_OP_KEEP = 0,
	VK_STENCIL_OP_ZERO,					// STENCIL_OP_ZERO = 1,
	VK_STENCIL_OP_REPLACE,				// STENCIL_OP_REPLACE = 2,
	VK_STENCIL_OP_INCREMENT_AND_CLAMP,	// STENCIL_OP_INCR = 3,
	VK_STENCIL_OP_DECREMENT_AND_CLAMP,	// STENCIL_OP_DECR = 4,
	VK_STENCIL_OP_INVERT,				// STENCIL_OP_INVERT = 5,
	VK_STENCIL_OP_INCREMENT_AND_WRAP,	// STENCIL_OP_INCR_WRAP = 6,
	VK_STENCIL_OP_DECREMENT_AND_WRAP,	// STENCIL_OP_DECR_WRAP = 7
};



void DepthStencilState::Init(GFXDevice* pDevice, const DepthStencilStateDecl &decl)
{
	memset(&m_createInfo, 0, sizeof(m_createInfo));

	m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	m_createInfo.depthTestEnable = decl.bDepthEnable ? VK_TRUE : VK_FALSE;
    m_createInfo.depthWriteEnable = decl.bDepthWrite ? VK_TRUE : VK_FALSE;
    m_createInfo.depthCompareOp = g_depthTestMap[decl.eDepthFunc];
    m_createInfo.depthBoundsTestEnable = VK_FALSE;
    m_createInfo.back.failOp = g_stencilOpMap[decl.eStencilFailOp];
    m_createInfo.back.depthFailOp = g_stencilOpMap[decl.eDepthFailOp];
    m_createInfo.back.passOp = g_stencilOpMap[decl.ePassOp];
    m_createInfo.back.compareOp = g_stencilTestMap[decl.eStencilTest];
    m_createInfo.back.writeMask = (uint32)decl.uMask[STENCIL_WRITE_MASK];
    m_createInfo.back.compareMask = (uint32)decl.uMask[STENCIL_CMP_MASK];
    m_createInfo.back.reference = (uint32)decl.uMask[STENCIL_REF];
    m_createInfo.stencilTestEnable = decl.bStencilEnable;
    // TODO: Re-implement seperate front and back stencil
    m_createInfo.front = m_createInfo.back;
}


}
