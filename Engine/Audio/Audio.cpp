/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/Containers/List.h"
#include "Engine/Audio/AudioComponents.pb.h"
#include "Engine/Audio/SoundFile.h"
#include "Engine/Graphics/Device/IHeadMountedDisplay.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "AudioListener.h"
#include <float.h>
#include "Audio.h"

#define DISABLE_SOUND 0

namespace usg
{

Audio::Audio():
m_listeners(5, false),
m_actors(256, false),
m_sounds(512, false)
{
	// Speaker angle values for stereo sound
	// Wrapped version of right, turned into a negative
	m_speakerInfo.fSpeakerHorAngles[0] = -Math::pi - Math::pi_over_2;
	m_speakerInfo.fSpeakerHorAngles[1] = -Math::pi_over_2;
	m_speakerInfo.fSpeakerHorAngles[2] = Math::pi_over_2;
	// Wrapped version of left turned into a positive
	m_speakerInfo.fSpeakerHorAngles[3] = Math::pi + Math::pi_over_2;

	m_speakerInfo.uChannelIndex[0] = SOUND_CHANNEL_RIGHT;
	m_speakerInfo.uChannelIndex[1] = SOUND_CHANNEL_LEFT;
	m_speakerInfo.uChannelIndex[2] = SOUND_CHANNEL_RIGHT;
	m_speakerInfo.uChannelIndex[3] = SOUND_CHANNEL_LEFT;

	m_speakerInfo.uSpeakerValues = 4;	// Only supporting stereo sound at the moment

	for (uint32 i = 0; i < _AudioType_count; i++)
	{
		m_bPaused[i] = false;
	}

#if 0
	SoundObject::PanningData panning;
	for (float fPan = -Math::pi; fPan < Math::pi; fPan += 0.2f)
	{
		DEBUG_PRINT("\n\n Pan: %f\n", fPan);
		InitPanningData(fPan, panning, 1.0f, 1.0f);
		DEBUG_PRINT("Far panning: %f %f\n", panning.fMatrix[SoundObject::SOUND_CHANNEL_LEFT], panning.fMatrix[SoundObject::SOUND_CHANNEL_RIGHT]);
		InitPanningData(fPan, panning, 0.999999999f, 1.0f);
		DEBUG_PRINT("Inner panning: %f %f\n", panning.fMatrix[SoundObject::SOUND_CHANNEL_LEFT], panning.fMatrix[SoundObject::SOUND_CHANNEL_RIGHT]);
		InitPanningData(fPan, panning, 0.5f, 1.0f);
		DEBUG_PRINT("Half inner: %f %f\n", panning.fMatrix[SoundObject::SOUND_CHANNEL_LEFT], panning.fMatrix[SoundObject::SOUND_CHANNEL_RIGHT]);
		InitPanningData(fPan, panning, 0.25f, 1.0f);
		DEBUG_PRINT("Mostly inner: %f %f\n", panning.fMatrix[SoundObject::SOUND_CHANNEL_LEFT], panning.fMatrix[SoundObject::SOUND_CHANNEL_RIGHT]);
	}
#endif
}

Audio::~Audio()
{
	// Kill all the sounds
	List<SoundData> removeList;
	for(FastPool<SoundData>::Iterator it = m_sounds.Begin(); !it.IsEnd(); ++it)
	{
		removeList.AddToEnd(*it);
	}

	for (List<SoundData>::Iterator it = removeList.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->hndl.Destroy(&(*it)->object);
		(*it)->object.Reset();
		m_sounds.Free(*it);
	}

	// Destory the sound files
	for(usg::vector<Archive>::iterator it = m_archives.begin(); it!=m_archives.end(); it++)
	{
		for(uint32 i=0; i< (*it).uFiles; i++)
		{
			(*it).ppSoundFiles[i]->Cleanup(this);
			vdelete(*it).ppSoundFiles[i];
			(*it).ppSoundFiles[i] = NULL;
		}

		vdelete[](*it).ppSoundFiles;
		(*it).ppSoundFiles = NULL;
	}

	m_archives.clear();
}

void Audio::LoadCustomArchive(const char* pszArchiveName, CustomSound* pSounds, uint32 uCount)
{
#if !DISABLE_SOUND

	Archive archive;
	str::Copy(archive.name, pszArchiveName, USG_MAX_PATH);

	archive.ppSoundFiles = vnew(ALLOC_AUDIO)SoundFile*[uCount];

	for (uint32 i = 0; i < uCount; i++)
	{
		const SoundFileDef* pDef = &pSounds[i].def;
		archive.ppSoundFiles[i] = m_platform.CreateSoundFile(pDef);
		ASSERT(archive.ppSoundFiles[i] != NULL);
		archive.ppSoundFiles[i]->InitRaw(pDef, pSounds[i].pRawData, pSounds[i].rawDataSize, this);
		ASSERT(m_soundHashes[pDef->crc] == NULL);
		m_soundHashes[pDef->crc] = archive.ppSoundFiles[i];
	}
#endif
	archive.uFiles = uCount;
	m_archives.push_back(archive);
}

void Audio::LoadSoundArchive(const char* pszArchiveName, const char* pszLocalizedSubdirName)
{
#if !DISABLE_SOUND
	AudioBank bank;
	ProtocolBufferFile test_vpb(pszArchiveName);
	bool bReadSucceeded = test_vpb.Read(&bank);

	uint32 uCount = (uint32)bank.soundFiles.m_decoderDelegate.data.count;

	Archive archive;
	str::Copy(archive.name, pszArchiveName, USG_MAX_PATH);

	archive.ppSoundFiles = vnew(ALLOC_AUDIO)SoundFile*[uCount];

	for (uint32 i = 0; i < uCount; i++)
	{
		const SoundFileDef* pDef = &bank.soundFiles[i];
		archive.ppSoundFiles[i] = m_platform.CreateSoundFile(pDef);
		ASSERT(archive.ppSoundFiles[i] != NULL);
		archive.ppSoundFiles[i]->Init(pDef, this, pszLocalizedSubdirName);
		ASSERT(m_soundHashes[pDef->crc] == NULL);
		m_soundHashes[pDef->crc] = archive.ppSoundFiles[i];
	}
#endif
	archive.uFiles = uCount;
	m_archives.push_back(archive);
}

void Audio::UnloadArchive(const char* pszArchiveName)
{
	uint32 uIndex = USG_INVALID_ID;
	for (uint32 i = 0; i < m_archives.size(); i++)
	{
		if (str::Compare(pszArchiveName, m_archives[i].name))
		{
			uIndex = i;
			break;
		}
	}

	ASSERT(uIndex != USG_INVALID_ID);
	if (uIndex != USG_INVALID_ID)
	{
		Archive& archive = m_archives[uIndex];
		for (uint32 i = 0; i < archive.uFiles; i++)
		{
			m_soundHashes.erase(archive.ppSoundFiles[i]->GetCRC());
			archive.ppSoundFiles[i]->Cleanup(this);
			vdelete archive.ppSoundFiles[i];
			archive.ppSoundFiles[i] = NULL;
		}

		vdelete[] archive.ppSoundFiles;
		archive.ppSoundFiles = NULL;

		m_archives.erase(&archive);
	}
}

void Audio::Init()
{
#if !DISABLE_SOUND
	m_platform.Init();

	for (FastPool<ActorData>::Iterator it = m_actors.EmptyBegin(); !it.IsEnd(); ++it)
	{
		(*it)->actor.Init();
	}

	for (FastPool<SoundData>::Iterator it = m_sounds.EmptyBegin(); !it.IsEnd(); ++it)
	{
		(*it)->object.Init(this);
	}

	for(uint32 i = 0; i < _AudioType_count; i++)
	{
		m_fVolume[i] = MAX_VOL;
	}
#endif
}


AudioListener* Audio::CreateListener(const Matrix4x4 &mMat, bool bRemoteOutput)
{
	AudioListener* pListener = m_listeners.Alloc();
#if !DISABLE_SOUND
	m_platform.AddListener(pListener);
	pListener->SetRemoteOutput(bRemoteOutput);
	pListener->SetMatrix(mMat);
#endif
	return pListener;
}


void Audio::DeleteListener(AudioListener* pRemove)
{
	m_platform.RemoveListener(pRemove);
	m_listeners.Free(pRemove);
}



void Audio::Update(float fElapsed)
{
#if !DISABLE_SOUND
	{
		for(FastPool<ActorData>::DynamicIterator it = m_actors.BeginDynamic(); !it.IsEnd(); ++it)
		{
			ActorData* pActor = (*it);
			if (!pActor->hndl.IsReferenced())
			{
				(*it)->hndl.Destroy(&(*it)->actor);
				it.RemoveElement();
			}
		}
	}

	for(FastPool<SoundData>::DynamicIterator it = m_sounds.BeginDynamic(); !it.IsEnd(); ++it)
	{
		SoundData* pSound = (*it);
		bool bRemove = false;
		// Remove the object either if it's not referenced and it's a looping sound, or if it's finished playing
		if (!pSound->hndl.IsReferenced() && !pSound->object.IsPlaying() )
		{
			bRemove = true;
		}
		else if(!pSound->hndl.IsReferenced() && pSound->object.GetLooping() && !pSound->object.IsStopping())
		{
			pSound->object.Stop(0.16f);
		}
			
		if(pSound->object.GetSoundActor().IsValid())
		{
			SetMixParams(pSound->object.GetSoundActor().GetPosition(), pSound->object.GetSoundActor().GetVelocity(), &pSound->object );
		}

		if ( !(pSound->object.GetRequestedPlayState() == PLAY_STATE_PLAYING) || !(m_bPaused[pSound->object.GetAudioType()]))
		{
			pSound->object.Update(fElapsed);
			pSound->object.ClearRequestedPlayState();
		}

		if (bRemove)
		{
			(*it)->hndl.Destroy(&(*it)->object);
			(*it)->object.Reset();
			it.RemoveElement();
		}
	}

	m_platform.Update(fElapsed);
#endif
}

AudioType Audio::GetAudioType(uint32 uSoundId)
{
	SoundFile* pFile = m_soundHashes[uSoundId];
	if (pFile)
	{
		return pFile->GetAudioType();
	}
	//ASSERT(false);
	return AUDIO_TYPE_SFX;
}

float Audio::GetVolume(uint32 uSoundId)
{
	SoundFile* pFile = m_soundHashes[uSoundId];
	if (pFile)
	{
		return pFile->GetInitialVolume();
	}
	//ASSERT(false);
	return 0.0f;
}

SoundHandle Audio::PrepareCustomStream(const StreamingSoundDef& def, float fVolume)
{
#if !DISABLE_SOUND	
	SoundActorHandle emptyHndl;
	SoundHandle handle;

	handle = CreateSound(emptyHndl);
	if (handle.IsValid())
	{
		//DEBUG_PRINT("[Audio::PrepareSound] %s\n", m_ppSoundFiles[uSoundId]->GetName().CStr());
		handle.GetObject()->SetCustomData(def);
		handle.SetVolume(fVolume);
	}
	return handle;

#else
	return SoundHandle();
#endif
}

SoundHandle Audio::Prepare2DSound(uint32 crc, const float fVolume, bool bPlay)
{
#if !DISABLE_SOUND	
	SoundActorHandle emptyHndl;	
	SoundHandle handle;
	SoundFile* pSoundFile = m_soundHashes[crc];
	if (!pSoundFile)
	{
		DEBUG_PRINT("Failed to find sound %u", crc);
	}
	if(pSoundFile)
	{
		handle = CreateSound(emptyHndl);
		if (handle.IsValid())
		{
			//DEBUG_PRINT("[Audio::PrepareSound] %s\n", m_ppSoundFiles[uSoundId]->GetName().CStr());
			pSoundFile->BindToSoundObject(handle.GetObject(), false);
			handle.GetObject()->SetSoundFile(pSoundFile);
			handle.GetObject()->ScaleVolumeBySystemVolume(
				m_fVolume[pSoundFile->GetAudioType()]
			);
			handle.SetVolume(fVolume);
			if (bPlay)
			{
				handle.Start();
			}
		}
	}

	return handle;
#else
	return SoundHandle();
#endif
}


bool Audio::ShouldPlay(Vector3f vPos, SoundFile* pSoundFile)
{
	// Just assume that looping sounds need to be played as they may move into range
	if (pSoundFile->GetLooping())
		return true;

	bool bInRange = false;
	float fMaxDistSq = pSoundFile->GetMaxDistance()*pSoundFile->GetMaxDistance();
	float fMinDistSq = FLT_MAX;

	for (FastPool<AudioListener>::Iterator it = m_listeners.Begin(); !it.IsEnd(); ++it)
	{
		float fCullDist = pSoundFile->GetMaxDistance() + (*it)->GetCullRadius();
		float fCullDistSq = fCullDist * fCullDist;
		float fDistSq = (*it)->GetMatrix().vPos().v3().GetSquaredDistanceFrom(vPos);
		
		bInRange |= fDistSq < fCullDistSq;
	}

	return bInRange;

}

SoundHandle Audio::Prepare3DSound(SoundActorHandle& actorHandle, uint32 crc, const float fVolume, bool bPlay)
{
#if !DISABLE_SOUND	
	// TODO: Add a distance check before even bothering to create the sound
	SoundFile* pSoundFile = m_soundHashes[crc];
	if (!pSoundFile)
	{
		DEBUG_PRINT("Failed to find sound %u", crc);
	}
	if (pSoundFile && ShouldPlay(actorHandle.GetPosition(), pSoundFile))
	{
		return Play3DSoundInt(actorHandle, pSoundFile, fVolume, bPlay);
	}
#endif
return SoundHandle();
}

SoundHandle Audio::Play3DSoundInt(SoundActorHandle& actorHandle, SoundFile* pSoundFile, const float fVolume, bool bPlay)
{
	SoundHandle handle;
	if (pSoundFile)
	{
		handle = CreateSound(actorHandle);
		if (handle.IsValid())
		{
			//DEBUG_PRINT("[Audio::Play3DSoundInt] %s\n", m_ppSoundFiles[uSoundId]->GetName().CStr());
			pSoundFile->BindToSoundObject(handle.GetObject(), true);
			handle.GetObject()->SetSoundFile(pSoundFile);
			handle.GetObject()->ScaleVolumeBySystemVolume(
				m_fVolume[pSoundFile->GetAudioType()]
			);
			if (bPlay)
			{
				handle.Start();
			}
		}
	}
	return handle;
}

SoundHandle Audio::Play3DSound(const Vector3f& vPos, uint32 crc, const float fVolume)
{
#if !DISABLE_SOUND	
	SoundFile* pSoundFile = m_soundHashes[crc];
	if (pSoundFile && ShouldPlay(vPos, pSoundFile))
	{
		SoundActorHandle actor = CreateSoundActor(vPos);
		return Play3DSoundInt(actor, pSoundFile, fVolume, true);
	}
#endif
	return SoundHandle();
}


void Audio::SetVolume(const AudioType eType, const float fVolume)
{
	ASSERT((pb_size_t)eType < _AudioType_count);
	ASSERT((fVolume >= 0.0f) && (fVolume <= MAX_VOL));

	m_fVolume[eType] = fVolume;

	for (FastPool<SoundData>::Iterator it = m_sounds.Begin(); !it.IsEnd(); ++it)
	{
		if ((*it)->object.GetSoundFile()->GetAudioType() == eType)
		{
			(*it)->hndl.GetObject()->ScaleVolumeBySystemVolume(fVolume);
		}
	}
}

float Audio::GetVolume(const AudioType eType)
{
	ASSERT((pb_size_t)eType < _AudioType_count);

	return m_fVolume[eType];
}

void Audio::StopAll(AudioType eType, float fTime)
{
	for (FastPool<SoundData>::Iterator it = m_sounds.Begin(); !it.IsEnd(); ++it)
	{
		if ((*it)->object.GetAudioType() == eType)
		{
			(*it)->hndl.Stop(fTime);
		}
	}
	m_bPaused[eType] = false;
}

void Audio::PauseAll(AudioType eType, float fTime)
{
	for (FastPool<SoundData>::Iterator it = m_sounds.Begin(); !it.IsEnd(); ++it)
	{
		if ((*it)->object.GetAudioType() == eType)
		{
			(*it)->hndl.Pause(fTime);
		}
	}
	m_bPaused[eType] = true;
}

void Audio::ResumeAll(AudioType eType, float fTime)
{
	for (FastPool<SoundData>::Iterator it = m_sounds.Begin(); !it.IsEnd(); ++it)
	{
		if ((*it)->object.GetAudioType() == eType && (*it)->object.IsPaused())
		{
			(*it)->hndl.Start(fTime);
		}
	}	
	m_bPaused[eType] = false;
}


void Audio::StartUsingHeadset(IHeadMountedDisplay* pHMD)
{
	m_platform.SetAudioDevice(pHMD->GetAudioDeviceName());
}


SoundActorHandle Audio::CreateSoundActor(const Vector3f &vPos)
{
#if !DISABLE_SOUND	
	ActorData* pActorData = m_actors.Alloc();
	if(pActorData)
	{
		pActorData->hndl.Init(&pActorData->actor);

		pActorData->hndl.SetPosition(vPos);
		return pActorData->hndl;
	}
#endif
	return SoundActorHandle();
}


SoundHandle Audio::CreateSound(SoundActorHandle& actor)
{
#if !DISABLE_SOUND	
	SoundData* pSoundData = m_sounds.Alloc();
	if(pSoundData)
	{
		pSoundData->hndl.Init(&pSoundData->object);
		pSoundData->object.SetActor(actor);

		return pSoundData->hndl;
	}
#endif
	return SoundHandle();
}


// Once we have determined the two speakers to pan between we then calculate their corresponding panning values
inline void CalculatePanning(float fAzimuthalAngle, float fSpeaker1Angle, float fSpeaker2Angle, float& fPanning1Out, float& fPanning2Out)
{
	float fBeta = (fAzimuthalAngle - fSpeaker1Angle) / (fSpeaker2Angle - fSpeaker1Angle);
	Math::SinCos(Math::pi_over_2 * fBeta, fPanning2Out, fPanning1Out);
}


// Not very efficent, but will work on any speaker configuration
void Audio::InitPanningData(float fAzimuthalAngle, PanningData& panning, float fDistance, float fSpeakerRadius)
{
	for (uint32 i = 0; i < SOUND_CHANNEL_COUNT; i++)
	{
		panning.fMatrix[i] = 0.0f;
	}

	if(fDistance >= fSpeakerRadius)
	{
		// Treat as a point sound
		for (uint32 i = 0; i < m_speakerInfo.uSpeakerValues-1; i++)
		{
			if (fAzimuthalAngle >= m_speakerInfo.fSpeakerHorAngles[i] &&
				fAzimuthalAngle <= m_speakerInfo.fSpeakerHorAngles[i + 1])
			{
				CalculatePanning(fAzimuthalAngle, m_speakerInfo.fSpeakerHorAngles[i], m_speakerInfo.fSpeakerHorAngles[i + 1],
					panning.fMatrix[m_speakerInfo.uChannelIndex[i]], panning.fMatrix[m_speakerInfo.uChannelIndex[i + 1]]);
				return;
			}
		}
		// We have an angle out of range
		ASSERT(false);
	}
	else
	{
		// Treat as an arc
		if(Math::IsEqual(fDistance, 0.0f))
		{
			const float fValue = sqrtf(1.0f/SOUND_CHANNEL_COUNT);
			for (uint32 i = 0; i < SOUND_CHANNEL_COUNT; i++)
			{
				panning.fMatrix[i] = fValue;		
			}
		}
		else
		{
			float fFocusAngle = (1.0f-(fDistance/fSpeakerRadius)) * Math::pi;
			CalculateInnerPanning(fAzimuthalAngle, panning, fFocusAngle);
		}
	}
}

inline float GetLinPanning(float fAzimuthalAngle, float fSpeaker1Angle, float fSpeaker2Angle)
{
	float fBeta = (fAzimuthalAngle - fSpeaker1Angle) / (fSpeaker2Angle - fSpeaker1Angle);
	return fBeta;
}


float GetInnerPanningForChannel(float fSpeakerAngle, float fLeftSpeaker, float fRightSpeaker, float fAzimuthalAngle, float fFocusAngle)
{
	float fValue = 0.0f;
	if (fAzimuthalAngle > fSpeakerAngle)
	{
		// Starts to the right of us
		float fArcLeft = fAzimuthalAngle - fFocusAngle;
		bool bOnLeft = fArcLeft < fSpeakerAngle;

		if (bOnLeft)	// We are inside the arc
			return 1.0f;

		if (fArcLeft < fRightSpeaker)
			fValue = 1.0f-GetLinPanning(fArcLeft, fSpeakerAngle, fRightSpeaker);

		// Check for wrap around to the next speaker
		float fArcRight = fAzimuthalAngle + fFocusAngle;
		
		fArcRight = Math::WrapValue(fArcRight, -Math::pi*2.f, 0.0f);
		if (fArcRight > fSpeakerAngle)
		{
			return 1.0f;	// Wrapped over us from the other side
		}
		else if (fArcRight > fLeftSpeaker)
		{
			float fWrapPanning = GetLinPanning(fArcRight, fLeftSpeaker, fSpeakerAngle);
			fValue = Math::Max(fValue, fWrapPanning);
		}
	}
	else
	{
		// Starts to the left of us
		float fArcRight = fAzimuthalAngle + fFocusAngle;
		bool bOnRight = fArcRight > fSpeakerAngle;

		if (bOnRight)
			return 1.0f;

		if (fArcRight > fLeftSpeaker)
			fValue = GetLinPanning(fArcRight, fLeftSpeaker, fSpeakerAngle);

		// Check for wrap around to the next speaker
		float fArcLeft = fAzimuthalAngle - fFocusAngle;
	
		// Wrap it around
		fArcLeft = Math::WrapValue(fArcLeft, 0.0f, Math::pi*2.f);
		if (fArcLeft < fSpeakerAngle)
		{
			return 1.0f;	// Wrapped over us from the other side
		}
		else if (fArcLeft < fRightSpeaker)
		{
			float fWrapPanning = 1.0f - GetLinPanning(fArcLeft, fSpeakerAngle, fRightSpeaker);
			fValue = Math::Max(fValue, fWrapPanning);
		}
		
	}

	return fValue;
}

void Audio::CalculateInnerPanning(float fAzimuthalAngle, PanningData& panning, float fFocusAngle)
{
	// This calculation is more expensive but shouldn't be called on many sounds
	// With a zero angle we need to maintain consistency with the standard calculations,
	// so we are assuming any speaker in the arc is at full blast

	uint32 uCenter = 0;
	uint32 uLeft = 0;
	uint32 uRight = 0;

	for (uint32 i = 1; i < m_speakerInfo.uSpeakerValues - 1; i++)
	{
		uint32 uIndex = m_speakerInfo.uChannelIndex[i];
		panning.fMatrix[uIndex] = GetInnerPanningForChannel(m_speakerInfo.fSpeakerHorAngles[i], m_speakerInfo.fSpeakerHorAngles[i - 1], m_speakerInfo.fSpeakerHorAngles[i + 1], fAzimuthalAngle, fFocusAngle);
	}


	float fSum = 0.0f;
	for (uint32 i = 0; i < SOUND_CHANNEL_COUNT; i++)
	{
		fSum += (panning.fMatrix[i] * panning.fMatrix[i]);
	}
	
	fSum = sqrtf(fSum);
	// Maintain constant power
	for (uint32 i = 0; i < SOUND_CHANNEL_COUNT; i++)
	{
		panning.fMatrix[i] /= fSum;
	}


}

void Audio::SetMixParams(const Vector3f& vPos, const Vector3f& vVel, SoundObject* object)
{
//	float fVolume;
	// Assuming only one listener for the moment, but we could easily have two 3d screens
	static const int maxListeners = 5;
	ASSERT(m_listeners.Size() < maxListeners);
	bool bShouldPlay = false;

	PanningData totalPanning;
	// Zero out the panning
	for (uint32 i = 0; i < SOUND_CHANNEL_COUNT; i++)
	{
		totalPanning.fMatrix[i] = 0.0f;
	}
	float fMaxAttenVal = 0.0f;
	float fDopplerAccum = 0.0f;
	float fAttenAccum = 0.0f;

	uint32 uNumberInRange = 0;
	const SoundFile* pSoundFile = object->GetSoundFile();//(*it)->GetMaxRange();

	for( FastPool<AudioListener>::Iterator it = m_listeners.Begin(); !it.IsEnd(); ++it)
	{
		float fMaxDistance = pSoundFile->GetMaxDistance();
		float fMinDistance = pSoundFile->GetMinDistance();
		float fCullDistance = fMaxDistance + (*it)->GetCullRadius();

		float fDistance = (*it)->GetAttenuationPos().GetDistanceFrom(vPos);

		float fAtten = 0.0f;
		float fRange = fMaxDistance - fMinDistance;

		bShouldPlay |= (object->GetLooping() || fDistance < fCullDistance);

		if(fDistance < fMaxDistance)
		{
			Vector3f vTransPos = (*it)->GetViewMatrix().TransformVec3(vPos, 1.0f);
			Vector3f vTransNorm(vTransPos.x, 0.0f, vTransPos.z);
			float fTransPosMag = vTransNorm.Magnitude();
			vTransNorm = vTransNorm.GetNormalisedIfNZero(Vector3f::Z_AXIS);

			float fAngle = acosf(vTransNorm.z) * Math::Sign(vTransNorm.x);
			float fSpeakerRadius = (*it)->GetSpeakerRadius();

			PanningData panning;
			InitPanningData(fAngle, panning, fTransPosMag, fSpeakerRadius);

			fDistance = Math::Clamp(fDistance, fMinDistance, fMaxDistance)-fMinDistance;
 			float fLerp = (fDistance / fRange);

			switch (pSoundFile->GetFalloff())
			{
			case AUDIO_FALLOFF_LINEAR:
				fAtten = 1.0f - (fDistance / fRange);
				break;
			case AUDIO_FALLOFF_LOGARITHMIC:
				fAtten = powf(0.5f, fLerp);
				break;
			default:
				ASSERT(false);
			}

			// Only handle the first one for now
			if (pSoundFile->GetDopplerFactor() > 0.0f)
			{
				Vector3f vTransVel = (*it)->GetViewMatrix().TransformVec3(vVel, 1.0f);
				fDopplerAccum = GetDoppler(*it, vTransPos.GetNormalisedIfNZero(), vTransVel, object) * fAtten;
			}


			for (uint32 i = 0; i < SOUND_CHANNEL_COUNT; i++)
			{
				totalPanning.fMatrix[i] += panning.fMatrix[i] * fAtten;
			}

			fAttenAccum += fAtten;

			uNumberInRange++;
		}
		
		fMaxAttenVal = Math::Max(fAtten, fMaxAttenVal);	
	}

	if (!bShouldPlay)
	{
		// Automatically cull at a distance
		object->Stop();
	}
	else if(uNumberInRange > 0 && fAttenAccum > Math::EPSILON)
	{
		object->SetAttenMul(fMaxAttenVal);

		float fMultiplier = 1.f / fAttenAccum;
		for (uint32 i = 0; i < SOUND_CHANNEL_COUNT; i++)
		{
			totalPanning.fMatrix[i] *= fMultiplier;
		}
		object->SetPanningData(totalPanning);
		if (pSoundFile->GetDopplerFactor() > 0.0f)
		{
			object->SetDopplerFactor(fDopplerAccum * fMultiplier);
		}
	}
	else
	{
		object->SetAttenMul(0.0f);
	}
}

float Audio::GetDoppler(const AudioListener* pListener, const Vector3f& vNormalisedPos, const Vector3f &vVelInListenrSpace, SoundObject* object)
{
	const SoundFile* pSoundFile = object->GetSoundFile();
	const float32 dopplerFactor = pSoundFile->GetDopplerFactor();

	float32 fSonicVelocity = pListener->GetSoundSpeed();

	float32 fActorVelocity = -DotProduct(vNormalisedPos, vVelInListenrSpace);
	float32 fListenerVelocity = -DotProduct(vNormalisedPos, pListener->GetListenerSpaceVelocity());

	fActorVelocity *= dopplerFactor;
	fListenerVelocity *= dopplerFactor;
	fActorVelocity = Math::Clamp(fActorVelocity, 0.0f, fSonicVelocity);
	float32 fPitch = (fSonicVelocity - fListenerVelocity) / (fSonicVelocity - fActorVelocity);

	return fPitch;
}

}
