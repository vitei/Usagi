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
#include "DummySoundFile.h"

namespace usg{

class SoundHandle;
class SoundHandle3D;
class AudioListener;
class SoundFile;
class SoundObject;

class Audio_ps
{
public:	
	Audio_ps();
	~Audio_ps();
	
	void Init();

	SoundFile* CreateSoundFile(const SoundFileDef* pDef) { return vnew(ALLOC_AUDIO) DummySoundFile(); }
	void LoadSound(uint32 uSoundId) {}
	void AddListener(AudioListener* pListener) {}
	void RemoveListener(AudioListener* pListener) {}

	void Update(float fElapsed);

private:
	PRIVATIZE_COPY(Audio_ps)


};

}


#endif
