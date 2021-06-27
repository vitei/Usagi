/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector2f.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Bloom.h"

namespace usg {

const int BLOOM_SAMPLE_COUNT = 15;


struct BloomConstants
{
	Vector4f	vOffsets[BLOOM_SAMPLE_COUNT];
};

struct BloomFinalConstants
{
	Vector4f	vBloomScale;
};

struct BloomBrightPassConstants
{
	Vector4f	vMiddleGray;
};

static const ShaderConstantDecl g_bloomConstantDef[] = 
{
	SHADER_CONSTANT_ELEMENT( BloomConstants, vOffsets,	CT_VECTOR_4, BLOOM_SAMPLE_COUNT ),
	SHADER_CONSTANT_END()
};

static const ShaderConstantDecl g_bloomFinalConstantDef[] = 
{
	SHADER_CONSTANT_ELEMENT( BloomFinalConstants, vBloomScale,	CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_END()
};

static const ShaderConstantDecl g_brightpassConstantDef[] = 
{
	SHADER_CONSTANT_ELEMENT( BloomBrightPassConstants, vMiddleGray,	CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_END()
};

static const DescriptorDeclaration g_descriptor1Tex[] =
{
	DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_ALL),
	DESCRIPTOR_END()
};

static const DescriptorDeclaration g_descriptor2Tex[] =
{
	DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(1,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_END()
};


Bloom::Bloom():
PostEffect(), m_pSys(NULL)
{
	SetRenderMask(RENDER_MASK_POST_EFFECT);
	SetLayer(LAYER_POST_PROCESS);
	SetPriority(20);
}


Bloom::~Bloom()
{

}

void Bloom::Init(GFXDevice* pDevice, ResourceMgr* pRes, PostFXSys* pSys, RenderTarget* pDst)
{
	m_pSys = pSys;
	m_pDestTarget = pDst;

	SamplerDecl pointDecl(SAMP_FILTER_POINT, SAMP_WRAP_CLAMP);
	SamplerDecl linearDecl(SAMP_FILTER_LINEAR, SAMP_WRAP_CLAMP);

	m_pointSampler = pDevice->GetSampler(pointDecl);
	m_linearSampler = pDevice->GetSampler(linearDecl);

	// Directional lighting, test against the terrain stencil value of 2
	PipelineStateDecl pipelineDecl;
	pipelineDecl.inputBindings[0].Init(GetVertexDeclaration(VT_POSITION));
	pipelineDecl.uInputBindingCount = 1;
	pipelineDecl.alphaState.SetColor0Only();

	RasterizerStateDecl& rasDecl = pipelineDecl.rasterizerState;
	rasDecl.eCullFace = CULL_FACE_NONE;

	DescriptorSetLayoutHndl desc1Tex = pDevice->GetDescriptorSetLayout(g_descriptor1Tex);
	DescriptorSetLayoutHndl desc2Tex = pDevice->GetDescriptorSetLayout(g_descriptor2Tex);

	pipelineDecl.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
	pipelineDecl.layout.uDescriptorSetCount = 2;

	// Initialise the render targets
	uint32 uScrWidth = pSys->GetFinalTargetWidth();
	uint32 uScrHeight = pSys->GetFinalTargetHeight();

	m_scaledSceneTex.Init(pDevice, uScrWidth / 4, uScrHeight / 4, CF_RGB_HDR, SAMPLE_COUNT_1_BIT, TU_FLAGS_OFFSCREEN_COLOR);
	m_brightPassTex.Init(pDevice, uScrWidth / 4, uScrHeight / 4, CF_RGB_HDR, SAMPLE_COUNT_1_BIT, TU_FLAGS_OFFSCREEN_COLOR);
	m_bloomSourceTex.Init(pDevice, uScrWidth / 8, uScrHeight / 8, CF_RGB_HDR, SAMPLE_COUNT_1_BIT, TU_FLAGS_OFFSCREEN_COLOR);

	m_scaledSceneRT.Init(pDevice, &m_scaledSceneTex);
	m_brightPassRT.Init(pDevice, &m_brightPassTex);
	m_bloomSourceRT.Init(pDevice, &m_bloomSourceTex);
	RenderTarget::RenderPassFlags flags;
	flags.uClearFlags = 0;
	flags.uStoreFlags = RenderTarget::RT_FLAG_COLOR_0;
	flags.uShaderReadFlags = RenderTarget::RT_FLAG_COLOR_0;
	m_scaledSceneRT.InitRenderPass(pDevice, flags);
	m_brightPassRT.InitRenderPass(pDevice, flags);
	m_bloomSourceRT.InitRenderPass(pDevice, flags);

	for (int i = 0; i < BLOOM_PASS_TEXTURES; i++)
	{
		m_bloomTex[i].Init(pDevice, uScrWidth / 8, uScrHeight / 8, CF_RGB_HDR, SAMPLE_COUNT_1_BIT, TU_FLAGS_OFFSCREEN_COLOR);
		m_bloomRT[i].Init(pDevice, &m_bloomTex[i]);
		m_bloomRT[i].InitRenderPass(pDevice, flags);
	}

	pipelineDecl.layout.descriptorSets[1] = desc1Tex;
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "PostProcess.BloomMain");
	m_bloomEffect = pDevice->GetPipelineState(m_bloomRT[1].GetRenderPass(), pipelineDecl);
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "PostProcess.BloomBrightPass");
	m_brightPassEffect	= pDevice->GetPipelineState(m_brightPassRT.GetRenderPass(), pipelineDecl);
	pipelineDecl.pEffect = pRes->GetEffect(pDevice, "PostProcess.BloomFinal");
	pipelineDecl.layout.descriptorSets[1] = desc2Tex;
	m_finalPassEffect = pDevice->GetPipelineState(m_pDestTarget->GetRenderPass(), pipelineDecl);

	m_gaussBlurPipeline = pSys->GetPlatform().GetGaussBlurPipeline(pDevice, pRes, m_bloomSourceRT.GetRenderPass());
	m_downscalePipeline = pSys->GetPlatform().GetDownscale4x4Pipeline(pDevice, pRes, m_scaledSceneRT.GetRenderPass());

	m_constants[PASS_HOR_BLOOM].Init(pDevice, g_bloomConstantDef);
	m_constants[PASS_VER_BLOOM].Init(pDevice, g_bloomConstantDef);
	m_constants[PASS_FINAL].Init(pDevice, g_bloomFinalConstantDef);
	m_constants[PASS_BRIGHT_PASS].Init(pDevice, g_bloomFinalConstantDef);

	m_descriptors[PASS_BRIGHT_PASS].Init(pDevice, desc1Tex);
	m_descriptors[PASS_HOR_BLOOM].Init(pDevice, desc1Tex);
	m_descriptors[PASS_VER_BLOOM].Init(pDevice, desc1Tex);
	m_descriptors[PASS_FINAL].Init(pDevice, desc2Tex);


	BloomBrightPassConstants* pConsts = m_constants[PASS_BRIGHT_PASS].Lock<BloomBrightPassConstants>();
	pConsts->vMiddleGray.x = 0.005f;
	m_constants[PASS_BRIGHT_PASS].Unlock();
	m_constants[PASS_BRIGHT_PASS].UpdateData(pDevice);

	BloomFinalConstants* pFinalConsts = m_constants[PASS_FINAL].Lock<BloomFinalConstants>();
	pFinalConsts->vBloomScale.x = 1.0f;
	m_constants[PASS_FINAL].Unlock();
	m_constants[PASS_FINAL].UpdateData(pDevice);

	SetOffsetsHor(pDevice, m_bloomRT[2].GetWidth(), 3.0f, 2.0f);
	SetOffsetsVer(pDevice, m_bloomRT[1].GetHeight(), 3.0f, 2.0f);


	pSys->GetPlatform().SetupDownscale4x4(pDevice, m_constants[PASS_4X4], m_descriptors[PASS_4X4], uScrWidth, uScrHeight);
	pSys->GetPlatform().SetupGaussBlur(pDevice, m_constants[PASS_GUASS_BRIGHT_PASS], m_descriptors[PASS_GUASS_BRIGHT_PASS],  m_brightPassRT.GetWidth() / 2, m_brightPassRT.GetHeight() / 2, 1.0f);
	pSys->GetPlatform().SetupGaussBlur(pDevice, m_constants[PASS_GUASS_BLOOM_SRC], m_descriptors[PASS_GUASS_BLOOM_SRC], m_bloomSourceRT.GetWidth() / 2, m_bloomSourceRT.GetHeight() / 2, 1.0f);

	m_descriptors[PASS_BRIGHT_PASS].SetImageSamplerPair(0, m_scaledSceneRT.GetColorTexture(), m_pointSampler);
	m_descriptors[PASS_GUASS_BRIGHT_PASS].SetImageSamplerPair(0, m_brightPassRT.GetColorTexture(), m_pointSampler);
	m_descriptors[PASS_GUASS_BLOOM_SRC].SetImageSamplerPair(0, m_bloomSourceRT.GetColorTexture(), m_pointSampler);
	m_descriptors[PASS_HOR_BLOOM].SetImageSamplerPair(0, m_bloomRT[2].GetColorTexture(), m_pointSampler);
	m_descriptors[PASS_VER_BLOOM].SetImageSamplerPair(0, m_bloomRT[1].GetColorTexture(), m_pointSampler);
	m_descriptors[PASS_FINAL].SetImageSamplerPair(1, m_bloomRT[0].GetColorTexture(), m_linearSampler);

	for (uint32 i = 0; i < PASS_COUNT; i++)
	{
		m_descriptors[i].SetConstantSetAtBinding(SHADER_CONSTANT_MATERIAL, &m_constants[i]);
	}

	for (uint32 i = PASS_BRIGHT_PASS; i < PASS_FINAL; i++)
	{
		// These passes don't change
		m_descriptors[i].UpdateDescriptors(pDevice);
	}
}


void Bloom::Cleanup(GFXDevice* pDevice)
{
	for (uint32 i = 0; i < PASS_COUNT; i++)
	{
		m_descriptors[i].Cleanup(pDevice);
	}

	for (uint32 i = 0; i < PASS_COUNT; i++)
	{
		m_constants[i].Cleanup(pDevice);
	}

	for (auto& it : m_bloomTex)
	{
		it.Cleanup(pDevice);
	}

	for (auto& it : m_bloomRT)
	{
		it.Cleanup(pDevice);
	}

	m_scaledSceneTex.Cleanup(pDevice);
	m_brightPassTex.Cleanup(pDevice);
	m_bloomSourceTex.Cleanup(pDevice);

	m_scaledSceneRT.Cleanup(pDevice);
	m_brightPassRT.Cleanup(pDevice);
	m_bloomSourceRT.Cleanup(pDevice);
}

void Bloom::SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst)
{
	m_pDestTarget = pDst;
}



