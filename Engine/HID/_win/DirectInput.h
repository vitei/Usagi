/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Code for managing common direct input code
*****************************************************************************/
#ifndef __USG_HID_PC_DIRECT_INPUT_H__
#define __USG_HID_PC_DIRECT_INPUT_H__

#include "Engine/HID/IGamepad.h"
#include "Engine/Core/stl/string.h"
#include "Engine/Core/stl/vector.h"
#include <dinput.h>

namespace usg {

	class Input_ps;

	class DirectInput
	{
	public:
		DirectInput();
		~DirectInput();


		bool Init();
		BOOL DeviceEnumCallback(const DIDEVICEINSTANCE* pInst);

		bool HasDevice(const char* szName);
		bool IsDeviceConnected(uint32 uIdx) const;
		bool IsGamepad(uint32 uIdx) const;
		usg::string GetName(uint32 uIdx) const;
		GUID GetGUIDForDevice(uint32 uIdx) const;
		void UpdateConnectedDevices();
		uint32 GetJoystickCount() const { return (uint32)m_joysticks.size(); }
		LPDIRECTINPUT8 GetDirectInput() { return m_pDI; }
		HWND GetWindow() { return m_window; }
	private:

		struct DeviceInfo
		{
			usg::string				instanceName;
			usg::string				productName;
			GUID					guid;
			bool					bIsGamepad;
			bool					bConnected;
		};

		DeviceInfo* GetDevice(const char* szName);
		const DeviceInfo* GetDevice(const char* szName) const;

		usg::vector<DeviceInfo>	m_joysticks;

		LPDIRECTINPUT8		m_pDI;
		HWND				m_window;
	};

}

#endif