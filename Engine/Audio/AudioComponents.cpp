#include "Engine/Common/Common.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Audio/AudioComponents.pb.h"

namespace usg
{

	template<>
	void OnDeactivate<SoundComponent>(Component<SoundComponent>& p, ComponentLoadHandles& handles)
	{
		// Make sure to clean up our handle
		p.GetRuntimeData().hndl.RemoveRef();
	}

	template<>
	void OnActivate<SoundActorComponent>(Component<SoundActorComponent>& s)
	{
		s.GetRuntimeData().hndl = Audio::Inst()->CreateSoundActor(Vector3f::ZERO);
	}

	template<>
	void OnDeactivate<SoundActorComponent>(Component<SoundActorComponent>& s, ComponentLoadHandles& handles)
	{
		// We shouldn't continuing holding a reference to this actor
		s.GetRuntimeData().hndl.RemoveRef();
	}

	template<>
	void OnActivate<AudioListenerComponent>(Component<AudioListenerComponent>& s)
	{
		Matrix4x4 mTmp;
		mTmp.LoadIdentity();
		s.GetRuntimeData().pListener = Audio::Inst()->CreateListener(mTmp, false);
	}

	template<>
	void OnDeactivate<AudioListenerComponent>(Component<AudioListenerComponent>& s, ComponentLoadHandles& handles)
	{
		Audio::Inst()->DeleteListener(s.GetRuntimeData().pListener);
	}

}