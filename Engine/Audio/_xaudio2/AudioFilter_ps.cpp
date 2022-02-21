/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Audio/SoundCallbacks.h"
#include "AudioFilter_ps.h"

namespace usg{


	AudioFilter_ps::AudioFilter_ps()
	{

	}

	AudioFilter_ps::~AudioFilter_ps()
	{

	}

	void AudioFilter_ps::Init(const AudioFilterDef* pDef, Audio* pAudio)
	{
		AudioFilter::InitInt(pDef);
		m_parameters.Frequency = pDef->fFrequency;
		m_parameters.OneOverQ = pDef->fOneOverQ;
		switch (pDef->eFilter)
		{
			case AUDIO_FILTER_LOW_PASS:
				m_parameters.Type = XAUDIO2_FILTER_TYPE::LowPassFilter;
				break;
			case AUDIO_FILTER_HIGH_PASS:
				m_parameters.Type = XAUDIO2_FILTER_TYPE::HighPassFilter;
				break;
			case AUDIO_FILTER_BAND_PASS:
				m_parameters.Type = XAUDIO2_FILTER_TYPE::BandPassFilter;
				break;
			case AUDIO_FILTER_NOTCH:
				m_parameters.Type = XAUDIO2_FILTER_TYPE::NotchFilter;
				break;
			case AUDIO_FILTER_LOW_PASS_ONE_POLE:
				m_parameters.Type = XAUDIO2_FILTER_TYPE::LowPassOnePoleFilter;
				break;
			case AUDIO_FILTER_HIGH_PASS_ONE_POLE:
				m_parameters.Type = XAUDIO2_FILTER_TYPE::HighPassOnePoleFilter;
				break;
			default:
				break;
		}
	}

}
