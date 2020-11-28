/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "SoundObject.h"

using namespace usg;

void SoundObject::Reset()
{
	m_pSoundFile = NULL;
	m_fVolume = 1.0f;
	m_fAtten = 1.0f;
	m_fPitch = 1.0f;
	m_fDopplerFactor = 1.0f;
	m_panningData.fMatrix[SOUND_CHANNEL_FRONT_LEFT] = 0.75f;
	m_panningData.fMatrix[SOUND_CHANNEL_FRONT_RIGHT] = 0.75f;
	m_soundActor.RemoveRef();
	m_fFileVolume = 1.0f;
	m_fScaledFileVolume = 1.0f;
	m_fade.Reset();
	m_platform.Reset();
	m_bCustomData = false;
}


void SoundObject::SetCustomData(const struct StreamingSoundDef& def)
{
	m_bCustomData = true;
	m_platform.SetCustomData(def);
}

void SoundObject::SetSoundFile(const SoundFile* pSoundFile)
{
	m_pSoundFile = pSoundFile;
	m_fFileVolume = pSoundFile->GetInitialVolume();
	m_fScaledFileVolume = m_fFileVolume;
	m_fPitch = pSoundFile->GetInitialPitch();
	m_uPriority = pSoundFile->GetPriority();
}

bool SoundObject::IsPlaying() const
{
	return (m_ePlayState == PLAY_STATE_PLAYING || m_ePlayState == PLAY_STATE_PAUSED
		|| m_platform.IsPlaying() || m_platform.IsPaused() || m_fade.IsActive() );
}


void SoundObject::Start(float fTime)
{
	m_ePlayState = PLAY_STATE_PLAYING;
	if (fTime == 0.0f)
	{
		m_fade.Reset(1.0f);
	}
	else
	{
		m_fade.Reset(0.0f);
		m_fade.Start(PLAY_STATE_PLAYING, fTime);
	}
}

void SoundObject::Stop(float fTime)
{
	if (fTime == 0.0f)
	{
		m_ePlayState = PLAY_STATE_STOPPED;
		m_fade.Reset();
	}
	else
	{
		m_fade.Start(PLAY_STATE_STOPPED, fTime);
	}
}


void SoundObject::Pause(float fTime)
{
	if (!IsPlaying())
		return;

	if (fTime == 0.0f)
	{
		m_ePlayState = PLAY_STATE_PAUSED;
		m_fade.Reset();
	}
	else
	{
		m_fade.Start(PLAY_STATE_PAUSED, fTime);
	}
}

void SoundObject::Update(float fElapsed)
{
	if (m_fade.IsActive())
	{
		bool bDone = m_fade.Update(fElapsed);
		if (bDone)
		{
			m_ePlayState = m_fade.GetTargetState();
			m_fade.Reset();
		}
	}
	m_platform.Update(this);
}
