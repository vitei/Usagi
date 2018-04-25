/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemUtil.h"
#include "ModuleManager.h"

namespace usg {

	ModuleManager::ModuleManager()
	{
	}


	ModuleManager::~ModuleManager()
	{
	}


	void ModuleManager::Init(GFXDevice* pDevice)
	{
		MemClear(&m_initData.allocators, sizeof(m_initData.allocators));
		m_initData.allocators.pAllocators[MEMTYPE_STANDARD] = mem::GetAllocator(MEMTYPE_STANDARD);
		m_initData.pDevice = pDevice;
	}

	bool ModuleManager::LoadModule(const char* szModuleName)
	{
		Module module;
		module.dllHandle = LoadLibrary(szModuleName);
		if (!module.dllHandle)
		{
			return false;
		}

		module.initFunction = (InitModule)GetProcAddress(module.dllHandle, "InitModule");
		module.destroyFunction = (DestroyModule)GetProcAddress(module.dllHandle, "DestroyModule");

		if (!module.initFunction || !module.destroyFunction)
		{
			return false;
		}

		module.interfaceSet = module.initFunction(m_initData);

		if (module.interfaceSet)
		{
			m_modules.push_back(module);
		}

		return true;
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