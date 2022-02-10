/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform specific implementation of instance of a sound
*****************************************************************************/
#ifndef __USG_AUDIO_AUDIO_FILTER_PS_H__
#define __USG_AUDIO_AUDIO_FILTER_PS_H__

#include "Engine/Audio/AudioFilter.h"
#include <xaudio2.h>

namespace usg{

	class Audio;

class AudioFilter_ps : public AudioFilter
{
public:	
	AudioFilter_ps();
	~AudioFilter_ps();

	virtual void Init(const AudioFilterDef* pDef, Audio* pAudio) override;

	const XAUDIO2_FILTER_PARAMETERS& GetParameters() const { return m_parameters; }
private:
	XAUDIO2_FILTER_PARAMETERS	m_parameters;
	
};

}

#endif
