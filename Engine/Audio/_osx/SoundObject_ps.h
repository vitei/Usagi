/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform specific implementation of instance of a sound
*****************************************************************************/
#ifndef __USG_AUDIO_SOUNDOBJECT_PS_H__
#define __USG_AUDIO_SOUNDOBJECT_PS_H__
#include "Engine/Common/Common.h"
#include "WaveFile.h"
//#include <xaudio2.h>
#include <OpenAL/al.h>
#include <OpenAL/alc.h>

namespace usg{

	class SoundObject;

class SoundObject_ps
{
public:	
	SoundObject_ps();
	~SoundObject_ps();

	void Init();
	void BindWaveFile(WaveFile& waveFile, bool bPositional, bool bLoop);
	void Reset();
	void Start();
	void Stop();
	void Update(const SoundObject* pParent);
	bool IsPlaying() const;

private:
	
	uint32					m_uSourceID;
	uint32					m_uChannels;
	bool					m_bValid;
};

}

#endif
