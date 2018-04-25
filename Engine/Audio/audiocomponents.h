#ifndef __ENGINE_AUDIO_COMPONENTS_H__
#define __ENGINE_AUDIO_COMPONENTS_H__
#include "Engine/Common/Common.h"
#include "Engine/Audio/SoundActorHandle.h"
#include "Engine/Audio/SoundHandle.h"
#include "Engine/Framework/Component.h"

namespace usg
{

	class AudioListener;

	template<>
	struct RuntimeData<SoundActorComponent>
	{
		SoundActorHandle hndl;
	};

	template<>
	struct RuntimeData<SoundComponent>
	{
		SoundHandle hndl;
	};

	template<>
	struct RuntimeData<AudioListenerComponent>
	{
		AudioListener* pListener;
	};
}

#endif