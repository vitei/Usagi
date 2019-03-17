/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/Modules/ModuleInterfaces.h"


DLL_EXPORT 	usg::ModuleInterfaceSet* OnModuleLoad(usg::ModuleLoadData& loadData);
DLL_EXPORT 	bool InitModule(usg::ModuleInitData& initData, class ModuleInterfaceSet* interfaceSet);
DLL_EXPORT 	void DestroyModule(usg::ModuleInitData& initData, usg::ModuleInterfaceSet* Set);
