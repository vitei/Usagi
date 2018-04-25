/****************************************************************************
//	Filename: AIComponentInit.cpp
//	Description: This is meant to be AI engine related code but since it is
//	currently not possible to call events outside of game specific code, for
//	the time being, it is being placed here.
*****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/AI/AgentNavigationUtil.h"
#include "Engine/AI/Targetting/TargetComponents.pb.h"

namespace usg
{

template <>
void OnActivate<usg::ai::Components::TargetComponent>(Component<usg::ai::Components::TargetComponent>& c)
{
	c.Modify().targetData.index = 0;
	c.Modify().targetData.target.entity = NULL;
}

template <>
void OnActivate<usg::ai::Components::TargetListComponent>(Component<usg::ai::Components::TargetListComponent>& c)
{
	c.GetRuntimeData().targets.clear();

	usg::ai::Components::TargetListRaycastComponent* pRaycastComponent = GameComponents<usg::ai::Components::TargetListRaycastComponent>::Create(c.GetEntity());
	pRaycastComponent->pLastRaycastEntity.entity = NULL;
	pRaycastComponent->uLastRaycastIndex = (uint32)(-1);
	pRaycastComponent->fTimer = -1.0f;
}

}	//	namespace usg
