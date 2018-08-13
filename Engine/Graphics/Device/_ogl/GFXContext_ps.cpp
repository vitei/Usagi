/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/Textures/DepthStencilBuffer.h"
#include "Engine/Graphics/Device/DescriptorSet.h"
#include "Engine/Graphics/Device/DescriptorData.h"
#include "Engine/Graphics/Device/IHeadMountedDisplay.h"
#include "Engine/Graphics/Device/DescriptorSetLayout.h"
#include "Engine/Graphics/Device/PipelineState.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Graphics/Device/Display.h"
#include "Engine/Graphics/Effects/InputBinding.h"
#include API_HEADER(Engine/Graphics/Device, RasterizerState.h)
#include API_HEADER(Engine/Graphics/Device, RenderPass.h)
#include API_HEADER(Engine/Graphics/Device, AlphaState.h)
#include API_HEADER(Engine/Graphics/Device, DepthStencilState.h)
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include "Engine/Graphics/Device/GFXContext.h"

#define BUFFER_OFFSET(i) ((void*)(i))

#define OGL_ERROR_DEBUGGING 0

#if OGL_ERROR_DEBUGGING
#define ERROR_CHECK  CHECK_OGL_ERROR();
#else
#define ERROR_CHECK
#endif

namespace usg {

static const GLuint g_primTypeMapping[PT_COUNT] =
{
    GL_POINTS,				 //PT_POINTS = 0,
    GL_TRIANGLES,			//PT_TRIANGLES,
	GL_LINES,				//PT_LINES,
	GL_TRIANGLES_ADJACENCY,	//PT_TRIANGLES_ADJ,
	GL_LINES_ADJACENCY,		//PT_LINES_ADJ,
//	GL_LINES,				 //PT_LINES,
//  GL_TRIANGLE_STRIP,		//PT_TRIANGLESTRIPS,	
};

GFXContext_ps::GFXContext_ps()
{
}

GFXContext_ps::~GFXContext_ps()
{

}

void GFXContext_ps::Init(GFXDevice* pDevice, GFXContext* pParent, bool bDeferred, uint32 uSizeMul)
{
	CHECK_OGL_ERROR();
	m_pParent = pParent;

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    

	// Shared vertex data
	PositionVertex verts[4] =
	{
		{ -1.f,  1.f,  0.5f }, // 0 - TL
		{  1.f,  1.f,  0.5f }, // 1 - TR
		{ -1.f,  -1.f, 0.5f }, // 2 - BL
		{  1.f,  -1.f, 0.5f }, // 3 - BR
	};

	uint8 iIndices[6] = 
	{
		2, 1, 0, 2, 3, 1, 
	};
	
    SamplerDecl samplerDecl(SF_POINT, SC_CLAMP);

    m_samplerHndl = pDevice->GetSampler(samplerDecl);

	SamplerDecl samplerDecLUT(SF_LINEAR, SC_CLAMP);

	m_lutSamplerHndl = pDevice->GetSampler(samplerDecLUT);

	CHECK_OGL_ERROR();
}

void GFXContext_ps::Transfer(RenderTarget* pTarget, Display* pDisplay)
{
	GFXDevice_ps::GetOGLContext().SetActive(pDisplay->GetPlatform().GetWindowHndl());
	pDisplay->GetPlatform().Transfer(pTarget);
	ERROR_CHECK
}

void GFXContext_ps::TransferSpectatorDisplay(IHeadMountedDisplay* pHMD, Display* pDisplay)
{
	GFXDevice_ps::GetOGLContext().SetActive(pDisplay->GetPlatform().GetWindowHndl());
	pHMD->TransferSpectatorDisplay(m_pParent, pDisplay);
	ERROR_CHECK
}



void GFXContext_ps::TransferRect(RenderTarget* pTarget, Display* pDisplay, const GFXBounds& srcBounds, const GFXBounds& dstBounds)
{
	pDisplay->GetPlatform().TransferRect(pTarget, srcBounds, dstBounds);
	ERROR_CHECK
}


void GFXContext_ps::SetRenderTargetLayer(const RenderTarget* pTarget, uint32 uLayer)
{
	const RenderTarget_ps& rtPS = pTarget->GetPlatform();
	GLuint uDst = pTarget->GetPlatform().GetLayerFBO(uLayer);
    
	glBindFramebuffer(GL_FRAMEBUFFER, uDst);
//	glDrawBuffers(rtPS.GetTargetCount(), rtPS.GetBindings());  
//	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, pTarget->GetDepthStencilBuffer()->GetTexture()->GetPlatform().GetTexHndl(), 0, uLayer);	

	uint32 uClearFlags = pTarget->GetRenderPass().GetContents()->GetClearFlags();
	uint32 uGlFlags = 0;
	uGlFlags |= (RenderTarget::RT_FLAG_DEPTH&uClearFlags) != 0 ? GL_DEPTH_BUFFER_BIT : 0;
	uGlFlags |= (RenderTarget::RT_FLAG_STENCIL&uClearFlags) != 0 ? GL_STENCIL_BUFFER_BIT : 0;
//	glClearStencil(0x00);
//	glClearDepth(1.0f);
	if(uGlFlags)
	{
		SetDepthWrite();
		glClear(uGlFlags);
		RestorePipelineState();
	}
	ERROR_CHECK
}


void GFXContext_ps::SetRenderTarget(RenderTarget* pTarget)
{
	if(pTarget)
	{
		const RenderTarget_ps& rtPS = pTarget->GetPlatform();
		glBindFramebuffer(GL_FRAMEBUFFER, rtPS.GetOGLFBO());

		glDrawBuffers(rtPS.GetTargetCount(), rtPS.GetBindings());  

		uint32 uClearFlags = pTarget->GetRenderPass().GetContents()->GetClearFlags();
		if (uClearFlags)
		{
			ClearRenderTarget(pTarget, uClearFlags);
		}

	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	ERROR_CHECK
}

void GFXContext_ps::RenderToDisplay(Display* pDisplay, uint32 uClearFlags)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	uint32 uWidth, uHeight;
	pDisplay->GetActualDimensions(uWidth, uHeight, false);
	glViewport(0, 0, (int)uWidth, (int)uHeight);

	bool bDepth = false;
	bool bStencil = false;
	bool bUseDepthStencil = false;


	SetDepthWrite();

	for (int i = 0; i < MAX_COLOR_TARGETS; i++)
	{
		uint32 uTargetFlag = RenderTarget::RT_FLAG_COLOR_0 << i;
		if ((uClearFlags & uTargetFlag) )
		{
			usg::Color color(0.0f, 0.0f, 0.0f, 0.0f);
			glClearBufferfv(GL_COLOR, i, (float*)&color);
		}
	}

	if (uClearFlags & RenderTarget::RT_FLAG_DEPTH)
	{
		glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
	}


	RestorePipelineState();
}

void GFXContext_ps::EndRTDraw(const RenderTarget* pTarget)
{
	pTarget->GetPlatform().EndDraw();
	ERROR_CHECK
}

void GFXContext_ps::ApplyViewport(const RenderTarget* pActiveRT, const Viewport &viewport)
{
	ASSERT(pActiveRT!=NULL);
	glViewport(viewport.GetLeft(), viewport.GetBottom(), viewport.GetWidth(), viewport.GetHeight());
	ERROR_CHECK
}


void GFXContext_ps::SetEffect(const Effect* pBinding)
{
    pBinding->GetPlatform().Apply();
	ERROR_CHECK
}

void GFXContext_ps::ClearRenderTarget(RenderTarget* pRT, uint32 uFlags)
{
	// FIXME: Handle multiple render targets
	uint32 uGlFlags = 0;

	bool bDepth				= false;
	bool bStencil			= false;
	bool bUseDepthStencil	= false;

	if(!pRT || pRT->GetPlatform().GetDepthTarget()!=NULL)
	{
		uGlFlags |= (RenderTarget::RT_FLAG_DEPTH&uFlags)!=0 ? GL_DEPTH: 0;
		uGlFlags |= (RenderTarget::RT_FLAG_STENCIL&uFlags)!=0 ? GL_STENCIL : 0;

		bUseDepthStencil = uGlFlags != 0;
	}

	SetDepthWrite();

	for(int i=0; i<MAX_COLOR_TARGETS; i++)
	{
		uint32 uTargetFlag = RenderTarget::RT_FLAG_COLOR_0 << i;
		if( (uFlags & uTargetFlag) && (!pRT || pRT->GetColorBuffer(i)!=NULL) )
		{
			glColorMaski(i, 0xff, 0xff, 0xff, 0xff);
			const Color &color = pRT ? pRT->GetClearColor(i) : Color::Black;
			glClearColor(color.r(), color.g(), color.b(), color.a());	
			glClearBufferfv(GL_COLOR, i, (float*)&color);
		}
	}
	
	if(uGlFlags)
	{
		glClearBufferfi(GL_DEPTH_STENCIL, 0, 1.0f, 0);
	}


	RestorePipelineState();
	ERROR_CHECK
}


void GFXContext_ps::SetVertexBuffer(const VertexBuffer* pBuffer, const InputBinding* pBinding, uint32 uSlot)
{
 	glBindBuffer(GL_ARRAY_BUFFER, pBuffer->GetPlatform().GetBuffer());
	pBinding->GetPlatform().UpdateVBOMapping(uSlot);   
	ERROR_CHECK
}

void GFXContext_ps::DrawImmediate(uint32 uVertCount, uint32 uOffset)
{
	ERROR_CHECK
    glDrawArrays(m_ePrimitiveType, uOffset, uVertCount);
	ERROR_CHECK
}

void GFXContext_ps::DrawIndexed(const IndexBuffer* pBuffer, uint32 uStartIndex, uint32 uIndexCount, uint32 uInstances)
{
	const IndexBuffer_ps& ibPS = pBuffer->GetPlatform();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibPS.GetBuffer());

