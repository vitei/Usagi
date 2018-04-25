/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _SCRIPT_H
#define _SCRIPT_H

#include "Engine/Framework/Script/Script.pb.h"
#include "Engine/Framework/Script/LuaVM.h"

namespace usg
{

template<>
struct RuntimeData<Script>
{
	LuaVM::Module module;
};

template<>
void OnLoaded<Script>(Component<Script>& script, ComponentLoadHandles& handles, bool bWasPreviouslyCalled);

template<>
void OnDeactivate<Script>(Component<Script>& script, ComponentLoadHandles& handles);
}

#endif
