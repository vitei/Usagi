/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include API_HEADER(Engine/Graphics/Device, AlphaState.h)
#include API_HEADER(Engine/Graphics/Device, RasterizerState.h)
#include API_HEADER(Engine/Graphics/Device, DepthStencilState.h)
#include API_HEADER(Engine/Graphics/Device, PipelineLayout.h)
#include API_HEADER(Engine/Graphics/Device, RenderPass.h)
#include API_HEADER(Engine/Graphics/Textures, Sampler.h)
#include "Engine/Graphics/Device/DescriptorSetLayoutDecl.h"
#include "Engine/Graphics/Device/DescriptorSetLayout.h"
#include "Engine/Graphics/Device/RenderPassInitData.h"
#include "Engine/Graphics/Device/PipelineState.h"
#include "Engine/Graphics/Effects/InputBinding.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/GFXHandles.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Resource/ResourceMgr.h"
#include "RenderStateMgr.h"

namespace usg {


// FIXME: Move over to a system capable of resizing
const int MAX_STATE_BLOCKS = 255;
const int MAX_SAMPLERS = 128;
const int MAX_PIPELINE_STATES = 4096;
const int MAX_DESCRIPTOR_LAYOUTS = 50;
const int MAX_PIPELINE_STATE_LAYOUTS = 50;
const int MAX_RENDER_PASSES = 50;
const int MAX_INPUT_BINDING = 50;

template <class DeclarationType, class ResourceType, class HandleType, uint32 COUNT>
class StateResources
{
public:
	StateResources() { m_uPairs = 0; m_uStaticPairs = 0;}
	~StateResources() {}
	
	void SetDefault(HandleType defaultIn) { m_default = defaultIn; }
	uint32 GetCount() const { return m_uPairs; }
	const DeclarationType* GetDeclaration(const HandleType& handle) const { ASSERT(handle.GetValue() < m_uPairs); return &m_pairings[handle.GetValue()].decl; }

	struct Pairing
	{
		DeclarationType		decl;
		ResourceType*		state;
	};

	HandleType GetState(const DeclarationType* pDecl, GFXDevice* pDevice)
	{
		if (pDecl == NULL)
		{
			return m_default;
		}

		for (uint32 i = 0; i < m_uPairs; i++)
		{
			if (*pDecl == m_pairings[i].decl)
			{
				return HandleType(m_pairings[i].state, i);
			}
		}

		if (m_uPairs < COUNT)
		{
			m_pairings[m_uPairs].decl = *pDecl;
			m_pairings[m_uPairs].state = vnew(ALLOC_OBJECT) ResourceType;
			m_pairings[m_uPairs].state->Init(pDevice, m_pairings[m_uPairs].decl, m_uPairs);
			m_uPairs++;
			uint32 uIndex = m_uPairs - 1;
			return HandleType(m_pairings[uIndex].state, uIndex);
		}

		ASSERT(false);
		return HandleType(m_pairings[0].state, 0);
	}

	void FinishedStaticLoad()
	{
		m_uStaticPairs = m_uPairs;
	}

	void ClearDynamicResources()
	{
		for(uint32 i=m_uStaticPairs; i<m_uPairs; i++)
		{
			vdelete m_pairings[i].state;
		}
		m_uPairs = m_uStaticPairs;

	}

	void ClearAllResources()
	{
		for (uint32 i = 0; i < m_uPairs; i++)
		{
			vdelete m_pairings[i].state;
		}
		m_uPairs = 0;

	}

	void ClearAllResources(usg::GFXDevice* pDevice)
	{
		for (uint32 i = 0; i < m_uPairs; i++)
		{
			m_pairings[i].state->Cleanup(pDevice);
		}
		ClearDynamicResources();
	}

