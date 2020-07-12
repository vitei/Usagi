/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/HID/Input.h"
#include "Engine/HID/Gamepad.h"
#include "DebugCamera.h"

namespace usg {

DebugCamera::DebugCamera(void)
{
	Init();
}

DebugCamera::~DebugCamera(void)
{
}


void DebugCamera::Init()
{
	m_bActive = false;
	m_modelMat.LoadIdentity();
	m_uSpeed = 0;
	m_controller.Init();
}



void DebugCamera::Update(float dt)
{
	m_controller.Update(dt);

	if (m_controller.GetBool(DebugCameraController::DEBUG_CAM_BOOL_INCR_SPEED))
	{
		m_uSpeed = (m_uSpeed + 1)%4;
	}
	if (m_controller.GetBool(DebugCameraController::DEBUG_CAM_BOOL_DECR_SPEED))
	{
		m_uSpeed = (m_uSpeed - 1)%4;
	}

	float fSpeedMul = (float)(m_uSpeed+1);
	float fMoveSpeed = 15.0f*fSpeedMul;
	float fRotSpeed = 2.0f;

	float fRotateX = m_controller.GetFloat(DebugCameraController::DEBUG_CAM_AXIS_ROTATE_X);
	float fRotateY = m_controller.GetFloat(DebugCameraController::DEBUG_CAM_AXIS_ROTATE_Y);
	float fMoveX = m_controller.GetFloat(DebugCameraController::DEBUG_CAM_AXIS_MOVE_X);
	float fMoveY = m_controller.GetFloat(DebugCameraController::DEBUG_CAM_AXIS_MOVE_Y);
	float fMoveZ = m_controller.GetFloat(DebugCameraController::DEBUG_CAM_AXIS_MOVE_Z);

	float fRotateZ = m_controller.GetFloat(DebugCameraController::DEBUG_CAM_AXIS_ROTATE_Z);
	
	const Matrix4x4 &modelMat = m_modelMat;

	Vector4f xAxis = modelMat.vRight();
	Vector4f yAxis = modelMat.vUp();
	Vector4f zAxis = modelMat.vFace();

	Vector4f vPosition = m_modelMat.vPos();
	vPosition += fMoveY*dt*zAxis*fMoveSpeed;
	vPosition += fMoveX*dt*xAxis*fMoveSpeed;
	vPosition += fMoveZ*dt*yAxis*fMoveSpeed;


	ASSERT(vPosition.w == 1.0f);

	Matrix4x4 rotMat;
	rotMat.MakeRotAboutAxis( xAxis, -dt*fRotateY*fRotSpeed );
	yAxis = yAxis * rotMat;
	zAxis = zAxis * rotMat;

	rotMat.MakeRotAboutAxis( yAxis, -dt*fRotateX*fRotSpeed );
	//rotMat.MakeRotateY(-dt*fRotateX*fRotSpeed);
	xAxis = xAxis * rotMat;
	zAxis = zAxis * rotMat;

	rotMat.MakeRotAboutAxis( zAxis, dt*fRotateZ*fRotSpeed );
	xAxis = xAxis * rotMat;
	yAxis = yAxis * rotMat;

	zAxis.Normalise();
	yAxis.Normalise();
	xAxis.Normalise();

	Matrix4x4 tmp;
	tmp.ModelMatrix(xAxis, yAxis, zAxis, vPosition);
	m_modelMat = tmp;
	ASSERT(tmp.vPos().w == 1.0f);
}

}