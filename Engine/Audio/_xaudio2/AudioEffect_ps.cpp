/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Audio/SoundCallbacks.h"
#include "AudioEffect_ps.h"
#include <xaudio2fx.h>

namespace usg{


	AudioEffect_ps::AudioEffect_ps()
	{

	}

	AudioEffect_ps::~AudioEffect_ps()
	{

	}

	void AudioEffect_ps::Init(const AudioEffectDef* pDef, Audio* pAudio)
	{
		AudioEffect::InitInt(pDef);

		if (pDef->eEffectType != AUDIO_EFFECT_REVERB)
		{
			ASSERT(false);
			return;
		}
		
		ReverbEffectDef* pReverb = (ReverbEffectDef*)pDef;

		memset(&m_params, 0, sizeof(m_params));
		m_params.DecayTime = pReverb->decayTime;
		m_params.Density = pReverb->density;
		m_params.DisableLateField = XAUDIO2FX_REVERB_DEFAULT_DISABLE_LATE_FIELD;
		m_params.EarlyDiffusion = XAUDIO2FX_REVERB_DEFAULT_EARLY_DIFFUSION;
		m_params.LateDiffusion = XAUDIO2FX_REVERB_DEFAULT_LATE_DIFFUSION;
		m_params.HighEQCutoff = XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_CUTOFF;
		m_params.LowEQCutoff = XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_CUTOFF;
		m_params.PositionLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION;
		m_params.PositionRight = XAUDIO2FX_REVERB_DEFAULT_POSITION;
		m_params.PositionMatrixLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
		m_params.PositionMatrixRight = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX;
		m_params.RearDelay = XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY;
		m_params.ReflectionsDelay = pReverb->reflectionsDelay;
		m_params.ReflectionsGain = pReverb->reflectionsGain;
		m_params.ReverbDelay = (BYTE)pReverb->reverbDelay;
		m_params.ReverbGain = pReverb->reverbGain;
		m_params.RoomFilterFreq = pReverb->roomFilterFreq;
		m_params.RoomFilterHF = pReverb->roomFilterHF;
		m_params.RoomFilterMain = pReverb->roomFilterMain;
		m_params.RoomSize = pReverb->roomSize;
		m_params.SideDelay = XAUDIO2FX_REVERB_DEFAULT_7POINT1_SIDE_DELAY;
		m_params.WetDryMix = pReverb->wetDryMix;
	}

}
