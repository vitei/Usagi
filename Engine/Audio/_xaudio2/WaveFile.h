/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Wavefile loader for the XAudio2 interface
*****************************************************************************/
#ifndef __USG_AUDIO_XAUDIO2_WAVEFILE_H__
#define __USG_AUDIO_XAUDIO2_WAVEFILE_H__
#include "Engine/Common/Common.h"
#include "Engine/Audio/SoundFile.h"
#include "Engine/Audio/Shared/WaveFileReader.h"
#include <xaudio2.h>

namespace usg
{

	class Audio;

	class WaveFile  : public SoundFile
	{
	public:	
		WaveFile();
		~WaveFile();

		virtual void Init(const SoundFileDef* pSoundFile, Audio* pAudio, const char* pszLocalizedSubdir = NULL);
		virtual void Cleanup(Audio* pAudio) {}
		virtual void BindToSoundObject(class SoundObject* pSoundObject, bool bPositional);
		BYTE* GetData() { return m_pData; }
		WAVEFORMATEX& GetFormat() { return m_format; }
		uint32 GetSize() const { return m_uSize; }

		uint32 const GetLoopStart() { return m_loopStart; }
		uint32 const GetLoopLength() { return m_loopLength; }

		void BindToSound(class SoundObject_ps* pSoundObject, uint32 uPriority);

	private:
		WAVEFORMATEX	m_format;
		BYTE*			m_pData;
		uint32			m_uSize;
		uint32			m_loopStart;
		uint32 			m_loopLength;
	};

}

#endif
