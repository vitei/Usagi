/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "OculusHMD.h"
#include "OculusTouch.h"
#include "dllmain.h"

BOOL WINAPI DllMain(HANDLE hinstDLL, ULONG fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		return TRUE;

	case DLL_PROCESS_DETACH:
		return TRUE;

	case DLL_THREAD_ATTACH:
		return TRUE;

	case DLL_THREAD_DETACH:		
		return TRUE;
	}

	return FALSE;
}

class OculusModuleSet : public usg::ModuleInterfaceSet
{
public:
	OculusModuleSet() {}
	~OculusModuleSet() {}

	void Init(usg::OculusHMD* pOculusHMD)
	{
		m_pOculusHMD = pOculusHMD;
		m_oculusTouch.Init(pOculusHMD);
	}

	void Cleanup(usg::ModuleInitData& initData)
	{
		m_pOculusHMD->Cleanup(initData.pDevice);
	}

	virtual uint32 GetInterfaceCount() override
	{
		return 2;
	}
	virtual usg::ModuleInterface* GetInterface(uint32 uIndex)
	{
		switch (uIndex)
		{
		case 0:
			return m_pOculusHMD;
		case 1:
			return &m_oculusTouch;
		default:
			ASSERT(false);
		}

		return nullptr;
	}

	usg::OculusHMD* m_pOculusHMD;
	usg::OculusTouch m_oculusTouch;
};


bool InitModule(usg::ModuleInitData& initData, class ModuleInterfaceSet* interfaceSet)
{
	OculusModuleSet* pSet = (OculusModuleSet*)interfaceSet;
	if (pSet)
	{
		return pSet->m_pOculusHMD->Init(initData.pDevice);
	}
	return false;
}

usg::ModuleInterfaceSet* OnModuleLoad(usg::ModuleLoadData& loadData)
{
	usg::mem::InitialiseAllocators(&loadData.allocators);
	usg::OculusHMD* pOculusHMD = usg::OculusHMD::TryCreate();
	OculusModuleSet* pSet = nullptr;
	if (pOculusHMD)
	{
		pSet = vnew(usg::ALLOC_OBJECT)OculusModuleSet;
		pSet->Init(pOculusHMD);
	}
	return pSet;
}


void DestroyModule(usg::ModuleInitData& initData, usg::ModuleInterfaceSet* pSet)
{
	if (pSet)
	{
		OculusModuleSet* pOculusSet = (OculusModuleSet*)pSet;
		pOculusSet->m_pOculusHMD->Cleanup(initData.pDevice);
		vdelete pSet;
	}
}
