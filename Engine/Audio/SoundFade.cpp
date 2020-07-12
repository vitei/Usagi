#include "Engine/Common/Common.h"
#include "SoundFade.h"

namespace usg{

	SoundFade::SoundFade()
	{
		Reset();
	}

	SoundFade::~SoundFade()
	{

	}

	void SoundFade::Start(PLAY_STATE eTargetState, float fFrames)
	{
		ASSERT(fFrames > 0.0f);
		m_eTargetState = eTargetState;
		m_fPerFrameFade = 1.0f/fFrames;
		m_bActive = true;
		m_bFadeIn = eTargetState == PLAY_STATE_PLAYING;
	}

	bool SoundFade::Update(float fElapsed)
	{
		if(m_bActive)
		{
			if(m_bFadeIn)
			{
				m_fCurrent += m_fPerFrameFade * fElapsed;

				if(m_fCurrent >= 1.0f)
				{
					m_fCurrent = 1.0f;
					m_bActive = false;
					return true;
				}
			}
			else
			{
				m_fCurrent -= m_fPerFrameFade * fElapsed;

				if(m_fCurrent <= 0.0f)
				{
					m_fCurrent = 0.0f;
					m_bActive = false;
					return true;
				}
			}
		}

		return false;
	}

	void SoundFade::Reset(float fStartVolume)
	{
		m_fPerFrameFade = 0.0f;
		m_fCurrent = fStartVolume;
		m_eTargetState = PLAY_STATE_NONE;
		m_bActive = false;
	}
}


