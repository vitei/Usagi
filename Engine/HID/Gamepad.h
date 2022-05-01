 /****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Raw user input (mouse/keyboard/touchscreen/pointer etc)
*****************************************************************************/
#pragma once

#ifndef USG_HID_GAMEPAD_H
#define USG_HID_GAMEPAD_H

#include "Engine/HID/InputDefines.h"
#include "Engine/HID/InputStructs.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Core/stl/map.h"

namespace usg
{

	class GFXDevice;
	class IGamepad;

	class Gamepad final
	{
	public:
		Gamepad();
		~Gamepad();

		bool GetButtonDown(uint32 uButtonShift, ButtonState eState=BUTTON_STATE_PRESSED) const;
		float GetAxisValue(uint32 eAxis) const;
		bool GetScreenTouch(Vector2f& posOut) const { posOut = m_deviceState.vTouchPos; return m_deviceState.bScreenTouch; }
		void GetMatrix(Matrix4x4& out, GamepadHand eHand = GamepadHand::Left);
		bool IsConnected() const { return m_bConnected; }
		bool HasCapabilities(uint32 uCaps) const { return m_uCaps&uCaps; }
		void BindHardware(IGamepad* pGamepad);

		void ResetGyro();
		void ResetGyroDirection();

		// A variety of different input devices which we want to be able to use through much the same interface
		void Update(usg::GFXDevice* pDevice, float fElapsed);

		// Durations < 0.0f will never end and must be stoppped via handle
		uint32 Vibrate(float fLeft, float fRight, float fDuration);
		void UpdateVibration(uint32 uHandle, float fLeft, float fRight);
		void StopEffect(uint32 uHandle);
		void StopAllVibrations();
	protected:
		IGamepad*	m_pIGamepad;

		GamepadDeviceState m_deviceState;
		uint32		m_uCaps;
		bool		m_bConnected;

		struct VibrationData
		{
			float fDuration = 0.0f;
			float fLeftStrength = 0.0f;
			float fRightStrength = 0.0f;
		};

		map<uint32, VibrationData> m_vibrations;
	};


} // namespace usg

#endif // USG_HID_GAMEPAD_H
