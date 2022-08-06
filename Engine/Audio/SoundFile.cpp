/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: An abstract interface for loading sounds
 *****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Audio/Audio.h"
#include "SoundFile.h"

namespace usg
{

void SoundFile::InitInt(const SoundFileDef* pSoundFile, const string& strName, Audio* pAudio)
{
	m_bLooping = pSoundFile->loop;
	m_fInitVolume = pSoundFile->volume;
	m_fMinDistance = pSoundFile->minDistance;
	m_fMaxDistance = pSoundFile->maxDistance;
	m_eStacking = pSoundFile->eStacking;
	m_fLowPassAttenFactor = pSoundFile->lowPassAttenFactor;
	m_eType = pSoundFile->eType;
	m_strName = strName;
	m_eFalloff = pSoundFile->has_eFalloff ? pSoundFile->eFalloff : AudioFalloff_AUDIO_FALLOFF_LINEAR;
	m_fPitchRandomise = pSoundFile->has_pitchRandomisation ? pSoundFile->pitchRandomisation : 0.0f;
	m_fDopplerFactor = pSoundFile->has_dopplerFactor ? pSoundFile->dopplerFactor : 0.0f;
	m_fPitch = pSoundFile->has_basePitch ? pSoundFile->basePitch : 1.0f;
	m_uPriority = pSoundFile->has_priority ? pSoundFile->priority : 128;
	m_uCRC = pSoundFile->crc;

	m_pFilter = pSoundFile->filterCRC ? pAudio->GetFilter(pSoundFile->filterCRC) : nullptr;
	m_pRoom = pSoundFile->roomNameCRC ? pAudio->GetRoom(pSoundFile->roomNameCRC) : nullptr;
	for (uint32 i = 0; i < pSoundFile->effectCRCs_count; i++)
	{
		if(pSoundFile->effectCRCs[i])
		{
			m_effects.push_back(pAudio->GetEffect(pSoundFile->effectCRCs[i]));
		}
	}
}

}

