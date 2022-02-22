/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Audio/Audio.h"
#include "Audio_ps.h"
#include "AudioEffect_ps.h"
#include "AudioRoom_ps.h"

namespace usg{


	AudioRoom_ps::AudioRoom_ps()
	{

	}

	AudioRoom_ps::~AudioRoom_ps()
	{
		if (m_pSubmixVoice)
		{
			m_pSubmixVoice->DestroyVoice();
			m_pSubmixVoice = nullptr;
		}

		for (auto itr : m_effects)
		{
			itr->Release();
		}
		m_effects.clear();
	}

	void AudioRoom_ps::Init(const AudioRoomDef* pDef, Audio* pAudio)
	{
		m_uCRC = pDef->crc;

		Audio_ps* pAudioPS = &pAudio->GetPlatform();

		uint32 uChannelCount = pAudioPS->GetChannelCount(pAudio->GetChannelConfig());

		HRESULT hr = pAudioPS->GetEngine()->CreateSubmixVoice(&m_pSubmixVoice, uChannelCount, 48000);

		usg::vector<XAUDIO2_EFFECT_DESCRIPTOR> descriptors;

		for (uint32 i = 0; i < pDef->effectCrcs_count; i++)
		{
			AudioEffect_ps* pEffect = (AudioEffect_ps*)pAudio->GetEffect(pDef->effectCrcs[i]);
			IUnknown* pXAPO = pEffect->CreateEffect();

			XAUDIO2_EFFECT_DESCRIPTOR descriptor;
			descriptor.InitialState = uChannelCount;
			descriptor.OutputChannels = 1;
			descriptor.pEffect = pXAPO;

			descriptors.push_back(descriptor);

			m_effects.push_back(pXAPO);
		}

		if (descriptors.size() > 0)
		{
			XAUDIO2_EFFECT_CHAIN chain;
			chain.EffectCount = (uint32)descriptors.size();
			chain.pEffectDescriptors = &descriptors[0];

			m_pSubmixVoice->SetEffectChain(&chain);
		}

		for (uint32 i = 0; i < pDef->effectCrcs_count; i++)
		{
			AudioEffect_ps* pEffect = (AudioEffect_ps*)pAudio->GetEffect(pDef->effectCrcs[i]);
			m_pSubmixVoice->SetEffectParameters(i, &pEffect->GetParams(), sizeof(pEffect->GetParams()));
		}

	}

}
