/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: An abstract for the concept of an audio environment
 *****************************************************************************/
#pragma once
#include "Engine/Core/stl/string.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Audio/AudioBank.pb.h"

namespace usg
{
class Audio;

class AudioRoom
{
public:
	virtual void Init(const AudioRoomDef* pSoundFile, Audio* pAudio) = 0;
	uint32 GetCRC() const { return m_uCRC;  }
protected:
	uint32			m_uCRC;
};

}