	glDrawElementsInstanced(m_ePrimitiveType, (GLsizei)uIndexCount, ibPS.GetType(), BUFFER_OFFSET((memsize)pBuffer->GetPlatform().GetIndexSize() * (memsize)uStartIndex), (GLsizei)uInstances);
	// glDrawElementsInstanced(g_primTypeMapping[pBuffer->GetPrimType()], uIndexCount, ibPS.GetType(), BUFFER_OFFSET(0), uInstances);
	// glDrawArraysInstanced(g_primTypeMapping[ibPS.GetPrimType()], uStartIndex, uIndexCount, uInstances);
	ERROR_CHECK
}



void GFXContext_ps::SetDescriptorSet(const DescriptorSet* pSet, uint32 uIndex)
{
	ASSERT(pSet->GetValid());
	const DescriptorSetLayout* pLayout = pSet->GetLayoutDesc();
	const DescriptorData* pData = pSet->GetData();

	for (uint32 i = 0; i < pLayout->GetDeclarationCount(); i++)
	{
		const DescriptorDeclaration* pDecl = pLayout->GetDeclaration(i);
		for (uint32 j = 0; j < pDecl->uCount; j++)
		{
			uint32 uDataIndex = pLayout->GetResourceIndex(i, j);
			const DescriptorData* pInst = &pData[uDataIndex];
			switch (pDecl->eDescriptorType)
			{
			case DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			{
				const Texture_ps& texPS = pInst->texData.tex->GetPlatform();
				Sampler* pSampler = pInst->texData.sampler.GetContents();
				texPS.BindInt(pDecl->uBinding + j);
				texPS.SetSampler(pDecl->uBinding + j, pSampler);
				break;
			}
			case DESCRIPTOR_TYPE_CONSTANT_BUFFER:
			case DESCRIPTOR_TYPE_CONSTANT_BUFFER_DYNAMIC:
				//if(pDecl->shaderType != SHADER_FLAG_PIXEL)
				ASSERT(pInst->pConstBuffer->GetDirty() == false);
				pInst->pConstBuffer->GetPlatform().Bind((ShaderConstantType)(pDecl->uBinding + j));
				ERROR_CHECK
				break;
			default:
				ASSERT(false);
			}

			uDataIndex++;
		}
	}
	ERROR_CHECK
}


