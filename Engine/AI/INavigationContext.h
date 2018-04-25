/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Contains the data required to use navigation context for AI.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_CONTEXT_INTERFACE_NAVIGATION__
#define __USG_AI_BEHAVIOR_TREE_CONTEXT_INTERFACE_NAVIGATION__
#include "Engine/Common/Common.h"
#include "Engine/AI/NavigationWrapper.h"
namespace usg
{
namespace ai
{
class NavigationGrid;
class INavigationContext
{
public:
	INavigationContext(Components::AgentNavigationComponent& nav, const NavigationGrid* pNavGrid):
		m_nav(nav, pNavGrid){}

	NavigationWrapper& Navigation() { return m_nav; }
	const NavigationWrapper& Navigation() const { return m_nav; }

protected:
	NavigationWrapper m_nav;
};
}	//	namespace ai
}	//	namespace usg
#endif	//	__USG_AI_BEHAVIOR_TREE_CONTEXT_INTERFACE_NAVIGATION__
