#include "Engine/Common/Common.h"
#include "Engine/Graphics/Lights/SpotLight.h"
#include "Engine/Graphics/Lights/ProjectionLight.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/ShadowContext.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "ProjectionShadow.h"

namespace usg
{

	struct SpotShadowConstants
	{
		Matrix4x4   mLightMtx;
		Matrix4x4   mLightMtxVInv;
		Vector4f    vBias;
		Vector2f	vInvShadowDim;
	};

	static const ShaderConstantDecl g_pointShadowConsts[] =
	{
		SHADER_CONSTANT_ELEMENT(SpotShadowConstants, mLightMtx,		CT_MATRIX_44, 1),
		SHADER_CONSTANT_ELEMENT(SpotShadowConstants, mLightMtxVInv, CT_MATRIX_44, 1),
		SHADER_CONSTANT_ELEMENT(SpotShadowConstants, vBias,			CT_VECTOR_4, 1),
		SHADER_CONSTANT_ELEMENT(SpotShadowConstants, vInvShadowDim,	CT_VECTOR_2, 1),
		SHADER_CONSTANT_END()
	};

	ProjectionShadow::ProjectionShadow()
	{
		m_pSceneContext = NULL;
		m_bNothingVisible = false;
	}

	ProjectionShadow::~ProjectionShadow()
	{
		ASSERT( m_pSceneContext == NULL );
	}

	void ProjectionShadow::Init(GFXDevice* pDevice, Scene* pScene, uint32 uResX, uint32 uResY)
	{
		m_depthBuffer.Init(pDevice, uResX, uResY, DF_DEPTH_32F);//DF_DEPTH_32F); //DF_DEPTH_24
		m_depthTarget.Init(pDevice, NULL, &m_depthBuffer);

		m_pSceneContext = pScene->CreateShadowContext(pDevice);
		m_pSceneContext->Init(&m_camera);
		m_pSceneContext->SetActive(false);

		m_readConstants.Init(pDevice, g_pointShadowConsts);

		m_bStatic = false;
		m_bEnableUpdate = true;
	}

	void ProjectionShadow::Cleanup(GFXDevice* pDevice, Scene* pScene)
	{
		if (m_pSceneContext)
		{
			pScene->DeleteShadowContext(m_pSceneContext);
		}
	}

	void ProjectionShadow::EnableUpdate(bool bEnable)
	{
		if (bEnable == m_bEnableUpdate)
			return;

		//m_pSceneContext->SetActive(bEnable);
		m_bEnableUpdate = bEnable;
	}
	
	void ProjectionShadow::GPUUpdate(GFXDevice* pDevice, Light* pLight)
	{
		m_pSceneContext->SetActive(m_bEnableUpdate);
		if (!m_bEnableUpdate)
			return;

		Matrix4x4 mViewMat;
		Matrix4x4 mProj;

		if (pLight->GetType() == LIGHT_TYPE_SPOT)
		{
			SpotLight* pSpotLight = (SpotLight*)pLight;
			Vector4f vLookAt = pSpotLight->GetPosition() + (pSpotLight->GetDirection() * pSpotLight->GetFar());
			Vector4f vDirection = pSpotLight->GetDirection();
			Vector4f vUp(0.0f, 1.0f, 0.0f, 0.0f);

			if (DotProduct(vUp, vDirection) > 0.8f)
			{
				vUp.Assign(1.0f, 0.0f, 0.0f, 0.0f);
			}

			mViewMat.LookAt(pSpotLight->GetPosition().v3(), vLookAt.v3(), vUp.v3());
			mProj.Perspective(pSpotLight->GetOuterCutoff()*2.f, 1.0f, 0.1f, pSpotLight->GetFar());
		}
		else if (pLight->GetType() == LIGHT_TYPE_PROJ)
		{
			ProjectionLight* pProjLight = (ProjectionLight*)pLight;
			mViewMat = pProjLight->GetViewMatrix();
			mProj = pProjLight->GetProjMatrix();
		}

		m_camera.SetUp(mViewMat, mProj);

		Matrix4x4 texBias = Matrix4x4::TextureBiasMatrix();

		SpotShadowConstants* pData = m_readConstants.Lock<SpotShadowConstants>();
		pData->mLightMtx = mViewMat * mProj * texBias;
		pData->mLightMtxVInv = Matrix4x4::Identity();	// FIXME: not implemented
		pData->vBias.Assign(-0.0015f, -0.0015f, -0.0015f, -0.0015f);
		pData->vInvShadowDim.Assign(1.f / (float)m_depthBuffer.GetWidth(), 1.f / (float)m_depthBuffer.GetHeight());
		m_readConstants.Unlock();
		m_readConstants.UpdateData(pDevice);
	}

	
	void ProjectionShadow::CreateShadowTex(GFXContext* pContext)
	{
		if (!m_bEnableUpdate)
			return;

		if (m_bNothingVisible && m_pSceneContext->GetVisibleGroupCount() == 0)
			return;	// We have a blank shadow tex, no need to clear it again

		m_bNothingVisible = m_pSceneContext->GetVisibleGroupCount() == 0;

		pContext->BeginGPUTag("SpotShadow");

		pContext->SetRenderTarget(&m_depthTarget);
		pContext->ClearRenderTarget(RenderTarget::RT_FLAG_DEPTH);
		m_pSceneContext->DrawScene(pContext);
		pContext->SetRenderTarget(NULL);

		pContext->EndGPUTag();
	}

	void ProjectionShadow::PostDraw()
	{
		// Static shadows are drawn for one frame and then their update is disabled
		if (m_bStatic && m_bEnableUpdate)
		{
			m_pSceneContext->SetActive(false);
			m_bEnableUpdate = false;
		}
	}



}