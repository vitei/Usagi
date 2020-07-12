/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PARTICLE_EMITTER_H_
#define _USG_PARTICLE_EMITTER_H_

#include "Engine/Scene/RenderNode.h"
#include "Engine/Maths/Sphere.h"
#include "Engine/Memory/Allocator.h"

namespace usg{

class GFXDevice;
class GFXContext;
class ParticleEffect;

#define PARTICLE_ALLOC(NameOfClass) \
	public: \
	static ::usg::ParticleEmitter* AllocFromPool() { return (::usg::ParticleEmitter*)m_spEffectPool->Alloc(); } \
	virtual void FreeFromPool() { /*Looks a bit iffy, but safe enough*/ return m_spEffectPool->Free(this); } \
	static void InitPool(uint32 uPoolSize, ::usg::GFXDevice* pDevice)	\
    {	\
		ASSERT(!m_spEffectPool);	\
		m_spEffectPool = vnew(usg::ALLOC_PARTICLES) ::usg::FastPool<NameOfClass>(uPoolSize, false); \
		for (::usg::FastPool<NameOfClass>::Iterator it = m_spEffectPool->EmptyBegin(); !it.IsEnd(); ++it) \
			(*it)->Alloc(pDevice); \
    } \
    static void FreePool() { if (m_spEffectPool) { vdelete m_spEffectPool; m_spEffectPool = NULL; } }  \
	private: \
    static ::usg::FastPool<NameOfClass>* m_spEffectPool;

class ParticleEmitter : public RenderNode
{
public:
    ParticleEmitter();
    virtual ~ParticleEmitter();

    void Alloc(GFXDevice* pDevice);
	
	virtual void Init(usg::GFXDevice* pDevice, const ParticleEffect* pParent);
	virtual void CleanUp(GFXDevice* pDevice) {}
	virtual bool Update(float fElapsed);
	virtual void UpdateBuffers(GFXDevice* pDevice) {}
	
	Vector4f GetPosition() const;
	Vector4f GetInterpolatedPos(float fInerpolation) const;
	virtual void FreeFromPool() { ASSERT(false); };
	void EnableEmission(bool bEnable) { m_bEmissionAllowed = bEnable; }
	virtual bool ActiveParticles() = 0;

	// Note not real time, called before init to override parameters
	virtual void SetScale(float fScale) { }
	// For any CPU based operations on vertices. Probably shouldn't be necessary on modern hardware,
	// but there is no getting around it on the 3DS
	virtual bool RequiresCPUUpdate() const { return false; }
	virtual void UpdateParticleCPUData(float fElapsed) { };

protected:
	bool CanEmit() const { return m_bEmissionAllowed; }
	const ParticleEffect* GetParent() const { return m_pParent; }
private:

	bool	m_bEmissionAllowed;
	const ParticleEffect*	m_pParent;
};

}

#endif // _USG_PARTICLE_EMITTER_H_
