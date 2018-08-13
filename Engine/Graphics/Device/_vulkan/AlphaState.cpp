/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include API_HEADER(Engine/Graphics/Device, AlphaState.h)

namespace usg {

const VkBlendFactor  g_blendFactorMap[] =
{
	VK_BLEND_FACTOR_ZERO,						// BLEND_FUNC_ZERO = 0
	VK_BLEND_FACTOR_ONE,						// BLEND_FUNC_ONE = 1
	VK_BLEND_FACTOR_SRC_COLOR,					// BLEND_FUNC_SRC_COLOR = 2
	VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,		// BLEND_FUNC_ONE_MINUS_SRC_COLOR = 3
	VK_BLEND_FACTOR_DST_COLOR,					// BLEND_FUNC_DST_COLOR = 4
	VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,		// BLEND_FUNC_ONE_MINUS_DST_COLOR = 5
	VK_BLEND_FACTOR_SRC_ALPHA,					// BLEND_FUNC_SRC_ALPHA = 6
	VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,		// BLEND_FUNC_ONE_MINUS_SRC_ALPHA = 7
	VK_BLEND_FACTOR_DST_ALPHA,					// BLEND_FUNC_DST_ALPHA = 8
	VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,		// BLEND_FUNC_ONE_MINUS_DST_ALPHA = 9
	VK_BLEND_FACTOR_CONSTANT_COLOR,				// BLEND_FUNC_CONSTANT_COLOR = 10
	VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,	// BLEND_FUNC_ONE_MINUS_CONSTANT_COLOR = 11
	VK_BLEND_FACTOR_CONSTANT_ALPHA,				// BLEND_FUNC_CONSTANT_ALPHA = 12
	VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA,	// BLEND_FUNC_ONE_MINUS_CONSTANT_ALPHA = 13
	VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,			// BLEND_FUNC_SRC_ALPHA_SATURATE = 14
};


const VkBlendOp  g_blendEqMap[] =
{
	VK_BLEND_OP_ADD,				// BLEND_EQUATION_ADD = 0
	VK_BLEND_OP_SUBTRACT,			// BLEND_EQUATION_SUBTRACT = 1
	VK_BLEND_OP_REVERSE_SUBTRACT,	// BLEND_EQUATION_REVERSE_SUBTRACT = 2
	VK_BLEND_OP_MIN,				// BLEND_EQUATION_MIN = 3
	VK_BLEND_OP_MAX					// BLEND_EQUATION_MAX = 4
};


static VkColorComponentFlags CalculateColorMask(uint8 uMask)
{
	VkColorComponentFlags flags = 0;

	flags |= (uMask & RT_MASK_RED)!= 0 ? VK_COLOR_COMPONENT_R_BIT : 0;
	flags |= (uMask & RT_MASK_GREEN)!= 0 ? VK_COLOR_COMPONENT_G_BIT : 0;
	flags |= (uMask & RT_MASK_BLUE)!= 0 ? VK_COLOR_COMPONENT_B_BIT : 0;
	flags |= (uMask & RT_MASK_ALPHA)!= 0 ? VK_COLOR_COMPONENT_A_BIT : 0;

	return flags;
}


void AlphaState::Init(GFXDevice* pDevice, const AlphaStateDecl &decl, uint32 uId)
{
	memset(&m_createInfo, 0, sizeof(m_createInfo));
	memset(m_attState, 0, sizeof(VkPipelineColorBlendAttachmentState)*MAX_COLOR_TARGETS);

	// TODO: Re-implement per target blending
	for(int i=0; i<MAX_COLOR_TARGETS; i++)
	{
		m_attState[i].blendEnable = decl.bBlendEnable;

		m_attState[i].colorBlendOp = g_blendEqMap[decl.blendEq];
		m_attState[i].alphaBlendOp = g_blendEqMap[decl.blendEqAlpha];
		m_attState[i].srcColorBlendFactor = g_blendFactorMap[decl.srcBlend];
		m_attState[i].dstColorBlendFactor = g_blendFactorMap[decl.dstBlend];
		m_attState[i].srcAlphaBlendFactor = g_blendFactorMap[decl.srcBlendAlpha];
		m_attState[i].dstAlphaBlendFactor = g_blendFactorMap[decl.dstBlendAlpha];

		m_attState[i].colorWriteMask	= CalculateColorMask(decl.uColorMask[i]);
	}

	m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	m_createInfo.attachmentCount = decl.uColorTargets;
	m_createInfo.pAttachments = m_attState;
}


}

