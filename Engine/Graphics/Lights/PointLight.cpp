/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Physics/CollisionDetection.h"
#include "Engine/Graphics/Shadows/OmniShadow.h"
#include "PointLight.h"

namespace usg {

struct PointLightData
{
	Vector4f	vPos;
	Vector4f	vColorSpec;
	Vector4f	vRange;
	Vector4f	vAmbient;
	//float		fLightInvRange;
	//float		fLightRange;
};

struct PointLightConsts
{
	PointLightData light[1];
	Vector2f 	   vInvShadowDim;
};


static const ShaderConstantDecl g_pointLightDecl[] =
{
	SHADER_CONSTANT_ELEMENT(PointLightData, vPos,			CT_VECTOR_4, 1),
	SHADER_CONSTANT_ELEMENT(PointLightData, vColorSpec,	CT_VECTOR_4, 1),
	SHADER_CONSTANT_ELEMENT(PointLightData, vRange,		CT_VECTOR_4, 1),
	SHADER_CONSTANT_ELEMENT(PointLightData, vAmbient,		CT_VECTOR_4, 1),
	//SHADER_CONSTANT_ELEMENT( PointLightData, fLightRange,		CT_FLOAT, 1 )
	SHADER_CONSTANT_END()
};

static const ShaderConstantDecl g_pointLightConstsDecl[] = 
{
	SHADER_CONSTANT_STRUCT_ARRAY(PointLightConsts, light, g_pointLightDecl, 1 ),
	SHADER_CONSTANT_ELEMENT(PointLightConsts, vInvShadowDim, CT_VECTOR_2, 1 ),
	SHADER_CONSTANT_END()
};


const DescriptorDeclaration g_pointLightDesc[] =
{
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_CUSTOM, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_ALL),
	DESCRIPTOR_END()
};

PointLight::PointLight()
:Light(LIGHT_TYPE_POS)
{
	m_fNear = 0.0f;
	m_fFar = 1.0f;
	m_bAtten = true;
	m_pShadow = nullptr;
}

PointLight::~PointLight(void)
{
}

void PointLight::Init(GFXDevice* pDevice, Scene* pScene, bool bSupportsShadow)
{
	// TODO: Move this stuff out, it should really require the GFX device
	m_constants.Init(pDevice, g_pointLightConstsDecl);
	DescriptorSetLayoutHndl desc = pDevice->GetDescriptorSetLayout(g_pointLightDesc);
	m_descriptorSet.Init(pDevice, desc);
	m_descriptorSet.SetConstantSet(0, &m_constants);
	m_descriptorSet.UpdateDescriptors(pDevice);

	if (bSupportsShadow)
	{
		m_pShadow = vnew(ALLOC_OBJECT) OmniShadow;
		m_pShadow->Init(pDevice, pScene, 1024, 1024);
	}

	Light::Init(pDevice, pScene, bSupportsShadow);
}


void PointLight::CleanUp(GFXDevice* pDevice, Scene* pScene)
{
	if (m_pShadow)
	{
		m_pShadow->Cleanup(pDevice, pScene);
		vdelete m_pShadow;
		m_pShadow = nullptr;
	}
	m_constants.CleanUp(pDevice);
	m_descriptorSet.CleanUp(pDevice);
	Light::CleanUp(pDevice, pScene);
}

void PointLight::GPUUpdate(GFXDevice* pDevice)
{
	if (m_bDirty)
	{
		PointLightConsts* pData = m_constants.Lock<PointLightConsts>();
		const Color& diffuse = GetDiffuse();
		const Color& ambient = GetAmbient();
		pData->light[0].vColorSpec.Assign(diffuse.r(), diffuse.g(), diffuse.b(), GetSpecular().r());
		pData->light[0].vPos = GetPosition();
		// TODO: Add support for the near range (pre-falloff)
		pData->light[0].vRange.Assign(1.0f / GetFar(), GetFar(), 1.0f / GetNear(), GetNear());
		ambient.FillV4(pData->light[0].vAmbient);
		if(m_pShadow)
		{
			m_pShadow->GetTexDim(pData->vInvShadowDim);
			pData->vInvShadowDim = Vector2f(1.0f, 1.0f) / pData->vInvShadowDim;
		}
		else
		{
			pData->vInvShadowDim.Assign(1.0f, 1.0f);
		}
		m_constants.Unlock();
		m_constants.UpdateData(pDevice);
	}
	if (m_pShadow)
	{
		m_pShadow->GPUUpdate(pDevice, this);
	}
}


void PointLight::ShadowRender(GFXContext* pContext)
{
	if (m_pShadow)
	{
		m_pShadow->CreateShadowTex(pContext);
	}
}

const DescriptorSet* PointLight::GetShadowDescriptorSet()
{
	if (m_pShadow)
	{
		return m_pShadow->GetDescriptorSet();
	}
	return nullptr;
}

bool PointLight::IsInRange(AABB& testBox)
{
	return DetectCollision( m_colSphere, testBox );
}

bool PointLight::IsInVolume(const Vector4f &vPos) const
{
	return vPos.GetSquaredDistanceFrom(m_position) < (m_fFar * m_fFar);
}

const Vector4f& PointLight::GetPosition() const
{
	return m_position;
}

}

