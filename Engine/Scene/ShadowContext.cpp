/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Scene/Camera/Camera.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Scene/ShadowContext.h"
#include "Engine/Graphics/Device/GFXContext.h"

namespace usg {

struct GlobalShadowConstants
{
	Matrix4x4	mProjMat;
	Matrix4x4	mViewMat;
};

static const ShaderConstantDecl g_globalShadowCBDecl[] =
{
	SHADER_CONSTANT_ELEMENT(GlobalShadowConstants, mProjMat,			CT_MATRIX_44, 1),
	SHADER_CONSTANT_ELEMENT(GlobalShadowConstants, mViewMat,			CT_MATRIX_44, 1),
	SHADER_CONSTANT_END()
};


ShadowContext::ShadowContext():
SceneContext()
{
	m_pCamera = nullptr;
}

ShadowContext::~ShadowContext()
{

}


void ShadowContext::InitDeviceData(GFXDevice* pDevice)
{
	m_globalConstants.Init(pDevice, g_globalShadowCBDecl);
	m_descriptorSet.Init(pDevice, pDevice->GetDescriptorSetLayout(SceneConsts::g_shadowGlobalDescriptorDecl));
	m_descriptorSet.SetConstantSet(0, &m_globalConstants);
	m_descriptorSet.UpdateDescriptors(pDevice);
	SetDeviceDataLoaded();

}

void ShadowContext::Cleanup(GFXDevice* pDevice)
{
	m_globalConstants.CleanUp(pDevice);
	m_descriptorSet.CleanUp(pDevice);
}

void ShadowContext::Init(const Camera* pCamera)
{
	SetRenderMask(RenderNode::RENDER_MASK_SHADOW_CAST);

	m_pCamera = pCamera;
	m_searchObject.Init(GetScene(), this, m_pCamera->GetFrustum(), RenderNode::RENDER_MASK_SHADOW_CAST);
}

void ShadowContext::ClearLists()
{
	m_drawList.Clear();
	Inherited::ClearLists();
}

void ShadowContext::Update(GFXDevice* pDevice)
{
	const Camera* pCamera = GetCamera();

	GlobalShadowConstants* globalData = m_globalConstants.Lock<GlobalShadowConstants>();

	// Add the light camera view and projection matrices here
	globalData->mProjMat		= pCamera->GetProjection();
	globalData->mViewMat		= pCamera->GetViewMatrix();

	m_globalConstants.Unlock();
	m_globalConstants.UpdateData(pDevice);

	for (List<RenderGroup>::Iterator it = GetVisibleGroups().Begin(); !it.IsEnd(); ++it)
	{
		RenderGroup* pGroup = *it;
		uint32 uNodeCount = pGroup->GetLODEntryCount(0);
		for(uint32 i=0; i < uNodeCount; i++ )
		{
			RenderNode* pNode = pGroup->GetLODRenderNode(0, i);
			if( (pNode->GetRenderMask() & RenderNode::RENDER_MASK_SHADOW_CAST)!=0 )
			{
				m_drawList.AddToEnd(pNode);
			}
		}
	}
}


void ShadowContext::DrawScene(GFXContext* pContext)
{
	pContext->SetDescriptorSet( &m_descriptorSet, 0 );
	RenderNode::RenderContext renderContext;
	renderContext.eRenderPass = RenderNode::RENDER_PASS_DEPTH;

	for(List<RenderNode>::Iterator it = m_drawList.Begin(); !it.IsEnd(); ++it)
	{
		RenderNode* node = (*it);
		node->Draw(pContext, renderContext);
	}
}

}
