/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Scene/Camera/Camera.h"
#include "Engine/Scene/RenderNode.h"
#include "Engine/Debug/Rendering/Debug3D.h"
#include "Engine/Scene/Scene.h"
#include "Engine/PostFX/PostEffect.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/PostFX/PostFXSys.h"
#include <stdio.h>


namespace usg {

	struct GlobalConstants
	{
		Matrix4x4	mProjMat;
		Matrix4x4	mViewMat;
		Matrix4x4	mPrevViewMat;
		Matrix4x4	mInvViewMat;
		Matrix4x4	mViewProjMat;
		Vector4f	vFogVars;
		Vector4f	vFogColor;
		Vector4f	vEyePos;
		Vector4f	vLookDir;
		Vector4f	vSceneOffset;
		Matrix4x4	mInvProjMat;
		Vector4f	vNearFar;
		Vector4f	vHemSkyColor;
		Vector4f	vHemGroundColor;
		Vector4f	vHemisphereDir;
		Vector4f	vShadowColor;
	};

	static const ShaderConstantDecl g_globalSceneCBDecl[] =
	{
		SHADER_CONSTANT_ELEMENT(GlobalConstants, mProjMat,			CT_MATRIX_44, 1),
		SHADER_CONSTANT_ELEMENT(GlobalConstants, mViewMat,			CT_MATRIX_44, 1),
		SHADER_CONSTANT_ELEMENT(GlobalConstants, mPrevViewMat,		CT_MATRIX_44, 1),
		SHADER_CONSTANT_ELEMENT(GlobalConstants, mInvViewMat,		CT_MATRIX_44, 1),
		SHADER_CONSTANT_ELEMENT(GlobalConstants, mViewProjMat,		CT_MATRIX_44, 1),
		SHADER_CONSTANT_ELEMENT(GlobalConstants, vFogVars,			CT_VECTOR_4, 1),
		SHADER_CONSTANT_ELEMENT(GlobalConstants, vFogColor,			CT_VECTOR_4, 1),
		SHADER_CONSTANT_ELEMENT(GlobalConstants, vEyePos,			CT_VECTOR_4, 1),
		SHADER_CONSTANT_ELEMENT(GlobalConstants, vLookDir,			CT_VECTOR_4, 1),
		SHADER_CONSTANT_ELEMENT(GlobalConstants, vSceneOffset, 		CT_VECTOR_4, 1),
		SHADER_CONSTANT_ELEMENT(GlobalConstants, mInvProjMat,		CT_MATRIX_44, 1),
		SHADER_CONSTANT_ELEMENT(GlobalConstants, vNearFar,			CT_VECTOR_4, 1),
		SHADER_CONSTANT_ELEMENT(GlobalConstants, vHemSkyColor,		CT_VECTOR_4, 1),
		SHADER_CONSTANT_ELEMENT(GlobalConstants, vHemGroundColor,	CT_VECTOR_4, 1),
		SHADER_CONSTANT_ELEMENT(GlobalConstants, vHemisphereDir,	CT_VECTOR_4, 1),
		SHADER_CONSTANT_ELEMENT(GlobalConstants, vShadowColor,		CT_VECTOR_4, 1),
		SHADER_CONSTANT_END()
	};

	ViewContext::ViewContext() :
		SceneContext()
	{
		m_uHighestLOD = 0;
		m_fLODBias = 1.0f;
		m_pPostFXSys = NULL;
		m_pCamera = nullptr;
		m_bFirstFrame = true;
		m_shadowColor.Assign(0.05f, 0.05f, 0.1f, 1.0f);
		m_eActiveViewType = VIEW_CENTRAL;

		for (int i = 0; i < RenderNode::LAYER_COUNT; i++)
		{
			m_uVisibleNodes[i] = 0;
		}
	}

