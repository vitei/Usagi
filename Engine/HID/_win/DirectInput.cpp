/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/HID/InputStructs.h"
#include "Engine/Core/_win/WinUtil.h"
#include <wbemidl.h>
#include <oleauto.h>
#include "DirectInput.h"


#define SAFE_RELEASE(a) if(a) { a->Release(); a=nullptr; }

namespace usg
{

	BOOL CALLBACK GlobalDeviceEnumCallback(LPCDIDEVICEINSTANCE pInst, LPVOID pUserData)
	{
		return ((DirectInput*)pUserData)->DeviceEnumCallback(pInst);
	}

	DirectInput::DirectInput()
	{

	}

	DirectInput::~DirectInput()
	{
		if (m_pDI)
		{
			m_pDI->Release();
			m_pDI = nullptr;
		}
	}


	bool DirectInput::Init()
	{
		m_window = WINUTIL::GetWindow();
		if ( DirectInput8Create(WINUTIL::GetInstanceHndl(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDI, NULL ) )
		{
			return false;
		}

		UpdateConnectedDevices();

		return true;
	}

	void DirectInput::UpdateConnectedDevices()
	{
		for (auto& device : m_joysticks)
		{
			device.bConnected = false;
		}
		m_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, (LPDIENUMDEVICESCALLBACK)GlobalDeviceEnumCallback, this, DIEDFL_ATTACHEDONLY);
	}

	bool DirectInput::HasDevice(const char* szName)
	{
		return GetDevice(szName) != nullptr;
	}

	bool DirectInput::IsDeviceConnected(uint32 uIdx) const
	{
		if (uIdx < m_joysticks.size())
		{
			return m_joysticks[uIdx].bConnected;
		}
		return false;
	}

	GUID DirectInput::GetGUIDForDevice(uint32 uIdx) const
	{
		if (uIdx < m_joysticks.size())
		{
			return m_joysticks[uIdx].guid;
		}
		return GUID();
	}

	DirectInput::DeviceInfo* DirectInput::GetDevice(const char* szName)
	{
		for (auto& device : m_joysticks)
		{
			if (device.productName == szName)
			{
				return &device;
			}
		}
		return nullptr;
	}

	const DirectInput::DeviceInfo* DirectInput::GetDevice(const char* szName) const
	{
		for (auto& device : m_joysticks)
		{
			if (device.productName == szName)
			{
				return &device;
			}
		}
		return nullptr;
	}

