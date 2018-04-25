/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _CLR_DEBUG_DEBUGCAMERA_H_
#define _CLR_DEBUG_DEBUGCAMERA_H_
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/HID/InputEnums.pb.h"

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
	bool ReadButton(uint32 uButtonShift, ButtonState eState = BUTTON_STATE_PRESSED) const;
	float ReadAxis(uint32 eAxis) const;
	
	Matrix4x4		m_modelMat;
	uint32			m_uSpeed;
	bool			m_bActive;
};

}

#endif
