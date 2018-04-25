/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Physics/CollisionDetection.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/RenderState.h"
#include "Engine/Graphics/Shadows/ProjectionShadow.h"
#include "ProjectionLight.h"

namespace usg {

struct ProjLightData
{
	Matrix4x4	mModelMat;
	Matrix4x4	mTextureMat;
	Matrix4x4	mViewProjInv;
	Vector4f	vPos;			// Extracted from the view matrix
	Vector4f	vDirection;		// Extracted from the view matrix
	Vector4f	vColorSpec;
	Vector4f	vAmbient;
	Vector4f	vRange;
};

static const ShaderConstantDecl g_projLightDecl[] = 
{
	SHADER_CONSTANT_ELEMENT( ProjLightData, mModelMat,			CT_MATRIX_44, 1 ),
	SHADER_CONSTANT_ELEMENT( ProjLightData, mTextureMat,		CT_MATRIX_44, 1 ),
	SHADER_CONSTANT_ELEMENT( ProjLightData, mViewProjInv,		CT_MATRIX_44, 1 ),
	SHADER_CONSTANT_ELEMENT( ProjLightData, vPos,				CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_ELEMENT( ProjLightData, vDirection,			CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_ELEMENT( ProjLightData, vColorSpec,			CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_ELEMENT( ProjLightData, vAmbient,			CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_ELEMENT( ProjLightData, vRange,				CT_VECTOR_4, 1),
	SHADER_CONSTANT_END()
};

const DescriptorDeclaration g_projLightDescDesc[] =
{
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_CUSTOM, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_ALL),
	DESCRIPTOR_ELEMENT(5, DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_END()
};

const DescriptorDeclaration g_projLightShadowDescDecl[] =
{
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_CUSTOM, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_ALL),
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_SHADOW, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_ALL),
	DESCRIPTOR_ELEMENT(15, DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_ALL),
	DESCRIPTOR_ELEMENT(5, DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_END()
};


ProjectionLight::ProjectionLight()
:Light(LIGHT_TYPE_PROJ)
{
	m_fNear = 0.0f;
	m_fFar = 0.0f;

	m_bDirty = false;
	m_pShadow = nullptr;

	m_projMat.LoadIdentity();
	m_viewMat.LoadIdentity();
	m_modelMat.LoadIdentity();
}

ProjectionLight::~ProjectionLight()
{
}

void ProjectionLight::Init(GFXDevice* pDevice, Scene* pScene, bool bSupportsShadow)
{
	m_constants.Init(pDevice, g_projLightDecl);
	DescriptorSetLayoutHndl desc = pDevice->GetDescriptorSetLayout(g_projLightDescDesc);
	m_descriptorSet.Init(pDevice, desc);
	m_descriptorSet.SetConstantSet(0, &m_constants);
	SamplerDecl linearDecl(SF_LINEAR, SC_CLAMP);
	m_samplerHndl = pDevice->GetSampler(linearDecl);

	if (bSupportsShadow)
	{
		m_pShadow = vnew(ALLOC_OBJECT) ProjectionShadow;
		m_pShadow->Init(pDevice, pScene, 1024, 1024);
		SamplerDecl samp(SF_LINEAR, SC_CLAMP);
		samp.bEnableCmp = true;
		samp.eCmpFnc = CF_LESS;

		desc = pDevice->GetDescriptorSetLayout(g_projLightShadowDescDecl);
		m_descriptorSetShadow.Init(pDevice, desc);
		m_descriptorSetShadow.SetConstantSet(0, &m_constants);
		m_descriptorSetShadow.SetConstantSet(1, m_pShadow->GetShadowConstants());
		m_descriptorSetShadow.SetImageSamplerPair(2, m_pShadow->GetShadowTexture(), pDevice->GetSampler(samp));
	}

	Light::Init(pDevice, pScene, bSupportsShadow);
}

void ProjectionLight::CleanUp(GFXDevice* pDevice, Scene* pScene)
{
	m_constants.CleanUp(pDevice);
	m_descriptorSet.CleanUp(pDevice);
	if (m_pShadow)
	{
		m_descriptorSetShadow.CleanUp(pDevice);
		m_pShadow->Cleanup(pDevice, pScene);
		m_pShadow = nullptr;
	}
}


void ProjectionLight::SetTexture(GFXDevice* pDevice, const TextureHndl& pTexture)
{
	m_descriptorSet.SetImageSamplerPair(1, pTexture, m_samplerHndl);
	m_descriptorSetShadow.SetImageSamplerPair(3, pTexture, m_samplerHndl);
	m_descriptorSet.UpdateDescriptors(pDevice);
	m_descriptorSetShadow.UpdateDescriptors(pDevice);
}

void ProjectionLight::GPUUpdate(GFXDevice* pDevice)
{
	if(m_bDirty)
	{
		ProjLightData* pData = m_constants.Lock<ProjLightData>();

		Matrix4x4 mViewMat;
		mViewMat.BuildCameraFromModel(m_modelMat);

		m_frustum.SetUp(m_projMat, mViewMat);

		Matrix4x4 mViewProj = m_viewMat * m_projMat;
		Matrix4x4 mViewProjInv;
		mViewProj.GetInverse(mViewProjInv);

		
		const Color& diffuse = GetDiffuse();
		const Color& ambient = GetAmbient();

		pData->mViewProjInv = mViewProjInv;
		pData->vColorSpec.Assign( diffuse.r(), diffuse.g(), diffuse.b(), GetSpecular().r() );
		ambient.FillV4(pData->vAmbient);
		pData->vPos = m_modelMat.vPos();
		pData->vDirection = m_modelMat.vFace();

		pData->mTextureMat = m_textureMat;

		pData->vRange.Assign(1.0f/GetFar(), GetFar(), 1.0f/GetNear(), GetNear());
		
		m_constants.Unlock();
		m_constants.UpdateData(pDevice);
	}

	if (m_pShadow)
	{
		m_pShadow->GPUUpdate(pDevice, this);
	}

	m_bDirty = false;
}


void ProjectionLight::ShadowRender(GFXContext* pContext)
{
	if (m_pShadow)
	{
		m_pShadow->CreateShadowTex(pContext);
	}
}


void ProjectionLight::SetProjectionMtx(const Matrix4x4& projMat)
{
	m_projMat = projMat;
	RegenerateInternals();
}

void ProjectionLight::SetViewMatrix(const Matrix4x4& viewMat)
{
	m_viewMat = viewMat;
	m_modelMat.BuildModelFromCamera(viewMat);
	RegenerateInternals();
}

void ProjectionLight::SetModelMatrix(const Matrix4x4& modelMat)
{
	m_modelMat = modelMat;
	m_viewMat.BuildCameraFromModel(modelMat);

	RegenerateInternals();
}


void ProjectionLight::RegenerateInternals()
{
	Matrix4x4 mViewProj = m_viewMat * m_projMat;
	Matrix4x4 mViewProjInv;
	m_frustum.SetUp(mViewProj);
	mViewProj.GetInverse(mViewProjInv);

	// Now update the points used for clip testing
	mViewProjInv.TransformPerspective(-1.0f, 1.0f, -1.0f, m_vCorners[0]);
	mViewProjInv.TransformPerspective(1.0f, 1.0f, -1.0f, m_vCorners[1]);
	mViewProjInv.TransformPerspective(1.0f, 1.0f, 1.0f, m_vCorners[2]);
	mViewProjInv.TransformPerspective(-1.0f, 1.0f, 1.0f, m_vCorners[3]);
	mViewProjInv.TransformPerspective(-1.0f, -1.0f, -1.0f, m_vCorners[4]);
	mViewProjInv.TransformPerspective(1.0f, -1.0f, -1.0f, m_vCorners[5]);
	mViewProjInv.TransformPerspective(1.0f, -1.0f, 1.0f, m_vCorners[6]);
	mViewProjInv.TransformPerspective(-1.0f, -1.0f, 1.0f, m_vCorners[7]);


	static Matrix4x4 mOffset(	0.5f,	0.0f,	0.0f,	0.0f,
				 		0.0f,	0.5f,	0.0f,	0.0f, 
				 		0.0f,	0.0f,	0.0f,	0.0f,
				 		0.5f,	0.5f, 	0.0f, 	1.0f );

	m_textureMat = mViewProj * mOffset;

	m_bDirty = true;
}

const DescriptorSet* ProjectionLight::GetDescriptorSet() const
{
	if (GetShadowEnabled())
	{
		return &m_descriptorSetShadow;
	}
	else
	{
		return &m_descriptorSet;
	}
}

}