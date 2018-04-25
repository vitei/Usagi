/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/HID/InputStructs.h"
#include "OculusHMD.h"
#include "OVR_CAPI.h"
#include "OculusTouch.h"

namespace usg{

const SHORT g_sDeadZone = 6000;

struct ControlMapping
{
	uint32		uAbstractID;
	ovrButton	uOVRID;
};

struct TouchMapping
{
	uint32		uAbstractID;
	ovrTouch	uOVRID;
};

static const ControlMapping g_controlMapping[] =
{
	{ GAMEPAD_BUTTON_A,			ovrButton_B },
	{ GAMEPAD_BUTTON_B,			ovrButton_A },
	{ GAMEPAD_BUTTON_X,			ovrButton_Y },
	{ GAMEPAD_BUTTON_Y,			ovrButton_X },
	{ GAMEPAD_BUTTON_START,		ovrButton_Enter },
	{ GAMEPAD_BUTTON_SELECT,	ovrButton_Back },
	{ GAMEPAD_BUTTON_THUMB_L,	ovrButton_LThumb },
	{ GAMEPAD_BUTTON_THUMB_R,	ovrButton_RThumb },
	{ GAMEPAD_BUTTON_HOME,		ovrButton_Home },
	// The following are not present on touch, XBox fallback
	{ GAMEPAD_BUTTON_L,			ovrButton_LShoulder },
	{ GAMEPAD_BUTTON_R,			ovrButton_RShoulder },
	{ GAMEPAD_BUTTON_UP,		ovrButton_Up },
	{ GAMEPAD_BUTTON_DOWN,		ovrButton_Down },
	{ GAMEPAD_BUTTON_LEFT,		ovrButton_Left },
	{ GAMEPAD_BUTTON_RIGHT,		ovrButton_Right },

	{ GAMEPAD_BUTTON_NONE,		ovrButton_EnumSize },
};

static const TouchMapping g_touchMapping[] =
{
	{ GAMEPAD_BUTTON_A,			ovrTouch_B },
	{ GAMEPAD_BUTTON_B,			ovrTouch_A },
	{ GAMEPAD_BUTTON_X,			ovrTouch_Y },
	{ GAMEPAD_BUTTON_Y,			ovrTouch_X },
	{ GAMEPAD_BUTTON_THUMB_L,	ovrTouch_LThumb },
	{ GAMEPAD_BUTTON_THUMB_R,	ovrTouch_RThumb },

	{ GAMEPAD_BUTTON_NONE,		ovrTouch_EnumSize }
};


static const TouchMapping g_fingerMapping[] =
{
	{ TOUCH_LEFT_INDEX_POINTING,	ovrTouch_LIndexPointing },
	{ TOUCH_RIGHT_INDEX_POINTING,	ovrTouch_RIndexPointing },
	{ TOUCH_LEFT_THUMB_UP,			ovrTouch_LThumbUp },
	{ TOUCH_RIGHT_THUMB_UP,			ovrTouch_RThumbUp },
	{ TOUCH_LEFT_THUMB_ON_REST,		ovrTouch_LThumbRest },
	{ TOUCH_RIGHT_THUMB_ON_REST,	ovrTouch_RThumbRest },
	{ TOUCH_NONE,					ovrTouch_EnumSize }
};

static void FlipHandedness(const ovrPosef* inPose, ovrPosef* outPose) {
	outPose->Orientation.x = inPose->Orientation.x;
	outPose->Orientation.y = inPose->Orientation.y;
	outPose->Orientation.z = -inPose->Orientation.z;
	outPose->Orientation.w = -inPose->Orientation.w;

	outPose->Position.x = inPose->Position.x;
	outPose->Position.y = inPose->Position.y;
	outPose->Position.z = -inPose->Position.z;
} 

OculusTouch::OculusTouch()
{
    m_bConnected = false;
	m_pHMD = nullptr;
}

OculusTouch::~OculusTouch()
{
}

void OculusTouch::Init(OculusHMD* pHMD)
{
	m_bConnected = pHMD != nullptr;
	m_pHMD = pHMD;
}

uint32 OculusTouch::GetCaps() const
{
	return CAP_DUAL_HANDED | CAP_ACCELEROMETER | CAP_LEFT_STICK | CAP_RIGHT_STICK;
}


void OculusTouch::Update(GFXDevice* pDevice, GamepadDeviceState& state)
{
	state.uButtonsPrevDown = state.uButtonsDown;
	state.uButtonsDown = 0;
	state.uButtonsContact = 0;
	state.uFingerStates = 0;

	OculusHMD* pOculusHMD = m_pHMD;
	if (!pOculusHMD)
	{
		m_bConnected = false;
		return;
	}
	else
	{
		m_bConnected = true;
	}
		
	// TODO: Return if not oculus
	uint32 uControllers = ovr_GetConnectedControllerTypes(pOculusHMD->GetSession());
	if ((uControllers && ovrControllerType_Touch) == 0)
	{
		m_bConnected = false;
		return;
	}

	ovrInputState inputState;
	ovrResult result = ovr_GetInputState(pOculusHMD->GetSession(), ovrControllerType_Touch, &inputState);
	if (!OVR_SUCCESS(result))
	{
		m_bConnected = false;
		return;
	}

	state.fAxisValues[GAMEPAD_AXIS_LEFT_X] = inputState.Thumbstick[ovrHand_Left].x;
	state.fAxisValues[GAMEPAD_AXIS_LEFT_Y] = inputState.Thumbstick[ovrHand_Left].y;
	state.fAxisValues[GAMEPAD_AXIS_RIGHT_X] = inputState.Thumbstick[ovrHand_Right].x;
	state.fAxisValues[GAMEPAD_AXIS_RIGHT_Y] = inputState.Thumbstick[ovrHand_Left].y;

	const ControlMapping *pMapping = g_controlMapping;
	while(pMapping->uAbstractID != GAMEPAD_BUTTON_NONE)
	{
		if(pMapping->uOVRID & inputState.Buttons)
		{
			state.uButtonsDown |= (1 << (pMapping->uAbstractID - 1));
		}
		pMapping++;
	}

	const TouchMapping *pTouchMapping = g_touchMapping;
	while (pTouchMapping->uAbstractID != GAMEPAD_BUTTON_NONE)
	{
		if (pTouchMapping->uOVRID & inputState.Touches)
		{
			state.uButtonsContact |= (1 << (pTouchMapping->uAbstractID - 1));
		}
		pTouchMapping++;
	}

	const TouchMapping *pFingerMapping = g_fingerMapping;
	while (pFingerMapping->uAbstractID != TOUCH_NONE)
	{
		if (pFingerMapping->uOVRID & inputState.Touches)
		{
			state.uFingerStates |= (1 << (pFingerMapping->uAbstractID - 1));
		}
		pFingerMapping++;
	}


	if(inputState.IndexTrigger[ovrHand_Left] > 0.5f)
		state.uButtonsDown |= (1 << (GAMEPAD_BUTTON_L - 1));

	if(inputState.IndexTrigger[ovrHand_Right] > 0.5f)
		state.uButtonsDown |= (1 << (GAMEPAD_BUTTON_R - 1));

	if (inputState.HandTrigger[ovrHand_Left] > 0.5f)
		state.uButtonsDown |= (1 << (GAMEPAD_BUTTON_ZL - 1));

	if (inputState.HandTrigger[ovrHand_Right] > 0.5f)
		state.uButtonsDown |= (1 << (GAMEPAD_BUTTON_ZR - 1));


	const ovrTrackingState& trackingState = pOculusHMD->GetTrackingState();
	state.leftHand = pOculusHMD->ConvertPose(trackingState.HandPoses[0].ThePose);
	state.rightHand = pOculusHMD->ConvertPose(trackingState.HandPoses[1].ThePose);

	state.fAxisValues[GAMEPAD_AXIS_GYRO_VEL_X] = trackingState.HandPoses[0].LinearVelocity.x;
	state.fAxisValues[GAMEPAD_AXIS_GYRO_VEL_Y] = trackingState.HandPoses[0].LinearVelocity.y;
	state.fAxisValues[GAMEPAD_AXIS_GYRO_VEL_Z] = -trackingState.HandPoses[0].LinearVelocity.z;

	state.fAxisValues[GAMEPAD_AXIS_GYRO_ANG_DX] = trackingState.HandPoses[0].AngularAcceleration.x;
	state.fAxisValues[GAMEPAD_AXIS_GYRO_ANG_DY] = trackingState.HandPoses[0].AngularAcceleration.y;
	state.fAxisValues[GAMEPAD_AXIS_GYRO_ANG_DZ] = -trackingState.HandPoses[0].AngularAcceleration.z;


	state.fAxisValues[GAMEPAD_AXIS_GYRO_VEL_X_R] = trackingState.HandPoses[1].LinearVelocity.x;
	state.fAxisValues[GAMEPAD_AXIS_GYRO_VEL_Y_R] = trackingState.HandPoses[1].LinearVelocity.y;
	state.fAxisValues[GAMEPAD_AXIS_GYRO_VEL_Z_R] = -trackingState.HandPoses[1].LinearVelocity.z;

	state.fAxisValues[GAMEPAD_AXIS_GYRO_ANG_DX_R] = trackingState.HandPoses[1].AngularAcceleration.x;
	state.fAxisValues[GAMEPAD_AXIS_GYRO_ANG_DY_R] = trackingState.HandPoses[1].AngularAcceleration.y;
	state.fAxisValues[GAMEPAD_AXIS_GYRO_ANG_DZ_R] = -trackingState.HandPoses[1].AngularAcceleration.z;

}




}