	void ClearDynamicResources(usg::GFXDevice* pDevice)
	{
		for (uint32 i = m_uStaticPairs; i < m_uPairs; i++)
		{
			m_pairings[i].state->Cleanup(pDevice);
		}
		ClearDynamicResources();

	}

private:
	HandleType			m_default;
	Pairing				m_pairings[COUNT];
	uint32				m_uPairs;
	uint32				m_uStaticPairs;

};



struct RenderStateMgr::PIMPL
{
	StateResources<AlphaStateDecl, AlphaState, AlphaStateHndl, MAX_STATE_BLOCKS>									alphaStates;
	StateResources<DepthStencilStateDecl, DepthStencilState, DepthStencilStateHndl, MAX_STATE_BLOCKS>				depthStates;
	StateResources<RasterizerStateDecl, RasterizerState, RasterizerStateHndl, MAX_STATE_BLOCKS>						rasterizerStates;
	StateResources<SamplerDecl, Sampler, SamplerHndl, MAX_SAMPLERS>													samplers;
	StateResources<PipelineInitData, PipelineState, PipelineStateHndl, MAX_PIPELINE_STATES>							pipelines;
	StateResources<RenderPassInitData, RenderPass, RenderPassHndl, MAX_RENDER_PASSES>								renderPasses;
	StateResources<DescriptorSetLayoutDecl, DescriptorSetLayout, DescriptorSetLayoutHndl, MAX_DESCRIPTOR_LAYOUTS>	descriptorLayouts;
	StateResources<PipelineLayoutDecl, PipelineLayout, PipelineLayoutHndl, MAX_DESCRIPTOR_LAYOUTS>					pipelineLayouts;

