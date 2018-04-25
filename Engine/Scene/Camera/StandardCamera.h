/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A world view
*****************************************************************************/
#ifndef _USG_STANDARD_CAMERA_H_
#define _USG_STANDARD_CAMERA_H_
#include "Engine/Common/Common.h"
#include "Engine/Scene/Camera/Camera.h"

namespace usg
{


	class StandardCamera : public Camera
	{
	public:
		StandardCamera(void);
		~StandardCamera(void);

		virtual void SetModelMatrix(const Matrix4x4& mModel) override;
		void SetUp(const Matrix4x4& viewMat, float32 fAspect, float32 fFov, float32 fNear, float32 fFar);
		void SetUp(const Matrix4x4& viewMat, const Matrix4x4& projMat);

		virtual const Matrix4x4& GetProjection(ViewType eType = VIEW_CENTRAL) const;
		virtual const Matrix4x4& GetViewMatrix(ViewType eType = VIEW_CENTRAL) const;
		virtual float GetNear(ViewType eType = VIEW_CENTRAL) const { return m_fNear; }
		virtual float GetFar(ViewType eType = VIEW_CENTRAL) const { return m_fFar; }

		virtual bool Is3D() const { return false; }

	private:
		void UpdateInternal();

		Matrix4x4	m_mView;
		Matrix4x4	m_mProjection;
		float 		m_fNear;
		float 		m_fFar;
	};

}

#endif