	void ViewContext::InitDeviceData(GFXDevice* pDevice)
	{
		const Scene* pScene = GetScene();
		SamplerDecl shadowSamp(SF_LINEAR, SC_CLAMP);
		shadowSamp.bEnableCmp = true;
		shadowSamp.eCmpFnc = CF_LESS;
		SamplerDecl pointDecl(SF_POINT, SC_CLAMP);
		SamplerHndl sampler = pDevice->GetSampler(pointDecl);
		SamplerHndl shadowSampler = pDevice->GetSampler(shadowSamp);
		TextureHndl dummyDepth = ResourceMgr::Inst()->GetTexture(pDevice, "white_default");
		m_LightingContext.Init(pDevice);
		for (int i = 0; i < VIEW_COUNT; i++)
		{
			m_globalConstants[i].Init(pDevice, g_globalSceneCBDecl);
			m_globalDescriptors[i].Init(pDevice, pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl));
			m_globalDescriptors[i].SetConstantSet(0, &m_globalConstants[i]);
			m_LightingContext.AddConstantsToDescriptor(m_globalDescriptors[i], 1);
			m_globalDescriptors[i].SetImageSamplerPair(2, dummyDepth, sampler);
			m_globalDescriptors[i].SetImageSamplerPair(3, pScene->GetLightMgr().GetShadowCascadeImage(), shadowSampler, 0);

			m_globalDescriptors[i].UpdateDescriptors(pDevice);

			m_globalDescriptorsWithDepth[i].Init(pDevice, pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl));
			m_globalDescriptorsWithDepth[i].SetConstantSet(0, &m_globalConstants[i]);
			m_LightingContext.AddConstantsToDescriptor(m_globalDescriptorsWithDepth[i], 1);
			m_globalDescriptorsWithDepth[i].SetImageSamplerPair(2, dummyDepth, sampler);
			m_globalDescriptorsWithDepth[i].SetImageSamplerPair(3, pScene->GetLightMgr().GetShadowCascadeImage(), shadowSampler, 0);

