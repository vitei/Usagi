/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Handle to an instance of a sound
*****************************************************************************/
#ifndef __USG_AUDIO_SOUNDACTORHANDLE_H__
#define __USG_AUDIO_SOUNDACTORHANDLE_H__

#include "Engine/Core/Containers/SafePointer.h"
#include "SoundActor.h"

namespace usg{

class SoundActorHandle :  public SafePointer<SoundActor>
{
public:
	SoundActorHandle() {}
	~SoundActorHandle() {}

	inline void SetPosition(const Vector3f& vPos);
	inline void SetVelocity(const Vector3f& vVel);
	inline Vector3f GetPosition() const;
	inline Vector3f GetVelocity() const;
	inline SoundActor_ps* GetPlatform3D();

	SAFEPOINTER_COPY(SoundActorHandle)

private:
};

inline void SoundActorHandle::SetPosition(const Vector3f& vPos)
{
	SoundActor* pActor = GetPointer();
	if(pActor)
	{
		pActor->SetPosition(vPos);
	}
}

inline void SoundActorHandle::SetVelocity(const Vector3f& vVel)
{
	SoundActor* pActor = GetPointer();
	if (pActor)
	{
		pActor->SetVelocity(vVel);
	}
}

inline Vector3f SoundActorHandle::GetPosition() const
{
	const SoundActor* pActor = GetPointer();
	if(pActor)
	{
		return pActor->GetPosition();
	}
	return Vector3f::ZERO;
}

inline Vector3f SoundActorHandle::GetVelocity() const
{
	const SoundActor* pActor = GetPointer();
	if (pActor)
	{
		return pActor->GetVelocity();
	}
	return Vector3f::ZERO;
}

inline SoundActor_ps* SoundActorHandle::GetPlatform3D()
{
	SoundActor* pActor = GetPointer();
	if(pActor)
	{
		return &pActor->GetPlatform3D();
	}

	ASSERT(false);	// Should never get here
	return NULL;
}

}

#endif
