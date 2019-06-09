/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: API specific camera implementation (such as creating left and
//	right eye matrices).
*****************************************************************************/
#ifndef _USG_SCENE_CAMERA_PC_H_
#define _USG_SCENE_CAMERA_PC_H_

#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Maths/MathUtil.h"

namespace usg{

class Camera_ps
{
public:
	Camera_ps(void) {}
	~Camera_ps(void) {}

	void Update(const Matrix4x4& viewMat, float fAspect,
				float32 fFov, float32 fNear, float32 fFar, bool bUse3D, bool bRightHanded)
	{
		m_mProj.Perspective(Math::DegToRad(fFov), fAspect, fNear, fFar);
		m_mView = viewMat;
	}

	const Matrix4x4& GetProjection(ViewType eView) const {	return m_mProj; }
	const Matrix4x4& GetView(ViewType eView) const { return m_mView; }

	
	bool StereoSupport() { return false; }

private:

	Matrix4x4				m_mProj;
	Matrix4x4				m_mView;
};

}

#endif
