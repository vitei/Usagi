#ifndef __USG_AUDIO_MUSIC_MANAGER_H__
#define __USG_AUDIO_MUSIC_MANAGER_H__

#include "Engine/Core/Singleton.h"
#include "Engine/Audio/AudioEvents.pb.h"
#include "Engine/Audio/SoundHandle.h"


namespace usg
{

class MusicManager : public usg::Singleton<MusicManager>
{
public:
	MusicManager();
	virtual ~MusicManager();

	enum FADE_TYPE
	{
		FADE_TYPE_NONE = 0,
		FADE_TYPE_FADE,
		FADE_TYPE_WAIT,	// Only valid for fade in
	};

	void PlayMusic(uint32 uSoundId, float fVolume = 1.0f, FADE_TYPE eFadeIn = FADE_TYPE_WAIT, FADE_TYPE eFadeOut = FADE_TYPE_FADE, float fFadeTime = 0.3f, uint32 uCrossFadeId = 0);
	void PauseMusic();
	void RestartMusic();
	void StopMusic(FADE_TYPE eFade, float fFadeTime = 0.3f);
	void Update(float fElapsed);

private:

	FADE_TYPE		m_eFadeIn;
	FADE_TYPE		m_eFadeOut;
	float			m_fMusicVol;
	float			m_fPrevHndlVol;
	float			m_fTargetVolume;

	float			m_fFadeInRate;
	float			m_fFadeOutRate;

	SoundHandle		m_musicHndl;
	SoundHandle		m_prevMusicHndl;	// For cross fade
	SoundHandle		m_crossFadeHndl;	

	uint32			m_uSoundId;
	uint32			m_uPrevSoundId;
	uint32			m_uCrossFadeId;
};

}


#endif
