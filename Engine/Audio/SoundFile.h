/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: An abstract interface for loading sounds
 *****************************************************************************/
#ifndef __USG_AUDIO_CTR_SOUND_FILE__
#define __USG_AUDIO_CTR_SOUND_FILE__
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Audio/AudioBank.pb.h"
#include "Engine/Core/String/U8String.h"

namespace usg
{
class Audio;

class SoundFile
{
public:
	virtual void Init(const SoundFileDef* pSoundFile, Audio* pAudio, const char* pszLocalizedSubdir = NULL) = 0;
	virtual void Cleanup(Audio* pAudio) = 0;
	virtual void BindToSoundObject(class SoundObject* pSoundObject, bool bPositional) = 0;
	
	virtual ~SoundFile() {};

	AudioType GetAudioType() const { return m_eType; }
	float GetInitialVolume() const { return m_fInitVolume; }
	float GetInitialPitch() const { return m_fPitch + Math::RangedRandom(-m_fPitchRandomise, m_fPitchRandomise);  }
	bool GetLooping() const { return m_bLooping; }
	float GetMaxDistance() const { return m_fMaxDistance; }
	float GetMinDistance() const { return m_fMinDistance; }
	float GetDopplerFactor() const { return m_fDopplerFactor; }
	const U8String& GetName() const { return m_strName; }
	AudioFalloff GetFalloff() const { return m_eFalloff;  }
	uint32 GetPriority() const { return m_uPriority; }
	uint32 GetCRC() const { return m_uCRC;  }
protected:
	void InitInt(const SoundFileDef* pSoundFile, const U8String& strName)
	{
		m_bLooping = pSoundFile->loop;
		m_fInitVolume = pSoundFile->volume;
		m_fMinDistance = pSoundFile->minDistance;
		m_fMaxDistance = pSoundFile->maxDistance;
		m_eType = pSoundFile->eType;
		m_strName = strName;
		m_eFalloff = pSoundFile->has_eFalloff ? pSoundFile->eFalloff : AudioFalloff_AUDIO_FALLOFF_LINEAR;
		m_fPitchRandomise = pSoundFile->has_pitchRandomisation ? pSoundFile->pitchRandomisation : 0.0f;
		m_fDopplerFactor = pSoundFile->has_dopplerFactor ? pSoundFile->dopplerFactor : 0.0f;
		m_fPitch = pSoundFile->has_basePitch ? pSoundFile->basePitch : 1.0f;
		m_uPriority = pSoundFile->has_priority ? pSoundFile->priority : 128;
		m_uCRC = pSoundFile->crc;
	}

	bool	m_bLooping;
	float	m_fInitVolume;
	float	m_fMinDistance;
	float	m_fMaxDistance;
	float	m_fPitchRandomise;
	float	m_fDopplerFactor;
	float	m_fPitch;
	uint32 m_uCRC;
	uint32 m_uPriority;
	AudioType m_eType;
	AudioFalloff m_eFalloff;
	U8String m_strName;
};

}

#endif
