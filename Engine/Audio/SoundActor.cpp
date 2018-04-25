/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "SoundActor.h"

namespace usg{

SoundActor::SoundActor()
{

}

SoundActor::~SoundActor()
{

}

void SoundActor::Init()
{
	m_position = V3F_ZERO;
	m_vVelocity = V3F_ZERO;
	m_platform3D.Init();
	m_platform3D.SetPosition(m_position);
}

void SoundActor::SetPosition(const Vector3f& vPos)
{
	m_position = vPos;
	m_platform3D.SetPosition(m_position);
}

}