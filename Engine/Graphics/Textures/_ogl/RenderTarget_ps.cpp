/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Viewports/Viewport.h"
#include "Engine/Graphics/Textures/DepthStencilBuffer.h"
#include "Engine/Graphics/Textures/ColorBuffer.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Textures/TGAFile.h"
#include API_HEADER(Engine/Graphics/Textures, RenderTarget_ps.h)

namespace usg {

static RenderTarget_ps* g_pAttachedTarget = NULL;

GLenum g_bindingArray[] = 
{
	GL_COLOR_ATTACHMENT0,
	GL_COLOR_ATTACHMENT1,
	GL_COLOR_ATTACHMENT2,
	GL_COLOR_ATTACHMENT3,
	GL_COLOR_ATTACHMENT4,
	GL_COLOR_ATTACHMENT5,
	GL_COLOR_ATTACHMENT6,
	GL_COLOR_ATTACHMENT7
};

RenderTarget_ps::RenderTarget_ps()
{
	m_FBO		= GL_INVALID_INDEX;

	for (uint32 i = 0; i < MAX_COLOR_TARGETS; i++)
	{
		m_pColorBuffers[i] = nullptr;
	}
	m_pDepthTarget= NULL;
	m_uTargets = 0;
	m_uSlices = 0;
}

RenderTarget_ps::~RenderTarget_ps()
{
	ASSERT(m_FBO == GL_INVALID_INDEX);
}

void RenderTarget_ps::CleanUp(GFXDevice* pDevice)
{
	Reset();
}

void RenderTarget_ps::Reset()
{
	if (m_FBO != GL_INVALID_INDEX)
	{
		glDeleteFramebuffers(1, &m_FBO);
	}
	if (m_uSlices > 1)
	{
		glDeleteFramebuffers(m_uSlices, m_layerFBO);
	}

	m_FBO = GL_INVALID_INDEX;

	m_pDepthTarget = NULL;
	m_uTargets = 0;
	m_uSlices = 0;
}


void RenderTarget_ps::InitMRT(usg::GFXDevice* pDevice, uint32 uCount, ColorBuffer** ppColorBuffers, DepthStencilBuffer* pDepth)
{
	Reset();
	m_uTargets = uCount;
	m_pDepthTarget = pDepth;

	uint32 uWidth;
	uint32 uHeight;

	if (uCount > 0)
	{
		uWidth  = ppColorBuffers[0]->GetWidth();
		uHeight = ppColorBuffers[0]->GetHeight();
	}
	else
	{
		uWidth  = pDepth->GetWidth();
		uHeight = pDepth->GetHeight();
	}

	ASSERT(uCount < MAX_COLOR_TARGETS);

	m_fullScreenVP.InitViewport(0, 0, uWidth, uHeight);

	glGenFramebuffers(1, &m_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

	bool bLocCheck[MAX_COLOR_TARGETS] = {};

	m_uSlices = 1;

	for(uint32 i=0; i<uCount; i++)
	{
		ColorBuffer* pCol = ppColorBuffers[i];
		ASSERT(pCol->GetWidth() == uWidth);
		ASSERT(pCol->GetHeight() == uHeight);

		m_uSlices = pCol->GetSlices();

		uint32 uLoc = pCol->GetRTLoc();
		ASSERT(bLocCheck[uLoc]==false);
		bLocCheck[uLoc] = true;
		m_bindings[i] = g_bindingArray[uLoc];
		m_pColorBuffers[i] = pCol;
		pCol->GetTexture()->GetPlatform().BindInt(0);

		if(pCol->GetType() == ColorBuffer::TYPE_2D)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, m_bindings[i], GL_TEXTURE_2D, pCol->GetTexture()->GetPlatform().GetTexHndl(), 0);
		}
		else
		{
			// TODO: Rendering to cubes, we can do multiple faces at once, or one at a time
			glFramebufferTexture(GL_FRAMEBUFFER, m_bindings[i], pCol->GetTexture()->GetPlatform().GetTexHndl(), 0);
		}
	}
	if(uCount==0)
	{
		glDrawBuffer(GL_NONE);
	}

