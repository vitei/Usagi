/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Scene/Frustum.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/Shadows/ShadowCascade.h"
#include "Engine/Graphics/Lights/LightSpec.pb.h"
#include "Engine/Graphics/Lights/LightMgr.h"
#include "Engine/Graphics/Lights/PointLight.h"
#include "Engine/Graphics/Lights/DirLight.h"
#include "Engine/Graphics/Lights/SpotLight.h"
#include "Engine/Graphics/Lights/ProjectionLight.h"

namespace usg {

static const uint32 g_uShadowResMap[] =
{
	4096,
	2048,
	1536,
	1024,
};

LightMgr::LightMgr(void):
m_pParent(nullptr)
{
	m_skyColor.Assign(1.0f, 1.0f, 1.0f, 1.0f);
	m_groundColor.Assign(1.0f, 1.0f, 1.0f, 1.0f);
	m_hemisphereDir.Assign(1.0f, 1.0f, 1.0f);
	m_hemipshereLerp = 0.5f;
	m_uShadowedDirLights = 0;
	m_uShadowedDirLightIndex = UINT_MAX;
	m_uActiveFrame = UINT_MAX;
	m_bLightTexDirty = false;
	m_uShadowCastingFlags = RENDER_MASK_ALL;
	m_uShadowMapRes = g_uShadowResMap[m_qualitySettings.uShadowQuality];
}

LightMgr::~LightMgr(void)
{
}

void LightMgr::SetShadowCastingFlags(uint32 uFlags)
{
	m_uShadowCastingFlags = uFlags;
	for (auto itr : m_dirLights.GetActiveLights())
	{
		itr->SetShadowCastFlags(uFlags);
	}

	for (auto itr : m_pointLights.GetActiveLights())
	{
		itr->SetShadowCastFlags(uFlags);
	}

	for (auto itr : m_spotLights.GetActiveLights())
	{
		itr->SetShadowCastFlags(uFlags);
	}
}

void LightMgr::Init(GFXDevice* pDevice, Scene* pParent)
{
	m_pParent = pParent;

	// Set up an initial dummy array for binding purposes
	m_cascadeBuffer.InitArray(pDevice, 32, 32, 2, DepthFormat::DEPTH_32F, SAMPLE_COUNT_1_BIT);//DF_DEPTH_32F); //DF_DEPTH_24
	m_cascadeTarget.Init(pDevice, NULL, &m_cascadeBuffer);
	usg::RenderTarget::RenderPassFlags flags;
	flags.uClearFlags = RenderTarget::RT_FLAG_DEPTH;
	flags.uStoreFlags = RenderTarget::RT_FLAG_DEPTH;
	flags.uShaderReadFlags = RenderTarget::RT_FLAG_DEPTH;
	m_cascadeTarget.InitRenderPass(pDevice, flags);
	m_bLightTexDirty = true;
}


void LightMgr::SetQualitySettings(GFXDevice* pDevice, const QualitySettings& settings)
{
	if (m_qualitySettings.uShadowQuality == settings.uShadowQuality
		&& m_qualitySettings.bDirectionalShadows == settings.bDirectionalShadows
		&& m_qualitySettings.bPointShadows == settings.bPointShadows
		&& m_qualitySettings.bSpotShadows == settings.bSpotShadows)
		return;
	// TODO: Handle resizing after layers have been created (could be a useful performance optimization)
	m_qualitySettings = settings;
	m_uShadowMapRes = g_uShadowResMap[m_qualitySettings.uShadowQuality];

	InitShadowCascade(pDevice, ShadowCascade::CASCADE_COUNT * m_uShadowedDirLights);
}


void LightMgr::InitShadowCascade(GFXDevice* pDevice, uint32 uLayers)
{
	if (m_cascadeBuffer.GetWidth() != m_uShadowMapRes || uLayers != m_cascadeBuffer.GetSlices())
	{
		m_cascadeBuffer.Cleanup(pDevice);
		m_cascadeTarget.Cleanup(pDevice);
		m_cascadeBuffer.InitArray(pDevice, m_uShadowMapRes, m_uShadowMapRes, uLayers, DepthFormat::DEPTH_32F);
		m_cascadeTarget.Init(pDevice, NULL, &m_cascadeBuffer);
		usg::RenderTarget::RenderPassFlags flags;
		flags.uClearFlags = RenderTarget::RT_FLAG_DEPTH;
		flags.uStoreFlags = RenderTarget::RT_FLAG_DEPTH;
		flags.uShaderReadFlags = RenderTarget::RT_FLAG_DEPTH;
		m_cascadeTarget.InitRenderPass(pDevice, flags);
		m_bLightTexDirty = true;
	}
}


void LightMgr::Cleanup(GFXDevice* pDevice)
{
	m_dirLights.Cleanup(pDevice, m_pParent);
	m_spotLights.Cleanup(pDevice, m_pParent);
	m_pointLights.Cleanup(pDevice, m_pParent);
	m_projLights.Cleanup(pDevice, m_pParent);
	m_cascadeTarget.Cleanup(pDevice);
	m_cascadeBuffer.Cleanup(pDevice);
}


void LightMgr::Update(float fDelta, uint32 uFrame)
{
	// TODO: Handle multiple viewcontexts for the shodows
	ViewContext* pContext= m_pParent->GetViewContext(0);
	m_uActiveFrame = uFrame;

	m_uShadowedDirLights = 0;
	m_uShadowedDirLightIndex = UINT_MAX;

	list<DirLight*> dirLights;
	GetActiveDirLights(dirLights);

	// Now find the most influential shadowed directional light and make it the first
	uint32 uCascadeIndex = 0;
	uint32 i=0;
	for (auto itr : dirLights)
	{
		if (itr->GetShadowEnabled())
		{
			itr->GetCascade()->AssignRenderTarget(&m_cascadeTarget, uCascadeIndex);
			uCascadeIndex += ShadowCascade::CASCADE_COUNT;
			if (m_uShadowedDirLightIndex == UINT_MAX)
			{
				m_uShadowedDirLightIndex = i;
			}
			m_uShadowedDirLights++;
		}
		i++;
	}

	if (m_uShadowedDirLightIndex == UINT_MAX)
	{
		// No shadowed lights, set the index to be beyond the list
		m_uShadowedDirLightIndex = (uint32)dirLights.size();
	}

	if (pContext->GetCamera())
	{
		for (auto itr : dirLights)
		{
			itr->UpdateCascade(*pContext->GetCamera(), 0);
		}
	}


}


TextureHndl	LightMgr::GetShadowCascadeImage() const
{
	return m_cascadeBuffer.GetTexture();
}

void LightMgr::GPUUpdate(GFXDevice* pDevice)
{
	for (auto itr : m_dirLights.GetActiveLights())
	{
		itr->GPUUpdate(pDevice);
	}

	for (auto itr : m_pointLights.GetActiveLights())
	{
		itr->GPUUpdate(pDevice);
	}

	for (auto itr : m_spotLights.GetActiveLights())
	{
		itr->GPUUpdate(pDevice);
	}

	for (auto itr : m_projLights.GetActiveLights())
	{
		itr->GPUUpdate(pDevice);
	}
}


void LightMgr::GlobalShadowRender(GFXContext* pContext, Scene* pScene)
{
	for (auto itr : m_pointLights.GetActiveLights())
	{
		if (itr->GetVisibleFrame() == pScene->GetFrame())
		{
			itr->ShadowRender(pContext);
		}
	}

	for (auto itr : m_spotLights.GetActiveLights())
	{
		if (itr->GetVisibleFrame() == pScene->GetFrame())
		{
			itr->ShadowRender(pContext);
		}
	}

	for (auto itr : m_projLights.GetActiveLights())
	{
		if (itr->GetVisibleFrame() == pScene->GetFrame())
		{
			itr->ShadowRender(pContext);
		}
	}
}

void LightMgr::ViewShadowRender(GFXContext* pContext, Scene* pScene, ViewContext* pView)
{
	for (auto itr : m_dirLights.GetActiveLights())
	{
		if(itr->ShadowRender(pContext))
		{
			m_bLightTexDirty = false;
		}
	}

	if(m_bLightTexDirty)
	{
		// Clear the shadow texture (and resolve the input layout for vulkan)
		for(uint32 i=0; i<m_cascadeBuffer.GetSlices(); i++ )
		{
			pContext->SetRenderTargetLayer(&m_cascadeTarget, i);
			pContext->SetRenderTarget(NULL);
		}
	}
}


DirLight* LightMgr::AddDirectionalLight(GFXDevice* pDevice, bool bSupportsShadow, const char* szName)
{
	DirLight* pLight = m_dirLights.GetLight(pDevice, m_pParent, bSupportsShadow && m_qualitySettings.bDirectionalShadows);	

	ASSERT(pLight);

	if(szName)
		pLight->SetName(szName);

	pLight->SetShadowCastFlags(m_uShadowCastingFlags);

	if (bSupportsShadow)
		m_uShadowedDirLights++;

	if (m_cascadeBuffer.GetSlices() < ShadowCascade::CASCADE_COUNT * m_uShadowedDirLights)
	{
		InitShadowCascade(pDevice, ShadowCascade::CASCADE_COUNT * m_uShadowedDirLights);
	}

	return pLight;
}

void LightMgr::RemoveDirLight(DirLight* pLight)
{
	// FIXME: Adjust shadowed lights
	m_dirLights.Free(pLight);
}

PointLight* LightMgr::AddPointLight(GFXDevice* pDevice, bool bSupportsShadow, const char* szName)
{
	PointLight* pLight = m_pointLights.GetLight(pDevice, m_pParent, bSupportsShadow && m_qualitySettings.bPointShadows);
	if(szName)
		pLight->SetName(szName);

	pLight->SetShadowCastFlags(m_uShadowCastingFlags);

	return pLight;
}

void LightMgr::RemovePointLight(PointLight* pLight)
{
	return m_pointLights.Free(pLight);
}


SpotLight* LightMgr::AddSpotLight(GFXDevice* pDevice, bool bSupportsShadow, const char* szName)
{
	SpotLight* pLight = m_spotLights.GetLight(pDevice, m_pParent, bSupportsShadow && m_qualitySettings.bSpotShadows);
	if(szName)
		pLight->SetName(szName);

	pLight->SetShadowCastFlags(m_uShadowCastingFlags);


	return pLight;
}

void LightMgr::RemoveSpotLight(SpotLight* pLight)
{
	return m_spotLights.Free(pLight);
}


ProjectionLight* LightMgr::AddProjectionLight(GFXDevice* pDevice, bool bSupportsShadow, const char* szName)
{
	ProjectionLight* pLight = m_projLights.GetLight(pDevice, m_pParent, bSupportsShadow && m_qualitySettings.bSpotShadows);
	if(szName)
		pLight->SetName(szName);
	return pLight;	
}

void LightMgr::RemoveProjectionLight(ProjectionLight* pLight)
{
	return m_projLights.Free(pLight);
}


void LightMgr::GetActiveDirLights(list<DirLight*>& lightsOut) const
{
	lightsOut.clear();
	for(auto it = m_dirLights.GetActiveLights().begin(); it!=m_dirLights.GetActiveLights().end(); ++it)
	{
		if( (*it)->IsActive() )
		{
			(*it)->SetVisibleFrame(m_uActiveFrame);
			if ((*it)->GetShadowEnabled())
			{
				lightsOut.push_back(*it);
			}
			else
			{
				lightsOut.push_front(*it);
			}
		}
	}

	// FIXME: Since the switch to eastl this is comparing the pointers. 
	// Could re-enable but shadow location is the only important thing atm
	//lightsOut.sort();
}



void LightMgr::GetPointLightsInView(const Camera* pCamera, list<PointLight*>& lightsOut) const
{
	lightsOut.clear();
	// TODO: Give the point lights transforms and bounding volumes
	for (auto it = m_pointLights.GetActiveLights().begin(); it!=m_pointLights.GetActiveLights().end(); ++it)
	{
		if( (*it)->IsActive() && ((*it)->GetFar() > 0.0f || !(*it)->HasAttenuation()) )
		{
			// TODO: These lights should be culled like every object in the game using a quad tree
			if( !(*it)->HasAttenuation() || pCamera->GetFrustum().IsSphereInFrustum( (*it)->GetColSphere() ) )
			{
				(*it)->SetVisibleFrame(m_uActiveFrame);
				lightsOut.push_back(*it);
			}
		}
	}
}


void LightMgr::GetSpotLightsInView(const Camera* pCamera, list<SpotLight*>& lightsOut) const
{
	lightsOut.clear();
	// TODO: Give the point lights transforms and bounding volumes
	for (auto it = m_spotLights.GetActiveLights().begin(); it != m_spotLights.GetActiveLights().end(); ++it)
	{
		if( (*it)->IsActive() )
		{
			(*it)->SetVisibleFrame(m_uActiveFrame);
			// TODO: These lights should be culled like every object in the game using a quad tree
			if( !(*it)->HasAttenuation() || pCamera->GetFrustum().IsSphereInFrustum( (*it)->GetColSphere() ) )
			{
				lightsOut.push_back(*it);
			}
		}
	}
}


void LightMgr::GetProjectionLightsInView(const Camera* pCamera, list<ProjectionLight*>& lightsOut) const
{
	lightsOut.clear();
	// TODO: Give the point lights transforms and bounding volumes
	for (auto it = m_projLights.GetActiveLights().begin(); it != m_projLights.GetActiveLights().end(); ++it)
	{
		if( (*it)->IsActive() )
		{
			(*it)->SetVisibleFrame(m_uActiveFrame);
			if( (*it)->GetFrustum().ArePointsInFrustum( (*it)->GetCorners(), 8 ) )
			{
				lightsOut.push_back(*it);
			}
		}
	}
}


Light* LightMgr::FindLight(const char* szName)
{
	usg::string name(szName);
	for (auto it : m_pointLights.GetActiveLights() )
	{
		if( it->GetName() == name )
		{
			return it;
		}
	}

	for (auto it : m_dirLights.GetActiveLights())
	{
		if (it->GetName() == name)
		{
			return it;
		}
	}

	for (auto it : m_spotLights.GetActiveLights())
	{
		if (it->GetName() == name)
		{
			return it;
		}
	}

	return NULL;
}



Light* LightMgr::CreateLight(GFXDevice* pDevice, ResourceMgr* pResMgr, const LightSpec &light)
{
	Light* newLight = NULL;
	switch (light.base.kind)
	{
	case LightKind_DIRECTIONAL:
	{
		newLight = AddDirectionalLight(pDevice, light.base.bShadow);
		Vector4f vDir(light.direction, 0.0);
		vDir.Normalise();
		newLight->SetDirection(vDir);
	}
	break;
	case LightKind_POINT:
	{
		PointLight* pPoint = AddPointLight(pDevice, light.base.bShadow);
		if (light.atten.bEnabled)
		{
			pPoint->SetRange(light.atten.fNear, light.atten.fFar);
		}
		else
		{
			pPoint->EnableAttenuation(false);
		}
		newLight = pPoint;
	}

	break;
	case LightKind_SPOT:
	{
		SpotLight* pSpot = AddSpotLight(pDevice, light.base.bShadow);
		Vector4f vDir(light.direction, 0.0);
		vDir.Normalise();
		pSpot->SetDirection(vDir);

		pSpot->SetInnerCutoff(Math::DegToRad(light.spot.fInnerAngle));
		pSpot->SetOuterCutoff(Math::DegToRad(light.spot.fOuterAngle));

		if (light.atten.bEnabled)
		{
			pSpot->SetRange(light.atten.fNear, light.atten.fFar);
		}
		else
		{
			pSpot->EnableAttenuation(false);
		}

		newLight = pSpot;
	}
	break;
	case LightKind_PROJECTION:
	{
		ProjectionLight* pProj = AddProjectionLight(pDevice, light.base.bShadow);
		Matrix4x4 mProj;
		mProj.Perspective(Math::DegToRad(light.proj.fFov), light.proj.fAspect, 0.1f, light.atten.fFar);
		pProj->SetProjectionMtx(mProj);
		pProj->SetTexture(pDevice, pResMgr->GetTexture(pDevice, light.proj.texName));
		pProj->SetRange(light.atten.fNear, light.atten.fFar);

		// TODO: Set the texture

		newLight = pProj;
	}
	break;
	}

	if (newLight)
	{
		newLight->SetDiffuse(light.base.diffuse);
		newLight->SetAmbient(light.base.ambient);
		newLight->SetSpecularColor(light.base.specular);
		newLight->SetShadowExcludeFlags(light.base.uShadowExclFlags);
		newLight->SwitchOn(true);
	}

	return newLight;
}


void LightMgr::RemoveLight(const Light* pLight)
{
	ASSERT(pLight != NULL);
	switch (pLight->GetType())
	{
	case LIGHT_TYPE_DIR: 
		RemoveDirLight((DirLight*)pLight);
		break;
	case LIGHT_TYPE_POS:
		RemovePointLight((PointLight*)pLight);
		break;
	case LIGHT_TYPE_PROJ:
		RemoveProjectionLight((ProjectionLight*)pLight);
		break;
	case LIGHT_TYPE_SPOT:
		RemoveSpotLight((SpotLight*)pLight);
		break;
	default:
		ASSERT(false);
	}
}


}
