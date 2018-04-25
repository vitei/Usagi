/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
//	AgentNavigation.cpp
#include "Engine/Common/Common.h"
#include "AgentNavigation.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Framework/SystemCoordinator.h"

namespace usg
{

template <>
void OnActivate<usg::ai::Components::AgentNavigationComponent>(Component<usg::ai::Components::AgentNavigationComponent>& c)
{
	c.GetData().pData = NULL;
	usg::ai::AgentNavigationUtil::Alloc(&c.GetData());
}

template <>
void OnDeactivate<usg::ai::Components::AgentNavigationComponent>(Component<usg::ai::Components::AgentNavigationComponent>& c, ComponentLoadHandles& handles)
{
	usg::ai::AgentNavigationUtil::Free(&c.GetData());
}

}