	if(pDepth)
	{
		m_uSlices = pDepth->GetSlices();
		// TODO: Could we theoretically have a stencil only target?
		GLenum attach = pDepth->HasStencil() ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
		if (m_uSlices == 1)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, attach, GL_TEXTURE_2D, pDepth->GetTexture()->GetPlatform().GetTexHndl(), 0);
		}
		else
		{
			glFramebufferTexture(GL_FRAMEBUFFER, attach, pDepth->GetTexture()->GetPlatform().GetTexHndl(), 0);
		}
		
	}

	CHECK_OGL_ERROR();
	ASSERT(glIsFramebuffer(m_FBO)==GL_TRUE);

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	ASSERT(status==GL_FRAMEBUFFER_COMPLETE);

	if (m_uSlices > 1)
	{
		ASSERT(uCount == 1 || pDepth);	// Only supporting one at the moment
		glGenFramebuffers(m_uSlices, m_layerFBO);
		
		for (uint32 i = 0; i < m_uSlices; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, m_layerFBO[i]);
			if (uCount>0)
			{
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ppColorBuffers[0]->GetTexture()->GetPlatform().GetTexHndl(), 0, i);
			}
			else
			{
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
			}
			if(pDepth)
			{
				GLenum attach = pDepth->HasStencil() ? GL_DEPTH_STENCIL_ATTACHMENT : GL_DEPTH_ATTACHMENT;
				glFramebufferTextureLayer(GL_FRAMEBUFFER, attach, pDepth->GetTexture()->GetPlatform().GetTexHndl(), 0, i);	
			}
			ASSERT(glIsFramebuffer(m_layerFBO[i]) == GL_TRUE);
		}
	}
	
	// Unbind our framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	
}

void RenderTarget_ps::Resize(GFXDevice* pDevice, uint32 uCount, ColorBuffer** ppColorBuffers, DepthStencilBuffer* pDepth)
{
	if ((m_uTargets > 0) || (m_pDepthTarget != NULL))
	{
		InitMRT(pDevice, m_uTargets, m_pColorBuffers, m_pDepthTarget);
	}
}

void RenderTarget_ps::SetClearColor(const Color& col, uint32 uTarget)
{
    
}



GLuint RenderTarget_ps::GetTexture(uint32 uId) const
{
	ASSERT(uId<m_uTargets);
	return m_pColorBuffers[uId]->GetTexture()->GetPlatform().GetTexHndl();
}

void RenderTarget_ps::BlitMSToScreen(const Viewport* pViewport, int fOffsetX, int fOffsetY)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBlitFramebuffer(0, 0, GetWidth(), GetHeight(),
		fOffsetX+pViewport->GetLeft(), fOffsetY, fOffsetX+pViewport->GetLeft()+pViewport->GetWidth(), fOffsetY+pViewport->GetHeight(),
		GL_COLOR_BUFFER_BIT,
		GL_NEAREST);
	
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	
}

bool RenderTarget_ps::SaveToFile(const char* szFileName)
{
	const uint32 textureWidth  = GetWidth();
	const uint32 textureHeight = GetHeight();

	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_FBO);

	BYTE* pixels = (BYTE*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_DEBUG, 3 * textureWidth * textureHeight);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadPixels(0, 0, textureWidth, textureHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	TGAFile fileOut;
	fileOut.SetData(pixels, CF_RGB_888, textureWidth, textureHeight);
	fileOut.Save(szFileName);

	mem::Free(MEMTYPE_STANDARD, pixels);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	return true;
}


// TODO: Assert has the various types
const TextureHndl& RenderTarget_ps::GetColorTexture(uint32 uTex) const
{
	ASSERT(uTex < m_uTargets);
	return m_pColorBuffers[uTex]->GetTexture();
}


const TextureHndl& RenderTarget_ps::GetDepthTexture() const
{
	ASSERT(m_pDepthTarget!=NULL);
	return m_pDepthTarget->GetTexture();
}

uint32 RenderTarget_ps::GetWidth() const
{
	ASSERT(m_pColorBuffers[0] || m_pDepthTarget);
	return m_pColorBuffers[0] ? m_pColorBuffers[0]->GetWidth() : m_pDepthTarget->GetWidth();
}

uint32 RenderTarget_ps::GetHeight() const
{
	ASSERT(m_pColorBuffers[0] || m_pDepthTarget);
	return m_pColorBuffers[0] ? m_pColorBuffers[0]->GetHeight() : m_pDepthTarget->GetHeight();
}


void RenderTarget_ps::EndDraw() const
{
	for(uint32 i=0; i<m_uTargets; i++)
	{
		if(m_pColorBuffers[i]->GetMipCount() > 1)
		{
			glActiveTexture(GL_TEXTURE15);	// Bind to a texture unit that won't interfere
			glBindTexture(GL_TEXTURE_2D, m_pColorBuffers[i]->GetPlatform().GetTexture()->GetPlatform().GetTexHndl());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			// FIXME: Should really only generate upto the requested mip level, but doens't matter if PC is slow
			glGenerateMipmap(GL_TEXTURE_2D);
		}
	}
}

}

