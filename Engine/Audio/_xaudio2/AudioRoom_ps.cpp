/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Audio/Audio.h"
#include "Audio_ps.h"
#include "AudioFilter_ps.h"
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

		UINT32 flags = 0;
		if (pDef->filterCRC)
		{
			flags |= XAUDIO2_VOICE_USEFILTER;
		}
		HRESULT hr = pAudioPS->GetEngine()->CreateSubmixVoice(&m_pSubmixVoice, uChannelCount, 48000, flags);
		HRCHECK(hr);

		usg::vector<XAUDIO2_EFFECT_DESCRIPTOR> descriptors;

		for (uint32 i = 0; i < pDef->effectCRCs_count; i++)
		{
			AudioEffect_ps* pEffect = (AudioEffect_ps*)pAudio->GetEffect(pDef->effectCRCs[i]);
			IUnknown* pXAPO = pEffect->CreateEffect();

			XAUDIO2_EFFECT_DESCRIPTOR descriptor;
			descriptor.InitialState = true;
			descriptor.OutputChannels = uChannelCount;
			descriptor.pEffect = pXAPO;

			descriptors.push_back(descriptor);

			m_effects.push_back(pXAPO);
		}

		if (pDef->filterCRC)
		{
			AudioFilter_ps* pFilter = (AudioFilter_ps*)pAudio->GetFilter(pDef->filterCRC);

			if (pFilter)
			{
				hr = m_pSubmixVoice->SetFilterParameters(&pFilter->GetParameters());
				HRCHECK(hr);

			}
		}


		if (descriptors.size() > 0)
		{
			XAUDIO2_EFFECT_CHAIN chain;
			chain.EffectCount = (uint32)descriptors.size();
			chain.pEffectDescriptors = &descriptors[0];

			hr = m_pSubmixVoice->SetEffectChain(&chain);
		}

		for (uint32 i = 0; i < pDef->effectCRCs_count; i++)
		{
			AudioEffect_ps* pEffect = (AudioEffect_ps*)pAudio->GetEffect(pDef->effectCRCs[i]);
			hr = m_pSubmixVoice->SetEffectParameters(i, &pEffect->GetParams(), sizeof(pEffect->GetParams()));
			HRCHECK(hr);
		}

	}

}