			m_globalDescriptorsWithDepth[i].UpdateDescriptors(pDevice);
		}

		SetDeviceDataLoaded();
	}

	void ViewContext::Cleanup(GFXDevice* pDevice)
	{
		m_LightingContext.Cleanup(pDevice);

		for (int i = 0; i < VIEW_COUNT; i++)
		{
			m_globalConstants[i].CleanUp(pDevice);
			m_globalDescriptors[i].CleanUp(pDevice);
			m_globalDescriptorsWithDepth[i].CleanUp(pDevice);
		}
	}

	ViewContext::~ViewContext()
	{

	}

	void ViewContext::Init(GFXDevice* pDevice, const Camera* pCamera, PostFXSys* pFXSys, uint32 uHighestLOD, uint32 uRenderMask)
	{
		SetHighestLOD(uHighestLOD);
		SetRenderMask(uRenderMask);
		m_pPostFXSys = pFXSys;

		m_pCamera = pCamera;
		m_searchObject.Init(GetScene(), this, m_pCamera->GetFrustum(), uRenderMask);
		Debug3D::GetRenderer()->InitContextData(pDevice, this);
	}



	void ViewContext::SetHighestLOD(uint32 uLOD)
	{
		m_uHighestLOD = uLOD;
	}

	void ViewContext::SetLODBias(float fBias)
	{
		m_fLODBias = fBias;
	}

	void ViewContext::ClearLists()
	{
		for (int i = 0; i < RenderNode::LAYER_COUNT; i++)
		{
			//m_drawLists[i].Clear();
			m_uVisibleNodes[i] = 0;
		}

		m_LightingContext.ClearLists();

		Inherited::ClearLists();
	}

	inline int CompareSortedNodes(const void* a, const void* b)   // comparison function
	{
		const RenderNode* arg1 = *(const RenderNode**)(a);
		const RenderNode* arg2 = *(const RenderNode**)(b);
		if (arg1->GetDepthSortValue() > arg2->GetDepthSortValue()) return -1;
		if (arg1->GetDepthSortValue() < arg2->GetDepthSortValue()) return 1;

		if (*arg1 < *arg2) return -1;
		if (*arg1 > *arg2) return 1;
		return 0;
	}

	inline int CompareNodes(const void* a, const void* b)   // comparison function
	{
		const RenderNode* arg1 = *(const RenderNode**)(a);
		const RenderNode* arg2 = *(const RenderNode**)(b);
		if (*arg1 < *arg2) return -1;
		if (*arg1 > *arg2) return 1;
		return 0;
	}

	void ViewContext::Update(GFXDevice* pDevice)
	{
		const Camera* pCamera = GetCamera();
		const Scene* pScene = GetScene();
		const uint32 uRenderMask = GetRenderMask();

		Vector4f vCameraPos = pCamera->GetPos();

		Matrix4x4 mTmp;

		m_LightingContext.Update(pDevice, this);

		// Lock the buffer
		for (int i = 0; i < VIEW_COUNT; i++)
		{
			GlobalConstants* globalData = m_globalConstants[i].Lock<GlobalConstants>();

			globalData->mProjMat = pCamera->GetProjection((ViewType)i);
			Matrix4x4 mViewMat = pCamera->GetViewMatrix((ViewType)i);
			Vector4f vOffset = GetScene()->GetSceneOffset();
			vOffset = vOffset * mViewMat;
			mViewMat._41 += vOffset.x;
			mViewMat._42 += vOffset.y;
			mViewMat._43 += vOffset.z;
			if (m_bFirstFrame)
			{
				globalData->mPrevViewMat = mViewMat;
			}
			else
			{
				globalData->mPrevViewMat = globalData->mViewMat;
			}
			globalData->mViewMat = mViewMat;//pCamera->GetViewMatrix((ViewType)i); 

			// FIXME: This is all hardcoded to the first fog, if we go back to using vertex shader based fog this needs
			// to be addressed
			globalData->vFogVars.x = m_fog[0].GetMinDepth();
			globalData->vEyePos = pCamera->GetPos((ViewType)i) - pScene->GetSceneOffset();
			globalData->vSceneOffset = pScene->GetSceneOffset();

			float fRange = m_fog[0].GetMaxDepth() - m_fog[0].GetMinDepth();
			if (fRange > 0.0f)
				globalData->vFogVars.y = 1.f / fRange;
			else
				globalData->vFogVars.y = 0.0f;

			globalData->vFogVars.z = m_fog[0].GetMaxDepth();
			globalData->vLookDir = pCamera->GetFacing();

			const LightMgr& lightMgr = pScene->GetLightMgr();

			// TODO: Set the hemisphere direction and ground color
			globalData->vHemisphereDir = Vector4f(pCamera->GetViewMatrix((ViewType)i).TransformVec3(lightMgr.GetHemisphereDir(), 0.0f),
				lightMgr.GetHemisphereLerp());
			lightMgr.GetGroundColor().FillV4(globalData->vHemGroundColor);
			lightMgr.GetSkyColor().FillV4(globalData->vHemSkyColor);
			globalData->mViewProjMat = pCamera->GetViewMatrix((ViewType)i) * pCamera->GetProjection((ViewType)i);
			pCamera->GetProjection((ViewType)i).GetInverse(globalData->mInvProjMat);
			globalData->vNearFar.x = pCamera->GetNear();
			globalData->vNearFar.y = pCamera->GetFar();
			globalData->vNearFar.z = pCamera->GetNear() / pCamera->GetFar();
			m_fog[0].GetColor().FillV4(globalData->vFogColor);
			pCamera->GetViewMatrix((ViewType)i).GetInverse(globalData->mInvViewMat);
			globalData->vShadowColor = m_shadowColor;
			m_globalConstants[i].Unlock();
			m_globalConstants[i].UpdateData(pDevice);

			if (m_pPostFXSys)
			{
				// Always update as the image could be resized
				//if (m_globalDescriptorsWithDepth[i].GetTextureAtBinding(14) != m_pPostFXSys->GetLinearDepthTex() )
				{
					// Update the linear depth texture
					m_globalDescriptorsWithDepth[i].SetImageSamplerPairAtBinding(14, m_pPostFXSys->GetLinearDepthTex(), m_globalDescriptorsWithDepth[i].GetSamplerAtBinding(14));
				}
			}
			
			m_globalDescriptors[i].SetImageSamplerPairAtBinding(15, pScene->GetLightMgr().GetShadowCascadeImage(), m_globalDescriptorsWithDepth[i].GetSamplerAtBinding(15));
			m_globalDescriptorsWithDepth[i].SetImageSamplerPairAtBinding(15, pScene->GetLightMgr().GetShadowCascadeImage(), m_globalDescriptorsWithDepth[i].GetSamplerAtBinding(15));
			m_globalDescriptors[i].UpdateDescriptors(pDevice);
			m_globalDescriptorsWithDepth[i].UpdateDescriptors(pDevice);
		}

		// TODO: Handle maximum LOD ID
		// FIXME: Scene culling

		float fFOVBias = 1.0f;//60.f / pCamera->GetFov();
		fFOVBias = fFOVBias * fFOVBias * m_fLODBias;

		uint32 uLodId;
		for (List<RenderGroup>::Iterator it = GetVisibleGroups().Begin(); !it.IsEnd(); ++it)
		{
			RenderGroup* pGroup = *it;
			if (pGroup->GetLod(vCameraPos, uLodId, fFOVBias))
			{
				uint32 uNodeCount = pGroup->GetLODEntryCount(uLodId);
				for (uint32 i = 0; i < uNodeCount; i++)
				{
					RenderNode* pNode = pGroup->GetLODRenderNode(uLodId, i);
					if ((pNode->GetRenderMask() & uRenderMask) != 0)
					{
						//m_drawLists[pNode->GetLayer()].AddToEnd(pNode);
						uint32 uLayer = pNode->GetLayer();
						m_pVisibleNodes[uLayer][m_uVisibleNodes[uLayer]] = pNode;
						m_uVisibleNodes[uLayer]++;
					}
				}
			}
		}

		if (m_pPostFXSys)
		{
			for (uint32 i = 0; i < m_pPostFXSys->GetPostEffectCount(); i++)
			{
				PostEffect* pEffect = m_pPostFXSys->GetEffect(i);
				if ((pEffect->GetRenderMask() & uRenderMask) != 0)
				{
					//m_drawLists[pEffect->GetLayer()].AddToEnd(pEffect);
					uint32 uLayer = pEffect->GetLayer();
					m_pVisibleNodes[uLayer][m_uVisibleNodes[uLayer]] = pEffect;
					m_uVisibleNodes[uLayer]++;
				}
			}
		}

		// Sort by layer, id, possibly material, and by distance is transparent
		// FIXME: This needs to be more intelligent, an insertion sort when adding/ removing a node
		for (int uLayer = 0; uLayer < RenderNode::LAYER_COUNT; uLayer++)
		{
			if (uLayer == RenderNode::LAYER_TRANSLUCENT)
			{
				qsort(m_pVisibleNodes[uLayer], m_uVisibleNodes[uLayer], sizeof(RenderNode*), CompareSortedNodes);
			}
			else
			{
				qsort(m_pVisibleNodes[uLayer], m_uVisibleNodes[uLayer], sizeof(RenderNode*), CompareNodes);
			}
		}

		m_bFirstFrame = false;

	}


	void ViewContext::PreDraw(GFXContext* pContext, ViewType eViewType)
	{
		//pContext->SetConstantBuffer( SHADER_CONSTANT_GLOBAL, &m_globalConstants[eViewType], SHADER_FLAG_VS_GS );
		pContext->SetDescriptorSet(&m_globalDescriptors[eViewType], 0);
		m_eActiveViewType = eViewType;
	}


	void ViewContext::DrawScene(GFXContext* pContext)
	{
		Scene* pScene = GetScene();
		// We only suport one light for forward rendering, set it now
		m_LightingContext.SetPrimaryShadowDesc(pContext);
		// FIXME: Should be split out into volume and directional, only the directional lights need rendering per view context
		// Iterate through the list of visible objects, drawing them
		RenderNode::RenderContext renderContext;
		renderContext.eRenderPass = m_pPostFXSys->IsEffectEnabled(PostFXSys::EFFECT_DEFERRED_SHADING) ? RenderNode::RENDER_PASS_DEFERRED : RenderNode::RENDER_PASS_FORWARD;
		// FIXME: Handle multiple views
		renderContext.pGlobalDescriptors = &m_globalDescriptors[m_eActiveViewType];
		renderContext.pPostFX = m_pPostFXSys;

		for (uint32 uLayer = 0; uLayer < RenderNode::LAYER_COUNT; uLayer++)
		{
			//for(List<RenderNode>::Iterator it = m_drawLists[uLayer].Begin(); !it.IsEnd(); ++it)
			for (uint32 i = 0; i < m_uVisibleNodes[uLayer]; i++)
			{
				RenderNode* node = m_pVisibleNodes[uLayer][i];
				node->Draw(pContext, renderContext);
			}

			if (uLayer == RenderNode::LAYER_DEFERRED_SHADING)
			{
				renderContext.eRenderPass = RenderNode::RENDER_PASS_FORWARD;
				if (m_pPostFXSys)
				{
					m_pPostFXSys->SetPostDepthDescriptors(pContext);
				}
				pContext->SetDescriptorSet(&m_globalDescriptorsWithDepth[m_eActiveViewType], 0);
				renderContext.pGlobalDescriptors = &m_globalDescriptorsWithDepth[m_eActiveViewType];
			}
			// TODO: Probably are going to want callbacks at the end of certain layers, for grabbing
			// the linear depth information etc
		}
	}

	const SceneRenderPasses& ViewContext::GetRenderPasses() const
	{
		return m_pPostFXSys->GetRenderPasses();
	}

	SceneRenderPasses& ViewContext::GetRenderPasses()
	{
		return m_pPostFXSys->GetRenderPasses();
	}

	void ViewContext::SetShadowColor(usg::Color& color)
	{
		color.FillV4(m_shadowColor);
	}

}
