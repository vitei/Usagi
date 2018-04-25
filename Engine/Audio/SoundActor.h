/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Handle to an instance of a sound
*****************************************************************************/
#ifndef __USG_AUDIO_SOUNDACTOR_H__
#define __USG_AUDIO_SOUNDACTOR_H__
#include "Engine/Common/Common.h"
#include AUDIO_HEADER(Engine/Audio, SoundActor_ps.h)
#include "Engine/Core/Containers/SafePointer.h"

namespace usg{

class SoundActor
{
public:
	SoundActor();
	~SoundActor();

	void Init();

	void SetPosition(const Vector3f& vPos);
	const Vector3f& GetPosition() const { return m_position; }
	void SetVelocity(const Vector3f& velocity) { m_vVelocity = velocity; }
	const Vector3f& GetVelocity() const { return m_vVelocity; }

	SoundActor_ps& GetPlatform3D() { return m_platform3D; }

private:
	SoundActor_ps 		m_platform3D;
	Vector3f 			m_position;
	Vector3f			m_vVelocity;
};

}

#endif
