/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Scene/Camera/Camera.h"
#include "Engine/Graphics/Lights/LightMgr.h"
#include "Engine/Scene/RenderNode.h"
#include "Engine/Debug/Rendering/Debug3D.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/SceneSearchObject.h"
#include "Engine/Scene/LightingContext.h"
#include "Engine/Graphics/Fog.h"
#include "Engine/Graphics/Device/DescriptorSet.h"
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
		float		fAspect;
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
		SHADER_CONSTANT_ELEMENT(GlobalConstants, fAspect,			CT_FLOAT, 1),
		SHADER_CONSTANT_END()
	};

	struct ViewContext::PIMPL
	{
		PIMPL()
		{
			pPostFXSys = NULL;
			pCamera = nullptr;
			eActiveViewType = VIEW_CENTRAL;

			for (int i = 0; i < RenderLayer::LAYER_COUNT; i++)
			{
				uVisibleNodes[i] = 0;
			}
		}
		DescriptorSet			globalDescriptors[VIEW_COUNT];
		DescriptorSet			globalDescriptorsWithDepth[VIEW_COUNT];
		ConstantSet				globalConstants[VIEW_COUNT];

		const Camera*			pCamera;
		SceneSearchFrustum		searchObject;
		PostFXSys*				pPostFXSys;
		ViewType				eActiveViewType;
		LightingContext			lightingContext;

		// Arbitrarily assigning fog to the scene context
		Fog						fog[MAX_FOGS];

		RenderNode*				pVisibleNodes[RenderLayer::LAYER_COUNT][MAX_NODES_PER_LAYER];
		uint32					uVisibleNodes[RenderLayer::LAYER_COUNT];
	};


	ViewContext::ViewContext() :
		SceneContext()
	{
		m_fLODBias = 1.0f;
		m_bFirstFrame = true;
		m_shadowColor.Assign(0.05f, 0.05f, 0.1f, 1.0f);
		m_uHighestLOD = 0;
		m_pImpl = vnew(ALLOC_OBJECT) PIMPL;

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
		m_pImpl->lightingContext.Init(pDevice);
		for (int i = 0; i < VIEW_COUNT; i++)
		{
			m_pImpl->globalConstants[i].Init(pDevice, g_globalSceneCBDecl);
			m_pImpl->globalDescriptors[i].Init(pDevice, pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl));
			m_pImpl->globalDescriptors[i].SetConstantSet(0, &m_pImpl->globalConstants[i]);
			m_pImpl->lightingContext.AddConstantsToDescriptor(m_pImpl->globalDescriptors[i], 1);
			m_pImpl->globalDescriptors[i].SetImageSamplerPair(2, dummyDepth, sampler);
			m_pImpl->globalDescriptors[i].SetImageSamplerPair(3, pScene->GetLightMgr().GetShadowCascadeImage(), shadowSampler, 0);

			m_pImpl->globalDescriptors[i].UpdateDescriptors(pDevice);

			m_pImpl->globalDescriptorsWithDepth[i].Init(pDevice, pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl));
			m_pImpl->globalDescriptorsWithDepth[i].SetConstantSet(0, &m_pImpl->globalConstants[i]);
			m_pImpl->lightingContext.AddConstantsToDescriptor(m_pImpl->globalDescriptorsWithDepth[i], 1);
			m_pImpl->globalDescriptorsWithDepth[i].SetImageSamplerPair(2, dummyDepth, sampler);
			m_pImpl->globalDescriptorsWithDepth[i].SetImageSamplerPair(3, pScene->GetLightMgr().GetShadowCascadeImage(), shadowSampler, 0);

			m_pImpl->globalDescriptorsWithDepth[i].UpdateDescriptors(pDevice);
		}

		SetDeviceDataLoaded();
	}

	void ViewContext::Cleanup(GFXDevice* pDevice)
	{
		m_pImpl->lightingContext.Cleanup(pDevice);

		for (int i = 0; i < VIEW_COUNT; i++)
		{
			m_pImpl->globalConstants[i].CleanUp(pDevice);
			m_pImpl->globalDescriptors[i].CleanUp(pDevice);
			m_pImpl->globalDescriptorsWithDepth[i].CleanUp(pDevice);
		}
	}

	ViewContext::~ViewContext()
	{
		vdelete m_pImpl;
	}

	void ViewContext::Init(GFXDevice* pDevice, PostFXSys* pFXSys, uint32 uHighestLOD, uint32 uRenderMask)
	{
		SetHighestLOD(uHighestLOD);
		SetRenderMask(uRenderMask);
		m_pImpl->pPostFXSys = pFXSys;

		m_pImpl->searchObject.Init(GetScene(), this, uRenderMask);
		Debug3D::GetRenderer()->InitContextData(pDevice, this);
	}


	const Camera* ViewContext::GetCamera() const 	
	{ 
		return m_pImpl->pCamera; 
	}

	Octree::SearchObject& ViewContext::GetSearchObject() 
	{
		return m_pImpl->searchObject;
	}

	LightingContext& ViewContext::GetLightingContext()
	{
		return m_pImpl->lightingContext;
	}

	void ViewContext::SetCamera(const Camera* pCamera)
	{
		m_pImpl->pCamera = pCamera;
		usg::Fog& fog = GetFog();
		fog.SetMinDepth(pCamera->GetFar() * 0.8f);
		fog.SetMaxDepth(pCamera->GetFar());
		m_pImpl->searchObject.SetFrustum(&pCamera->GetFrustum());
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
		for (int i = 0; i < RenderLayer::LAYER_COUNT; i++)
		{
			//m_drawLists[i].Clear();
			m_pImpl->uVisibleNodes[i] = 0;
		}

		m_pImpl->lightingContext.ClearLists();

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


	Fog& ViewContext::GetFog(uint32 uIndex)
	{ 
		ASSERT(uIndex < MAX_FOGS);
		return m_pImpl->fog[uIndex]; 
	}
	
	PostFXSys* ViewContext::GetPostFXSys()
	{
		return m_pImpl->pPostFXSys; 
	}

	void ViewContext::Update(GFXDevice* pDevice)
	{
		const Camera* pCamera = GetCamera();
		const Scene* pScene = GetScene();
		const uint32 uRenderMask = GetRenderMask();

		Vector4f vCameraPos = pCamera->GetPos();

		Matrix4x4 mTmp;

		m_pImpl->lightingContext.Update(pDevice, this);

		// Lock the buffer
		for (int i = 0; i < VIEW_COUNT; i++)
		{
			GlobalConstants* globalData = m_pImpl->globalConstants[i].Lock<GlobalConstants>();

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
			globalData->vFogVars.x = m_pImpl->fog[0].GetMinDepth();
			globalData->vEyePos = pCamera->GetPos((ViewType)i) - pScene->GetSceneOffset();
			globalData->vSceneOffset = pScene->GetSceneOffset();

			float fRange = m_pImpl->fog[0].GetMaxDepth() - m_pImpl->fog[0].GetMinDepth();
			if (fRange > 0.0f)
				globalData->vFogVars.y = 1.f / fRange;
			else
				globalData->vFogVars.y = 0.0f;

			globalData->vFogVars.z = m_pImpl->fog[0].GetMaxDepth();
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
			m_pImpl->fog[0].GetColor().FillV4(globalData->vFogColor);
			pCamera->GetViewMatrix((ViewType)i).GetInverse(globalData->mInvViewMat);
			globalData->vShadowColor = m_shadowColor;
			if (m_pImpl->pPostFXSys)
			{
				globalData->fAspect = m_pImpl->pPostFXSys->GetInitialRT()->GetWidth() / (float)m_pImpl->pPostFXSys->GetInitialRT()->GetHeight();
			}
			else
			{
				globalData->fAspect = 1.0f;
			}
			m_pImpl->globalConstants[i].Unlock();
			m_pImpl->globalConstants[i].UpdateData(pDevice);

			if (m_pImpl->pPostFXSys)
			{
				// Always update as the image could be resized
				//if (m_globalDescriptorsWithDepth[i].GetTextureAtBinding(14) != m_pPostFXSys->GetLinearDepthTex() )
				{
					// Update the linear depth texture
					m_pImpl->globalDescriptorsWithDepth[i].SetImageSamplerPairAtBinding(14, m_pImpl->pPostFXSys->GetLinearDepthTex(), m_pImpl->globalDescriptorsWithDepth[i].GetSamplerAtBinding(14));
				}
			}
			
			m_pImpl->globalDescriptors[i].SetImageSamplerPairAtBinding(15, pScene->GetLightMgr().GetShadowCascadeImage(), m_pImpl->globalDescriptorsWithDepth[i].GetSamplerAtBinding(15));
			m_pImpl->globalDescriptorsWithDepth[i].SetImageSamplerPairAtBinding(15, pScene->GetLightMgr().GetShadowCascadeImage(), m_pImpl->globalDescriptorsWithDepth[i].GetSamplerAtBinding(15));
			m_pImpl->globalDescriptors[i].UpdateDescriptors(pDevice);
			m_pImpl->globalDescriptorsWithDepth[i].UpdateDescriptors(pDevice);
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
						m_pImpl->pVisibleNodes[uLayer][m_pImpl->uVisibleNodes[uLayer]] = pNode;
						m_pImpl->uVisibleNodes[uLayer]++;
					}
				}
			}
		}

		if (m_pImpl->pPostFXSys)
		{
			for (uint32 i = 0; i < m_pImpl->pPostFXSys->GetPostEffectCount(); i++)
			{
				PostEffect* pEffect = m_pImpl->pPostFXSys->GetEffect(i);
				if ((pEffect->GetRenderMask() & uRenderMask) != 0)
				{
					//m_drawLists[pEffect->GetLayer()].AddToEnd(pEffect);
					uint32 uLayer = pEffect->GetLayer();
					m_pImpl->pVisibleNodes[uLayer][m_pImpl->uVisibleNodes[uLayer]] = pEffect;
					m_pImpl->uVisibleNodes[uLayer]++;
				}
			}
		}

		// Sort by layer, id, possibly material, and by distance is transparent
		// FIXME: This needs to be more intelligent, an insertion sort when adding/ removing a node
		for (int uLayer = 0; uLayer < RenderLayer::LAYER_COUNT; uLayer++)
		{
			if (uLayer == RenderLayer::LAYER_TRANSLUCENT)
			{
				qsort(m_pImpl->pVisibleNodes[uLayer], m_pImpl->uVisibleNodes[uLayer], sizeof(RenderNode*), CompareSortedNodes);
			}
			else
			{
				qsort(m_pImpl->pVisibleNodes[uLayer], m_pImpl->uVisibleNodes[uLayer], sizeof(RenderNode*), CompareNodes);
			}
		}

		m_bFirstFrame = false;

	}


	void ViewContext::PreDraw(GFXContext* pContext, ViewType eViewType)
	{
		//pContext->SetConstantBuffer( SHADER_CONSTANT_GLOBAL, &m_globalConstants[eViewType], SHADER_FLAG_VS_GS );
		pContext->SetDescriptorSet(&m_pImpl->globalDescriptors[eViewType], 0);
		m_pImpl->eActiveViewType = eViewType;
	}


	void ViewContext::DrawScene(GFXContext* pContext)
	{
		Scene* pScene = GetScene();
		// We only suport one light for forward rendering, set it now
		m_pImpl->lightingContext.SetPrimaryShadowDesc(pContext);
		// FIXME: Should be split out into volume and directional, only the directional lights need rendering per view context
		// Iterate through the list of visible objects, drawing them
		RenderNode::RenderContext renderContext;
		if (m_pImpl->pPostFXSys)
		{
			renderContext.eRenderPass = m_pImpl->pPostFXSys->IsEffectEnabled(PostFXSys::EFFECT_DEFERRED_SHADING) ? RenderNode::RENDER_PASS_DEFERRED : RenderNode::RENDER_PASS_FORWARD;
		}
		else
		{
			renderContext.eRenderPass = RenderNode::RENDER_PASS_FORWARD;
		}
		// FIXME: Handle multiple views
		renderContext.pGlobalDescriptors = &m_pImpl->globalDescriptors[m_pImpl->eActiveViewType];
		renderContext.pPostFX = m_pImpl->pPostFXSys;

		for (uint32 uLayer = 0; uLayer < RenderLayer::LAYER_COUNT; uLayer++)
		{
			//for(List<RenderNode>::Iterator it = m_drawLists[uLayer].Begin(); !it.IsEnd(); ++it)
			for (uint32 i = 0; i < m_pImpl->uVisibleNodes[uLayer]; i++)
			{
				RenderNode* node = m_pImpl->pVisibleNodes[uLayer][i];
				node->Draw(pContext, renderContext);
			}

			if (uLayer == RenderLayer::LAYER_OPAQUE_UNLIT)
			{
				renderContext.eRenderPass = RenderNode::RENDER_PASS_FORWARD;
				if (m_pImpl->pPostFXSys)
				{
					m_pImpl->pPostFXSys->SetPostDepthDescriptors(pContext);
				}
				pContext->SetDescriptorSet(&m_pImpl->globalDescriptorsWithDepth[m_pImpl->eActiveViewType], 0);
				renderContext.pGlobalDescriptors = &m_pImpl->globalDescriptorsWithDepth[m_pImpl->eActiveViewType];
			}
			// TODO: Probably are going to want callbacks at the end of certain layers, for grabbing
			// the linear depth information etc
		}
	}

	const SceneRenderPasses& ViewContext::GetRenderPasses() const
	{
		return m_pImpl->pPostFXSys->GetRenderPasses();
	}

	SceneRenderPasses& ViewContext::GetRenderPasses()
	{
		return m_pImpl->pPostFXSys->GetRenderPasses();
	}

	void ViewContext::SetShadowColor(usg::Color& color)
	{
		color.FillV4(m_shadowColor);
	}

}
