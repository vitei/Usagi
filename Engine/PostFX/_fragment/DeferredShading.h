/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Perform the post processing on a scene rendered outputting
//	a G-buffer
*****************************************************************************/
#ifndef _USG_POSTFX_DEFERRED_SHADING_DEFERRED_SHADING_H_
#define _USG_POSTFX_DEFERRED_SHADING_DEFERRED_SHADING_H_

#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/PostFX/PostEffect.h"

namespace usg {

class PostFXSys;
class PointLight;
class SpotLight;
class ConstantSet;

class DeferredShading : public PostEffect
{
public:
	DeferredShading();
	~DeferredShading();

	virtual void Init(GFXDevice* pDevice, ResourceMgr* pResource, PostFXSys* pSys);
	virtual void Cleanup(GFXDevice* pDevice);
	virtual void Resize(GFXDevice* pDevice, uint32 uWidth, uint32 uHeight);
	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);
	virtual void SetDestTarget(GFXDevice* pDevice, RenderTarget* pDst);
	void SetSourceTarget(GFXDevice* pDevice, RenderTarget* pTarget);

	virtual bool ReadsTexture(Input eInput) const override;
	virtual bool WritesTexture(Input eInput) const;
	virtual void SetTexture(GFXDevice* pDevice, Input eInput, const TextureHndl& texture) override;
	virtual void PassDataSet(GFXDevice* pDevice) override;
	virtual bool LoadsTexture(Input eInput) const override;

private:


	struct VolumeShader
	{
		Pipeline	pLightingEffect;
		Pipeline	pLightingNoSpecEffect;
		Pipeline	pLightingShadowEffect;
		Pipeline	pLightingShadowLowEffect;
		Pipeline	pLightingFarPlaneEffect;
		Pipeline	pLightingFarPlaneNoSpecEffect;
		Pipeline	pLightingFarPlaneShadowEffect;
		Pipeline	pLightingFarPlaneShadowLowEffect;
		Pipeline	pStencilWriteEffect;
	};

	struct MeshData
	{
		const VertexBuffer*		pVB;
		const IndexBuffer*		pIB;
		const DescriptorSet*	pDescriptorSet;
		const DescriptorSet*	pShadowDescriptorSet;
	};

	void MakeSphere(GFXDevice* pDevice);
	void MakeCone(GFXDevice* pDevice);
	void MakeFrustum(GFXDevice* pDevice);

	void DrawProjectionLights(GFXContext* pContext);
	void DrawLightVolume(GFXContext* pContext, const MeshData& mesh, const VolumeShader& shaders, bool bSpecular = true, bool bShadow = false, bool bSmoothShadow = true);
	void DrawLightVolumeFarPlane(GFXContext* pContext, const MeshData& mesh, const VolumeShader& shaders, bool bSpecular = true, bool bShadow = false, bool bSmoothShadow = true);

	void DrawMesh(GFXContext* pContext, const MeshData& mesh);

	enum 
	{
		MAX_EXTRA_DIR_LIGHTS = 4,
	};

	PostFXSys*				m_pSys;
	RenderTarget*			m_pDestTarget;
	SamplerHndl				m_samplerHndl;
	SamplerHndl 			m_linSamplerHndl;

	Pipeline				m_baseDirPass;
	Pipeline				m_additionalShadowPass[MAX_EXTRA_DIR_LIGHTS];
	DescriptorSet			m_readDescriptors;
	
	VolumeShader			m_spotShaders;
	VolumeShader			m_sphereShaders;
	VolumeShader			m_projShaders;


	VertexBuffer			m_sphereVB;
	IndexBuffer				m_sphereIB;

	VertexBuffer			m_coneVB;
	IndexBuffer				m_coneIB;

	VertexBuffer			m_frustumMesh;
	IndexBuffer				m_frustumIB;
};

}

#endif