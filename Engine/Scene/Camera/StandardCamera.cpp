/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "StandardCamera.h"

namespace usg
{

	StandardCamera::StandardCamera(void)
	{
		m_fNear			= 0.1;
		m_fFar			= 1000.0f;
	}

	StandardCamera::~StandardCamera(void)
	{
	}


	void StandardCamera::SetUp(const Matrix4x4& viewMat, float32 fAspect, float32 fFov, float32 fNear, float32 fFar)
	{
		m_mView = viewMat;
		if(IsRightHanded())
		{
			m_mProjection.PerspectiveRH(Math::DegToRad(fFov), fAspect, fNear, fFar);
		}
		else
		{
			m_mProjection.Perspective(Math::DegToRad(fFov), fAspect, fNear, fFar);
		}

		m_fNear		= fNear;
		m_fFar		= fFar;

		UpdateInternal();
	}

	void StandardCamera::SetModelMatrix(const Matrix4x4& mModel)
	{
		m_mView.BuildCameraFromModel(mModel);
		m_mModel = m_mModel;

		// Rebuild the frustum
		m_frustum.SetUp(m_mProjection, m_mView);
	}


	void StandardCamera::SetUp(const Matrix4x4& viewMat, const Matrix4x4& projMat)
	{
		// Not recommeded as the base set up
		m_mModel.BuildModelFromCamera(viewMat);

		m_mView = viewMat;
		m_mProjection = projMat;

		Frustum testFrustum;
		testFrustum.SetUp(projMat);

		// TODO: Confirm all of this
		m_fNear = -testFrustum.GetPlane(Frustum::PLANE_NEAR).GetDistance();
		m_fFar = testFrustum.GetPlane(Frustum::PLANE_FAR).GetDistance();

		UpdateInternal();
	}


	void StandardCamera::UpdateInternal()
	{
		m_mModel.BuildModelFromCamera(m_mView);
		m_frustum.SetUp(m_mProjection, m_mView);
	}


	const Matrix4x4& StandardCamera::GetProjection(ViewType eType) const
	{
		return m_mProjection;
	}

	const Matrix4x4& StandardCamera::GetViewMatrix(ViewType eType) const
	{
		return m_mView;
	}

}