/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PARTICLE_EFFECT_H_
#define _USG_PARTICLE_EFFECT_H_

#include "Engine/Scene/TransformNode.h"
#include "Engine/Core/stl/list.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "ParticleEmitter.h"

namespace usg{

class GFXDevice;
class GFXContext;
class ConstantSet;

class ParticleEffect
{
public:
    ParticleEffect();
    ~ParticleEffect();

	enum class EffectEventType
	{
		EmitterTriggered,
		Count
	};

	struct EffectEvent
	{
		EffectEventType	eEvent;
		uint32			evtHash;
		Matrix4x4		mTransform;
	};

	void Init(GFXDevice* pDevice, Scene* pScene, const Matrix4x4& mMat, const Vector3f& vVelocity = Vector3f(0.0f, 0.0f, 0.0f), float fScale = 1.0f);
	void Cleanup(GFXDevice* pDevice);
	bool Update(float fElapsed);
	void UpdateBuffers(GFXDevice* pDevice);
	void AddEmitter(GFXDevice* pDevice, ParticleEmitter* pEmitter);	
	void RemoveEmitter(ParticleEmitter* pEmitter);	// Really for editor use only
	void SetWorldMat(const Matrix4x4& mMat);
	void WorldShifted(const Vector3f& vShift);
	void Kill(bool bForce = false);
	Vector4f GetPosition(float fLerp) const;
	bool IsAlive();
	void SetMultiplier(float fMul) { m_fMultiplier = fMul; }
	float GetMultiplier() const { return m_fMultiplier; }
	const Matrix4x4& GetMatrix() const { return m_mTransform; }
	const Vector3f& GetSystemVelocity() const { return m_vSysVelocity; }
	void SetSystemVelocity(const Vector3f &vVelocity) { m_vSysVelocity = vVelocity;  }
	const ConstantSet* GetConstantSet() const { return &m_constantSet;  }
	void EnableEmission(bool bEnable);
	
	void AddEvent(EffectEventType eEvent, uint32 uEventHash, Matrix4x4	mTransform);
	const usg::vector< EffectEvent>& GetFrameEvents() { return m_frameEvents; }

private:
	PRIVATIZE_COPY(ParticleEffect)

	void Cleanup();

	enum SystemState
	{
		STATE_RUNNING = 0,
		STATE_SHUTDOWN,
		STATE_INACTIVE
	};

	list<ParticleEmitter*>	m_emitters;
	usg::vector< EffectEvent>	m_frameEvents;

	// We group all of the emitters in any given effect to a single bounding area
	Matrix4x4				m_mTransform;
	ConstantSet				m_constantSet;
	Vector4f				m_vPrevPos;
	RenderGroup*			m_pRenderGroup;
	Scene*            		m_pScene;
	SystemState				m_eState;
	Vector3f				m_vSysVelocity;
	float					m_fMultiplier;
	float					m_fScale;
};

}


#endif // _USG_PARTICLE_SYSTEM_H_
