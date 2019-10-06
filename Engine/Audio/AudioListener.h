/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Camera location info for a sound receiver
*****************************************************************************/
#ifndef __USG_AUDIO_AUDIO_LISTENER_H__
#define __USG_AUDIO_AUDIO_LISTENER_H__

#include "Engine/Maths/Matrix4x4.h"
#include AUDIO_HEADER(Engine/Audio/,AudioListener_ps.h)

namespace usg{

class AudioListener
{
public:	
	AudioListener() { m_fCullRadius = 6.0f; m_vVelocity = Vector3f::ZERO; m_fSoundSpeed = 340.f; m_fSpeakerRadius = 1.0f; }
	~AudioListener() {}

	void Init() { m_platform.Init(); }
	void SetSoundSpeed(float fSonic) { m_fSoundSpeed = fSonic; }
	void SetRemoteOutput(bool bRemoteOutput) { m_platform.SetRemoteOutput(bRemoteOutput); }
	void SetMatrix(const Matrix4x4& mLoc, bool bIsViewMatrix = false, bool bUseAsAttenPos = true);
	void SetAttenuationPos(const Vector3f& vPos); 
	const Matrix4x4& GetMatrix() const { return m_matrix; }
	const Matrix4x4& GetViewMatrix() const { return m_viewMatrix; }
	const Vector3f& GetVelocity() const { return m_vVelocity;  }
	const Vector3f& GetListenerSpaceVelocity() const { return m_vListernSpaceVel;  }
	const Vector3f& GetAttenuationPos() const { return m_vAttenPos; }
	void SetVelocity(const Vector3f& vVelocity) { m_vVelocity = vVelocity; UpdateListenerSpaceVel(); }

	AudioListener_ps& GetPlatform()  { return m_platform; }
	void SetCullRadius(float fRadius) { m_fCullRadius = fRadius;  }
	float GetCullRadius() const { return m_fCullRadius; }
	float GetSoundSpeed() const { return m_fSoundSpeed;  }
	// The inner range in which we stop treating sounds as point sources and spread out their influence
	float GetSpeakerRadius() const { return m_fSpeakerRadius; }

private:
	void UpdateListenerSpaceVel();
	// TODO: Remove the platform specific audio listener until such time as our principle platform has
	// accelerated audio;
	AudioListener_ps	m_platform;
	Matrix4x4			m_matrix;
	Matrix4x4			m_viewMatrix;
	Vector3f			m_vAttenPos;
	Vector3f			m_vVelocity;
	Vector3f			m_vListernSpaceVel;
	float				m_fSpeakerRadius;
	float				m_fCullRadius;
	float				m_fSoundSpeed;
};

}

#endif
