/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Camera.h"

namespace usg{

Camera::Camera(void)
{
	m_bRightHanded	= false;
	m_uID = 0;
	m_uRenderMask = usg::RENDER_MASK_ALL;
}

Camera::~Camera(void)
{
}


float Camera::GetInvDepthRange(ViewType eType) const
{
	return 1.0f / (GetFar(eType) - GetNear(eType));
}

}