/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Audio/_xaudio2/DummySoundFile.h"
#include "Audio_ps.h"

namespace usg{

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

		// Causes a crash
	//	CoUninitialize();
	}
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

	for (uint32 i = 0; i < _AudioType_count; i++)
	{
		// FIXME: Before we can use these we need audio types to be converted to the right settings
		HRESULT hr = m_pXAudio2->CreateSubmixVoice(&m_pSubmixVoices[i], 1, 44100);
		ASSERT(hr==S_OK);
	}

	m_bInitialised = true;
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

void Audio_ps::AddListener(AudioListener* pListener)
{

}

void Audio_ps::RemoveListener(AudioListener* pListener)
{

}


}

