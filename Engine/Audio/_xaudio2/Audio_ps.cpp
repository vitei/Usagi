/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Audio/_xaudio2/DummySoundFile.h"
#include "Engine/Audio/_xaudio2/AudioFilter_ps.h"
#include "Engine/Audio/_xaudio2/AudioEffect_ps.h"
#include "Engine/Audio/_xaudio2/AudioRoom_ps.h"
#include "Audio_ps.h"

namespace usg{

static const uint32 g_suChannelCount[] =
{
	2, // CHANNEL_CONFIG_2_0,
	2, // CHANNEL_CONFIG_HEADPHONES,
	3, // CHANNEL_CONFIG_2_1,
	6, // CHANNEL_CONFIG_5_1,
	8, // CHANNEL_CONFIG_7_1,
};

static_assert(ARRAY_SIZE(g_suChannelCount) == CHANNEL_CONFIG_COUNT, "Mismatched channel count");

uint32 Audio_ps::GetChannelCount(ChannelConfig eChannelConfig)
{
	return g_suChannelCount[eChannelConfig];
}

Audio_ps::Audio_ps()
{
	m_pXAudio2 = nullptr;
	m_bInitialised = false;
	m_pMasteringVoice = nullptr;

	for (uint32 i = 0; i < _AudioType_count; i++)
	{
		m_pSubmixVoices[i] = nullptr;
	}
}

Audio_ps::~Audio_ps()
{
	if(m_bInitialised)
	{
		m_pXAudio2->StopEngine();
		m_pMasteringVoice->DestroyVoice();
		m_pMasteringVoice = nullptr;
		for (uint32 i = 0; i < _AudioType_count; i++)
		{
			if(m_pSubmixVoices[i])
			{
				m_pSubmixVoices[i]->DestroyVoice();
				m_pSubmixVoices[i] = nullptr;
			}
		}
		if(m_pXAudio2)
		{
			m_pXAudio2->Release();
			m_pXAudio2 = NULL;
		}

		CoUninitialize();
	}
}

void Audio_ps::EnableEffect(AudioType eType, AudioEffect* pEffect)
{
	
}

void Audio_ps::DisableEffect(AudioType eType, AudioEffect* pEffect)
{

}


void Audio_ps::Init()
{
	CoInitializeEx( NULL, COINIT_MULTITHREADED );

	if( FAILED( XAudio2Create( &m_pXAudio2, 0 ) ) )
	{
		CoUninitialize();
		//ASSERT(false);
		return;
	}

	if( FAILED( m_pXAudio2->CreateMasteringVoice( &m_pMasteringVoice ) ) )
	{
		CoUninitialize();
		//ASSERT(false);
		return;
	}



	m_bInitialised = true;
}

void Audio_ps::SetOutputChannelConfig(ChannelConfig eChannelConfig)
{
	for (uint32 i = 0; i < _AudioType_count; i++)
	{
		// Note this means all active voices need to be stopped and restarted
		if (m_pSubmixVoices[i])
		{
			m_pSubmixVoices[i]->DestroyVoice();
			m_pSubmixVoices[i] = nullptr;
		}
		HRESULT hr = m_pXAudio2->CreateSubmixVoice(&m_pSubmixVoices[i], GetChannelCount(eChannelConfig), 48000);
		ASSERT(hr == S_OK);
	}
}


void Audio_ps::SetAudioDevice(char16* deviceName)
{
	if (m_pMasteringVoice)
	{
		m_pMasteringVoice->DestroyVoice();
		m_pMasteringVoice = nullptr;
	}

	OutputDebugStringW(deviceName);
	if (FAILED(m_pXAudio2->CreateMasteringVoice(&m_pMasteringVoice, 0, 0, 0, deviceName)))
	{
		CoUninitialize();
		ASSERT(false);
		return;
	}
}

void Audio_ps::Update(float fElapsed)
{
}


SoundFile* Audio_ps::CreateSoundFile(const SoundFileDef* pDef)
{
	if(m_bInitialised)
	{
		return vnew(ALLOC_AUDIO) WaveFile();
	}
	else
	{
		return vnew(ALLOC_AUDIO) DummySoundFile();
	}
	
}

AudioFilter* Audio_ps::CreateAudioFilter(const AudioFilterDef* pDef)
{
	return vnew(ALLOC_AUDIO) AudioFilter_ps;
}

AudioEffect* Audio_ps::CreateAudioEffect(const AudioEffectDef* pDef)
{
	return vnew(ALLOC_AUDIO) AudioEffect_ps;
}

AudioRoom* Audio_ps::CreateAudioRoom(const AudioRoomDef* pDef)
{
	return vnew(ALLOC_AUDIO) AudioRoom_ps;
}

void Audio_ps::AddListener(AudioListener* pListener)
{

}

void Audio_ps::RemoveListener(AudioListener* pListener)
{

}


}

