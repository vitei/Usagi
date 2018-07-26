/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A visible light
*****************************************************************************/
#ifndef _USG_GRAPHICS_LIGHT_PROJECTION_LIGHT_H_
#define _USG_GRAPHICS_LIGHT_PROJECTION_LIGHT_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Graphics/Device/DescriptorSet.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Scene/Frustum.h"
#include "Light.h"

namespace usg {

class Camera;
class LightMgr;
class Texture;
class ProjetionShadow;

class ProjectionLight : public Light
{
public:
	ProjectionLight();
	virtual ~ProjectionLight(void);

	virtual void Init(GFXDevice* pDevice, Scene* pScene, bool bSupportsShadow) override;
	virtual void CleanUp(GFXDevice* pDevice, Scene* pScene) override;
	void	SetProjectionMtx(const Matrix4x4& projMat);
	// Use one of the two, they will the reverse calculate the other
	void 	SetViewMatrix(const Matrix4x4& viewMat);
	void 	SetModelMatrix(const Matrix4x4& modelMat);
	void	SetTexture(GFXDevice* pDevice, const TextureHndl& pTexture);

	float	GetNear() const			{ return m_fNear; }
	float	GetFar() const			{ return m_fFar; }

	const Frustum&		GetFrustum() const		{ return m_frustum; }
	const Vector4f&		GetCorner(uint32 uCorner) const { ASSERT(uCorner < 8); return m_vCorners[uCorner]; }
	const Vector4f*		GetCorners() { return m_vCorners; }

	void SetRange(float fNear, float fFar);

	void GPUUpdate(GFXDevice* pDevice) override;

	const ConstantSet* GetConstantSet() const { return &m_constants; }
	const DescriptorSet* GetDescriptorSet() const;
	const Matrix4x4& GetModelMatrix() { return m_modelMat; }

	const Matrix4x4& GetProjMatrix() const { return m_projMat; }
	const Matrix4x4& GetViewMatrix() const { return m_viewMat; }

	void ShadowRender(GFXContext* pContext) override;

	static const DescriptorDeclaration* GetDescriptorDecl();

protected:
	 void RegenerateInternals();

	ConstantSet			m_constants;
	DescriptorSet		m_descriptorSet;
	DescriptorSet		m_descriptorSetShadow;
	SamplerHndl			m_samplerHndl;
	ProjectionShadow*	m_pShadow;

	float			m_fNear;
	float			m_fFar;
	Matrix4x4		m_projMat;
	Matrix4x4		m_viewMat;
	Matrix4x4		m_modelMat;
	Matrix4x4		m_textureMat;

	// Use the projection frustum for the culling of this
	Frustum			m_frustum;
	Vector4f		m_vCorners[8];

private:
	ProjectionLight(ProjectionLight &rhs) : Light(LIGHT_TYPE_PROJ) { ASSERT(false); } 
	ProjectionLight& operator=(ProjectionLight &rhs) { ASSERT(false); return *this; }
};




inline void ProjectionLight::SetRange(float fNear, float fFar)
{
	ASSERT(fFar > fNear);
	m_fNear = fNear;

	m_fFar = fFar;
	m_bDirty = true;
}

}

#endif
