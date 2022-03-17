/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Stores the header for a streamed file so we can kick off
//	playback as quickly as possible
*****************************************************************************/
#ifndef __USG_AUDIO_DUMMYSOUNDFILE_H__
#define __USG_AUDIO_DUMMYSOUNDFILE_H__

#include "Engine/Audio/SoundFile.h"
#include "Engine/Resource/PakFile.h"

namespace usg{


	class DummySoundFile  : public SoundFile
	{
	public:	
		DummySoundFile() {}
		~DummySoundFile() {}

		virtual void Init(const SoundFileDef* pSoundFile, const PakFileRaw* pPak, Audio* pAudio, const char* pszLocalizedSubdir = NULL) { }
		virtual void InitRaw(const SoundFileDef* pSoundFile, const void* pData, size_t rawDataSize, Audio* pAudio) { }
		virtual void Cleanup(Audio* pAudio) { }
		virtual void BindToSoundObject(class SoundObject* pSoundObject, bool bPositional) {}

	private:

	};

}

#endif
