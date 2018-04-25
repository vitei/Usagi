/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/HID/IGamepad.h"
#include "Engine/HID/Input.h"
#include "Engine/Audio/Audio.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/IHeadMountedDisplay.h"
#include "Engine/Core/Modules/ModuleManager.h"
#include "GameUtil.h"

namespace usg
{
	void HookupModules(GFXDevice* pDevice)
	{
		{
			uint32 uHMDId = IHeadMountedDisplay::GetModuleTypeNameStatic();
			uint32 uHMDCount = ModuleManager::Inst()->GetNumberOfInterfacesForType(uHMDId);

			for (uint32 i = 0; i < uHMDCount; i++)
			{
				ModuleInterface* pInterface = ModuleManager::Inst()->GetInterfaceOfType(uHMDId, i);
				pDevice->SetHeadMountedDisplay((IHeadMountedDisplay*)pInterface);
				usg::Audio::Inst()->StartUsingHeadset((IHeadMountedDisplay*)pInterface);
			}
		}

		{
			uint32 uGamepadId = IGamepad::GetModuleTypeNameStatic();
			uint32 uGamepadCount = ModuleManager::Inst()->GetNumberOfInterfacesForType(uGamepadId);

			for (uint32 i = 0; i < uGamepadCount; i++)
			{
				ModuleInterface* pInterface = ModuleManager::Inst()->GetInterfaceOfType(uGamepadId, i);
				Input::RegisterGamepad((IGamepad*)pInterface);
			}
		}
	}
}