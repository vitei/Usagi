/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: 3D and streaming audio interface
*****************************************************************************/
#ifndef __USG_AUDIO_DUMMY_AUDIO_PS_H__
#define __USG_AUDIO_DUMMY_AUDIO_PS_H__

#include "Engine/Audio/SoundActorHandle.h"
#include "Engine/Maths/Vector4f.h"
#include "Engine/Audio/AudioBank.pb.h"
#include "Engine/Memory/Mem.h"
#include "Engine/Audio/AudioDefs.h"
#include "WaveFile.h"

namespace usg{

class SoundHandle;
class SoundHandle3D;
class AudioListener;
class SoundFile;
class SoundObject;
class IHeadMountedDisplay;

class Audio_ps
{
public:	
	Audio_ps();
	~Audio_ps();
	
	void Init();

	SoundFile* CreateSoundFile(const SoundFileDef* pDef);
	AudioFilter* CreateAudioFilter(const AudioFilterDef* pDef);
	AudioEffect* CreateAudioEffect(const AudioEffectDef* pDef);

	void AddListener(AudioListener* pListener);
	void RemoveListener(AudioListener* pListener);
	void Update(float fElapsed);
	void SetAudioDevice(char16* deviceName);
	void SetOutputChannelConfig(ChannelConfig eChannelConfig);
	static uint32 GetChannelCount(ChannelConfig eChannelConfig);

	void EnableEffect(AudioType eType, AudioEffect* pEffect);
	void DisableEffect(AudioType eType, AudioEffect* pEffect);

	IXAudio2* GetEngine() { return m_pXAudio2; }
	IXAudio2MasteringVoice* GetMasteringVoice() { return m_pMasteringVoice; }
	IXAudio2SubmixVoice* GetSubmixVoice(AudioType eType) { return m_pSubmixVoices[eType]; }
private:
	PRIVATIZE_COPY(Audio_ps)

	bool	m_bInitialised;
	IXAudio2* m_pXAudio2;
	IXAudio2MasteringVoice* m_pMasteringVoice;
	IXAudio2SubmixVoice*	m_pSubmixVoices[_AudioType_count];

};

}


#endif
