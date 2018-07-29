/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/LightingContext.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Graphics/Shadows/ShadowCascade.h"
#include "Engine/Graphics/Lights/LightMgr.h"

namespace usg {

#define MAX_LIGHTS	8
#define MAX_CASCADES 2

struct LightConsts
{
	Vector4f vPosition;
	Vector4f vDirection;
	Vector4f vDiffuse;
	Vector4f vAmbient;
	Vector4f vSpecular;
	Vector4f vRange;
	float	fCosSpotCutoff;
	float	fOuterSpotRadius;
	float	fCosInnerSpotCutoff;
};

struct LightingConsts
{
	int				iDirLightCount;
	int				iCascadeLightStart;
	int				iPointLightCount;
	int				iSpotLightCount;
	Vector4f		vGlobalAmbient;
	LightConsts		lights[MAX_LIGHTS];
	ShadowCascade::ShadowReadConstants cascades[MAX_CASCADES];
};

static const ShaderConstantDecl g_lightDecl[] = 
{
	SHADER_CONSTANT_ELEMENT( LightConsts, vPosition,			CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_ELEMENT( LightConsts, vDirection,			CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_ELEMENT( LightConsts, vDiffuse,				CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_ELEMENT( LightConsts, vAmbient,				CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_ELEMENT( LightConsts, vSpecular,			CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_ELEMENT( LightConsts, vRange,				CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_ELEMENT( LightConsts, fCosSpotCutoff,		CT_FLOAT, 1 ),
	SHADER_CONSTANT_ELEMENT( LightConsts, fOuterSpotRadius,		CT_FLOAT, 1 ),
	SHADER_CONSTANT_ELEMENT( LightConsts, fCosInnerSpotCutoff,	CT_FLOAT, 1 ),
	SHADER_CONSTANT_END()
};

static const ShaderConstantDecl g_dirLightingDecl[] = 
{
	SHADER_CONSTANT_ELEMENT( LightingConsts, iDirLightCount, CT_INT, 1),
	SHADER_CONSTANT_ELEMENT(LightingConsts, iCascadeLightStart, CT_INT, 1),
	SHADER_CONSTANT_ELEMENT( LightingConsts, iPointLightCount, CT_INT, 1),
	SHADER_CONSTANT_ELEMENT( LightingConsts, iSpotLightCount, CT_INT, 1),
	SHADER_CONSTANT_ELEMENT( LightingConsts, vGlobalAmbient, CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_STRUCT_ARRAY(LightingConsts, lights, g_lightDecl, MAX_LIGHTS ),
	SHADER_CONSTANT_STRUCT_ARRAY(LightingConsts, cascades, ShadowCascade::GetDecl(), MAX_CASCADES),
	SHADER_CONSTANT_END()
};

// Ok for lights as we shouldn't have that many in view, but for any other object we'd want to pre-calculate the distances
class DistanceCmp
{
public:
	DistanceCmp(const Vector4f& vCameraPos) { m_vCameraPos = vCameraPos; }
	bool operator() (const PointLight &i, const PointLight &j) const
	{
		Vector3f vDirI = i.GetPosition().v3() - m_vCameraPos.v3();
		Vector3f vDirJ = j.GetPosition().v3() - m_vCameraPos.v3();
		float fDistISq = DotProduct(vDirI, vDirI) - (i.GetFar()*i.GetFar());
		float fDistJSq = DotProduct(vDirJ, vDirJ) - (j.GetFar()*j.GetFar());
		return (fDistISq<fDistJSq);
	}
private:
	Vector4f m_vCameraPos;
};

LightingContext::LightingContext()
{

}

LightingContext::~LightingContext()
{

}


void LightingContext::Init(GFXDevice* pDevice)
{
	m_lightingConstants.Init(pDevice, g_dirLightingDecl);
}

void LightingContext::Cleanup(GFXDevice* pDevice)
{
	m_lightingConstants.CleanUp(pDevice);
}

void LightingContext::Update(GFXDevice* pDevice, SceneContext* pCtxt)
{
	Scene* pScene=  pCtxt->GetScene();
	pScene->GetLightMgr().GetActiveDirLights(m_visDirLights);
	pScene->GetLightMgr().GetPointLightsInView(pCtxt->GetCamera(), m_visPointLights);
	pScene->GetLightMgr().GetSpotLightsInView(pCtxt->GetCamera(), m_visSpotLights);
	pScene->GetLightMgr().GetProjectionLightsInView(pCtxt->GetCamera(), m_visProjLights);

	DistanceCmp cmp(pCtxt->GetCamera()->GetPos());
	m_visPointLights.Sort(cmp);
	m_ambient = pCtxt->GetScene()->GetLightMgr().GetAmbientLight();

	Matrix4x4 mViewMat = pCtxt->GetCamera()->GetViewMatrix(VIEW_CENTRAL);

	const List<DirLight>& dirLights = m_visDirLights;
	const List<PointLight>& pointLights = m_visPointLights;
	const List<SpotLight>& spotLights = m_visSpotLights;

	LightingConsts* pLightingData = m_lightingConstants.Lock<LightingConsts>();

	// FIXME: A global ambient value (not that we really want much at all)
	pScene->GetLightMgr().GetAmbientLight().FillV4(pLightingData->vGlobalAmbient);
	pLightingData->iDirLightCount = 0;
	pLightingData->iPointLightCount = 0;
	pLightingData->iSpotLightCount = 0;
	pLightingData->iCascadeLightStart = pCtxt->GetScene()->GetLightMgr().GetShadowedDirLightIndex();

	// FIXME: If we re-implemenet deferred shading we need the per light constants to be updated somewhere
	uint32 uLightCount = 0;
	uint32 uCascadeCount = 0;
	m_cascadeTextures.clear();
	for(List<DirLight>::Iterator it = dirLights.Begin(); !it.IsEnd() && uLightCount < MAX_LIGHTS; ++it)
	{
		const DirLight* pLight = *it;

		LightConsts* pLightConst = &pLightingData->lights[uLightCount];
		if (pLight->GetShadowEnabled())
		{
			if (uCascadeCount >= MAX_CASCADES)
			{
				// Ignore this light
				DEBUG_PRINT("Dropping additional shadow cascades\n");
				break;
			}
			if (pLightingData->iCascadeLightStart == (-1))
			{
				pLightingData->iCascadeLightStart = (int)uLightCount;
			}
			pLightingData->cascades[uCascadeCount] = pLight->GetCascade()->GetShadowReadConstantData();
			m_cascadeTextures.push_back(pLight->GetCascade()->GetTexture());
			uCascadeCount++;
		}
		else
		{
			// We are expecting shadowed lights to come last
			ASSERT(uCascadeCount == 0);
		}
		pLightingData->iDirLightCount++;		
		pLightConst->vDirection = (-pLight->GetDirection() * mViewMat);
		pLight->GetDiffuse().FillV4(pLightConst->vDiffuse);
		pLight->GetSpecular().FillV4(pLightConst->vSpecular);
		pLightingData->vGlobalAmbient.x += pLight->GetAmbient().r();
		pLightingData->vGlobalAmbient.y += pLight->GetAmbient().g();
		pLightingData->vGlobalAmbient.z += pLight->GetAmbient().b();
		
		uLightCount++;
	}

	
	for(List<PointLight>::Iterator it = pointLights.Begin(); !it.IsEnd() && uLightCount < MAX_LIGHTS; ++it)
	{
		const PointLight* pLight = *it;

		if(pLight->GetFar() > 0.0f)
		{
			LightConsts* pLightConst = &pLightingData->lights[uLightCount];
			pLightingData->iPointLightCount++;
			//pLightConst->vDirection = (-pLight->GetDirection() * mViewMat);
			pLightConst->vPosition = (pLight->GetPosition() * mViewMat);
			pLight->GetDiffuse().FillV4(pLightConst->vDiffuse);
			pLight->GetSpecular().FillV4(pLightConst->vSpecular);
			pLight->GetAmbient().FillV4(pLightConst->vAmbient);
			pLightConst->vRange.Assign(1.0f/pLight->GetFar(), pLight->GetFar(), pLight->GetFar()*pLight->GetFar(), pLight->GetNear());

			uLightCount++;
		}
	}
	
	for(List<SpotLight>::Iterator it = spotLights.Begin(); !it.IsEnd() && uLightCount < MAX_LIGHTS; ++it)
	{
		const SpotLight* pLight = *it;

		LightConsts* pLightConst = &pLightingData->lights[uLightCount];
		pLightingData->iSpotLightCount++;
		pLightConst->vDirection = (pLight->GetDirection() * mViewMat);
		pLightConst->vPosition = (pLight->GetPosition() * mViewMat);
		pLight->GetDiffuse().FillV4(pLightConst->vDiffuse);
		pLight->GetSpecular().FillV4(pLightConst->vSpecular);
		pLight->GetAmbient().FillV4(pLightConst->vAmbient);
		if(pLight->HasAttenuation())
			pLightConst->vRange.Assign(1.0f/pLight->GetFar(), pLight->GetFar(), 0.0f, 0.0f);
		else
		pLightConst->vRange.Assign(-1.0f, 0.0f, 0.0f, 0.0f);
		pLightConst->fCosSpotCutoff = cosf(pLight->GetOuterCutoff());
		pLightConst->fOuterSpotRadius =  tanf(pLight->GetOuterCutoff()) * pLight->GetFar();
		pLightConst->fCosInnerSpotCutoff = cosf(pLight->GetInnerCutoff());

		uLightCount++;
	}

	m_lightingConstants.Unlock();
	m_lightingConstants.UpdateData(pDevice);
}



void LightingContext::ClearLists()
{
	m_visDirLights.Clear();
	m_visPointLights.Clear();
}

const List<DirLight>& LightingContext::GetActiveDirLights() const
{
	return m_visDirLights;
}

const List<PointLight>& LightingContext::GetPointLightsInView() const
{
	return m_visPointLights;
}

const List<SpotLight>& LightingContext::GetSpotLightsInView() const
{
	return m_visSpotLights;
}

const List<ProjectionLight>& LightingContext::GetProjLightsInView() const
{
	return m_visProjLights;
}

const vector<TextureHndl>& LightingContext::GetCascadeTextures() const
{
	return m_cascadeTextures;
}

void LightingContext::AddConstantsToDescriptor(DescriptorSet& desc, uint32 uIndex) const
{
	desc.SetConstantSet(uIndex, &m_lightingConstants);
}

void LightingContext::SetPrimaryShadowDesc(GFXContext* pContext)
{

}

}