void Bloom::Resize(GFXDevice* pDevice, uint32 uScrWidth, uint32 uSrcHeight)
{
	// TODO: Resize the intermediate targets
	m_scaledSceneTex.Resize(pDevice, uScrWidth / 4, uSrcHeight / 4);
	m_brightPassTex.Resize(pDevice, uScrWidth / 4, uSrcHeight / 4);
	m_bloomSourceTex.Resize(pDevice, uScrWidth / 8, uSrcHeight / 8);

	m_scaledSceneRT.Resize(pDevice);
	m_brightPassRT.Resize(pDevice);
	m_bloomSourceRT.Resize(pDevice);

	for (int i = 0; i < BLOOM_PASS_TEXTURES; i++)
	{
		m_bloomTex[i].Resize(pDevice, uScrWidth / 8, uScrWidth / 8);
		m_bloomRT[i].Resize(pDevice);
	}

	for (uint32 i = 0; i < PASS_COUNT; i++)
	{
		// These passes don't change
		m_descriptors[i].UpdateDescriptors(pDevice);
	}
}

void Bloom::SetSourceTarget(GFXDevice* pDevice, RenderTarget* pTarget)
{
	m_descriptors[PASS_4X4].SetImageSamplerPair(0, pTarget->GetColorTexture(0), m_linearSampler);
	m_descriptors[PASS_FINAL].SetImageSamplerPair(0, pTarget->GetColorTexture(0), m_pointSampler);
	m_descriptors[PASS_4X4].UpdateDescriptors(pDevice);
	m_descriptors[PASS_FINAL].UpdateDescriptors(pDevice);
}