	BOOL IsXInputDevice(const GUID* pGuidProductFromDirectInput)
	{
		IWbemLocator*           pIWbemLocator = NULL;
		IEnumWbemClassObject*   pEnumDevices = NULL;
		IWbemClassObject*       pDevices[20] = { 0 };
		IWbemServices*          pIWbemServices = NULL;
		BSTR                    bstrNamespace = NULL;
		BSTR                    bstrDeviceID = NULL;
		BSTR                    bstrClassName = NULL;
		DWORD                   uReturned = 0;
		bool                    bIsXinputDevice = false;
		UINT                    iDevice = 0;
		VARIANT                 var;
		HRESULT                 hr;

		// CoInit if needed
		hr = CoInitialize(NULL);
		bool bCleanupCOM = SUCCEEDED(hr);

		// Create WMI
		hr = CoCreateInstance(__uuidof(WbemLocator),
			NULL,
			CLSCTX_INPROC_SERVER,
			__uuidof(IWbemLocator),
			(LPVOID*)&pIWbemLocator);
		if (FAILED(hr) || pIWbemLocator == NULL)
			goto LCleanup;

		bstrNamespace = SysAllocString(L"\\\\.\\root\\cimv2"); if (bstrNamespace == NULL) goto LCleanup;
		bstrClassName = SysAllocString(L"Win32_PNPEntity");   if (bstrClassName == NULL) goto LCleanup;
		bstrDeviceID = SysAllocString(L"DeviceID");          if (bstrDeviceID == NULL)  goto LCleanup;

		// Connect to WMI 
		hr = pIWbemLocator->ConnectServer(bstrNamespace, NULL, NULL, 0L,
			0L, NULL, NULL, &pIWbemServices);
		if (FAILED(hr) || pIWbemServices == NULL)
			goto LCleanup;

		// Switch security level to IMPERSONATE. 
		CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
			RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

		hr = pIWbemServices->CreateInstanceEnum(bstrClassName, 0, NULL, &pEnumDevices);
		if (FAILED(hr) || pEnumDevices == NULL)
			goto LCleanup;

		// Loop over all devices
		for (;; )
		{
			// Get 20 at a time
			hr = pEnumDevices->Next(10000, 20, pDevices, &uReturned);
			if (FAILED(hr))
				goto LCleanup;
			if (uReturned == 0)
				break;

			for (iDevice = 0; iDevice < uReturned; iDevice++)
			{
				// For each device, get its device ID
				hr = pDevices[iDevice]->Get(bstrDeviceID, 0L, &var, NULL, NULL);
				if (SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal != NULL)
				{
					// Check if the device ID contains "IG_".  If it does, then it's an XInput device
					// This information can not be found from DirectInput 
					if (wcsstr(var.bstrVal, L"IG_"))
					{
						// If it does, then get the VID/PID from var.bstrVal
						DWORD dwPid = 0, dwVid = 0;
						WCHAR* strVid = wcsstr(var.bstrVal, L"VID_");
						if (strVid && swscanf_s(strVid, L"VID_%4X", &dwVid) != 1)
							dwVid = 0;
						WCHAR* strPid = wcsstr(var.bstrVal, L"PID_");
						if (strPid && swscanf_s(strPid, L"PID_%4X", &dwPid) != 1)
							dwPid = 0;

						// Compare the VID/PID to the DInput device
						DWORD dwVidPid = MAKELONG(dwVid, dwPid);
						if (dwVidPid == pGuidProductFromDirectInput->Data1)
						{
							bIsXinputDevice = true;
							goto LCleanup;
						}
					}
				}
				SAFE_RELEASE(pDevices[iDevice]);
			}
		}

	LCleanup:
		if (bstrNamespace)
			SysFreeString(bstrNamespace);
		if (bstrDeviceID)
			SysFreeString(bstrDeviceID);
		if (bstrClassName)
			SysFreeString(bstrClassName);
		for (iDevice = 0; iDevice < 20; iDevice++)
			SAFE_RELEASE(pDevices[iDevice]);
		SAFE_RELEASE(pEnumDevices);
		SAFE_RELEASE(pIWbemLocator);
		SAFE_RELEASE(pIWbemServices);

		if (bCleanupCOM)
			CoUninitialize();

		return bIsXinputDevice;
	}

	bool DirectInput::IsGamepad(uint32 uIdx) const
	{
		if (uIdx < m_joysticks.size())
		{
			return m_joysticks[uIdx].bIsGamepad;
		}
		return false;
	}

	bool DirectInput::IsThrottle(uint32 uIdx) const
	{
		if (uIdx < m_joysticks.size())
		{
			return m_joysticks[uIdx].bIsThrottle;
		}
		return false;
	}

	usg::string DirectInput::GetName(uint32 uIdx) const
	{
		if (uIdx < m_joysticks.size())
		{
			return m_joysticks[uIdx].productName;
		}
		return "";
	}

	BOOL DirectInput::DeviceEnumCallback(const DIDEVICEINSTANCE* pInst)
	{
		DeviceInfo* pInfo = GetDevice(pInst->tszInstanceName);
		if (IsXInputDevice(&pInst->guidProduct))
			return DIENUM_CONTINUE;
		if (!pInfo)
		{
			if (!pInfo)
			{
				pInfo = &m_joysticks.push_back();
				pInfo->productName = pInst->tszProductName;
				pInfo->instanceName = pInst->tszInstanceName;
				pInfo->guid = pInst->guidInstance;

				switch (pInst->guidProduct.Data1)
				{
				case 0x0ba0054c: 
				case 0x05C4054C: 
				case 0x09cc054c: 
				case 0x0ce6054c: 
					pInfo->bIsGamepad = true;
				default:
					pInfo->bIsGamepad = false;
				}

				switch (pInst->guidProduct.Data1)
				{
				case 0xb687044f:
					pInfo->bIsThrottle = true;
				default:
					pInfo->bIsThrottle = false;
				}

				if ( str::Find( pInfo->productName.c_str(), "Throttle") != nullptr)
				{
					pInfo->bIsThrottle = true;
				}

			}
			pInfo->bConnected = true;
		}

		return DIENUM_CONTINUE;
	}

}