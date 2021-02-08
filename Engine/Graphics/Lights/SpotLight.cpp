/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Physics/CollisionDetection.h"
#include "Engine/Graphics/Shadows/ProjectionShadow.h"
#include "SpotLight.h"

namespace usg {


struct SpotLightData
{
	Matrix4x4	mRotMat;
	Vector4f	vPos;
	Vector4f	vColorSpec;
	Vector4f	vAmbient;
	Vector4f	vRange;
	Vector4f	vDirection;
	float 		fCosSpotCutoff;
	float 		fOuterSpotRadius;
	float 		fCosInnerSpotCutoff;
};

static const ShaderConstantDecl g_spotLightDecl[] =
{
	SHADER_CONSTANT_ELEMENT(SpotLightData, mRotMat,			CT_MATRIX_44, 1),
	SHADER_CONSTANT_ELEMENT(SpotLightData, vPos,				CT_VECTOR_4, 1),
	SHADER_CONSTANT_ELEMENT(SpotLightData, vColorSpec,			CT_VECTOR_4, 1),
	SHADER_CONSTANT_ELEMENT(SpotLightData, vAmbient,			CT_VECTOR_4, 1),
	SHADER_CONSTANT_ELEMENT(SpotLightData, vRange,				CT_VECTOR_4, 1),
	SHADER_CONSTANT_ELEMENT(SpotLightData, vDirection,			CT_VECTOR_4, 1),
	SHADER_CONSTANT_ELEMENT(SpotLightData,	fCosSpotCutoff,		CT_FLOAT, 1),
	SHADER_CONSTANT_ELEMENT(SpotLightData,	fOuterSpotRadius,	CT_FLOAT, 1),
	SHADER_CONSTANT_ELEMENT(SpotLightData,  fCosInnerSpotCutoff,CT_FLOAT, 1),
	SHADER_CONSTANT_END()
};

const DescriptorDeclaration g_spotLightDescDecl[] =
{
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_CUSTOM_3, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_ALL),
	DESCRIPTOR_END()
};

const DescriptorDeclaration g_spotLightShadowDescDecl[] =
{
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_CUSTOM_3, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_ALL),
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_SHADOW, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_ALL),
	DESCRIPTOR_ELEMENT(15, DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_ALL),
	DESCRIPTOR_END()
};


const DescriptorDeclaration* SpotLight::GetDescriptorDecl()
{
	return g_spotLightDescDecl;
}

const DescriptorDeclaration* SpotLight::GetDescriptorDeclShadow()
{
	return g_spotLightShadowDescDecl;
}

SpotLight::SpotLight()
:Light(LIGHT_TYPE_SPOT)
{
	m_position.Assign(0.0f, 0.0f, 0.0f, 1.0f);
	m_direction.Assign(0.0f, 0.0f, 0.0f, 0.0f);
	m_fNear = 0.0f;
	m_fFar = 10.0f;
	m_bAtten = true;
	m_fOuterCutoff = 15.0f;
	m_fInnerCutoff = 5.0f;
	m_pShadow = nullptr;
}

SpotLight::~SpotLight(void)
{
}

void SpotLight::Init(GFXDevice* pDevice, Scene* pScene, bool bSupportsShadow)
{
	m_constants.Init(pDevice, g_spotLightDecl);
	DescriptorSetLayoutHndl desc = pDevice->GetDescriptorSetLayout(g_spotLightDescDecl);
	m_descriptorSet.Init(pDevice, desc);
	m_descriptorSet.SetConstantSet(0, &m_constants);
	m_descriptorSet.UpdateDescriptors(pDevice);

	if (bSupportsShadow)
	{
		m_pShadow = vnew(ALLOC_OBJECT) ProjectionShadow;
		m_pShadow->Init(pDevice, pScene, 1024, 1024);
		SamplerDecl samp(SAMP_FILTER_LINEAR, SAMP_WRAP_CLAMP);
		samp.bEnableCmp = true;
		samp.eCmpFnc = CF_LESS;

		desc = pDevice->GetDescriptorSetLayout(g_spotLightShadowDescDecl);
		m_descriptorSetShadow.Init(pDevice, desc);
		m_descriptorSetShadow.SetConstantSet(0, &m_constants);
		m_descriptorSetShadow.SetConstantSet(1, m_pShadow->GetShadowConstants());
		m_descriptorSetShadow.SetImageSamplerPair(2, m_pShadow->GetShadowTexture(), pDevice->GetSampler(samp));
		m_descriptorSetShadow.UpdateDescriptors(pDevice);
	}

	Light::Init(pDevice, pScene, bSupportsShadow);
}