	InputBinding*			inputBindings[MAX_INPUT_BINDING];
	uint32					uInputBindings;
	uint32					uDynamicBindingStart;
};

RenderStateMgr::RenderStateMgr()
{
	m_pImpl = vnew(ALLOC_GFX_INTERNAL) PIMPL;

	m_pImpl->uInputBindings = 0;
	m_pImpl->uDynamicBindingStart = 0;
}

RenderStateMgr::~RenderStateMgr()
{
	for (uint32 i = 0; i < m_pImpl->uInputBindings; i++)
	{
		vdelete m_pImpl->inputBindings[i];
	}
	m_pImpl->uInputBindings = m_pImpl->uDynamicBindingStart;

	vdelete m_pImpl;
	m_pImpl = NULL;
}

void RenderStateMgr::InitDefaults(GFXDevice* pDevice)
{
	AlphaStateDecl			defaultAlpha;
	RasterizerStateDecl		defaultRast;
	DepthStencilStateDecl	defaultDepthStencil;

	m_pImpl->alphaStates.SetDefault(GetAlphaState(&defaultAlpha, pDevice));
	m_pImpl->rasterizerStates.SetDefault(GetRasterizerState(&defaultRast, pDevice));
	m_pImpl->depthStates.SetDefault(GetDepthStencilState(&defaultDepthStencil, pDevice));
}

void RenderStateMgr::Cleanup(GFXDevice* pDevice)
{
	m_pImpl->alphaStates.ClearAllResources();
	m_pImpl->depthStates.ClearAllResources();
	m_pImpl->rasterizerStates.ClearAllResources();
	m_pImpl->samplers.ClearAllResources(pDevice);
	m_pImpl->pipelines.ClearAllResources(pDevice);
	m_pImpl->renderPasses.ClearAllResources(pDevice);
	m_pImpl->descriptorLayouts.ClearAllResources(pDevice);
	m_pImpl->pipelineLayouts.ClearAllResources();
}

AlphaStateHndl RenderStateMgr::GetAlphaState(const AlphaStateDecl* pDecl, GFXDevice* pDevice)
{
	return m_pImpl->alphaStates.GetState(pDecl, pDevice);
}

RasterizerStateHndl RenderStateMgr::GetRasterizerState(const RasterizerStateDecl* pDecl, GFXDevice* pDevice)
{
	return m_pImpl->rasterizerStates.GetState(pDecl, pDevice);
}


DepthStencilStateHndl RenderStateMgr::GetDepthStencilState(const DepthStencilStateDecl* pDecl, GFXDevice* pDevice)
{
	return m_pImpl->depthStates.GetState(pDecl, pDevice);
}

PipelineLayoutHndl RenderStateMgr::GetPipelineLayout(const PipelineLayoutDecl* pDecl, GFXDevice* pDevice)
{
	return m_pImpl->pipelineLayouts.GetState(pDecl, pDevice);
}

DescriptorSetLayoutHndl RenderStateMgr::GetDescriptorSetLayout(const DescriptorDeclaration* pDecl, GFXDevice* pDevice)
{
	DescriptorSetLayoutDecl decl;
	decl.Init(pDecl);
	return m_pImpl->descriptorLayouts.GetState(&decl, pDevice);
}

RenderPassHndl RenderStateMgr::GetRenderPass(const RenderPassDecl& decl, GFXDevice* pDevice)
{
	RenderPassInitData cmpData;
	cmpData.InitForComparison(decl);

	RenderPassHndl hndl = m_pImpl->renderPasses.GetState(&cmpData, pDevice);
	return hndl;
}

PipelineStateHndl RenderStateMgr::GetPipelineState(const RenderPassHndl& renderPass, const PipelineStateDecl& decl, GFXDevice* pDevice)
{
	AlphaStateHndl alpha = GetAlphaState(&decl.alphaState, pDevice);
	DepthStencilStateHndl depth = GetDepthStencilState(&decl.depthState, pDevice);
	RasterizerStateHndl ras = GetRasterizerState(&decl.rasterizerState, pDevice);
	PipelineLayoutHndl layout = GetPipelineLayout(&decl.layout, pDevice);

#ifdef DEBUG_BUILD
	for (uint32 i = 0; i < decl.layout.uDescriptorSetCount; i++)
	{
		ASSERT(decl.layout.descriptorSets[i].IsValid());
	}
#endif

	// Replace the pointers with our own internal ones so we don't end up creating additional PSOs
	PipelineInitData pipelineInit;
	pipelineInit.alpha = alpha;
	pipelineInit.depth = depth;
	pipelineInit.ras = ras;
	pipelineInit.ePrimType = decl.ePrimType;
	pipelineInit.pBinding = GetInputBinding(pDevice, &decl);
	pipelineInit.pEffect = decl.pEffect;;
	pipelineInit.layout = layout;
	pipelineInit.eSampleCount = decl.eSampleCount;
	pipelineInit.renderPass = renderPass;
	pipelineInit.InitCmpValue();

	PipelineStateHndl hndl = m_pImpl->pipelines.GetState(&pipelineInit, pDevice);
	return hndl;
}

InputBindingHndl RenderStateMgr::GetInputBinding(GFXDevice* pDevice, const PipelineStateDecl* pDecl)
{
	// TODO: Replace all of this
	uint32 uDeclIds[MAX_VERTEX_BUFFERS];
	ASSERT(pDecl->uInputBindingCount > 0);
	for (uint32 i = 0; i < pDecl->uInputBindingCount; i++)
	{
		VertexDeclaration decl;
		decl.InitDecl(pDecl->inputBindings[i].pElements, pDecl->inputBindings[i].eStepRate == VERTEX_INPUT_RATE_VERTEX ? 0 : pDecl->inputBindings[i].uStepSize, pDecl->inputBindings[i].uVertexSize);
		uDeclIds[i] = VertexDeclaration::GetDeclId(decl);
	}

	// FIXME: Remove the effect binding from the resource manager and pass the effect in directly
	return GetInputBindingMultiStream(pDevice, uDeclIds, pDecl->uInputBindingCount);
}

InputBindingHndl RenderStateMgr::GetInputBindingMultiStream(GFXDevice* pDevice, uint32* puDeclIds, uint32 uBufferCount)
{
	for (uint32 i = 0; i < m_pImpl->uInputBindings; i++)
	{
		if (m_pImpl->inputBindings[i]->IsDecl(puDeclIds, uBufferCount))
		{
			return InputBindingHndl(m_pImpl->inputBindings[i], i);
		}
	}
	if (m_pImpl->uInputBindings < MAX_INPUT_BINDING)
	{
		InputBinding* pNewBinding = vnew(ALLOC_GFX_SHADER) InputBinding;
		pNewBinding->InitMultiStream(pDevice, puDeclIds, uBufferCount);
		m_pImpl->inputBindings[m_pImpl->uInputBindings] = pNewBinding;
		uint32 uIndex = m_pImpl->uInputBindings;
		m_pImpl->uInputBindings++;
		return InputBindingHndl(m_pImpl->inputBindings[uIndex], uIndex);
	}
	else
	{
		ASSERT(false);
		return InputBindingHndl();
	}
}

SamplerHndl RenderStateMgr::GetSamplerState(const SamplerDecl& decl, GFXDevice* pDevice)
{
	return m_pImpl->samplers.GetState(&decl, pDevice);
}


bool RenderStateMgr::UsesBlendColor(AlphaStateHndl hndl)
{
	const AlphaStateDecl* pState = m_pImpl->alphaStates.GetDeclaration(hndl);
	return pState->UsesBlendColor();
}

void RenderStateMgr::GetAlphaDeclaration(const AlphaStateHndl alpha, AlphaStateDecl& out)
{
	if(alpha.GetValue() < m_pImpl->alphaStates.GetCount())
	{
		out = *m_pImpl->alphaStates.GetDeclaration(alpha);
	}
	else
	{
		ASSERT(false);
	}
}

void RenderStateMgr::GetRaserizerDeclaration(const RasterizerStateHndl raster, RasterizerStateDecl& out)
{
	if (raster.GetValue() < m_pImpl->rasterizerStates.GetCount())
	{
		out = *m_pImpl->rasterizerStates.GetDeclaration(raster);
	}
	else
	{
		ASSERT(false);
	}
}

void RenderStateMgr::GetDepthStencilDeclaration(const DepthStencilStateHndl ds, DepthStencilStateDecl& out)
{
	if (ds.GetValue() < m_pImpl->depthStates.GetCount())
	{
		out = *m_pImpl->depthStates.GetDeclaration(ds);
	}
	else
	{
		ASSERT(false);
	}
}


void RenderStateMgr::GetPipelineLayoutDeclaration(const PipelineLayoutHndl layout, PipelineLayoutDecl& out)
{
	if (layout.GetValue() < m_pImpl->pipelineLayouts.GetCount())
	{
		out = *m_pImpl->pipelineLayouts.GetDeclaration(layout);
	}
	else
	{
		ASSERT(false);
	}
}

// Due to the size of these we are reconstructing them from their handles, there could be 10s of thousands
void RenderStateMgr::GetPipelineStateDeclaration(const PipelineStateHndl pipeline, PipelineStateDecl& out, RenderPassHndl& passOut)
{
	if (pipeline.GetValue() < m_pImpl->pipelines.GetCount())
	{
		const PipelineInitData* pInitData = m_pImpl->pipelines.GetDeclaration(pipeline);
		GetAlphaDeclaration(pInitData->alpha, out.alphaState);
		GetDepthStencilDeclaration(pInitData->depth, out.depthState);
		GetRaserizerDeclaration(pInitData->ras, out.rasterizerState);
		GetPipelineLayoutDeclaration(pInitData->layout, out.layout);
		out.pEffect = pInitData->pEffect;
		out.uInputBindingCount = pInitData->pBinding.GetContents()->GetDeclCount();
		out.ePrimType = pInitData->ePrimType;
		passOut = pInitData->renderPass;
		for (uint32 i = 0; i < out.uInputBindingCount; i++)
		{
			const VertexDeclaration* pDecl = VertexDeclaration::GetDecl(pInitData->pBinding.GetContents()->GetDeclId(i));
			out.inputBindings[i].eStepRate = pDecl->GetInstanceDiv() == 0 ? VERTEX_INPUT_RATE_VERTEX : VERTEX_INPUT_RATE_INSTANCE;
			out.inputBindings[i].uBindSlot = i;	// FIXME: Not guaranteed but we are deprecating this function
			out.inputBindings[i].pElements = pDecl->GetElements();
			out.inputBindings[i].uStepSize = pDecl->GetInstanceDiv();
			out.inputBindings[i].uVertexSize = (uint32)pDecl->GetSize();
		}
	}
	else
	{
		ASSERT(false);
	}
}


void RenderStateMgr::FinishedStaticLoad()
{
	m_pImpl->alphaStates.FinishedStaticLoad();
	m_pImpl->depthStates.FinishedStaticLoad();
	m_pImpl->rasterizerStates.FinishedStaticLoad();
	m_pImpl->samplers.FinishedStaticLoad();
	m_pImpl->pipelines.FinishedStaticLoad();
	m_pImpl->renderPasses.FinishedStaticLoad();
	m_pImpl->descriptorLayouts.FinishedStaticLoad();
	m_pImpl->pipelineLayouts.FinishedStaticLoad();

	m_pImpl->uDynamicBindingStart = m_pImpl->uInputBindings;
}


void RenderStateMgr::ClearDynamicResources(usg::GFXDevice* pDevice)
{
	m_pImpl->alphaStates.ClearDynamicResources();
	m_pImpl->depthStates.ClearDynamicResources();
	m_pImpl->rasterizerStates.ClearDynamicResources();
	m_pImpl->samplers.ClearDynamicResources(pDevice);
	m_pImpl->pipelines.ClearDynamicResources(pDevice);
	m_pImpl->renderPasses.ClearDynamicResources(pDevice);
	m_pImpl->descriptorLayouts.ClearDynamicResources(pDevice);
	m_pImpl->pipelineLayouts.ClearDynamicResources();

	for (uint32 i = m_pImpl->uDynamicBindingStart; i < m_pImpl->uInputBindings; i++)
	{
		vdelete m_pImpl->inputBindings[i];
		m_pImpl->inputBindings[i] = NULL;
	}
	m_pImpl->uInputBindings = m_pImpl->uDynamicBindingStart;
}


}
