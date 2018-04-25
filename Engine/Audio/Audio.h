/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: 3D and streaming audio interface
*****************************************************************************/
#ifndef __USG_AUDIO_AUDIO_H__
#define __USG_AUDIO_AUDIO_H__

#include "Engine/Common/Common.h"
#include "Engine/Core/Singleton.h"
#include "Engine/Audio/AudioListener.h"
#include "Engine/Framework/ComponentEntity.h"
#include "Engine/Memory/FastPool.h"
#include "Engine/Core/stl/hash_map.h"
#include "Engine/Core/stl/vector.h"
#include "SoundActorHandle.h"
#include "SoundHandle.h"
#include AUDIO_HEADER(Engine/Audio/, Audio_ps.h)

#define MAX_VOL 1.0f

namespace usg
{

class SoundHandle;
class Vector3f;
class SoundFile;
class IHeadMountedDisplay;

class Audio : public Singleton<Audio>
{
public:	
	struct SoundData
	{
		SoundHandle 	hndl;
		SoundObject		object;
	};

	Audio();
	~Audio();

	void Init();
	void Update(float fElapsed);

	void LoadSoundArchive(const char* pszArchiveName, const char* pszLocalizedSubdirName = NULL);
	void UnloadArchive(const char* pszArchiveName);
	AudioType GetAudioType(uint32 uSoundId);
	float GetVolume(uint32 uSoundId);
	SoundHandle Prepare2DSound(uint32 crc, const float fVolume, bool bPlay=true );
	SoundHandle Prepare3DSound(SoundActorHandle& actorHandle, uint32 crc, const float fVolume, bool bPlay = true);
	// For when we want a one shot sound that isn't going to move with an actor
	SoundHandle Play3DSound(const Vector3f& vPos, uint32 uSoundId, const float fVolume);
	void SetVolume(const AudioType eType, const float fVolume);
	float GetVolume(const AudioType eType);
	void StopAll(AudioType eType, float fTime=0.2f);
	void PauseAll(AudioType eType, float fTime = 0.2f);
	void ResumeAll(AudioType eType, float fTime = 0.2f);
	void StartUsingHeadset(IHeadMountedDisplay* pHMD);

	SoundActorHandle CreateSoundActor(const Vector3f &vPos);

	Audio_ps& GetPlatform() { return m_platform; }

	AudioListener*	CreateListener(const Matrix4x4 &mMat, bool bRemoteOutput);
	void			DeleteListener(AudioListener* pRemove);
	void			SetMixParams(const Vector3f& vPos, const Vector3f& vVel, SoundObject* objectBase);
	float			GetDoppler(const AudioListener* pListener, const Vector3f& vScreenPos, const Vector3f &vVelocity, SoundObject* objectBase);
	FastPool<SoundData>* GetPool() { return &m_sounds; }

private:
	bool ShouldPlay(Vector3f vPos, SoundFile* pSoundFile);
	SoundHandle Play3DSoundInt(SoundActorHandle& actorHandle, SoundFile* pSoundFile, const float fVolume, bool bPlay);
	SoundHandle CreateSound(SoundActorHandle& actor);
	void InitPanningData(float fAzimuthalAngle, PanningData& panning, float fDistance, float fSpeakerRadius);
	void CalculateInnerPanning(float fAzimuthalAngle, PanningData& panning, float fFocusAngle);

	struct ActorData
	{
		SoundActorHandle hndl;
		SoundActor 		 actor;
	};

	struct Archive
	{
		char			name[USG_MAX_PATH];
		SoundFile**		ppSoundFiles;
		uint32			uFiles;
	};
	
	usg::vector<Archive>			m_archives;
	hash_map<uint32, SoundFile*>	m_soundHashes;
	FastPool<AudioListener>			m_listeners;
	FastPool<ActorData>				m_actors;
	FastPool<SoundData>				m_sounds;
	float							m_fVolume[_AudioType_count];
	bool							m_bPaused[_AudioType_count];

	struct SpeakerInfo
	{
		float	fSpeakerHorAngles[SOUND_CHANNEL_COUNT+2];	// +2 because we include the wrapped values
		uint32	uChannelIndex[SOUND_CHANNEL_COUNT + 2];
		uint32  uSpeakerValues;
	};

	SpeakerInfo	m_speakerInfo;

	Audio_ps	m_platform;
};

}

#endif

