/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: An abstract interface for loading sounds
 *****************************************************************************/
#ifndef __USG_AUDIO_AUDIO_FILTER__
#define __USG_AUDIO_AUDIO_FILTER__
#include "Engine/Core/stl/string.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Audio/AudioBank.pb.h"

namespace usg
{
class Audio;

class AudioFilter
{
public:
	virtual void Init(const AudioFilterDef* pSoundFile, Audio* pAudio) = 0;
	const string& GetName() const { return m_name; }
	uint32 GetCRC() const { return m_uCRC;  }
protected:
	void InitInt(const AudioFilterDef* pFilter)
	{
		m_eType = pFilter->eFilter;
		m_fFrequency = pFilter->fFrequency;
		m_fOneOverQ = pFilter->fOneOverQ;
		m_name = pFilter->enumName;
		m_uCRC = pFilter->crc;
	}

	string			m_name;
	AudioFilterType m_eType;
	float			m_fFrequency;
	float			m_fOneOverQ;
	uint32			m_uCRC;
};

}

#endif
