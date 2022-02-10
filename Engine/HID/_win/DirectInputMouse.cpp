/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Input_ps.h"
#include "DirectInput.h"
#include "DirectInputMouse.h"

namespace usg
{

	bool DirectInputMouse::Init(DirectInput* pInput, uint32 uDeviceNum)
	{
		m_pInput = pInput;
		for (uint32 i = 0; i < _MouseButton_count; i++)
		{
			m_prevButtons[i] = false;
			m_buttons[i] = false;
		}

		m_cage.left = -1;
		m_cage.right = -1;
		m_cage.top = -1;
		m_cage.bottom = -1;

		//Try to create the device
		if (FAILED(pInput->GetDirectInput()->CreateDevice(GUID_SysMouse, &m_pDevice, NULL)))
		{
			return false;
		}

		//Set the data format
		if (FAILED(m_pDevice->SetDataFormat(&c_dfDIMouse2)))
		{
			return false;
		}

		if (FAILED(m_pDevice->SetCooperativeLevel(pInput->GetWindow(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
		{
			return false;
		}

		if (!(m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL)))
		{
			ASSERT_MSG(false, "CreateEvent Failed");
			return false;
		}

		if (FAILED(m_pDevice->SetEventNotification(m_hEvent)))
		{
			ASSERT_MSG(false, "Set event notification failed");
			return false;
		}

		DIPROPDWORD dipdw;
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = 16;

		if (FAILED(m_pDevice->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
		{
			ASSERT_MSG(false, "Set property failed");
			return false;
		}

		m_pDevice->Acquire();


		return true;

	}

	void DirectInputMouse::Update()
	{
		for (uint32 i = 0; i < MOUSE_BUTTON_NONE; i++)
		{
			m_prevButtons[i] = m_buttons[i];
		}

		DIDEVICEOBJECTDATA od[16];
		DWORD dwNumElem = 16;

		bool bCage = (m_cage.right != -1);

		DIMOUSESTATE2 ms;

		m_pDevice->Poll();

		DWORD size = sizeof(ms);

		HRESULT hr;
		hr = m_pDevice->GetDeviceState(size, &ms);


		for(int i=0; i<MOUSE_BUTTON_NONE; i++)
		{
			m_buttons[i] = ms.rgbButtons[od[i].dwData & 0x80];
		}

		RECT screen;
		GetWindowRect(m_pInput->GetWindow(), &screen);

		float fWidth = (float)(screen.right - screen.left);
		float fHeight = (float)(screen.bottom - screen.top);

		m_fAxis[MOUSE_DELTA_X] = (float)ms.lX;
		m_fAxis[MOUSE_DELTA_Y] = (float)ms.lY;
		m_fAxis[MOUSE_POS_X] = Math::Clamp(m_fAxis[MOUSE_POS_X] + m_fAxis[MOUSE_DELTA_X], 0.0f, fWidth);
		m_fAxis[MOUSE_POS_Y] = Math::Clamp(m_fAxis[MOUSE_POS_Y] + m_fAxis[MOUSE_DELTA_Y], 0.0f, fHeight);

		m_fAxis[MOUSE_NORM_POS_X] = ((m_fAxis[MOUSE_POS_X] / fWidth) * 2.0f) - 1.0f;
		m_fAxis[MOUSE_NORM_ASPECT_POS_X] = usg::Math::Clamp(((m_fAxis[MOUSE_POS_X] / fHeight) * 2.0f) - 1.0f, -1.0f, 1.0f);
		m_fAxis[MOUSE_NORM_POS_Y] = ((m_fAxis[MOUSE_POS_Y] / fHeight) * 2.0f) - 1.0f;

		m_fAxis[MOUSE_DELTA_X_NORM] = ((m_fAxis[MOUSE_DELTA_X] / fWidth) * 2.0f) - 1.0f;
		m_fAxis[MOUSE_DELTA_X_NORM_ASPECT_X] = usg::Math::Clamp(((m_fAxis[MOUSE_DELTA_X] / fHeight) * 2.0f) - 1.0f, -1.0f, 1.0f);
		m_fAxis[MOUSE_DELTA_Y_NORM] = ((m_fAxis[MOUSE_DELTA_Y] / fHeight) * 2.0f) - 1.0f;

		long mouseWheel = ms.lZ;
		if (abs(mouseWheel) > 0)
			m_fAxis[MOUSE_DELTA_WHEEL] = Math::Sign((float)mouseWheel);
		else
			m_fAxis[MOUSE_DELTA_WHEEL] = 0.0f;

	}
}