#include "Engine/Common/Common.h"
#include "NavigationWrapper.h"

namespace usg
{

	namespace ai
	{

		NavigationWrapper::NavigationWrapper(Components::AgentNavigationComponent& nav, const usg::ai::NavigationGrid* pNavGrid) :
			m_nav(nav)
		{
			AgentNavigationUtil::SetNavigationGrid(nav, pNavGrid);
		}

	}

}