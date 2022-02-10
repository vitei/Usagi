/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PARTICLE_SYSTEM_H_
#define _USG_PARTICLE_SYSTEM_H_

#include "Engine/Memory/MemHeap.h"
#include "Engine/Particles/Scripted/EffectGroup.pb.h"
#include "Scripted/ScriptEmitter.h"
#include "RibbonTrail.h"
#include "ParticleEffect.h"
#include "ParticleEffectHndl.h"
#include "ParticleSet.h"

namespace usg{

class ParticleMgr
{
public:
    ParticleMgr(uint32 uMaxEffects = 60);
    ~ParticleMgr();

	void Init(GFXDevice* pDevice, Scene* pScene, ParticleSet* pSet);
	void Cleanup(GFXDevice* pDevice, Scene* pScene);
	bool Update(float fElapsed);
	void UpdateBuffers(usg::GFXDevice* pDevice);
	void CreateInstances(GFXDevice* pDevice, uint32 uInstances=8);

	ParticleEffectHndl CreateEffect(const Matrix4x4& mMat, const Vector3f& vVelocity, const char* szName, bool bCustom = true, float fScale = 1.0f);
	void SetEnvironmentColor(const Color& color) { m_envColor = color;  }
	const Color& GetEnvironmentColor() { return m_envColor;  }

	// Do not call directly
	void FreeScriptedEffect(ScriptEmitter* pEmitter);
	void FreeRibbon(RibbonTrail* pRibbon);
	Scene* GetScene() { return m_pScene; }

protected:
	ParticleSet*			m_pParticleSet;
	Scene*					m_pScene;
	GFXDevice*				m_pDevice;
	Color					m_envColor;

	// For storing all of the allocated instances of an effect
	class EmitterInstances
	{
	public:
		EmitterInstances();
		~EmitterInstances();

		void Init(GFXDevice* pDevice, ParticleMgr& mgr, const char* szName);
		void Cleanup(GFXDevice* pDevice);
		void UpdatePreloadCount(ParticleMgr& mgr, uint32 uCount);
		void ClearPreloadCount();
		const usg::string& GetName() { return m_name; }
		ScriptEmitter* GetInstance(GFXDevice* pDevice, ParticleMgr& mgr);
		void FreeInstance(ScriptEmitter* pInstance);
		void PreloadInstances(GFXDevice* pDevice, ParticleMgr& mgr);
	private:
		ParticleEmitterResHndl		m_resHndl;
		usg::string					m_name;
		list<ScriptEmitter*>		m_activeEmitters;
		list<ScriptEmitter*>		m_freeEmitters;
		uint32						m_uPreloadCount;
	};

	class EffectResources
	{
	public:
		EffectResources();
		~EffectResources();

		void Init(GFXDevice* pDevice, ParticleMgr& mgr, const char* szName);
		void UpdatePreloadCount(ParticleMgr& mgr);
		const usg::string& GetName() const { return m_name;  }
		void AddEmitters(GFXDevice* pDevice, ParticleMgr& mgr, ParticleEffect& effect);
		ParticleEffectResHndl& GetResHndl() { return m_resHndl; }
	private:
		usg::string				m_name;
		uint32					m_uPreloadCount;
		ParticleEffectResHndl	m_resHndl;
		EmitterInstances*		m_pInstanceGroups[particles::EffectGroup::emitters_max_count];
	};

	struct EffectData
	{
		ParticleEffectHndl	hndl;
		ParticleEffect 		effect;
	};

	EmitterInstances* GetEmitterInstance(GFXDevice* pDevice, const char* szName);
	EffectResources* GetEffectResource(GFXDevice* pDevice, const char* szName);

	FastPool<EmitterInstances>	m_emitters;
	FastPool<EffectResources>	m_effectResources;
	FastPool<EffectData>		m_effects;

	list<RibbonTrail*>			m_activeRibbons;
	list<RibbonTrail*>			m_freeRibbons;
};

}

#endif // _USG_PARTICLE_SYSTEM_H_
