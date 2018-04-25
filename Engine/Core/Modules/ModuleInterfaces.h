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

	typedef ModuleInterfaceSet* (far *InitModule)(ModuleInitData& pInitData);
	typedef void				(far *DestroyModule)(ModuleInitData& pInitData, ModuleInterfaceSet* Set);
	
}


#endif
