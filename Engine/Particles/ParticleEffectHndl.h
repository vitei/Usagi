/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PARTICLE_EFFECT_HNDL_H_
#define _USG_PARTICLE_EFFECT_HNDL_H_

#include "Engine/Core/Containers/SafePointer.h"
#include "ParticleEffect.h"

namespace usg{

class GFXDevice;
class GFXContext;

class ParticleEffectHndl : public SafePointer<ParticleEffect>
{
public:
    ParticleEffectHndl() {}
    ~ParticleEffectHndl() {}

	inline void SetWorldMat(const Matrix4x4& mMat);
	inline void SetVelocity(const Vector3f& vVelocity);
	inline void SetMultiplier(const float fMul);
	inline void Kill(bool bForce = false);
	inline void ForceUpdate(float fElapsed);
	inline void EnableEmission(bool bEnable);
	inline void WorldShifted(const usg::Vector3f& vShift);
	inline void GetEffectEvents(usg::vector<ParticleEffect::EffectEvent>& eventsOut);

	SAFEPOINTER_COPY(ParticleEffectHndl)

private:
};


inline void ParticleEffectHndl::SetWorldMat(const Matrix4x4& mMat)
{
	ParticleEffect* pEffect = GetPointer();
	if(pEffect)
	{
		pEffect->SetWorldMat(mMat);
	}
}

inline void ParticleEffectHndl::Kill(bool bForce)
{
	ParticleEffect* pEffect = GetPointer();
	if(pEffect)
	{
		pEffect->Kill(bForce);
	}
	RemoveRef();
}


inline void ParticleEffectHndl::GetEffectEvents(usg::vector<ParticleEffect::EffectEvent>& eventsOut)
{
	ParticleEffect* pEffect = GetPointer();
	if (pEffect)
	{
		eventsOut = pEffect->GetFrameEvents();
	}
}

inline void ParticleEffectHndl::SetVelocity(const Vector3f& vVelocity)
{
	ParticleEffect* pEffect = GetPointer();
	if(pEffect)
	{
		pEffect->SetSystemVelocity(vVelocity);
	}	
}


inline void ParticleEffectHndl::WorldShifted(const Vector3f& vShift)
{
	ParticleEffect* pEffect = GetPointer();
	if (pEffect)
	{
		pEffect->WorldShifted(vShift);
	}
}

inline void ParticleEffectHndl::EnableEmission(bool bEnable)
{
	ParticleEffect* pEffect = GetPointer();
	if (pEffect)
	{
		pEffect->EnableEmission(bEnable);
	}
}


inline void ParticleEffectHndl::SetMultiplier(const float fMul)
{
	ParticleEffect* pEffect = GetPointer();
	if(pEffect)
	{
		pEffect->SetMultiplier(fMul);
	}		
}

inline void ParticleEffectHndl::ForceUpdate(float fElapsed)
{
	ParticleEffect* pEffect = GetPointer();
	if(pEffect)
	{
		pEffect->Update(fElapsed);
	}		
}

}

#endif // _USG_PARTICLE_SYSTEM_H_
