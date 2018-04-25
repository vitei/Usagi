/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Graphics/Device/IHeadMountedDisplay.h"
#include "HMDCamera.h"

namespace usg
{

	HMDCamera::HMDCamera(void)
	{
		m_pDisplay = nullptr;
		m_fNear = 0.1f;
		m_fFar = 1000.0f;
		m_mWorldOffset = Matrix4x4::Identity();
	}

	HMDCamera::~HMDCamera(void)
	{
	}

	void HMDCamera::Init(IHeadMountedDisplay* pDisplay, float fNear, float fFar)
	{
		m_pDisplay = pDisplay;
		ChangeNearFar(fNear, fFar);
	}

	void HMDCamera::ChangeNearFar(float fNear, float fFar)
	{
		m_fNear = fNear;
		m_fFar = fFar;
	}
	
	void HMDCamera::Update()
	{
		if(!m_pDisplay)
		{
			return;
		}
		m_mProj[VIEW_LEFT_EYE] = m_pDisplay->GetProjectionMatrix(IHeadMountedDisplay::Eye::Left, m_fNear, m_fFar);
		m_mProj[VIEW_RIGHT_EYE] = m_pDisplay->GetProjectionMatrix(IHeadMountedDisplay::Eye::Right, m_fNear, m_fFar);
		m_mProj[VIEW_CENTRAL].Perspective(45.0, 1.0, m_fNear, m_fFar);	// FIXME: Should get values based on the combined matrices of the eyes

		
		m_pDisplay->GetEyeTransform(IHeadMountedDisplay::Eye::Left, m_mTransformMats[VIEW_LEFT_EYE]);
		m_pDisplay->GetEyeTransform(IHeadMountedDisplay::Eye::Right, m_mTransformMats[VIEW_RIGHT_EYE]);
		m_pDisplay->GetHMDTransform(m_mTransformMats[VIEW_CENTRAL]);

		for (uint32 i = 0; i < VIEW_COUNT; i++)
		{
			m_mTransformMats[i] = m_mTransformMats[i] * m_mWorldOffset;
			m_mView[i].BuildCameraFromModel(m_mTransformMats[i]);
		}

		m_mModel = m_mTransformMats[VIEW_CENTRAL];
		m_frustum.SetUpLR(m_mProj[VIEW_LEFT_EYE], m_mView[VIEW_LEFT_EYE], m_mProj[VIEW_RIGHT_EYE], m_mView[VIEW_RIGHT_EYE]);
	}
	
	void HMDCamera::SetModelMatrix(const Matrix4x4& mMat)
	{
		m_mWorldOffset = mMat;
	}
	

	const Matrix4x4& HMDCamera::GetProjection(ViewType eType) const
	{
		return m_mProj[eType];
	}
	
	const Matrix4x4& HMDCamera::GetViewMatrix(ViewType eType) const
	{
		return m_mView[eType];
	}
	
	float HMDCamera::GetNear(ViewType eType) const
	{
		return m_fNear;
	}
	
	float HMDCamera::GetFar(ViewType eType) const
	{
		return m_fFar;
	}
	

}