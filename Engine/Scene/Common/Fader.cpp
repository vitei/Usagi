/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Scene/Model/ModelEffectEvents.pb.h"
#include "Fader.h"


namespace usg
{


#define FADE_SPEED 8.0f		// 1/8th of a second

	static float sTime = 1.0f;
	static int sFadeType = 0;
	static float sfAlpha = 1.0f;


#define NUM_VERTICES 6

	struct FadeConstants
	{
		float	fFade;
	};

	const ShaderConstantDecl g_fadeCBDecl[] =
	{
		SHADER_CONSTANT_ELEMENT(FadeConstants, fFade,	CT_FLOAT, 1),
		SHADER_CONSTANT_END()
	};


	static const usg::DescriptorDeclaration g_descriptorDecl[] =
	{
		DESCRIPTOR_ELEMENT(usg::SHADER_CONSTANT_MATERIAL,	usg::DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, usg::SHADER_FLAG_PIXEL),
		DESCRIPTOR_END()
	};


	void Fader::Draw(usg::GFXContext* pContext, bool upper)
	{
		if (sfAlpha > 0.0f)
		{
			if (!m_bWipeLower && !upper)
				return;

			pContext->BeginGPUTag("Fader");
			pContext->SetPipelineState(m_pipelineState);
			pContext->SetDescriptorSet(&m_descriptorSet, 0);
			pContext->SetVertexBuffer(&m_VertexBuffer);
			pContext->DrawImmediate(6);
			pContext->EndGPUTag();
		}
	}


	static void SetVertex(int num, const usg::Vector3f &p, usg::PositionVertex& out)
	{
		out.x = p.x;
		out.y = p.y;
		out.z = p.z;
	}


	void Fader::Init(usg::GFXDevice* pDevice, const usg::RenderPassHndl& renderPass)
	{
		usg::PipelineStateDecl pipeline;
		pipeline.ePrimType = usg::PT_TRIANGLES;

		pipeline.inputBindings[0].Init(usg::GetVertexDeclaration(usg::VT_POSITION));
		pipeline.uInputBindingCount = 1;

		// Note fader assumes it is before/after any global drawing
		usg::DescriptorSetLayoutHndl matDescriptors = pDevice->GetDescriptorSetLayout(g_descriptorDecl);
		pipeline.layout.descriptorSets[0] = matDescriptors;
		pipeline.layout.uDescriptorSetCount = 1;

		usg::DepthStencilStateDecl& depth = pipeline.depthState;
		depth.bDepthEnable = false;
		depth.bDepthWrite = false;
		depth.eDepthFunc = usg::DEPTH_TEST_ALWAYS;

		usg::RasterizerStateDecl& ras = pipeline.rasterizerState;
		ras.eCullFace = usg::CULL_FACE_NONE;

		usg::AlphaStateDecl& alpha = pipeline.alphaState;
		alpha.bBlendEnable = true;
		alpha.SetColor0Only();
		alpha.srcBlend = usg::BLEND_FUNC_SRC_ALPHA;
		alpha.dstBlend = usg::BLEND_FUNC_ONE_MINUS_SRC_ALPHA;

		pipeline.pEffect = usg::ResourceMgr::Inst()->GetEffect(pDevice, "PostProcess.Fader");
		m_pipelineState = pDevice->GetPipelineState(renderPass, pipeline);
		m_descriptorSet.Init(pDevice, pDevice->GetDescriptorSetLayout(g_descriptorDecl));


		usg::PositionVertex verts[6];


		SetVertex(0, usg::Vector3f(-1.0f, -1.0f, 0.5f), verts[0]);
		SetVertex(1, usg::Vector3f(1.0f, 1.0f, 0.5f), verts[1]);
		SetVertex(2, usg::Vector3f(-1.0f, 1.0f, 0.5f), verts[2]);

		SetVertex(3, usg::Vector3f(1.0f, 1.0f, 0.5f), verts[3]);
		SetVertex(4, usg::Vector3f(1.0f, -1.0f, 0.5f), verts[4]);
		SetVertex(5, usg::Vector3f(-1.0f, -1.0f, 0.5f), verts[5]);

		usg::Color overrideColor(0.0f, 0.0f, 0.0f, sfAlpha);
		m_constants.Init(pDevice, g_fadeCBDecl);
		m_descriptorSet.SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL, &m_constants);
		FadeConstants* pConst = m_constants.Lock<FadeConstants>();
		pConst->fFade = sfAlpha;
		m_constants.Unlock();
		m_VertexBuffer.Init(pDevice, verts, sizeof(usg::PositionVertex), 6, "Fader");
		m_constants.UpdateData(pDevice);
		m_descriptorSet.UpdateDescriptors(pDevice);
	}

	void Fader::CleanUpDeviceData(usg::GFXDevice* pDevice)
	{
		m_VertexBuffer.Cleanup(pDevice);
		m_constants.Cleanup(pDevice);
		m_descriptorSet.Cleanup(pDevice);
	}


	void Fader::Update(float fElapsed)
	{
		float fChange = FADE_SPEED * fElapsed;
		switch (sFadeType)
		{
		case FADE_IN:
			sTime += fChange;

			if (sTime >= 1.0f)
			{
				sTime = 1.0f;
				sFadeType = 0;
				sfAlpha = 0.0f;
			}
			else
			{
				sfAlpha = (1.0f - sTime);
			}

			break;

		case FADE_OUT:
			sTime += fChange;

			if (sTime >= 1.0f)
			{
				sTime = 1.0f;
				sFadeType = 0;
				sfAlpha = 1.f;
			}
			else
			{
				sfAlpha = (sTime);
			}

			break;

		case FADE_WIPE:
			sTime += fChange * 2.0f;

			if (sTime >= 1.0f)
			{
				sTime = 1.0f;
				sFadeType = 0;
				sfAlpha = 0.0f;
			}
			else
			{
				if (sTime > 0.5f)
				{
					sfAlpha = sTime * 2.0f;
				}
				else
				{
					sfAlpha = 1.0f - ((sTime - 0.5f) * 2.f);
				}
			}

			break;
		}
	}

	void Fader::GPUUpdate(usg::GFXDevice* pDevice)
	{
		if(sfAlpha > 0.0f)
		{
			FadeConstants* pConst = m_constants.Lock<FadeConstants>();
			pConst->fFade = sfAlpha;
			m_constants.Unlock();
			m_constants.UpdateData(pDevice);
			m_descriptorSet.UpdateDescriptors(pDevice);
		}
	}


	void Fader::StartFade(int type, bool bWipeLower)
	{
		// Already faded
		m_bWipeLower = bWipeLower;
		if (type == FADE_OUT && sfAlpha >= 1.f)
			return;

		if (type != sFadeType)
		{
			sFadeType = type;
			sTime = 0.0;
		}
	}


	bool Fader::IsFading()
	{
		//return sAlpha != 0 && sAlpha != 255;
		return sFadeType != 0;
	}

	void Fader::Blackout()
	{
		sfAlpha = 1.f;
	}

	bool Fader::IsBlackout(void)
	{
		return (sfAlpha >= 1.f);
	}

}