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
	SetRenderMask(LAYER_DEFERRED_SHADING);
	SetLayer(LAYER_OPAQUE_UNLIT);
	SetPriority(255);
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

}
