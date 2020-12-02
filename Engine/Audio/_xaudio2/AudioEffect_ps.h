/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform specific implementation of instance of a sound
*****************************************************************************/
#ifndef __USG_AUDIO_AUDIO_EFFECT_PS_H__
#define __USG_AUDIO_AUDIO_EFFECT_PS_H__

#include "Engine/Audio/AudioEffect.h"
#include <xaudio2.h>
#include <xaudio2fx.h>

namespace usg{

	class Audio;;

class AudioEffect_ps : public AudioEffect
{
public:	
	AudioEffect_ps();
	~AudioEffect_ps();

	virtual void Init(const AudioEffectDef* pEffect, Audio* pAudio) override;

	const XAUDIO2FX_REVERB_PARAMETERS& GetParams() const { return m_params; }
private:
	XAUDIO2FX_REVERB_PARAMETERS	m_params;
};

}

#endif
