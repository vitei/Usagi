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

		paramsIn.DecayHFRatio = pReverb->DecayHFRatio;
		paramsIn.DecayTime = pReverb->DecayTime;
		paramsIn.Density = pReverb->Density;
		paramsIn.Diffusion = pReverb->Diffusion;
		paramsIn.HFReference = pReverb->HFReference;
		paramsIn.Reflections = pReverb->Reflections;
		paramsIn.ReflectionsDelay = pReverb->ReflectionsDelay;
		paramsIn.Reverb = pReverb->Reverb;
		paramsIn.ReverbDelay = pReverb->ReverbDelay;
		paramsIn.Room = pReverb->Room;
		paramsIn.RoomHF = pReverb->RoomHF;
		paramsIn.RoomRolloffFactor = pReverb->RoomRolloffFactor;
		paramsIn.WetDryMix = pReverb->WetDryMix;

		ReverbConvertI3DL2ToNative(&paramsIn, &m_params);
	}

}
