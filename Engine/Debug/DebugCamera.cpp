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
}


bool DebugCamera::ReadButton(uint32 uButtonShift, ButtonState eState) const
{
	Gamepad* pGamepad = Input::GetGamepad(0);

	bool bRet = pGamepad->GetButtonDown(uButtonShift, eState);

#ifndef FINAL_BUILD
	Gamepad* pGamepad1 = Input::GetGamepad(1);
	if(pGamepad1)
	{
		bRet |= pGamepad1->GetButtonDown(uButtonShift, eState);
	}
#endif

	return bRet;
}

float DebugCamera::ReadAxis(uint32 eAxis) const
{
	Gamepad* pGamepad = Input::GetGamepad(0);

	float fRet = pGamepad->GetAxisValue(eAxis);

#ifndef FINAL_BUILD
	Gamepad* pGamepad1 = Input::GetGamepad(1);
	if (pGamepad1)
	{
		fRet += pGamepad1->GetAxisValue(eAxis);
	}
#endif

	return fRet;
}


void DebugCamera::Update(float dt)
{
	if (ReadButton(GAMEPAD_BUTTON_THUMB_R|GAMEPAD_BUTTON_RIGHT, BUTTON_STATE_PRESSED))
	{
		m_uSpeed = (m_uSpeed + 1)%4;
	}
	if (ReadButton(GAMEPAD_BUTTON_LEFT, BUTTON_STATE_PRESSED))
	{
		m_uSpeed = (m_uSpeed - 1)%4;
	}

	float fSpeedMul = (float)(m_uSpeed+1);
	float fMoveSpeed = 15.0f*fSpeedMul;
	float fRotSpeed = 2.0f;

	float fRotateX = ReadAxis(GAMEPAD_AXIS_RIGHT_X) + ReadAxis(GAMEPAD_AXIS_POINTER_X);
	float fRotateY = ReadAxis(GAMEPAD_AXIS_RIGHT_Y) + ReadAxis(GAMEPAD_AXIS_POINTER_Y);
	float fMoveX = ReadAxis(GAMEPAD_AXIS_LEFT_X);
	float fMoveY = ReadAxis(GAMEPAD_AXIS_LEFT_Y);
	float fMoveZ = ReadButton(GAMEPAD_BUTTON_ZR, BUTTON_STATE_HELD) ? 1.0f : 0.0f;
	fMoveZ -= ReadButton(GAMEPAD_BUTTON_ZL, BUTTON_STATE_HELD) ? 1.0f : 0.0f;

	float fRotateZ = ReadButton(GAMEPAD_BUTTON_R, BUTTON_STATE_HELD) ? 1.0f : 0.0f;
	fRotateZ -= ReadButton(GAMEPAD_BUTTON_L, BUTTON_STATE_HELD) ? 1.0f : 0.0f;

	if(ReadButton(GAMEPAD_BUTTON_DOWN, BUTTON_STATE_HELD))
		fMoveZ -= 1.0f;
	if(ReadButton(GAMEPAD_BUTTON_UP, BUTTON_STATE_HELD))
		fMoveZ += 1.0f;
	
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