bool Bloom::Draw(GFXContext* pContext, RenderContext& renderContext)
{
	if (!GetEnabled())
		return false;

	pContext->BeginGPUTag("Bloom", Color::Green);
	// Downscale 4x4
	pContext->SetRenderTarget(&m_scaledSceneRT);
	pContext->SetPipelineState(m_downscalePipeline);
	pContext->SetDescriptorSet(&m_descriptors[PASS_4X4], 1);
	m_pSys->DrawFullScreenQuad(pContext);

	// Bright pass
	pContext->SetRenderTarget(&m_brightPassRT);
	pContext->SetPipelineState(m_brightPassEffect);
	pContext->SetDescriptorSet(&m_descriptors[PASS_BRIGHT_PASS], 1);
	m_pSys->DrawFullScreenQuad(pContext);

	// Gauss blur to the bloom source
	pContext->SetRenderTarget(&m_bloomSourceRT);
	pContext->SetPipelineState(m_gaussBlurPipeline);
	pContext->SetDescriptorSet(&m_descriptors[PASS_GUASS_BRIGHT_PASS], 1);
	m_pSys->DrawFullScreenQuad(pContext);
	

	// Perform another gaussian blur
	pContext->SetRenderTarget(&m_bloomRT[2]);
	pContext->SetPipelineState(m_gaussBlurPipeline);
	pContext->SetDescriptorSet(&m_descriptors[PASS_GUASS_BLOOM_SRC], 1);
	m_pSys->DrawFullScreenQuad(pContext);
	
	// Perform the horizontal bloom
	pContext->SetRenderTarget(&m_bloomRT[1]);
	pContext->SetPipelineState(m_bloomEffect);
	pContext->SetDescriptorSet(&m_descriptors[PASS_HOR_BLOOM], 1);
	m_pSys->DrawFullScreenQuad(pContext);

	// Perform the vertical bloom
	pContext->SetRenderTarget(&m_bloomRT[0]);
	pContext->SetPipelineState(m_bloomEffect);
	pContext->SetDescriptorSet(&m_descriptors[PASS_VER_BLOOM], 1);
	m_pSys->DrawFullScreenQuad(pContext);

	// Perform the vertical bloom, transferring into a non HDR destination RT
	pContext->SetRenderTarget(m_pDestTarget);
	pContext->SetDescriptorSet(&m_descriptors[PASS_FINAL], 1);
	pContext->SetPipelineState(m_finalPassEffect);
	m_pSys->DrawFullScreenQuad(pContext);

	pContext->EndGPUTag();

	return true;
}

