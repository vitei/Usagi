/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: An ai relateed component which contains a data regarding navigation, such as
//	current position, facing direction and where it wants to go
*****************************************************************************/
#ifndef __USG_AI_AGENT_NAVIGATION__
#define __USG_AI_AGENT_NAVIGATION__

#include "Engine/Framework/System.h"
#include "Engine/Framework/SystemCategories.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/AI/AgentNavigationUtil.h"

namespace usg
{

	namespace ai
	{

		namespace Components
		{

			//	Full definition declared in AgentNavigationUtil.cpp
			// FIXME: This should be turned into runtime data
			struct AgentNavigationComponent
			{
				struct Data;
				Data* pData;
			};

		}	//	namespace Components

	}
}

#endif	//	__USG_AI_AGENT_NAVIGATION__
