/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//  For classes which may be created in dlls and we want to ensure compatability
****************************************************************************/
#ifndef __USG_COMMON_USAGI_INTERFACE_H__
#define __USG_COMMON_USAGI_INTERFACE_H__

#include "Engine/Common/Common.h"

namespace usg
{
	class GFXDevice;

	class ModuleInterface
	{
	public:
		virtual const uint32 GetModuleTypeName() const = 0;
	};


	class ModuleInterfaceSet
	{
	public:
		virtual uint32 GetInterfaceCount() = 0;
		virtual ModuleInterface* GetInterface(uint32 uIndex) = 0;
	};

	struct ModuleInitData
	{
		mem::AllocatorData allocators;
		GFXDevice* pDevice;
	};

	struct ModuleLoadData
	{
		mem::AllocatorData allocators;
	};

	// Called when the module is loaded, may not be valid if dependent on core initialization (e.g. graphics)
	typedef ModuleInterfaceSet* (far *OnModuleLoad)(ModuleLoadData& pInitData);
	// The function called once the engine has been properly initialized
	typedef bool (far *InitModule)(ModuleInitData& pInitData, ModuleInterfaceSet* interfaceSet);
	// Called to clean up the data
	typedef void				(far *DestroyModule)(ModuleInitData& pInitData, ModuleInterfaceSet* Set);
	
}


#endif