void Bloom::SetOffsetsHor(GFXDevice* pDevice, uint32 uWidth, float fDeviation, float fMultiplier)
{
	float sampleOffsets[BLOOM_SAMPLE_COUNT];

	BloomConstants* pConstants = m_constants[PASS_HOR_BLOOM].Lock<BloomConstants>();
	Vector4f* pOffsets	= pConstants->vOffsets;
	float fWeights[BLOOM_SAMPLE_COUNT];

	GetOffsetsAndWeights(uWidth, fDeviation, fMultiplier, fWeights, sampleOffsets);

    for( int i = 0; i < BLOOM_SAMPLE_COUNT; i++ )
    {
		pOffsets[i].x	= sampleOffsets[i];
		pOffsets[i].y	= 0.0f;
		pOffsets[i].z	= fWeights[i];
		pOffsets[i].w	= 0.0f;
    }

	m_constants[PASS_HOR_BLOOM].Unlock();
	m_constants[PASS_HOR_BLOOM].UpdateData(pDevice);
}


void Bloom::SetOffsetsVer(GFXDevice* pDevice, uint32 uHeight, float fDeviation, float fMultiplier)
{
	float sampleOffsets[BLOOM_SAMPLE_COUNT];

	BloomConstants* pConstants = m_constants[PASS_VER_BLOOM].Lock<BloomConstants>();
	float fWeights[BLOOM_SAMPLE_COUNT];
	Vector4f* pOffsets	= pConstants->vOffsets;

	GetOffsetsAndWeights(uHeight, fDeviation, fMultiplier, fWeights, sampleOffsets);

    for( int i = 0; i < BLOOM_SAMPLE_COUNT; i++ )
    {
		pOffsets[i].x	= 0.0f;
		pOffsets[i].y	= sampleOffsets[i];
		pOffsets[i].z	= fWeights[i];
		pOffsets[i].w	= 0.0f;
    }

	m_constants[PASS_VER_BLOOM].Unlock();
	m_constants[PASS_VER_BLOOM].UpdateData(pDevice);
}

void Bloom::GetOffsetsAndWeights(uint32 texSize, float fDeviation, float fMultiplier, float* pWeights, float* pOffsets)
{
    int i = 0;
    float tu = 1.0f / ( float )texSize;

    // Fill the center texel
    float weight = fMultiplier * m_pSys->GetPlatform().GaussianDistribution( 0, 0, fDeviation );
    pWeights[0] = weight;

    pOffsets[0] = 0.0f;

    // Fill the first half
    for( i = 1; i < 8; i++ )
    {
        // Get the Gaussian intensity for this offset
        weight = fMultiplier * m_pSys->GetPlatform().GaussianDistribution( ( float )i, 0, fDeviation );
        pOffsets[i] = i * tu;

        pWeights[i] = weight;
    }

    // Mirror to the second half
    for( i = 8; i < 15; i++ )
    {
        memcpy(&pWeights[i], &pWeights[(i-7)], sizeof(float));
        pOffsets[i] = -pOffsets[i - 7];
    }
}



bool Bloom::ReadsTexture(Input eInput) const
{
	switch (eInput)
	{
	case PostEffect::Input::Color:
		return true;

	default:
		return false;

	}
}


bool Bloom::LoadsTexture(Input eInput) const
{
	return false;
}

void Bloom::SetTexture(GFXDevice* pDevice, Input eInput, const TextureHndl& texture)
{
	m_descriptors[PASS_4X4].SetImageSamplerPair(0, texture, m_linearSampler);
	m_descriptors[PASS_FINAL].SetImageSamplerPair(0, texture, m_pointSampler);
	m_descriptors[PASS_4X4].UpdateDescriptors(pDevice);
	m_descriptors[PASS_FINAL].UpdateDescriptors(pDevice);
}

}

