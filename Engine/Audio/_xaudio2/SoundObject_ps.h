/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform specific implementation of instance of a sound
*****************************************************************************/
#ifndef __USG_AUDIO_SOUNDOBJECT_PS_H__
#define __USG_AUDIO_SOUNDOBJECT_PS_H__
#include "Engine/Common/Common.h"
#include "WaveFile.h"
#include <xaudio2.h>

namespace usg{

	class SoundObject;

class SoundObject_ps
{
public:	
	SoundObject_ps();
	~SoundObject_ps();

	void Init(Audio* pAudio);
	void SetSoundFile(WaveFile* pWaveFile, bool bPositional, bool bLoop);
	void Reset();
	void Update(const SoundObject* pParent);
	bool IsPlaying() const;
	bool IsPaused() const { return m_bPaused; }
	bool IsReady() const { return true; }

	void BindWaveFile(WaveFile& waveFile, uint32 uPriority);
	void SetActiveTrack(uint32 uTrack, float fLerpTime) {  }
private:

	void Start();
	void Stop();
	void Pause();
	
	WaveFile*				m_pSoundFile;
	bool					m_bLooping;
	bool					m_bPositional;
	IXAudio2SourceVoice*	m_pSourceVoice;
	XAUDIO2_BUFFER			m_buffer;
	uint32					m_uChannels;
	bool					m_bPaused;
	bool					m_bValid;
};

}

#endif
