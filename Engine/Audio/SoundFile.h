/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: An abstract interface for loading sounds
 *****************************************************************************/
#ifndef __USG_AUDIO_CTR_SOUND_FILE__
#define __USG_AUDIO_CTR_SOUND_FILE__

#include "Engine/Maths/MathUtil.h"
#include "Engine/Audio/AudioBank.pb.h"
#include "Engine/Resource/PakFile.h"
#include "Engine/Core/stl/string.h"
#include "Engine/Core/stl/vector.h"

namespace usg
{
class Audio;
class AudioFilter;
class AudioEffect;
class AudioRoom;

class SoundFile
{
public:
	virtual void Init(const SoundFileDef* pSoundFile, const PakFileRaw* pPak, Audio* pAudio, const char* pszLocalizedSubdir = NULL) = 0;
	virtual void InitRaw(const SoundFileDef* pSoundFile, const void* pData, size_t rawDataSize, Audio* pAudio) = 0;
	virtual void Cleanup(Audio* pAudio) = 0;
	virtual void BindToSoundObject(class SoundObject* pSoundObject, bool bPositional) = 0;
	virtual float GetDuration() const { return 0.0f; }
	
	virtual ~SoundFile() {};

	AudioType GetAudioType() const { return m_eType; }
	float GetInitialVolume() const { return m_fInitVolume; }
	float GetInitialPitch() const { return m_fPitch + Math::RangedRandom(-m_fPitchRandomise, m_fPitchRandomise);  }
	bool GetLooping() const { return m_bLooping; }
	float GetMaxDistance() const { return m_fMaxDistance; }
	float GetMinDistance() const { return m_fMinDistance; }
	float GetDopplerFactor() const { return m_fDopplerFactor; }
	float GetLowPassAttenFactor() const { return m_fLowPassAttenFactor; }
	const string& GetName() const { return m_strName; }
	AudioFalloff GetFalloff() const { return m_eFalloff;  }
	AudioStacking GetStacking() const { return m_eStacking; }
	uint32 GetPriority() const { return m_uPriority; }
	uint32 GetCRC() const { return m_uCRC;  }

	const AudioFilter* GetFilter() const { return m_pFilter; }
	memsize GetEffectCount() const { return m_effects.size(); }
	const AudioEffect* GetEffect(memsize idx) const { return m_effects[idx]; }
	const AudioRoom* GetRoom() const { return m_pRoom; }
protected:
	void InitInt(const SoundFileDef* pSoundFile, const string& strName, Audio* pAudio);

	bool			m_bLooping;
	float			m_fInitVolume;
	float			m_fMinDistance;
	float			m_fMaxDistance;
	float			m_fPitchRandomise;
	float			m_fDopplerFactor;
	float 			m_fLowPassAttenFactor;
	float			m_fPitch;
	uint32			m_uCRC;
	uint32			m_uPriority;
	AudioType		m_eType;
	AudioFalloff	m_eFalloff;
	AudioStacking	m_eStacking;
	string			m_strName;

	const AudioFilter*				m_pFilter;
	const AudioRoom*				m_pRoom;
	usg::vector<const AudioEffect*>	m_effects;
};

}

#endif
