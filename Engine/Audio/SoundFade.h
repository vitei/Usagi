/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Handle to an instance of a sound
*****************************************************************************/
#ifndef __USG_AUDIO_SOUNDFADE_H__
#define __USG_AUDIO_SOUNDFADE_H__

#include "AudioDefs.h"

namespace usg{

class SoundFade
{
public:	
	SoundFade();
	~SoundFade();

	void Start(PLAY_STATE eTargetState, float fFrames, bool bFadeIn = false);
	float GetCurrentVolume() const { return m_fCurrent; }
	bool IsActive() const { return m_bActive; }
	PLAY_STATE GetTargetState() const { return m_eTargetState; }
	bool Update(float fElapsed);
	void Reset();

private:

	float				m_fPerFrameFade;
	float				m_fCurrent;
	PLAY_STATE   		m_eTargetState;
	bool 				m_bActive;
	bool				m_bFadeIn;
};

}

#endif
