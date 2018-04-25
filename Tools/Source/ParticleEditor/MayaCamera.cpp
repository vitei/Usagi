#include "Engine/Common/Common.h"
#include "Engine/HID/Input.h"
#include "Engine/HID/Keyboard.h"
#include "Engine/HID/Mouse.h"
#include "MayaCamera.h"

const float32 fRotationMultiplier = 0.005f;
const float32 fZoomMultiplier = 0.25f;
const float32 fPanningMultiplier = 0.05f;
const float32 fMinZoom = 2.5f;
const float32 fMaxZoom = 60.f;

void MayaCamera::Init(float fAspect)
{
	m_vLookAtPos.Assign(0.0f, 0.0f, 0.0f);
	m_vEyePosition.Assign(0.0f, 0.0f, 5.0f);
	usg::Matrix4x4 mCameraMat;
	mCameraMat.LoadIdentity();
	m_camera.SetUp(mCameraMat, fAspect, 60.0f, 1.0f, 500.0f);
	m_rotation.MakeRotateX(-0.25f);
	BuildCameraMatrix();
}

void MayaCamera::Update(float fElapsed)
{
	const usg::Keyboard* pKeyboard = usg::Input::GetKeyboard();
	const usg::Mouse* pMouse = usg::Input::GetMouse();
	if(pKeyboard->GetToggleHeld(usg::KEYBOARD_TOGGLE_ALT))
	{
		m_vEyePosition.z -= pMouse->GetAxis(usg::MOUSE_DELTA_WHEEL)*2.5f;
		if(pMouse->GetButton(usg::MOUSE_BUTTON_LEFT))
		{
			usg::Matrix4x4 mTmp;
			mTmp.MakeRotateYPR( -pMouse->GetAxis(usg::MOUSE_DELTA_X)*fRotationMultiplier, pMouse->GetAxis(usg::MOUSE_DELTA_Y)*fRotationMultiplier, 0.0f);
			m_rotation = mTmp * m_rotation;
		}
		if(pMouse->GetButton(usg::MOUSE_BUTTON_RIGHT))
		{
			m_vEyePosition.z -= pMouse->GetAxis(usg::MOUSE_DELTA_Y)*fZoomMultiplier;
		}
		if(pMouse->GetButton(usg::MOUSE_BUTTON_MIDDLE))
		{
			m_vEyePosition.x -= pMouse->GetAxis(usg::MOUSE_DELTA_X)*fPanningMultiplier;
			m_vEyePosition.y -= pMouse->GetAxis(usg::MOUSE_DELTA_Y)*fPanningMultiplier;
		}
		
	}

	m_vEyePosition.z = usg::Math::Clamp(m_vEyePosition.z, fMinZoom, fMaxZoom);

	if(pKeyboard->GetKey('F', usg::BUTTON_STATE_PRESSED))
	{
		m_vEyePosition.x = 0.0f;
		m_vEyePosition.y = 0.0f;
		m_rotation = usg::Matrix4x4::Identity();
	}


	BuildCameraMatrix();
}

void MayaCamera::BuildCameraMatrix()
{
	usg::Matrix4x4 mCameraMat;
	usg::Vector3f vEyePos = usg::Vector3f(0.0f, 0.0f, m_vEyePosition.z);
	usg::Vector3f vOffset = usg::Vector3f(m_vEyePosition.x, m_vEyePosition.y, 0.0f);
	vEyePos = m_rotation.TransformVec3(vEyePos, 0.0f);
	vOffset = m_rotation.TransformVec3(vOffset, 0.0f);
	mCameraMat.LookAt(vEyePos+vOffset, m_vLookAtPos+vOffset, usg::V3F_Y_AXIS);
	m_camera.SetUp(mCameraMat, m_camera.GetProjection());
}
