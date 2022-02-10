/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: An abstract interface for loading sounds
 *****************************************************************************/
#ifndef __USG_AUDIO_AUDIO_EFFECT__
#define __USG_AUDIO_AUDIO_EFFECT__
#include "Engine/Core/stl/string.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Audio/AudioBank.pb.h"

namespace usg
{
class Audio;

class AudioEffect
{
public:
	virtual void Init(const AudioEffectDef* pEffect, Audio* pAudio) = 0;
	const string& GetName() const { return m_name; }
	uint32 GetCRC() const { return m_uCRC;  }
protected:
	void InitInt(const AudioEffectDef* pEffect)
	{
		m_eType = pEffect->eEffectType;
		m_name = pEffect->enumName;
		m_uCRC = pEffect->crc;
	}

	string			m_name;
	AudioEffectType m_eType;
	uint32			m_uCRC;
};

}

#endif
