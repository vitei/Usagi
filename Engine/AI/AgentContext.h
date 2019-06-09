/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: An agent is something that moves around and is aware of what is
//	in it's vicinity. This inheriting from Both Target and Navigation context
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_AGENT_CONTEXT__
#define __USG_AI_BEHAVIOR_TREE_AGENT_CONTEXT__

#include "Engine/AI/INavigationContext.h"
#include "Engine/AI/ITargetContext.h"
namespace usg
{
namespace ai
{
class AgentContext : public INavigationContext, public ChooseTargetContext
{
public:
	AgentContext(AgentNavigationComponent& nav, 
		const usg::ai::NavigationGrid* pNavGrid, 
		const usg::ai::Components::TargetListComponent& targetList, 
		usg::ai::Components::TargetComponent& target):
		INavigationContext(nav, pNavGrid),
		ChooseTargetContext(targetList, target){}
};
}	//	namespace ai
}	//	namespace usg
#endif	//	__USG_AI_BEHAVIOR_TREE_AGENT_CONTEXT__
