/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector2f.h"
#include "Engine/PostFX/PostFXSys.h"
#include "Engine/Graphics/GFX.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Core/stl/vector.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "SetSceneTarget.h"

namespace usg {



SetSceneTarget::SetSceneTarget():
PostEffect()
{
	SetLayer(LAYER_SKY);
	SetPriority(0);
}


SetSceneTarget::~SetSceneTarget()
{

}

void SetSceneTarget::Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys)
{
	m_pSys = pSys;
	m_pDestTarget = nullptr;

}

void SetSceneTarget::Cleanup(GFXDevice* pDevice)
{

}


void SetSceneTarget::SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst)
{
	if (m_pDestTarget != pDst)
	{
		m_pDestTarget = pDst;
	}
}

bool SetSceneTarget::LoadsTexture(Input eInput) const
{
	// FIXME: Make dynamic for other necessary overrides
	return (eInput == PostEffect::Input::Color
			|| eInput == PostEffect::Input::Depth);
}

bool SetSceneTarget::ReadsTexture(Input eInput) const
{
	return false;
}

void SetSceneTarget::SetTexture(GFXDevice* pDevice, Input eInput, const TextureHndl& texture)
{

}

void SetSceneTarget::Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight)
{

}

bool SetSceneTarget::Draw(GFXContext* pContext, RenderContext& renderContext)
{
	if (!GetEnabled())
		return false;

	pContext->SetRenderTarget(m_pDestTarget);

	return true;
}


SetSceneLinDepthTarget::SetSceneLinDepthTarget() :
	SetSceneTarget()
{
	SetLayer(LAYER_DEFERRED_SHADING);
	SetPriority(255);
}


bool SetSceneLinDepthTarget::Draw(GFXContext* pContext, RenderContext& renderContext)
{
	if (!GetEnabled())
		return false;

	pContext->SetRenderTarget(nullptr);

	// TODO: This needs to be exposed through GFXContext even though for most API's it's unnecessary
#ifdef USE_VULKAN
	VkImageSubresourceRange subresourceRange{};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	pContext->GetPlatform().SetImageLayout(m_pDestTarget->GetColorBuffer(1)->GetTexture()->GetPlatform().GetImage(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, subresourceRange);
#endif

	pContext->SetRenderTarget(m_pDestTarget);

	return true;
}


bool SetSceneLinDepthTarget::LoadsTexture(Input eInput) const
{
	return (eInput == Input::Color || eInput == Input::Depth || eInput == Input::LinearDepth);
}

bool SetSceneLinDepthTarget::WritesTexture(Input eInput) const
{
	return (eInput == Input::Color || eInput == Input::Depth || eInput == Input::LinearDepth);
}

}
