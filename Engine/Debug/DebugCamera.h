/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _CLR_DEBUG_DEBUGCAMERA_H_
#define _CLR_DEBUG_DEBUGCAMERA_H_

#include "Engine/Maths/MathUtil.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/HID/InputEnums.pb.h"
#include "Engine/Debug/DebugCameraController.h"


namespace usg {

class DebugCamera
{
public:
	DebugCamera(void);
	~DebugCamera(void);

	void Init();

	void Update(float dt);

	void SetMatrix(const Matrix4x4& mat) { m_modelMat = mat; }
	const Matrix4x4& GetModelMat() { return m_modelMat;  }

	void SetActive(bool bActive) { m_bActive = bActive; }
	bool GetActive() { return m_bActive; }

private:	
	DebugCameraController	m_controller;
	Matrix4x4				m_modelMat;
	uint32					m_uSpeed;
	bool					m_bActive;
};

}

#endif
