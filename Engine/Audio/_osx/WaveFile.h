/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Wavefile loader for the XAudio2 interface
*****************************************************************************/
#ifndef __USG_AUDIO_XAUDIO2_WAVEFILE_H__
#define __USG_AUDIO_XAUDIO2_WAVEFILE_H__
#include "Engine/Common/Common.h"
#include "Engine/Audio/SoundFile.h"
#include "Engine/Audio/Shared/WaveFileReader.h"
//#include <xaudio2.h>

namespace usg{


	class WaveFile  : public SoundFile
	{
	public:	
		WaveFile();
		~WaveFile();

		virtual void Init(const SoundFileDef* pSoundFile, Audio* pAudio, const char* pszLocalizedSubdir = NULL);
		virtual void BindToSoundObject(class SoundObject* pSoundObject, bool bPositional);
		void* GetData() { return m_pData; }
		uint32 GetSize() const { return m_uSize; }
		uint32 GetChannels() const { return m_uChannels; }
		uint32 GetBitsPerSample() const { return m_uBitsPerSample; }
		uint32 GetSampleRate() const { return m_uSampleRate; }

	private:
		void*			m_pData;
		uint32			m_uSize;
		uint32			m_uChannels;
		uint32			m_uBitsPerSample;
		uint32			m_uSampleRate;
	};

}

#endif
