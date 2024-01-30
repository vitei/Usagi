/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Layout/Fonts/TextDrawer.h"
#include "Engine/Layout/Fonts/Text3D.h"

namespace usg {



	Text3D::Text3D() :
		m_bCanRender(false)
	{
		m_vWorldPos.Assign(0.0f, 0.0f, 0.0f);
		m_vOffset2D.Assign(0.0f, 0.0f);
		m_pScene = NULL;
		m_pRenderGroup = NULL;

		SetLayer(LAYER_TRANSLUCENT);
		SetPriority(128);

		SetDepthRange(0.0f, 1000000.f);
	}


	Text3D::~Text3D()
	{
	}

	void Text3D::Cleanup(GFXDevice* pDevice)
	{
		AddToScene(pDevice, false);

		m_text.Cleanup(pDevice);
		m_materialVSConstants.Cleanup(pDevice);
		m_materialGSConstants.Cleanup(pDevice);

		m_descriptorSet.Cleanup(pDevice);

	}

	void Text3D::Init(GFXDevice* pDevice, ResourceMgr* pRes, Scene* pScene, bool bDepthTest)
	{
		RenderPassHndl renderPass = pScene->GetRenderPasses(0).GetRenderPass(*this);

		m_text.Init(pDevice, pRes, renderPass);
		m_text.SetAlign(kTextAlignHOriginCenter| kTextAlignVOriginMiddle| kTextAlignCenter);

		m_materialVSConstants.Init(pDevice, TextDrawer::ms_constVSDecl);
		m_materialGSConstants.Init(pDevice, TextDrawer::ms_constGSDecl);

		TextDrawer::GSConstants3D* pGSConsts = m_materialGSConstants.Lock<TextDrawer::GSConstants3D>();
		// Put our co-ordiantes in the world range
		pGSConsts->projection.Orthographic(-1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f);
		m_materialGSConstants.Unlock();
		m_materialGSConstants.UpdateData(pDevice);
		m_pScene = pScene;

		m_descriptorSet.Init(pDevice, pDevice->GetDescriptorSetLayout(TextDrawer::ms_GlobalDescriptors3D));
		m_descriptorSet.SetConstantSet(0, &m_materialVSConstants);
		m_descriptorSet.SetConstantSet(1, &m_materialGSConstants);
		m_descriptorSet.UpdateDescriptors(pDevice);




		//AddToScene(pDevice, true);
	}


	void Text3D::AddToScene(usg::GFXDevice* pDevice, bool bAdd)
	{
		if (!m_pScene)
			return;

		if (bAdd && !m_pRenderGroup)
		{
			m_pRenderGroup = m_pScene->CreateRenderGroup(NULL);
			m_pRenderGroup->AddRenderNode(pDevice, (RenderNode*)this);
			m_pRenderGroup->SetSortPos(m_vWorldPos);
		}
		else if (!bAdd && m_pRenderGroup)
		{
			m_pScene->DeleteRenderGroup(m_pRenderGroup);
			m_pRenderGroup = NULL;
		}
	}


	bool Text3D::Draw(GFXContext* pContext, RenderContext& renderContext)
	{
		if (!m_bCanRender)
		{
			// Trying to render before having called UpdateBuffers first. Do not render for it may cause a crash. This boolean check can be removed when we stop creating entities
			// in the middle of a frame.
			return true;
		}
		pContext->BeginGPUTag("Text3D");
		// The default states
		pContext->SetDescriptorSet(&m_descriptorSet, 2);
		
		m_text.Draw(pContext, true);

		pContext->EndGPUTag();
		return true;
	}

	void Text3D::SetWorldPosition(const Vector3f& vWorldPos)
	{
		m_vWorldPos = vWorldPos;
		if (m_pRenderGroup)
		{
			m_pRenderGroup->SetSortPos(vWorldPos);
		}
	}

	void Text3D::Update(float deltaTime)
	{
		m_bCanRender = true;
		TextDrawer::VSConstants3D* pVSConsts = m_materialVSConstants.Lock<TextDrawer::VSConstants3D>();
		pVSConsts->depthRange.x = m_fNear;
		pVSConsts->depthRange.y = m_fFar;
		pVSConsts->depthRange.z = 1.f / (m_fFar - m_fNear);
		pVSConsts->depthRange.w = 0.0f;
		pVSConsts->position3D.Assign(m_vWorldPos, 1.0f);
		pVSConsts->offsetPos2D = m_vOffset2D;
		m_materialVSConstants.Unlock();
	}

	void Text3D::UpdateBuffers(GFXDevice* pDevice)
	{ 
		m_materialVSConstants.UpdateData(pDevice);
		m_descriptorSet.UpdateDescriptors(pDevice);
		m_text.UpdateBuffers(pDevice);
	}

	// Fading out in the world
	void Text3D::SetDepthRange(float fNear, float fFar)
	{
		m_fNear = fNear;
		m_fFar = fFar;
	}

	void Text3D::DisableDepthCull()
	{
		m_fNear = 1000000.f;
		m_fFar = m_fNear * 2.f;
	}
}