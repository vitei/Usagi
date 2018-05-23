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
#include "PipelineState.h"
#include "RenderStateMgr.h"

namespace usg {


void PipelineInitData::InitCmpValue()
{
	uCmpValue = 0;
	uCmpValue |= (uint64)alpha.GetValue();
	uCmpValue |= ((uint64)ras.GetValue()) << 8;
	uCmpValue |= (uint64)(depth.GetValue()) << 16;
	uCmpValue |= ((uint64)ePrimType) << 24;
	uCmpValue |= ((uint64)pBinding.GetValue()) << 32;
	uCmpValue |= ((uint64)layout.GetValue()) << 48;
	uCmpValue |= ((uint64)eSampleCount << 58);
}

PipelineState::PipelineState()
{
	m_uComparison = USG_INVALID_ID;
	m_pAlphaState = NULL;
	m_pDepthStencilState = NULL;
	m_pRasterizerState = NULL;
	m_ePrimType = PT_TRIANGLES;
}

PipelineState::~PipelineState()
{

}

void PipelineState::Init(GFXDevice* pDevice, const PipelineInitData& decl, uint32 uID)
{
	m_pAlphaState = decl.alpha.GetContents();
	m_alphaState = decl.alpha;
	m_alphaCmp = decl.alpha.GetValue();

	m_pDepthStencilState = decl.depth.GetContents();
	m_depthStencilState = decl.depth;
	m_depthCmp = decl.depth.GetValue();

	m_pRasterizerState = decl.ras.GetContents();
	m_rasterizerState = decl.ras;
	m_rasCmp = decl.ras.GetValue();

	m_ePrimType = decl.ePrimType;

	m_pInputBinding = decl.pBinding;
	m_effectHndl = decl.pEffect;

	m_renderPassHndl = decl.renderPass;

	m_platform.Init(pDevice, decl);
}


}
