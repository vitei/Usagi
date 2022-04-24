#include "Engine/Common/Common.h"
#include "Engine/Graphics/Lights/PointLight.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/OmniShadowContext.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "OmniShadow.h"

namespace usg
{

	OmniShadow::OmniShadow()
	{
		m_pShadowContext = nullptr;
		m_bEnableUpdate = true;
		m_bNothingVisible = false;
		m_bFirst = true;
	}

	OmniShadow::~OmniShadow()
	{
		ASSERT(m_pShadowContext == NULL );
	}

	void OmniShadow::Init(GFXDevice* pDevice, Scene* pScene, uint32 uResX, uint32 uResY)
	{
		m_cubeBuffer.InitCube(pDevice, uResX, uResY, DepthFormat::DEPTH_32F);
		m_cubeTarget.Init(pDevice, NULL, &m_cubeBuffer);
		usg::RenderTarget::RenderPassFlags flags;
		flags.uClearFlags = RenderTarget::RT_FLAG_DEPTH;
		flags.uStoreFlags = RenderTarget::RT_FLAG_DEPTH;
		flags.uShaderReadFlags = RenderTarget::RT_FLAG_DEPTH;
		m_cubeTarget.InitRenderPass(pDevice, flags);

		m_pShadowContext = pScene->CreateOmniShadowContext(pDevice);
		m_pShadowContext->Init(&m_sphere);
		m_pShadowContext->SetActive(false);

		m_bStatic = false;
		m_bEnableUpdate = true;
	}

	TextureHndl OmniShadow::GetShadowTexture()
	{
		return m_cubeBuffer.GetTexture();
	}

	void OmniShadow::SetShadowCastFlags(uint32 uFlags)
	{
		m_pShadowContext->SetShadowFlags(uFlags);
	}

	void OmniShadow::SetShadowExcludeFlags(uint32 uFlags)
	{
		m_pShadowContext->SetShadowExcludeFlags(uFlags);
	}


	void OmniShadow::Cleanup(GFXDevice* pDevice, Scene* pScene)
	{
		m_cubeBuffer.Cleanup(pDevice);
		m_cubeTarget.Cleanup(pDevice);
		if (m_pShadowContext)
		{
			pScene->DeleteOmniShadowContext(m_pShadowContext);
			m_pShadowContext = nullptr;
		}

		m_pShadowContext = nullptr;
		m_bEnableUpdate = true;
		m_bNothingVisible = false;
		m_bFirst = true;
	}

	void OmniShadow::EnableUpdate(bool bEnable)
	{
		if (bEnable == m_bEnableUpdate)
			return;

		m_pShadowContext->SetActive(bEnable);
		m_bEnableUpdate = bEnable;
	}


	void OmniShadow::GPUUpdate(GFXDevice* pDevice, PointLight* pLight)
	{
		m_pShadowContext->SetActive(m_bEnableUpdate);

		if (!m_bEnableUpdate)
			return;

		m_sphere.SetPos(pLight->GetPosition().v3());
		m_sphere.SetRadius(pLight->GetFar());
	}
	
	void OmniShadow::CreateShadowTex(GFXContext* pContext)
	{
		if (!m_bEnableUpdate)
			return;

		if (!m_bFirst)
		{
			// We run the first frame to avoid issues with texture layout
			if (m_bNothingVisible && m_pShadowContext->GetVisibleGroupCount() == 0)
				return;	// We have a blank shadow tex, no need to clear it again

			if (!m_pShadowContext->IsDirty())
				return;
		}

		m_bNothingVisible = m_pShadowContext->GetVisibleGroupCount() == 0;

		m_bFirst = false;

		pContext->BeginGPUTag("PointShadow", Color::Grey);

		pContext->SetRenderTarget(&m_cubeTarget);
		m_pShadowContext->DrawScene(pContext);
		pContext->SetRenderTarget(NULL);

		pContext->EndGPUTag();
	}

	void OmniShadow::PostDraw()
	{
		// Static shadows are drawn for one frame and then their update is disabled
		if (m_bStatic && m_bEnableUpdate)
		{
			m_pShadowContext->SetActive(false);
			m_bEnableUpdate = false;
		}
	}

	void OmniShadow::GetTexDim(Vector2f& vDimOut)
	{
		vDimOut.x = (float)m_cubeTarget.GetWidth();
		vDimOut.y = (float)m_cubeTarget.GetHeight();
	}



}