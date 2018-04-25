 /****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The abstract interface for a gamepad
*****************************************************************************/
#pragma once

#ifndef USG_HID_IGAMEPAD_H
#define USG_HID_IGAMEPAD_H
#include "Engine/Common/Common.h"
#include "Engine/Core/Modules/ModuleInterfaces.h"
#include "Engine/HID/InputDefines.h"
#include "Engine/Maths/Matrix4x4.h"

namespace usg
{

	class GFXDevice;
	struct GamepadDeviceState;

	class IGamepad : public ModuleInterface
	{
	public:
		IGamepad() {}
		virtual ~IGamepad() {}

		static const uint32 GetModuleTypeNameStatic() { return 'GPAD'; }
		virtual const uint32 GetModuleTypeName() const override {  return GetModuleTypeNameStatic(); }

		virtual uint32 GetCaps() const = 0;
		virtual bool IsConnected() const = 0;
		virtual void Update(usg::GFXDevice* pDevice, GamepadDeviceState& deviceStateOut) = 0;
		virtual void ResetGyro() {}
		virtual void ResetGyroDirection() {}
	


	};


} // namespace usg

#endif // USG_HID_IGAMEPAD_H
