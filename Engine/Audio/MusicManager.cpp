#include "Engine/Common/Common.h"
#include "Audio.h"
#include "MusicManager.h"

namespace usg
{
	MusicManager::MusicManager()
	{
		m_eFadeIn = FADE_TYPE_NONE;
		m_eFadeOut = FADE_TYPE_NONE;
		m_fMusicVol = 0.0f;
		m_fPrevHndlVol = 0.0f;
		m_fFadeInRate = 0.0f;
		m_fFadeOutRate = 0.0f;
		m_fTargetVolume = 1.0f;
		m_uSoundId = USG_INVALID_ID;
		m_uPrevSoundId = USG_INVALID_ID;
		m_uCrossFadeId = USG_INVALID_ID;
	}


	MusicManager::~MusicManager()
	{

	}

	void MusicManager::PlayMusic(uint32 uSoundId, float fVolume, FADE_TYPE eFadeIn, FADE_TYPE eFadeOut, float fFadeTime, bool bResume)
	{
		if (m_prevMusicHndl.IsValid() && m_uPrevSoundId != uSoundId)
		{
			// Safety check incase we tried to crossfade multiple sounds in a very brief period
			m_prevMusicHndl.Stop();
			m_prevMusicHndl.RemoveRef();
			m_uPrevSoundId = USG_INVALID_ID;
		}

		m_fTargetVolume = fVolume;


		//if(uSoundId != m_uCrossFadeId)
		{

			bool bDidResume = false;
			if (uSoundId == m_uPrevSoundId && m_prevMusicHndl.IsValid() && bResume)
			{
				// Swap the sounds around
				usg::SoundHandle music = m_musicHndl;
				m_musicHndl = m_prevMusicHndl;
				m_prevMusicHndl = music; 
				m_uPrevSoundId = m_uSoundId;
				m_uSoundId = uSoundId;

				if (eFadeIn != FADE_TYPE_WAIT)
				{
					m_musicHndl.Start();
				}
				else
				{
					m_musicHndl.SetVolume(fVolume);
				}

				m_eFadeOut = eFadeOut;
				bDidResume = true;

			}
			else if (m_musicHndl.IsValid())
			{
				StopMusic(eFadeOut, fFadeTime);
			}

			if (!m_prevMusicHndl.IsValid() && eFadeIn == FADE_TYPE_WAIT)
			{
				eFadeIn = FADE_TYPE_NONE;
			}

			if(!bDidResume)
			{
				switch (eFadeIn)
				{
				case FADE_TYPE_NONE:
					m_musicHndl = Audio::Inst()->Prepare2DSound(uSoundId, fVolume, true);	// Just play immediately at full volume
					break;
				case FADE_TYPE_FADE:
					m_musicHndl = Audio::Inst()->Prepare2DSound(uSoundId, 0.0f, true);
					break;
				case FADE_TYPE_WAIT:
					m_musicHndl = Audio::Inst()->Prepare2DSound(uSoundId, fVolume, false);	// Waiting on the music being faded out to end
					break;
				}
			}
		}

		m_fFadeInRate = fFadeTime > 0.0f ? 1.f/fFadeTime : 100.f;
		//m_uCrossFadeId = uCrossFadeId;
		m_fTargetVolume = fVolume;
		m_eFadeIn = eFadeIn;
		m_uSoundId = uSoundId;
	}

	void MusicManager::PauseMusic()
	{
		if(!m_musicHndl.IsValid())
		{
			return;
		}

		m_musicHndl.Pause();
	}

	void MusicManager::RestartMusic()
	{
		if(!m_musicHndl.IsValid())
		{
			return;
		}

		m_musicHndl.Start();
	}

	void MusicManager::StopMusic(FADE_TYPE eFade, float fFadeTime)
	{
		if (!m_musicHndl.IsValid())
			return;	// Nothing to stop

		if (fFadeTime == 0.0f)
			eFade = FADE_TYPE_NONE;

		switch (eFade)
		{
		case FADE_TYPE_NONE:
			m_musicHndl.Stop();
			m_musicHndl.RemoveRef();
			break;
		case FADE_TYPE_FADE:
			m_fFadeOutRate = 1.f / fFadeTime;
			// Switch around our shared pointers
			m_prevMusicHndl = m_musicHndl;
			m_uPrevSoundId = m_uSoundId;
			m_uSoundId = USG_INVALID_ID;
			m_musicHndl.RemoveRef();
			break;
		default:
			ASSERT(false);	// Not a valid fade for stopping music
		}

		m_eFadeOut = eFade;
		m_uSoundId = USG_INVALID_ID;
	}

	void MusicManager::Update(float fElapsed)
	{
		if (m_eFadeIn == FADE_TYPE_FADE)
		{
			float fVolume = m_musicHndl.GetVolume();
			fVolume = fVolume + (fElapsed*m_fFadeInRate);
			if (fVolume >= m_fTargetVolume)
			{
				fVolume = m_fTargetVolume;
				m_eFadeIn = FADE_TYPE_NONE;
			}
			m_musicHndl.SetVolume(fVolume);
		}

		if (m_eFadeOut == FADE_TYPE_FADE)
		{
			float fVolume = m_prevMusicHndl.GetVolume();
			fVolume = fVolume - (fElapsed * m_fFadeOutRate);
			if (fVolume <= 0.0f)
			{
				fVolume = 0.0f;
				m_eFadeOut = FADE_TYPE_NONE;
				if (m_eFadeIn == FADE_TYPE_WAIT)
				{
					m_musicHndl.Start();
					m_eFadeIn = FADE_TYPE_NONE;
				}

				// Don't remove the previous sound if its looping incase of resume
				// TODO: Should maybe pause?
				if (!m_prevMusicHndl.GetLooping())
				{
					m_prevMusicHndl.Stop();
					m_prevMusicHndl.RemoveRef();
					m_uPrevSoundId = USG_INVALID_ID;
				}
				else
				{
					m_prevMusicHndl.SetVolume(0.0f);
					m_prevMusicHndl.Pause(0.0f);
				}
			}
			else
			{
				m_prevMusicHndl.SetVolume(fVolume);
			}
		}
	}
}