void GFXContext_ps::SetScissorRect(const RenderTarget* pActiveTarget, uint32 uLeft, uint32 uBottom, uint32 uWidth, uint32 uHeight)
{
	glEnable(GL_SCISSOR_TEST);
	glScissor(uLeft, uBottom, uWidth, uHeight);
	ERROR_CHECK
}

void GFXContext_ps::DisableScissor(const RenderTarget* pActiveTarget, uint32 uLeft, uint32 uBottom, uint32 uWidth, uint32 uHeight)
{
	glDisable(GL_SCISSOR_TEST);
	ERROR_CHECK
}


void GFXContext_ps::SetBlendColor(const Color& blendColor)
{
	glBlendColor(blendColor.r(), blendColor.g(), blendColor.b(), blendColor.a());
}

void GFXContext_ps::SetPipelineState(PipelineStateHndl& hndl, PipelineStateHndl& prev)
{
	m_ePrimitiveType = g_primTypeMapping[hndl.GetContents()->GetPrimitiveType()];
	PipelineState& group = *hndl.GetContents();

	bool bAlpha, bDepth, bRas, bEffect;
	if (prev.GetContents())
	{
		hndl.GetContents()->GetStateDifferences(*prev.GetContents(), bAlpha, bDepth, bRas);
		bEffect = group.GetEffect() != prev.GetContents()->GetEffect();
	}
	else
	{
		bAlpha = true; bDepth = true; bRas = true; bEffect = true;
	}

	// TODO: Move all of this into platform specifc pipeline state
	if (bEffect)
	{
		SetEffect(group.GetEffect().get());
	}
	if (bAlpha)
	{
		group.GetAlphaStateInt()->Apply();
	}
	if (bDepth)
	{
		group.GetDepthStencilStateInt()->Apply();
	}

	if (bRas)
	{
		group.GetRasterizerStateInt()->Apply(false);
	}
	ERROR_CHECK
}

// FIXME: Push and pop are deprecated but this nonsense isn't necessary with anyother API
// stick with this until we move over to Vulkan/ DX12
void GFXContext_ps::SetDepthWrite()
{
#if 1
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glEnable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 0xFF, 0xFF);
	glColorMaski(0, true, true, true, true);
#endif
}

void GFXContext_ps::RestorePipelineState()
{
	PipelineStateHndl pipeline = m_pParent->GetActivePipeline();
	if (pipeline.IsValid())
	{
		m_pParent->InvalidatePipelineOnly();
		m_pParent->SetPipelineState(pipeline);
	}
}

}