/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include API_HEADER(Engine/Graphics/Device, AlphaState.h)
#include API_HEADER(Engine/Graphics/Device, RasterizerState.h)
#include API_HEADER(Engine/Graphics/Device, DepthStencilState.h)
#include API_HEADER(Engine/Graphics/Textures, Sampler.h)
#include "Engine/Graphics/Device/GFXDevice.h"
#include "RenderState.h"
#include "RenderStateMgr.h"

namespace usg {


AlphaStateDecl::AlphaStateDecl()
{
	bBlendEnable	= false;
	uColorTargets = MAX_COLOR_TARGETS;
	for(int i=0; i<MAX_COLOR_TARGETS; i++)
	{
		uColorMask[i]	= RT_MASK_ALL;
	}

	srcBlend		= BLEND_FUNC_ONE;
	dstBlend		= BLEND_FUNC_ZERO;
	blendEq			= BLEND_EQUATION_ADD;
	srcBlendAlpha	= BLEND_FUNC_ONE;
	dstBlendAlpha	= BLEND_FUNC_ZERO;
	blendEqAlpha	= BLEND_EQUATION_ADD;
	eAlphaTest 		= ALPHA_TEST_ALWAYS;
	uAlphaRef		= 255;
}

AlphaStateDecl::~AlphaStateDecl()
{

}

void AlphaStateDecl::InitFromDefinition(const AlphaStateGroup &def)
{
	bBlendEnable = !(def.rgbSrcFunc == BLEND_FUNC_ONE &&
					def.rgbDestFunc == BLEND_FUNC_ZERO &&
					def.alphaSrcFunc == BLEND_FUNC_ONE  &&
					def.alphaDestFunc == BLEND_FUNC_ZERO &&
					def.alphaOp == BLEND_EQUATION_ADD &&
					def.rgbOp == BLEND_EQUATION_ADD );

	for(int i=0; i<MAX_COLOR_TARGETS; i++)
	{
		uColorMask[i]	= RT_MASK_ALL;
	}

	srcBlend		= def.rgbSrcFunc;
	dstBlend		= def.rgbDestFunc;
	blendEq			= def.rgbOp;
	srcBlendAlpha	= def.alphaSrcFunc;
	dstBlendAlpha	= def.alphaDestFunc;
	blendEqAlpha	= def.alphaOp;
	eAlphaTest 		= def.alphaTestFunc;
	uAlphaRef		= (uint8)(def.alphaTestReference*255.f);

}

void AlphaStateDecl::SetColor0Only()
{
	uColorTargets = 1;
	uColorMask[0]	= RT_MASK_ALL;

	for(int i=1; i<MAX_COLOR_TARGETS; i++)
	{
		uColorMask[i]	= RT_MASK_NONE;
	}
}


void AlphaStateDecl::EnableMultipleTargets(uint32 uCount)
{
	uColorTargets = uCount;

	for (uint32 i = 0; i < uCount; i++)
	{
		uColorMask[i] = RT_MASK_ALL;
	}

	for (int i = uCount; i < MAX_COLOR_TARGETS; i++)
	{
		uColorMask[i] = RT_MASK_NONE;
	}
}

void AlphaStateDecl::SetDepthOnly()
{
	for(int i=0; i<MAX_COLOR_TARGETS; i++)
	{
		uColorMask[i]	= RT_MASK_NONE;
	}
}


bool AlphaStateDecl::UsesBlendColor() const
{
	bool bUsesConstantAlpha = srcBlend >= BLEND_FUNC_CONSTANT_COLOR && srcBlend <= BLEND_FUNC_ONE_MINUS_CONSTANT_ALPHA;
	bUsesConstantAlpha |= dstBlend >= BLEND_FUNC_CONSTANT_COLOR && dstBlend <= BLEND_FUNC_ONE_MINUS_CONSTANT_ALPHA;
	bUsesConstantAlpha |= srcBlendAlpha >= BLEND_FUNC_CONSTANT_COLOR && srcBlendAlpha <= BLEND_FUNC_ONE_MINUS_CONSTANT_ALPHA;
	bUsesConstantAlpha |= dstBlendAlpha >= BLEND_FUNC_CONSTANT_COLOR && dstBlendAlpha <= BLEND_FUNC_ONE_MINUS_CONSTANT_ALPHA;

	return bUsesConstantAlpha;
}

bool AlphaStateDecl::operator==(const AlphaStateDecl& rhs) const
{
	// TODO: Some states are only relevant dependent on other settings
	// Return true if irrelevant settings are enabled
	if(rhs.bBlendEnable != bBlendEnable)
		return false;

	if (rhs.uColorTargets != uColorTargets)
		return false;

	for(uint32 i=0; i<uColorTargets; i++)
	{
		if(uColorMask[i] != rhs.uColorMask[i])
			return false;
	}

	if(bBlendEnable)
	{
		if( srcBlend != rhs.srcBlend
			|| dstBlend != rhs.dstBlend
			|| blendEq != rhs.blendEq
			|| srcBlendAlpha != rhs.srcBlendAlpha
			|| dstBlendAlpha != rhs.dstBlendAlpha
			|| blendEqAlpha != rhs.blendEqAlpha )
		{
			return false;
		}
	}

	if(eAlphaTest != rhs.eAlphaTest)
		return false;

	if(eAlphaTest != ALPHA_TEST_ALWAYS && eAlphaTest != ALPHA_TEST_NEVER)
	{
		if( uAlphaRef != rhs.uAlphaRef )
			return false;
	}

	return true;
}


RasterizerStateDecl::RasterizerStateDecl()
{
	eCullFace = CULL_FACE_BACK;
	fDepthBias	= 0.0f;
	bUseDepthBias = false;
	bMultisample = false;
	bWireframe = false;
}

RasterizerStateDecl::~RasterizerStateDecl()
{

}

bool RasterizerStateDecl::operator==(const RasterizerStateDecl& rhs) const
{
	if(rhs.bUseDepthBias != bUseDepthBias)
		return false;

	if(bUseDepthBias)
	{
		if( !Math::IsEqual(fDepthBias, rhs.fDepthBias) )
		{
			return false;
		}
	}

	return ( eCullFace == rhs.eCullFace
		&& bMultisample == rhs.bMultisample
		&& bWireframe == rhs.bWireframe );
}

RenderPassDecl::Attachment::Attachment()
{
	eAttachType = ATTACH_COLOR;
	format.eColor = CF_RGBA_8888;
	uAttachFlags = 0;
	eLoadOp = LOAD_OP_DONT_CARE;
	eStoreOp = STORE_OP_STORE;
	eSamples = SAMPLE_COUNT_1_BIT;
	eInitialLayout = LAYOUT_UNDEFINED;
	eFinalLayout = LAYOUT_COLOR_ATTACHMENT;
}

RenderPassDecl::RenderPassDecl()
{
	pAttachments = nullptr;
	uAttachments = 0;
	pSubPasses = nullptr;
	uSubPasses = 0;
	pDependencies = nullptr;
	uDependencies = 0;
}

RenderPassDecl::~RenderPassDecl()
{

}

RenderPassDecl::SubPass::SubPass()
{
	pInputAttachments = nullptr;
	pColorAttachments = nullptr;
	pResolveAttachments = nullptr;
	puPreserveIndices = nullptr;
	pDepthAttachment = nullptr;
	
	uInputCount = 0;
	uColorCount = 0;
	uPreserveCount = 0;
}


RenderPassDecl::Dependency::Dependency()
{
	uSrcSubPass = 0;
	uDstSubPass = 0;
	uSrcStageFlags = 0;
	uDstStageFlags = 0;
	uSrcAccessFlags = 0;
	uDstAccessFlags = 0;
}


bool RenderPassDecl::operator==(const RenderPassDecl& rhs) const
{
	// FIXME: Optimise this with a CRC - this will serve us for now
	if (rhs.uAttachments != uAttachments
		|| rhs.uSubPasses != uSubPasses
		|| rhs.uDependencies != uDependencies)
	{
		return false;
	}

	for (uint32 i = 0; i < uAttachments; i++)
	{
		if (memcmp(&rhs.pAttachments[i], &pAttachments[i], sizeof(RenderPassDecl::Attachment)) != 0)
		{
			return false;
		}
	}

	for (uint32 i = 0; i < uDependencies; i++)
	{
		if (memcmp(&rhs.pDependencies[i], &pDependencies[i], sizeof(RenderPassDecl::Dependency)) != 0)
		{
			return false;
		}
	}

	for (uint32 uPass = 0; uPass < uSubPasses; uPass++)
	{
		const SubPass& pass = pSubPasses[uPass];
		const SubPass& passRH = rhs.pSubPasses[uPass];
		if (pass.uInputCount != passRH.uInputCount
			|| pass.uInputCount != passRH.uInputCount
			|| pass.uInputCount != passRH.uInputCount
			|| pass.uInputCount != passRH.uInputCount)
		{
			return false;
		}

		// FIXME: The order shouldn't matter
		for (uint32 uInput = 0; uInput < pass.uInputCount; uInput++)
		{
			if (pass.pInputAttachments[uInput].eLayout != passRH.pInputAttachments[uInput].eLayout
				|| pass.pInputAttachments[uInput].uIndex != passRH.pInputAttachments[uInput].uIndex)
			{
				return false;
			}
		}

		for (uint32 uColor = 0; uColor < pass.uColorCount; uColor++)
		{
			if (pass.pColorAttachments[uColor].eLayout != passRH.pColorAttachments[uColor].eLayout
				|| pass.pColorAttachments[uColor].uIndex != passRH.pColorAttachments[uColor].uIndex)
			{
				return false;
			}
		}

		if (pass.pResolveAttachments)
		{
			for (uint32 uResolve = 0; uResolve < pass.uColorCount; uResolve++)
			{
				if (pass.pResolveAttachments[uResolve].eLayout != passRH.pResolveAttachments[uResolve].eLayout
					|| pass.pResolveAttachments[uResolve].uIndex != passRH.pResolveAttachments[uResolve].uIndex)
				{
					return false;
				}
			}
		}

		if ((pass.pDepthAttachment && !passRH.pDepthAttachment) || (!pass.pDepthAttachment && passRH.pDepthAttachment))
		{
			return false;
		}

		if (pass.pDepthAttachment)
		{
			if ((pass.pDepthAttachment->eLayout != passRH.pDepthAttachment->eLayout)
				|| (pass.pDepthAttachment->uIndex != passRH.pDepthAttachment->uIndex))
			{
				return false;
			}
		}

		for (uint32 uPreserve = 0; uPreserve < pass.uPreserveCount; uPreserve++)
		{
			if (passRH.puPreserveIndices[uPreserve] != passRH.puPreserveIndices[uPreserve] )
			{
				return false;
			}
		}
	}

	// It all matched
	return true;
}

const RenderPassDecl::Dependency* RenderPassDecl::ExternalColorDependencyIn()
{
	return &ExternalColorDependenciesInAndOut()[0];
}

const RenderPassDecl::Dependency* RenderPassDecl::ExternalColorDependencyOut()
{
	return &ExternalColorDependenciesInAndOut()[1];
}

const RenderPassDecl::Dependency* RenderPassDecl::ExternalColorDependenciesInAndOut()
{
	static Dependency DefaultExternals[2];
	static bool bHasInit = false;
	if (!bHasInit)
	{
		DefaultExternals[0].uSrcSubPass = SUBPASS_EXTERNAL;
		DefaultExternals[0].uDstSubPass = 0;
		DefaultExternals[0].uSrcStageFlags = SF_BOTTOM_OF_PIPE;
		DefaultExternals[0].uDstStageFlags = SF_COLOR_ATTACHMENT_OUTPUT;
		DefaultExternals[0].uSrcAccessFlags = AC_MEMORY_READ;
		DefaultExternals[0].uDstAccessFlags = AC_COLOR_ATTACHMENT_READ | AC_COLOR_ATTACHMENT_WRITE;

		DefaultExternals[1].uSrcSubPass = 0;
		DefaultExternals[1].uDstSubPass = SUBPASS_EXTERNAL;
		DefaultExternals[1].uSrcStageFlags = SF_COLOR_ATTACHMENT_OUTPUT;
		DefaultExternals[1].uDstStageFlags = SF_BOTTOM_OF_PIPE;
		DefaultExternals[1].uSrcAccessFlags = AC_COLOR_ATTACHMENT_READ | AC_COLOR_ATTACHMENT_WRITE;
		DefaultExternals[1].uDstAccessFlags = AC_MEMORY_READ;
		bHasInit = true;
	}

	return DefaultExternals;
}

DepthStencilStateDecl::DepthStencilStateDecl()
{
	bDepthEnable	= false;
	bDepthWrite		= false;
	eDepthFunc		= DEPTH_TEST_ALWAYS;

	// Stencil operations
	bStencilEnable = false;
	eStencilTest	= STENCIL_TEST_ALWAYS;
	for (uint32 i = 0; i < SOP_TYPE_COUNT; i++)
	{
		eStencilOps[i] = STENCIL_OP_KEEP;
	}

	SetMask(0xFF, 0, 0xFF);
}



DepthStencilStateDecl::~DepthStencilStateDecl()
{

}

void DepthStencilStateDecl::SetMask(uint8 uReadMask, uint8 uWriteMask, uint8 uRef, bool bFront, bool bBack)
{
	if (bFront)
	{
		uMask[STENCIL_CMP_MASK] = uReadMask;
		uMask[STENCIL_WRITE_MASK] = uWriteMask;
		uMask[STENCIL_REF] = uRef;
	}

	if (bBack)
	{
		uMask[STENCIL_CMP_MASK_BACK] = uReadMask;
		uMask[STENCIL_WRITE_MASK_BACK] = uWriteMask;
		uMask[STENCIL_REF_BACK] = uRef;
	}
}

void DepthStencilStateDecl::SetOperation(StencilOp ePassOp, StencilOp eStencilFailOp, StencilOp eDepthFailOp, bool bFront, bool bBack)
{
	if (bFront)
	{
		eStencilOps[SOP_TYPE_PASS] = ePassOp;
		eStencilOps[SOP_TYPE_STENCIL_FAIL] = eStencilFailOp;
		eStencilOps[SOP_TYPE_DEPTH_FAIL] = eDepthFailOp;
	}

	if (bBack)
	{
		eStencilOps[SOP_TYPE_PASS_BACK] = ePassOp;
		eStencilOps[SOP_TYPE_STENCIL_FAIL_BACK] = eStencilFailOp;
		eStencilOps[SOP_TYPE_DEPTH_FAIL_BACK] = eDepthFailOp;
	}
}



bool DepthStencilStateDecl::operator==(const DepthStencilStateDecl& rhs) const
{
	for(int i=0; i<STENCIL_REF_TYPE; i++)
	{
		if(uMask[i]!=rhs.uMask[i])
			return false;
	}

	for (int i = 0; i < SOP_TYPE_COUNT; i++)
	{
		if (eStencilOps[i] != rhs.eStencilOps[i])
			return false;
	}

	return( bDepthEnable	== rhs.bDepthEnable
		&& bDepthWrite		== rhs.bDepthWrite
		&& eDepthFunc		== rhs.eDepthFunc
		&& bStencilEnable	== rhs.bStencilEnable
		&& eStencilTest		== rhs.eStencilTest );
}


SamplerDecl::SamplerDecl(SamplerFilter eFilterIn, SamplerClamp eClampIn)
{
	eFilterMin = eFilterIn;
	eFilterMag = eFilterIn;
	eClampU = eClampIn;
	eClampV = eClampIn;
	bEnableCmp = false;
	eCmpFnc = CF_ALWAYS;
	eAnisoLevel = ANISO_LEVEL_1;
	eMipFilter = MF_POINT;
	bProjection = false;
	LodBias = 0.0f;
	LodMinLevel = 0;
}

SamplerDecl::SamplerDecl()
{
	eFilterMin = SF_LINEAR;
	eFilterMag = SF_LINEAR;
	eClampU = SC_CLAMP;
	eClampV = SC_CLAMP;
	bEnableCmp = false;
	eCmpFnc = CF_ALWAYS;
	eAnisoLevel = ANISO_LEVEL_1;
	eMipFilter = MF_POINT;
	bProjection = false;
	LodBias = 0.0f;
	LodMinLevel = 0;
}

SamplerDecl::~SamplerDecl()
{

}

void SamplerDecl::SetFilter(SamplerFilter eFilter)
{
	eFilterMin = eFilterMag = eFilter;
}

void SamplerDecl::SetClamp(SamplerClamp eClamp)
{
	eClampU = eClampV = eClamp;
}


bool SamplerDecl::operator==(const SamplerDecl& rhs) const
{
	return (rhs.eFilterMin == eFilterMin && rhs.eFilterMag == eFilterMag
		&& rhs.eClampU == eClampU && rhs.eClampV == eClampV
		&& rhs.eCmpFnc == eCmpFnc && bEnableCmp == rhs.bEnableCmp
		&& rhs.eAnisoLevel == eAnisoLevel
		&& rhs.eMipFilter == eMipFilter
		&& rhs.bProjection == bProjection
		&& Math::IsEqual(rhs.LodBias, LodBias, 0.05f)
		&& rhs.LodMinLevel == LodMinLevel);
}

PipelineStateDecl::PipelineStateDecl()
{
	ePrimType = PT_TRIANGLES;
	pEffect = NULL;
	uInputBindingCount = 0;
	eSampleCount = SAMPLE_COUNT_1_BIT;
}

PipelineStateDecl::~PipelineStateDecl()
{

}


PipelineStateDecl::InputBinding::InputBinding()
{
	uBindSlot = (uint32)(-1);
	pElements = NULL;
	eStepRate = VERTEX_INPUT_RATE_VERTEX;
	uStepSize = 0;
	uVertexSize = 0;
}

void PipelineStateDecl::InputBinding::Init(const VertexElement* pElementIn, uint32 uBindIn, VertexInputRate eRateIn, uint32 uStepIn)
{
	uBindSlot = uBindIn;
	pElements = pElementIn;
	eStepRate = eRateIn;
	uStepSize = uStepIn;
}

PipelineLayoutDecl::PipelineLayoutDecl()
{
	uDescriptorSetCount = 0;
}

PipelineLayoutDecl::~PipelineLayoutDecl()
{

}

}
