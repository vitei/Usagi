#include "Engine/Common/Common.h"
#include "Engine/Audio/AudioListener.h"

namespace usg{

	void AudioListener::SetMatrix(const Matrix4x4& mLoc, bool bIsViewMatrix, bool bUseAsAttenPos)
	{
		if (bIsViewMatrix)
		{
			m_platform.SetMatrix(mLoc);
			m_viewMatrix = mLoc;
			m_matrix.BuildModelFromCamera(mLoc);
		}
		else
		{
			m_viewMatrix.BuildCameraFromModel(mLoc);
			m_platform.SetMatrix(m_viewMatrix);
			m_matrix = mLoc;
		}
		if (bUseAsAttenPos)
		{
			SetAttenuationPos(m_matrix.vPos().v3());
		}
		UpdateListenerSpaceVel();
	}

	void AudioListener::SetAttenuationPos(const Vector3f& vPos)
	{
		m_vAttenPos = vPos;
	}

	void AudioListener::UpdateListenerSpaceVel()
	{
		m_vListernSpaceVel = m_viewMatrix.TransformVec3(m_vVelocity, 0.0f);
	}

}

