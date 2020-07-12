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
	m_position = Vector3f::ZERO;
	m_vVelocity = Vector3f::ZERO;
	m_platform3D.Init();
	m_platform3D.SetPosition(m_position);
}

void SoundActor::SetPosition(const Vector3f& vPos)
{
	m_position = vPos;
	m_platform3D.SetPosition(m_position);
}

}