void SpotLight::Cleanup(GFXDevice* pDevice, Scene* pScene)
{
	m_constants.Cleanup(pDevice);
	m_descriptorSet.Cleanup(pDevice);
	if (m_pShadow)
	{
		m_descriptorSetShadow.Cleanup(pDevice);
		m_pShadow->Cleanup(pDevice, pScene);
		m_pShadow = nullptr;
	}
}

void SpotLight::GPUUpdate(GFXDevice* pDevice)
{
	SpotLightData* pData = m_constants.Lock<SpotLightData>();
	const Color& diffuse = GetDiffuse();
	pData->mRotMat = MakeRotationDir(GetDirection());
	pData->vColorSpec.Assign(diffuse.r(), diffuse.g(), diffuse.b(), GetSpecular().r());
	pData->vPos = GetPosition();
	pData->vDirection = GetDirection();
	// TODO: Add support for the near range (pre-falloff)
	pData->vRange.Assign(1.0f / GetFar(), GetFar(), 0.0f, 0.0f);
	pData->fCosSpotCutoff = cosf(GetOuterCutoff());

	// The radius of the geometrys termination point
	pData->fOuterSpotRadius = tanf(GetOuterCutoff()) * GetFar();

	pData->fCosInnerSpotCutoff = cosf(GetInnerCutoff());

	m_constants.Unlock();
	m_constants.UpdateData(pDevice);

	if (m_pShadow)
	{
		m_pShadow->GPUUpdate(pDevice, this);
		m_descriptorSetShadow.UpdateDescriptors(pDevice);
	}
	else
	{
		m_descriptorSet.UpdateDescriptors(pDevice);
	}

}


void SpotLight::ShadowRender(GFXContext* pContext)
{
	if (m_pShadow)
	{
		m_pShadow->CreateShadowTex(pContext);
	}
}

bool SpotLight::IsInRange(const AABB& testBox)
{
	return DetectCollision( m_colSphere, testBox );
}

bool SpotLight::IsInVolume(const Vector4f &vPos) const
{
	return vPos.GetSquaredDistanceFrom(m_position) < (m_fFar * m_fFar);
}

const Vector4f& SpotLight::GetPosition() const
{
	return m_position;
}

const Vector4f& SpotLight::GetDirection() const
{
	return m_direction;
}

void SpotLight::UpdateSpherePosRadius()
{
	// FIXME: Do something smarter than this
	m_colSphere.SetPos(m_position.v3());
	m_colSphere.SetRadius(m_fFar);
}


Matrix4x4 SpotLight::MakeRotationDir(const Vector4f& vDirection)
{
	Vector4f vUp(0.0f, 1.0f, 0.0f, 0.0f);
	Vector4f vRight(1.0f, 0.0f, 0.0f, 0.0f);
	Vector4f vAxis2;

	if (fabsf(DotProduct(vUp, vDirection)) < 0.9f)
	{
		vAxis2 = vUp;
	}
	else
	{
		vAxis2 = vRight;
	}

	Vector4f vXAxis = CrossProduct(vAxis2, vDirection);
	vXAxis.Normalise();

	Vector4f vYAxis = CrossProduct(vDirection, vXAxis);
	vYAxis.Normalise();

	Matrix4x4 mTmp;
	mTmp.LoadIdentity();

	mTmp.SetRight(vXAxis);
	mTmp.SetUp(vYAxis);
	mTmp.SetFace(vDirection);

	return mTmp;
}

const DescriptorSet* SpotLight::GetDescriptorSet(bool bWithShadow) const
{
	// FIXME: If shadow is disabled we need to return the standard set
	if (GetShadowEnabled() && bWithShadow)
	{
		return &m_descriptorSetShadow;
	}
	else
	{
		return &m_descriptorSet;
	}
}


}

