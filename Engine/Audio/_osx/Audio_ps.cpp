/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Audio/_xaudio2/DummySoundFile.h"
#include "Audio_ps.h"

namespace usg{

Audio_ps::Audio_ps()
{
	m_pOpenALDevice = NULL;
	m_pOpenALContext = NULL;
	m_bInitialised = false;
}

Audio_ps::~Audio_ps()
{
	if(m_bInitialised)
	{

	}
}

void Audio_ps::Init()
{
	
	if (!m_bInitialised)
	{
		m_pOpenALDevice = alcOpenDevice(NULL);
		m_pOpenALContext = alcCreateContext(m_pOpenALDevice, NULL);
		alcMakeContextCurrent(m_pOpenALContext);
	}
	
	
	m_bInitialised = true;
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

