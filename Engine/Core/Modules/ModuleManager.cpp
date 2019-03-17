/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemUtil.h"
#include "ModuleManager.h"

namespace usg {

	ModuleManager::ModuleManager():
		m_bInitCalled(false)
	{

	}


	ModuleManager::~ModuleManager()
	{
		
	}


	void ModuleManager::PreInit()
	{
		m_loadData.allocators.pAllocators[MEMTYPE_STANDARD] = mem::GetAllocator(MEMTYPE_STANDARD);
		ASSERT(m_loadData.allocators.pAllocators[MEMTYPE_STANDARD] != nullptr);
	}


	void ModuleManager::PostInit(GFXDevice* pDevice)
	{
		ASSERT(m_loadData.allocators.pAllocators[MEMTYPE_STANDARD] != nullptr);
		MemClear(&m_initData.allocators, sizeof(m_initData.allocators));
		for (int i = 0; i < MEMTYPE_COUNT; i++)
		{
			m_initData.allocators.pAllocators[MEMTYPE_STANDARD] = mem::GetAllocator(MEMTYPE_STANDARD);
		}
		m_initData.pDevice = pDevice;
		ASSERT(!m_bInitCalled);
		for (Module& module : m_modules)
		{
			ASSERT(module.interfaceSet != nullptr);
			module.initFunction(m_initData, module.interfaceSet);
		}
		m_bInitCalled = true;
	}

	bool ModuleManager::LoadModule(const char* szModuleName)
	{
		Module module;
		module.dllHandle = LoadLibrary(szModuleName);
		if (!module.dllHandle)
		{
			return false;
		}

		module.moduleName = szModuleName;
		module.loadFunction = (usg::OnModuleLoad)GetProcAddress(module.dllHandle, "OnModuleLoad");
		module.initFunction = (InitModule)GetProcAddress(module.dllHandle, "InitModule");
		module.destroyFunction = (DestroyModule)GetProcAddress(module.dllHandle, "DestroyModule");

		if (!module.initFunction || !module.destroyFunction || !module.loadFunction)
		{
			return false;
		}

		module.interfaceSet = module.loadFunction(m_loadData);
		if (!module.interfaceSet)
		{
			return false;
		}

		if (m_bInitCalled)
		{
			if (!module.initFunction(m_initData, module.interfaceSet))
			{
				return false;
			}
		}

		if (module.interfaceSet)
		{
			m_modules.push_back(module);
		}

		return true;
	}

	HMODULE ModuleManager::GetModule(usg::string moduleName)
	{
		for (Module& module : m_modules)
		{
			if (module.moduleName == moduleName)
			{
				return module.dllHandle;
			}
		}

		return nullptr;
	}

	void ModuleManager::Shutdown(GFXDevice* pDevice)
	{
		for (usg::vector<Module>::iterator it = m_modules.begin(); it!= m_modules.end(); ++it)
		{
			it->destroyFunction(m_initData, it->interfaceSet);
			it->interfaceSet = nullptr;
		}

		m_modules.clear();
	}

	uint32 ModuleManager::GetNumberOfInterfacesForType(uint32 uTypeId)
	{
		uint32 uCount = 0;
		for (uint32 i = 0; i < m_modules.size(); i++)
		{
			for (uint32 j = 0; j < m_modules[i].interfaceSet->GetInterfaceCount(); j++)
			{
				if (m_modules[i].interfaceSet->GetInterface(j)->GetModuleTypeName() == uTypeId)
				{
					uCount++;
				}
			}
		}
		return uCount;
	}


	ModuleInterface* ModuleManager::GetInterfaceOfType(uint32 uTypeId, uint32 uIndex)
	{
		uint32 uCount = 0;
		for (uint32 i = 0; i < m_modules.size(); i++)
		{
			for (uint32 j = 0; j < m_modules[i].interfaceSet->GetInterfaceCount(); j++)
			{
				if (m_modules[i].interfaceSet->GetInterface(j)->GetModuleTypeName() == uTypeId)
				{
					if (uCount == uIndex)
					{
						return m_modules[i].interfaceSet->GetInterface(j);
					}
					uCount++;
				}
			}
		}
		return nullptr;
	}


}