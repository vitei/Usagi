/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: When using an HMD the final projection and view matrices are
//	dictated by the hardware, so we abstract that away
*****************************************************************************/
#ifndef _USG_HMD_CAMERA_H_
#define _USG_HMD_CAMERA_H_
#include "Engine/Common/Common.h"
#include "Engine/Scene/Camera/Camera.h"

namespace usg
{
	class IHeadMountedDisplay;

	class HMDCamera : public Camera
	{
	public:
		HMDCamera(void);
		~HMDCamera(void);
		
		void Init(IHeadMountedDisplay* pDisplay, float fNear, float fFar);
		void ChangeNearFar(float fNear, float fFar);
		void Update();
		virtual void SetModelMatrix(const Matrix4x4& mModel) override;

		virtual const Matrix4x4& GetProjection(ViewType eType = VIEW_CENTRAL) const;
		virtual const Matrix4x4& GetViewMatrix(ViewType eType = VIEW_CENTRAL) const;
		virtual float GetNear(ViewType eType = VIEW_CENTRAL) const;
		virtual float GetFar(ViewType eType = VIEW_CENTRAL) const;
		virtual Vector4f GetPos(ViewType eType) const override { return m_mTransformMats[eType].vPos(); }

		virtual bool Is3D() const { return true; }

	private:
		const IHeadMountedDisplay*	m_pDisplay;
		Matrix4x4	m_mProj[VIEW_COUNT];
		Matrix4x4	m_mView[VIEW_COUNT];
		Matrix4x4	m_mTransformMats[VIEW_COUNT];
		Matrix4x4	m_mWorldOffset;
		float 		m_fNear;
		float 		m_fFar;
	};

}

#endif
