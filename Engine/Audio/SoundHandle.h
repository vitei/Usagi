/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Handle to an instance of a sound
*****************************************************************************/
#ifndef __USG_AUDIO_SOUNDHANDLE_H__
#define __USG_AUDIO_SOUNDHANDLE_H__
#include "Engine/Common/Common.h"
#include "Engine/Core/Containers/SafePointer.h"
#include "SoundObject.h"

namespace usg{

class SoundHandle :  public SafePointer<SoundObject>
{
public:
	SoundHandle() {}
	~SoundHandle() {}

	void Start(float fTime=0.0f);
	void Stop(float fTime=0.0f);
	void Pause(float fTime=0.0f);
	bool IsPlaying() const;
	void SetVolume(float fVolume);
	void SetPitch(float fPitch);
	float GetRandomPitch() const;
	float GetVolume() const;
	bool IsValid() const { return GetPointer()!=NULL; }
	void SetActiveTrack(uint32 uTrack, float fFadeTime);

	SoundActorHandle GetSoundActor();

	// Not safe, only call if you are certain the pointer is valid, otherwise use
	// the accessor functions
	SoundObject* GetObject();
	SoundObject_ps* GetPlatform();

	SAFEPOINTER_COPY(SoundHandle)

private:
};



inline void SoundHandle::Start(float fTime)
{
	SoundObject* pObject = GetPointer();
	if(pObject)
	{
		pObject->Start(fTime);
	}
}

inline void SoundHandle::Pause(float fTime)
{
	SoundObject* pObject = GetPointer();
	if (pObject)
	{
		pObject->Pause(fTime);
	}
}


inline void SoundHandle::SetVolume(float fVolume)
{
	SoundObject* pObject = GetPointer();
	if(pObject)
	{
		pObject->SetVolume(fVolume);
	}
}

inline void SoundHandle::SetPitch(float fPitch)
{
	SoundObject* pObject = GetPointer();
	if (pObject)
	{
		pObject->SetPitch(fPitch);
	}
}

inline float SoundHandle::GetRandomPitch() const
{
	const SoundObject* pObject = GetPointer();
	if (pObject)
	{
		return pObject->GetRandomPitch();
	}
	return 1.0f;
}

inline float SoundHandle::GetVolume() const
{
	const SoundObject* pObject = GetPointer();
	if(pObject)
	{
		return pObject->GetVolume();
	}
	return 0.0f;
}


inline void SoundHandle::Stop(float fTime)
{
	SoundObject* pObject = GetPointer();
	if(pObject)
	{
		pObject->Stop(fTime);
	}
}


inline bool SoundHandle::IsPlaying() const
{
	const SoundObject* pObject = GetPointer();
	if(pObject)
	{
		return pObject->IsPlaying();
	}
	return false;
}

inline void SoundHandle::SetActiveTrack(uint32 uTrack, float fTime)
{
	SoundObject* pObject = GetPointer();
	if (pObject)
	{
		pObject->SetActiveTrack(uTrack, fTime);
	}
}



inline SoundObject_ps* SoundHandle::GetPlatform()
{
	SoundObject* pObject = GetPointer();
	if(pObject)
	{
		return &pObject->GetPlatform();
	}
	return NULL;
}

inline SoundActorHandle SoundHandle::GetSoundActor()
{
	SoundActorHandle retHndl;
	SoundObject* pObject = GetPointer();
	if(pObject)
	{
		retHndl = pObject->GetSoundActor();
	}
	return retHndl;
}

inline SoundObject* SoundHandle::GetObject()
{
	SoundObject* pObject = GetPointer();
	ASSERT(pObject!=NULL);
	return pObject;
}

}

#endif
