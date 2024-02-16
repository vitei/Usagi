/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Graphics/Device/PipelineState.h"
#include "RenderGroup.h"
#include "RenderNode.h"

namespace usg {

const uint32 uMaxEffectValues = 0xFFF;
const uint32 uMaxTextureValues = 0xFF;


const uint64 COMPARISON_MASK_PRIORITY	= (0x0FF0000000000000);
const uint64 COMPARISON_MASK_EFFECT		= (0x000000FFFFFFFF00);
const uint64 COMPARISON_MASK_TEXTURE	= (0x00000000000000FF);


RenderNode::RenderNode()
:m_eLayer(LAYER_OPAQUE),
m_uPriority(0),
m_uRenderMask(RENDER_MASK_ALL),
m_bHasShadow(false),
m_bAnimated(false)
{
	m_pParent = NULL;
	m_bPostEffect = false;
	m_uComparisonVal = 0;
}

RenderNode::~RenderNode()
{

}

void RenderNode::SetParent(RenderGroup* pParent)
{
	m_pParent = pParent;
}


void RenderNode::SetLayer(RenderLayer eLayer)
{
	ASSERT(eLayer < LAYER_COUNT); 
	m_eLayer = eLayer; 
	if (m_pParent)
	{
		m_pParent->MarkRenderPassDirty();
	}
}

void RenderNode::SetMaterialCmpVal(const EffectHndl& hndl, const Texture* pTexture0)
{
	uint64 uMask = (COMPARISON_SHIFT_TEXTURE | COMPARISON_MASK_EFFECT);
	m_uComparisonVal &= ~uMask;

	uint64 uEffectValue = (uint64)hndl.get();
	uEffectValue = (uEffectValue << COMPARISON_SHIFT_EFFECT) & COMPARISON_MASK_EFFECT;

	uint64 uTextureValue = pTexture0 ? (pTexture0->GetId() % uMaxTextureValues) : 0;
	uTextureValue = (uTextureValue << COMPARISON_SHIFT_TEXTURE) & COMPARISON_MASK_TEXTURE;

	m_uComparisonVal |= ((uTextureValue | uEffectValue) & (COMPARISON_MASK_TEXTURE | COMPARISON_MASK_EFFECT));
}

void RenderNode::SetMaterialCmpVal(const PipelineStateHndl& hndl , const Texture* pTexture0)
{
	SetMaterialCmpVal(hndl.GetContents()->GetEffect(), pTexture0);
}

void RenderNode::SetPriorityCmpValue()
{
	m_uComparisonVal &= ~COMPARISON_MASK_PRIORITY;
	uint64 uValue = (((uint64)m_uPriority) << COMPARISON_SHIFT_PRIORITY) & COMPARISON_MASK_PRIORITY;
	m_uComparisonVal |= (uValue&COMPARISON_MASK_PRIORITY);
}



void RenderNode::SetRenderMask(uint32 uMask)
{
	uMask = (m_uRenderMask&RENDER_MASK_SHADOW_CAST) | (uMask&~RENDER_MASK_SHADOW_CAST);
	SetRenderMaskIncShadow(uMask);
}

void RenderNode::SetRenderMaskIncShadow(uint32 uMask)
{
	m_uRenderMask = uMask;
	m_bHasShadow = uMask & RENDER_MASK_SHADOW_CAST;
	if (m_pParent)
	{
		m_pParent->UpdateMask();
	}
}

void RenderNode::SetHasShadow(bool bShadow)
{
	m_bHasShadow = bShadow;
	if(bShadow)
	{
		SetRenderMask(m_uRenderMask|RENDER_MASK_SHADOW_CAST);
	}
	else
	{
		SetRenderMask(m_uRenderMask&~RENDER_MASK_SHADOW_CAST);
	}
}

}

