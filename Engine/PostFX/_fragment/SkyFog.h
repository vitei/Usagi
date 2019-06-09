/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A cube map skybox
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_SKY_H_
#define _USG_GRAPHICS_SCENE_SKY_H_

#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/PostFX/PostEffect.h"

namespace usg {

class Model;
class Texture;
class Camera;
class RenderGroup;
class Scene;

class SkyFog : public PostEffect
{
public:
	SkyFog();
	virtual ~SkyFog();

	virtual void Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys, RenderTarget* pDst);
	virtual void CleanUp(GFXDevice* pDevice) override;
	virtual void SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst);
	virtual void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
	void SetTexture(GFXDevice* pDevice, const TextureHndl& skyTex, const TextureHndl& linDepth);
	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);

private:
	void MakeCube(GFXDevice* pDevice);
	void MakeSphere(GFXDevice* pDevice, float fScale);

	PRIVATIZE_COPY(SkyFog)

	// FIXME: We should be grabbing effects and textures from a resource manager
	RenderTarget*			m_pDestTarget;
	Material				m_materialNoFade;
	Material 				m_materialFade;
	IndexBuffer				m_indexBuffer;
	VertexBuffer			m_vertexBuffer;
	SamplerHndl				m_samplerHndl;
	SamplerHndl				m_linearSampl;
	bool					m_bUseDepthTex;
	bool					m_bValid;

};

}

#endif

