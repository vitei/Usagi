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
		XAUDIO2FX_REVERB_I3DL2_PARAMETERS paramsIn;

		paramsIn.DecayHFRatio = pReverb->decayHFRatio;
		paramsIn.DecayTime = pReverb->decayTime;
		paramsIn.Density = pReverb->density;
		paramsIn.Diffusion = pReverb->diffusion;
		paramsIn.HFReference = pReverb->hfReference;
		paramsIn.Reflections = pReverb->reflections;
		paramsIn.ReflectionsDelay = pReverb->reflectionsDelay;
		paramsIn.Reverb = pReverb->reverb;
		paramsIn.ReverbDelay = pReverb->reverbDelay;
		paramsIn.Room = pReverb->room;
		paramsIn.RoomHF = pReverb->roomHF;
		paramsIn.RoomRolloffFactor = pReverb->roomRolloffFactor;
		paramsIn.WetDryMix = pReverb->wetDryMix;

		ReverbConvertI3DL2ToNative(&paramsIn, &m_params);
	}

}
