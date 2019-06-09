/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Resource/ParticleEffectResource.h"
#include "ParticleMgr.h"

namespace usg{

ParticleMgr::EmitterInstances::EmitterInstances() :  m_activeEmitters(10), m_freeEmitters(10), m_uPreloadCount(0)
{

}

ParticleMgr::EmitterInstances::~EmitterInstances()
{
	for (List<ScriptEmitter>::Iterator it = m_freeEmitters.Begin(); !it.IsEnd(); ++it)
	{
		vdelete *it;
	}

	for (List<ScriptEmitter>::Iterator it = m_activeEmitters.Begin(); !it.IsEnd(); ++it)
	{
		vdelete *it;
	}
}

void ParticleMgr::EmitterInstances::Init(GFXDevice* pDevice, ParticleMgr& mgr, const char* szName)
{
	m_name = szName;
	m_resHndl = ResourceMgr::Inst()->GetParticleEmitter(pDevice, szName);
}

void ParticleMgr::EmitterInstances::CleanUp(GFXDevice* pDevice)
{
	for (List<ScriptEmitter>::Iterator it = m_freeEmitters.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->CleanUp(pDevice);
	}

	for (List<ScriptEmitter>::Iterator it = m_activeEmitters.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->CleanUp(pDevice);
	}
}

void ParticleMgr::EmitterInstances::UpdatePreloadCount(ParticleMgr& mgr, uint32 uCount)
{
	m_uPreloadCount += uCount;
}

void ParticleMgr::EmitterInstances::ClearPreloadCount()
{
	m_uPreloadCount = 0;
}

ScriptEmitter* ParticleMgr::EmitterInstances::GetInstance(GFXDevice* pDevice, ParticleMgr& mgr)
{
	ScriptEmitter* pEmitter = m_freeEmitters.PopFront();
	if (!pEmitter)
	{
		pEmitter = vnew(ALLOC_PARTICLES) ScriptEmitter();
		pEmitter->Alloc(pDevice, &mgr, m_name.CStr());
	}
	m_activeEmitters.AddToFront(pEmitter);

	return pEmitter;
}

void ParticleMgr::EmitterInstances::FreeInstance(ScriptEmitter* pInstance)
{
	m_activeEmitters.Remove(pInstance);
	m_freeEmitters.AddToFront(pInstance);
}

void ParticleMgr::EmitterInstances::PreloadInstances(GFXDevice* pDevice, ParticleMgr& mgr)
{
	if (m_freeEmitters.GetSize() == 0)
	{
		for (uint32 i = 0; i < m_uPreloadCount; i++)
		{
			ScriptEmitter* pEmitter = vnew(ALLOC_PARTICLES) ScriptEmitter();
			pEmitter->Alloc(pDevice, &mgr, m_name.CStr());
			m_freeEmitters.AddToFront(pEmitter);
		}
	}
}

ParticleMgr::EffectResources::EffectResources()
{
	m_uPreloadCount = 1;
}

ParticleMgr::EffectResources::~EffectResources()
{
}

void ParticleMgr::EffectResources::Init(GFXDevice* pDevice, ParticleMgr& mgr, const char* szName)
{
	m_resHndl = ResourceMgr::Inst()->GetParticleEffect(szName);
	m_name = szName;
	if (m_resHndl->GetEffectGroup().has_uPreloadCount)
	{
		m_uPreloadCount = m_resHndl->GetEffectGroup().uPreloadCount;
	}

	for (uint32 i = 0; i < m_resHndl->GetEmitterCount(); i++)
	{
		m_pInstanceGroups[i] = mgr.GetEmitterInstance(pDevice, m_resHndl->GetEffectGroup().emitters[i].emitterName);
	}
}

void ParticleMgr::EffectResources::UpdatePreloadCount(ParticleMgr& mgr)
{
	for (uint32 i = 0; i < m_resHndl->GetEmitterCount(); i++)
	{
		m_pInstanceGroups[i]->UpdatePreloadCount(mgr, m_uPreloadCount);
	}
}

void ParticleMgr::EffectResources::AddEmitters(GFXDevice* pDevice, ParticleMgr& mgr, ParticleEffect& effect)
{
	const particles::EffectGroup& definition = m_resHndl->GetEffectGroup();

	for (uint32 i = 0; i < m_resHndl->GetEmitterCount(); i++)
	{
		const particles::EmitterData& emitter = definition.emitters[i];
		ScriptEmitter* pEmitter = m_pInstanceGroups[i]->GetInstance(pDevice, mgr);

		// Set the instance specific data for this emitter
		Matrix4x4 mScale, mTransRot;
		mScale.MakeScale(definition.emitters[i].vScale);
		mTransRot.LoadIdentity();
		Vector3f vRot = definition.emitters[i].vRotation;
		mTransRot.MakeRotate(Math::DegToRad(vRot.x), -Math::DegToRad(vRot.y), Math::DegToRad(vRot.z));
		mTransRot.SetTranslation(definition.emitters[i].vPosition);

		pEmitter->SetInstanceData(mScale * mTransRot, definition.emitters[i].fParticleScale, definition.emitters[i].fReleaseFrame);
		effect.AddEmitter(pDevice, pEmitter);
	}
}


ParticleMgr::ParticleMgr(uint32 uMaxEffects):
m_emitters(60, true, false),
m_effectResources(40, true, false),
m_effects(uMaxEffects)
{
	m_pParticleSet = NULL;
	m_envColor.Assign(1.0f, 1.0f, 1.0f, 1.0f);
}

ParticleMgr::~ParticleMgr()
{
	for(List<RibbonTrail>::Iterator it = m_freeRibbons.Begin(); !it.IsEnd(); ++it)
	{
		vdelete *it;
	}

	for(List<RibbonTrail>::Iterator it = m_activeRibbons.Begin(); !it.IsEnd(); ++it)
	{
		vdelete *it;
	}
}


void ParticleMgr::Init(GFXDevice* pDevice, Scene* pScene, ParticleSet* pSet)
{
	m_pScene = pScene;
	m_pDevice = pDevice;
	m_pParticleSet = pSet;
}

void ParticleMgr::CleanUp(GFXDevice* pDevice, Scene* pScene)
{
	for (FastPool<EffectData>::DynamicIterator it = m_effects.BeginDynamic(); !it.IsEnd(); ++it)
	{
		(*it)->hndl.Destroy(&(*it)->effect);
		it.RemoveElement();
	}

	for (FastPool<EffectData>::Iterator it = m_effects.EmptyBegin(); !it.IsEnd(); ++it)
	{
		(*it)->effect.CleanUp(pDevice);
	}

	for (FastPool<EmitterInstances>::Iterator it = m_emitters.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->CleanUp(pDevice);
	}
}
	
bool ParticleMgr::Update(float fElapsed)
{
	for(FastPool<EffectData>::DynamicIterator it = m_effects.BeginDynamic(); !it.IsEnd(); ++it)
	{
		ParticleEffect* pEffect = &(*it)->effect;
		pEffect->Update(fElapsed);
		if(!pEffect->IsAlive())
		{
			(*it)->hndl.Destroy(&(*it)->effect);
			it.RemoveElement();
		}
	}


	return true;
}

void ParticleMgr::UpdateBuffers(usg::GFXDevice* pDevice)
{
	for (FastPool<EffectData>::Iterator it = m_effects.Begin(); !it.IsEnd(); ++it)
	{
		ParticleEffect* pEffect = &(*it)->effect;
		pEffect->UpdateBuffers(pDevice);
	}
}

void ParticleMgr::CreateInstances(GFXDevice* pDevice, uint32 uInstances)
{
	// FIXME: Pre-loading will have to be refactored
#if 0
	uint32 uParticleEffects = ResourceMgr::Inst()->GetParticleEffectCount();
	const float fMaxRibbonTime = 1.2f;

	// Clear all emitters preload count
	for (FastPool<EmitterInstances>::Iterator it = m_emitters.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->ClearPreloadCount();
	}

	for(uint32 uEffect=0; uEffect<uParticleEffects; uEffect++)
	{
		const ParticleEffectResource* pEffect = ResourceMgr::Inst()->GetParticleEffectByIndex(uEffect);
		const particles::EffectGroup& definition = pEffect->GetEffectGroup();

		EffectResources* pRes = GetEffectResource(pDevice, pEffect->GetName().CStr());
		// Register the number of instances we need for this effect
		pRes->UpdatePreloadCount(*this);


		// Just get the textures, we won't create ribbons for every type
		for(uint32 i=0; i<definition.ribbons_count; i++)
		{
			for (uint32 uInstance = 0; uInstance < uInstances; uInstance++)
			{
				const particles::RibbonData& ribbon = definition.ribbons[i];
				ASSERT(ribbon.fLifeTime < fMaxRibbonTime);
				U8String srcName = ribbon.textureName;
				if (str::StartsWithToken(ribbon.textureName, "ribbon/"))
				{
					srcName.RemovePath();	// FIXME: Hack for old particle data
				}
				srcName = U8String("ribbon/") + srcName;
				usg::ResourceMgr::Inst()->GetTexture(pDevice, srcName.CStr());
			}
		}
	}

	for (FastPool<EmitterInstances>::Iterator it = m_emitters.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->PreloadInstances(pDevice, *this);
	}

	// FIXME: TANK HACK
	// Now preload us enough ribbons for 2 missiles per player
	particles::RibbonData ribbon;
	particles::RibbonData_init(&ribbon);
	ribbon.fLifeTime = fMaxRibbonTime;
	str::Copy(ribbon.textureName, "ribbon/trail", sizeof(ribbon.textureName));
	ribbon.fLineWidth = 1.0f;
	for (uint32 i = 0; i < 12; i++)
	{
		RibbonTrail* pTrail = vnew(ALLOC_PARTICLES) RibbonTrail;
		pTrail->Alloc(m_pDevice, this, &ribbon);
		m_freeRibbons.AddToEnd(pTrail);
	}
#endif
}

ParticleEffectHndl ParticleMgr::CreateEffect(const Matrix4x4& mMat, const Vector3f& vVelocity, const char* szName, bool bCustom, float fScale)
{
	EffectData* pEffectData = m_effects.Alloc();
	pEffectData->effect.Init(m_pDevice, m_pScene, mMat, vVelocity, fScale);

	if(!bCustom)
	{
		EffectResources* pEffect =GetEffectResource(m_pDevice, szName);
		ParticleEffectResHndl& resHndl = pEffect->GetResHndl();

		ASSERT(pEffect!=NULL);

		pEffect->AddEmitters(m_pDevice, *this, pEffectData->effect);

		// Now add the trails
		for(uint32 i=0; i<resHndl->GetRibbonCount(); i++)
		{
			RibbonTrail* pTrail = NULL;
			const particles::RibbonData& ribbon = resHndl->GetEffectGroup().ribbons[i];
			for(List<RibbonTrail>::Iterator it = m_freeRibbons.Begin(); !it.IsEnd(); ++it)
			{
				if((*it)->IsLargeEnough(ribbon))
				{
					pTrail = (*it);
					m_freeRibbons.Remove(pTrail);
					pTrail->SetDeclaration(m_pDevice, &ribbon);
					break;
				}
			}

			if(!pTrail)
			{
				pTrail = vnew(ALLOC_PARTICLES) RibbonTrail;
				pTrail->Alloc(m_pDevice, this, &ribbon);
			}

			pEffectData->effect.AddEmitter(m_pDevice, pTrail);

			m_activeRibbons.AddToEnd(pTrail);
		}

		
		pEffectData->hndl.Init(&pEffectData->effect);
		// TODO: Apply the emitter definition scale, position and rotation
		return pEffectData->hndl;
	}
	else
	{
		ASSERT(m_pParticleSet!=NULL);
		if (m_pParticleSet)
		{
			if (m_pParticleSet->CreateCustomEffect(szName, &pEffectData->effect))
			{
				pEffectData->hndl.Init(&pEffectData->effect);
				return pEffectData->hndl;
			}
		}
	}

	// Couldn't create the effect
	m_effects.Free(pEffectData);

	ASSERT(false);

	ParticleEffectHndl tmp;
	return tmp;
}


void ParticleMgr::FreeScriptedEffect(ScriptEmitter* pEmitter)
{
	for (FastPool<EmitterInstances>::Iterator it = m_emitters.Begin(); !it.IsEnd(); ++it)
	{
		if ((*it)->GetName() == pEmitter->GetScriptName())
		{
			(*it)->FreeInstance(pEmitter);
			return;
		}
	}

	ASSERT(false);
}

void ParticleMgr::FreeRibbon(RibbonTrail* pRibbon)
{
	m_freeRibbons.AddToEnd(pRibbon);
	m_activeRibbons.Remove(pRibbon);
}

ParticleMgr::EffectResources* ParticleMgr::GetEffectResource(GFXDevice* pDevice, const char* szName)
{
	U8String name = szName;
	for (FastPool<EffectResources>::Iterator it = m_effectResources.Begin(); !it.IsEnd(); ++it)
	{
		if ((*it)->GetName() == name)
		{
			return (*it);
		}
	}

	EffectResources* pResource = m_effectResources.Alloc();
	pResource->Init(pDevice, *this, szName);
	return pResource;
}

ParticleMgr::EmitterInstances* ParticleMgr::GetEmitterInstance(GFXDevice* pDevice, const char* szName)
{
	U8String name = szName;
	for (FastPool<EmitterInstances>::Iterator it = m_emitters.Begin(); !it.IsEnd(); ++it)
	{
		if ((*it)->GetName() == name)
		{
			return (*it);
		}
	}

	// We don't have one yet, so lets create it
	EmitterInstances* pInstances = m_emitters.Alloc();
	pInstances->Init(pDevice, *this, szName);
	return pInstances;
}

}
