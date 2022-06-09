/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: 3D and streaming audio interface
*****************************************************************************/
#ifndef __USG_AUDIO_AUDIO_H__
#define __USG_AUDIO_AUDIO_H__


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
class SoundCallbacks;
class AudioRoom;

struct StreamingSoundDef
{
	uint32 uChannels = 2;
	uint32 uBitsPerSample = 16;
	uint32 uSampleRate = 4800;
	usg::weak_ptr<SoundCallbacks> pCallbacks;
};

class Audio : public Singleton<Audio>
{
public:	
	struct SoundData
	{
		SoundHandle 	hndl;
		SoundObject		object;
	};

	struct CustomSound
	{
		SoundFileDef	def;
		void*			pRawData;
		size_t			rawDataSize;
	};

	Audio();
	~Audio();

	void Init();
	void SetChannelConfig(ChannelConfig eChannelConfig);	// Note will stop all active sounds! 
	ChannelConfig GetChannelConfig() const { return m_eChannelConfig; }
	void Update(float fElapsed);

	void LoadSoundArchive(const char* pszArchiveName, const char* pszLocalizedSubdirName = NULL);
	void UnloadArchive(const char* pszArchiveName);

	void LoadCustomArchive(const char* pszArchiveName, CustomSound* pSounds, uint32 uCount);

	void OriginOffset(usg::Vector3f vOrigin);
	AudioType GetAudioType(uint32 uSoundId);
	float GetAudioDuration(uint32 uSoundId);


	float GetVolume(uint32 uSoundId);
	SoundHandle PrepareCustomStream(const StreamingSoundDef& def, float fVolume = 1.0f);
	SoundHandle Prepare2DSound(uint32 crc, const float fVolume, bool bPlay=true );
	SoundHandle Prepare3DSound(SoundActorHandle& actorHandle, uint32 crc, const float fVolume, bool bPlay = true);
	// For when we want a one shot sound that isn't going to move with an actor
	SoundHandle Play3DSound(const Vector3f& vPos, uint32 uSoundId, const float fVolume);
	void SetVolume(const AudioType eType, const float fVolume);
	float GetVolume(const AudioType eType);
	void EnableEffect(const AudioType eType, uint32 uEffectCRC);
	void DisableEffect(const AudioType eType, uint32 uEffectCRC);

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

	AudioFilter* GetFilter(uint32 uCRC);
	AudioEffect* GetEffect(uint32 uCRC);
	AudioRoom* GetRoom(uint32 uCRC);

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
		SoundFile**		ppSoundFiles = nullptr;
		AudioFilter**	ppAudioFilters = nullptr;
		AudioEffect**	ppAudioEffects = nullptr;
		AudioRoom**		ppAudioRooms = nullptr;
		uint32			uFiles = 0;
		uint32			uFilters = 0;
		uint32			uEffects = 0;
		uint32			uRooms = 0;
	};
	
	usg::vector<Archive>			m_archives;
	hash_map<uint32, SoundFile*>	m_soundHashes;
	hash_map<uint32, AudioFilter*>	m_filterHashes;
	hash_map<uint32, AudioEffect*>	m_effectHashes;
	hash_map<uint32, AudioRoom*>	m_roomHashes;
	FastPool<AudioListener>			m_listeners;
	FastPool<ActorData>				m_actors;
	FastPool<SoundData>				m_sounds;
	float							m_fVolume[_AudioType_count];
	bool							m_bPaused[_AudioType_count];

	struct SpeakerInfo
	{
		SpeakerInfo() {}
		SpeakerInfo(float fAngle, uint32 uIndex) { fSpeakerHorAngle = fAngle; uChannelIndex = uIndex; }
		float fSpeakerHorAngle;	
		uint32 uChannelIndex;		
	};


	usg::vector<SpeakerInfo>	m_speakerInfo; // 2 greater than the actual number of speakers because we include the wrapped values
	ChannelConfig				m_eChannelConfig;

	Audio_ps					m_platform;
};

}

#endif

