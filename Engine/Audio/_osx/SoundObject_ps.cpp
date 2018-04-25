/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Memory/Mem.h"
#include "SoundObject_ps.h"

#include <AudioToolbox/AudioToolbox.h>

#define QUIET_PLEASE 1

namespace usg{

SoundObject_ps::SoundObject_ps()
{
	m_uChannels = 0;
	m_bValid = false;
}

SoundObject_ps::~SoundObject_ps()
{
	Reset();
}

void SoundObject_ps::Init()
{

}

static OSStatus readProc(void *inClientData, SInt64 position, UInt32 requestCount, void *buffer, UInt32 *actualCount)
{
	*actualCount = requestCount;
	return noErr;
}
	
static SInt64 getSizeProc(void* inClientData)
{
	WaveFile *pWav = (WaveFile*) inClientData;
	
	return pWav->GetSize();
}

	
void SoundObject_ps::Reset()
{
#ifndef QUIET_PLEASE
	
	if (m_uSourceID)
	{
		alDeleteSources(1, &m_uSourceID);
		m_uSourceID = 0;
		m_bValid = false;
	}
#endif
	
}

void SoundObject_ps::BindWaveFile(WaveFile& waveFile, bool bPositional, bool bLoop)
{
#ifndef QUIET_PLEASE
	
	alGenSources(1, &m_uSourceID);
	
	AudioFileID	refInputAudioFileID;

	
	ALuint buffer;
	ALenum format;
	
	alGenBuffers(1, &buffer);
	ASSERT(alGetError() == AL_NO_ERROR);
	
	
	if (waveFile.GetChannels() == 2)
		format = waveFile.GetBitsPerSample() == 8 ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;
	else if (waveFile.GetChannels() == 1)
		format = waveFile.GetBitsPerSample() == 8 ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
	else
		ASSERT(false);
	
	alBufferData(buffer, format, waveFile.GetData(), waveFile.GetSize(), waveFile.GetSampleRate());
	
	
	alSourcei(m_uSourceID, AL_BUFFER, buffer);
	alSourcef(m_uSourceID, AL_PITCH, 1.0f);
	alSourcef(m_uSourceID, AL_GAIN, 1.0f);
	
	alSourcei(m_uSourceID, AL_LOOPING, bLoop ? AL_TRUE : AL_FALSE);

	ASSERT(alGetError() == AL_NO_ERROR);

	m_bValid = true;
#endif
	
}

void SoundObject_ps::Start()
{
#ifndef QUIET_PLEASE
	if(m_bValid)
		alSourcePlay(m_uSourceID);
#endif
}

void SoundObject_ps::Stop()
{
#ifndef QUIET_PLEASE
	if(m_bValid)
		alSourceStop(m_uSourceID);
#endif
}

void SoundObject_ps::Update(const SoundObject* pParent)
{
#ifndef QUIET_PLEASE

	if(!m_bValid)
		return;

	static float32 matrixCoefficients[2 * 8];
	memset(matrixCoefficients, 0, sizeof(float)*2*8);
	// Merging left and right channels of the sound
	for(uint32 i=0; i<m_uChannels; i++)
	{
		matrixCoefficients[0+(i*m_uChannels)] = pParent->GetPanningData().fMatrix[SoundObject::SOUND_CHANNEL_LEFT];
		matrixCoefficients[1+(i*m_uChannels)] = pParent->GetPanningData().fMatrix[SoundObject::SOUND_CHANNEL_RIGHT];
	}
	
	float fVolume = pParent->GetVolume() * pParent->GetAttenMul();
	
	//ASSERT(fVolume >= 0.0f && fVolume <= 1.0f);

//	m_pSourceVoice->SetVolume(pParent->GetVolume() * pParent->GetAttenMul());
	
//	m_pSourceVoice->SetOutputMatrix(NULL, m_uChannels, 1, matrixCoefficients);
	
//	alSourcef(m_uSourceID, AL_GAIN, pParent->GetVolume() * pParent->GetAttenMul());
	

#endif
}


bool SoundObject_ps::IsPlaying() const
{
#ifndef QUIET_PLEASE
	if(!m_bValid)
		return false;

	ALenum state;
	
	alGetSourcei(m_uSourceID, AL_SOURCE_STATE, &state);
	
	return (state == AL_PLAYING);
#else
	return false;
#endif
}

	
	
}
