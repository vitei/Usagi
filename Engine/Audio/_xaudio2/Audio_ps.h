/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: 3D and streaming audio interface
*****************************************************************************/
#ifndef __USG_AUDIO_DUMMY_AUDIO_PS_H__
#define __USG_AUDIO_DUMMY_AUDIO_PS_H__
#include "Engine/Common/Common.h"
#include "Engine/Audio/SoundActorHandle.h"
#include "Engine/Maths/Vector4f.h"
#include "Engine/Audio/AudioBank.pb.h"
#include "Engine/Memory/Mem.h"
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
	void LoadSound(uint32 uSoundId);
	void AddListener(AudioListener* pListener);
	void RemoveListener(AudioListener* pListener);
	void Update(float fElapsed);
	void SetAudioDevice(char16* deviceName);

	IXAudio2* GetEngine() { return m_pXAudio2; }
	IXAudio2MasteringVoice* GetMasteringVoice() { return m_pMasteringVoice; }
private:
	PRIVATIZE_COPY(Audio_ps)

	bool	m_bInitialised;
	IXAudio2* m_pXAudio2;
	IXAudio2MasteringVoice* m_pMasteringVoice;

};

}


#endif
