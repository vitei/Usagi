/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The common accessor interface for a camera
*****************************************************************************/
#ifndef _USG_CAMERA_H_
#define _USG_CAMERA_H_

#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Maths/Vector4f.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Scene/Frustum.h"

namespace usg{


class Camera
{
public:
	Camera(void);
	~Camera(void);
	
	virtual void SetModelMatrix(const Matrix4x4& mModel) = 0;
	const Matrix4x4& GetModelMatrix() const { return m_mModel; }
	// 3D views should create a frustum which encompasses both
	const Frustum& GetFrustum() const		{ return m_frustum; }

	virtual const Matrix4x4& GetProjection(ViewType eType = VIEW_CENTRAL) const = 0;
	virtual const Matrix4x4& GetViewMatrix(ViewType eType = VIEW_CENTRAL) const = 0;
	virtual float GetNear(ViewType eType = VIEW_CENTRAL) const = 0;
	virtual float GetFar(ViewType eType = VIEW_CENTRAL) const = 0;
	float GetInvDepthRange(ViewType eType = VIEW_CENTRAL) const;
	virtual bool Is3D() const = 0;

	virtual Vector4f GetPos(ViewType eType) const	{ return m_mModel.vPos(); }
	
	Vector4f GetPos() const { return m_mModel.vPos(); }

	Vector4f GetFacing() const		{return m_mModel.vFace();  }

	void SetRightHanded(bool bRightHanded) { m_bRightHanded = bRightHanded; }
	bool IsRightHanded() const { return m_bRightHanded; }

	void SetID(uint32 uID) { m_uID = uID; }
	uint32 GetID() const { return m_uID; }
	void SetRenderMask(uint32 uRenderMask) { m_uRenderMask = uRenderMask; }
	uint32 GetRenderMask() const { return m_uRenderMask; }

protected:
	Frustum		m_frustum;
	Matrix4x4	m_mModel;
	uint32		m_uID;
	uint32		m_uRenderMask;
private:
	
	bool m_bRightHanded;
};

}

#endif
