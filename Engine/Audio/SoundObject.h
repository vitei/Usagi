/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Handle to an instance of a sound
*****************************************************************************/
#ifndef __USG_AUDIO_SOUNDOBJECT_H__
#define __USG_AUDIO_SOUNDOBJECT_H__

#include "SoundActorHandle.h"
#include "SoundFile.h"
#include "AudioDefs.h"
#include "SoundFade.h"
#include AUDIO_HEADER(Engine/Audio, SoundObject_ps.h)

namespace usg{

class SoundObject
{
public:	
	SoundObject() { Reset(); }
	~SoundObject() {}

	void Init(Audio* pAudio) { m_platform.Init(pAudio); }
	void SetCustomData(const struct StreamingSoundDef& def);
	void Reset();
	void Start(float fTime = 0.0f);
	void Pause(float fTime = 0.3f);
	void Stop(float fTime = 0.0f);
	void Update(float fElapsed);
	bool IsPlaying() const;
	bool IsReady() const { return m_platform.IsReady(); }
	bool IsPaused() const { return m_platform.IsPaused(); }
	bool IsStopping() const { return m_ePlayState == PLAY_STATE_STOPPED || m_fade.IsActive();  }
	void SetVolume(float fVolume) { m_fVolume = fVolume; }
	void SetPitch(float fPitch) { m_fPitch = fPitch; }
	float GetPitch() const { return m_fPitch;  }
	float GetVolume() const { return m_fVolume; }
	float GetFileVolume() const { return m_fFileVolume;  }
	void ScaleVolumeBySystemVolume(float fVolume) {
		m_fScaledFileVolume = m_fFileVolume * fVolume;
	}
	float GetAdjVolume() const { return m_fScaledFileVolume * m_fVolume * m_fade.GetCurrentVolume();  }
	bool GetLooping() const;
	float GetRandomPitch() const { return m_pSoundFile->GetInitialPitch(); }

	SoundObject_ps& GetPlatform() { return m_platform; }
	SoundActorHandle GetSoundActor() { return m_soundActor; }

	void SetActor(SoundActorHandle actor) { m_soundActor = actor; }
	void SetSoundFile(const SoundFile* pSoundFile);
	const SoundFile* GetSoundFile() { return m_pSoundFile; }
	PLAY_STATE GetRequestedPlayState() const { return m_ePlayState; }
	void ClearRequestedPlayState() { m_ePlayState = PLAY_STATE_NONE; }

	void SetPanningData(const PanningData& panningData) { m_panningData = panningData; }
	const PanningData& GetPanningData() const { return m_panningData; }
	float GetAttenMul() const { return m_fAtten; }
	float GetDopplerFactor() const { return m_fDopplerFactor;  }
	void SetAttenMul(float fVal) { m_fAtten = fVal; }
	void SetDopplerFactor(float fVal) { m_fDopplerFactor = fVal;  }
	uint32 GetPriority() const { return (uint32)(m_uPriority * m_fAtten); }

	void SetActiveTrack(uint32 uTrack, float fLerpTime) { m_platform.SetActiveTrack(uTrack, fLerpTime);  }

	void SubmitData(void* pData, memsize size) { m_platform.SubmitData(pData, size); }

private:


	const SoundFile*	m_pSoundFile;			
	SoundObject_ps		m_platform;
	SoundFade			m_fade;
	float				m_fVolume;
	float				m_fFileVolume;
	float				m_fScaledFileVolume;
	float				m_fDopplerFactor;
	PanningData			m_panningData;
	SoundActorHandle	m_soundActor;
	PLAY_STATE			m_ePlayState;
	float				m_fAtten;
	float				m_fPitch;
	uint32				m_uPriority;
	bool				m_bCustomData;
};

inline bool SoundObject::GetLooping() const
{
	if(!m_pSoundFile)
		return false;

	return m_pSoundFile->GetLooping();
}

}

#endif
