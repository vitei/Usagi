/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A simple class loading additional modules
//  Would need to be significantly reworked to if we added more modules, 
//  just adding something quickly so that we can remove Oculus as a dependency
//	some of the code is platform dependent and would also need to be moved out
*****************************************************************************/
#pragma once
#include "Engine/Common/Common.h"
#include "Engine/Core/stl/vector.h"
#include "Engine/Core/Singleton.h"
#include "Engine/Core/Modules/ModuleInterfaces.h"

namespace usg
{
	class ModuleManager : public Singleton<ModuleManager>
	{
	public:
		ModuleManager();
		~ModuleManager();

		void Init(GFXDevice* pDevice);
		void Shutdown(GFXDevice* pDevice);
		bool LoadModule(const char* szModuleName);
		uint32 GetNumberOfInterfacesForType(uint32 uTypeId);
		ModuleInterface* GetInterfaceOfType(uint32 uTypeId, uint32 uIndex);

	private:
		struct Module
		{
			HMODULE dllHandle;
			InitModule initFunction;
			DestroyModule destroyFunction;
			ModuleInterfaceSet* interfaceSet;
		};

		ModuleInitData		m_initData;
		usg::vector<Module> m_modules;
	};


}
