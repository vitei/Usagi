/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A visible light
*****************************************************************************/
#pragma once

#ifndef USG_GRAPHICS_LIGHT_SPOT_LIGHT_H
#define USG_GRAPHICS_LIGHT_SPOT_LIGHT_H

#include "Engine/Maths/Sphere.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Graphics/Device/DescriptorSet.h"
#include "Engine/Maths/MathUtil.h"
#include "Light.h"

namespace usg {

class Camera;
class LightMgr;
class AABB;
class ProjectionShadow;

class SpotLight : public Light
{
public:
	SpotLight();
	virtual ~SpotLight(void);

	virtual void	Init(GFXDevice* pDevice, Scene* pScene, bool bSupportsShadow) override;
	virtual void Cleanup(GFXDevice* pDevice, Scene* pScene) override;
	virtual void	SetPosition(const Vector4f &position);
	virtual void 	SetDirection(const Vector4f &direction);
	void 	SetOuterCutoff(float fOuterCutoff);
	void	SetInnerCutoff(float fInnerCutoff);

	float	GetNear() const			{ return m_fNear; }
	float	GetFar() const			{ return m_fFar; }
	float 	GetOuterCutoff() const  { return m_fOuterCutoff; }
	float	GetInnerCutoff() const 	{ return m_fInnerCutoff; }

	virtual const Vector4f&	GetPosition() const;
	virtual const Vector4f&	GetDirection() const;
	const usg::Sphere&		GetColSphere() const		{ return m_colSphere; }

	void SetRange(float fNear, float fFar);
	bool IsInRange(const AABB& testBox);
	bool IsInVolume(const Vector4f& vPos) const;
	void EnableAttenuation(bool bEnable) { m_bAtten = bEnable; }
	bool HasAttenuation() const { return m_bAtten; }

	void GPUUpdate(GFXDevice* pDevice) override;
	bool ShadowRender(GFXContext* pContext) override;
	
	const ConstantSet* GetConstantSet() const { return &m_constants; }
	const DescriptorSet* GetDescriptorSet(bool bWidthShadow) const;
	const ProjectionShadow* GetShadow() const { return m_pShadow; }

	static const DescriptorDeclaration* GetDescriptorDecl();
	static const DescriptorDeclaration* GetDescriptorDeclShadow();

protected:
	void UpdateSpherePosRadius();

	Matrix4x4 MakeRotationDir(const Vector4f& vDirection);

	ConstantSet			m_constants;
	DescriptorSet		m_descriptorSet;
	DescriptorSet		m_descriptorSetShadow;
	ProjectionShadow*	m_pShadow;

	Vector4f		m_position;
	Vector4f		m_direction;
	float			m_fNear;
	float			m_fFar;
	float			m_fOuterCutoff;
	float			m_fInnerCutoff;
	bool			m_bAtten;

	// Fixme, we're treating this as a sphere which is wasteful indeed
	usg::Sphere		m_colSphere;	

private:
	SpotLight(SpotLight &rhs) : Light(LIGHT_TYPE_SPOT) { ASSERT(false); } 
	SpotLight& operator=(SpotLight &rhs) { ASSERT(false); return *this; }
};


inline void SpotLight::SetPosition(const Vector4f &position)
{
	m_position = position;
	UpdateSpherePosRadius();
	m_bDirty = true;
}

inline void SpotLight::SetDirection(const Vector4f &direction)
{
	m_direction = direction;
	m_direction.Normalise();
	m_bDirty = true;
}

inline void SpotLight::SetOuterCutoff( float fOuterCutoff)
{
	m_fOuterCutoff = fOuterCutoff;
	UpdateSpherePosRadius();
	ASSERT(fOuterCutoff < Math::pi_over_2);
	m_bDirty = true;
}

inline void SpotLight::SetInnerCutoff( float fInnerCutoff)
{
	m_fInnerCutoff = fInnerCutoff;
	ASSERT(fInnerCutoff < Math::pi_over_2);
	m_bDirty = true;
}


inline void SpotLight::SetRange(float fNear, float fFar)
{
	ASSERT(fFar > fNear);
	UpdateSpherePosRadius();
	m_fNear = fNear;

	m_fFar = fFar;
	m_bDirty = true;
}

} // namespace usg

#endif // USG_GRAPHICS_LIGHT_SPOT_LIGHT_H
