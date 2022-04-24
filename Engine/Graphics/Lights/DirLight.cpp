/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Lights/DirLight.h"
#include "Engine/Graphics/Shadows/ShadowCascade.h"

namespace usg {

DirLight::DirLight()
:Light(LIGHT_TYPE_DIR),
m_pShadowCascade(nullptr)
{
}


DirLight::~DirLight()
{

}

void DirLight::Init(GFXDevice* pDevice, Scene* pScene, bool bSupportsShadow)
{
	if (bSupportsShadow)
	{
		m_pShadowCascade = vnew(ALLOC_OBJECT) ShadowCascade;
		m_pShadowCascade->Init(pDevice, pScene, this);
	}

	Light::Init(pDevice, pScene, bSupportsShadow);
}

void DirLight::Cleanup(GFXDevice* pDevice, Scene* pScene)
{
	if (m_pShadowCascade)
	{
		m_pShadowCascade->Cleanup(pDevice, pScene);
		vdelete m_pShadowCascade;
		m_pShadowCascade = nullptr;
	}
	Light::Cleanup(pDevice, pScene);
}


void DirLight::UpdateCascade(const usg::Camera& camera, uint32 uContextId)
{
	if (GetShadowEnabled())
	{
		m_pShadowCascade->Update(camera);
	}
}


void DirLight::GPUUpdate(GFXDevice* pDevice)
{
	if (GetShadowEnabled())
	{
		m_pShadowCascade->GPUUpdate(pDevice);
	}
}

bool DirLight::ShadowRender(GFXContext* pContext)
{
	if (GetShadowEnabled())
	{
		m_pShadowCascade->CreateShadowTex(pContext);
		return true;
	}
	return false;
}

void DirLight::SetDirection(const Vector4f &direction)
{
	ASSERT( direction.w == 0.0f );
	direction.GetNormalised(m_direction);
	m_bDirty = true;
}

const Vector4f& DirLight::GetDirection() const
{
	return m_direction;
}

void DirLight::SetShadowCastFlags(uint32 uFlags)
{
	if (m_pShadowCascade)
	{
		m_pShadowCascade->SetShadowCastFlags(uFlags);
	}
}

bool DirLight::operator < (const DirLight& rhs) const
{
	// Put the non shadowed lights first
	if (rhs.GetShadowEnabled() && !GetShadowEnabled())
	{
		return true;
	}

	// Now weight based on the intensity
	Vector4f vRhsDiff, vLhsDiffuse;
	GetDiffuse().FillV4(vLhsDiffuse);
	rhs.GetDiffuse().FillV4(vRhsDiff);
	if (vRhsDiff.MagnitudeSq() > vLhsDiffuse.MagnitudeSq())
	{
		return true;
	}

	return false;
}

}
