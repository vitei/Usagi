/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform specific implementation of an audio room
*****************************************************************************/
#pragma once

#include "Engine/Audio/AudioRoom.h"
#include <xaudio2.h>

namespace usg{

	class Audio;

class AudioRoom_ps : public AudioRoom
{
public:	
	AudioRoom_ps();
	~AudioRoom_ps();

	virtual void Init(const AudioRoomDef* pDef, Audio* pAudio) override;

	IXAudio2Voice* GetVoice() { return m_pSubmixVoice; }
	
private:
	IXAudio2SubmixVoice*	m_pSubmixVoice = nullptr;
	usg::vector<IUnknown*>	m_effects;
};

}

