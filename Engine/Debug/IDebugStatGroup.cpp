/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Debug/DebugStats.h"

namespace usg {

#ifndef FINAL_BUILD
	IDebugStatGroup::~IDebugStatGroup()
	{
		if (m_pOwner)
		{
			m_pOwner->DeregisterGroup(this);
		}
	}
#endif


}

