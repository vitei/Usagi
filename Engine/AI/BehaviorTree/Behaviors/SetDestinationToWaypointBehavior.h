/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2016
//	Description: Sets the destination to a position of choice.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_SET_DESTINATIONTOWAYPOINT_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_SET_DESTINATIONTOWAYPOINT_BEHAVIOR__

#include "Engine/AI/AICommon.h"
#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
#include "Engine/Maths/Vector3f.h"
#include "Engine/Core/Utility.h"
#include "Engine/Framework/Signal.h"
#include "Engine/AI/INavigationContext.h"

namespace usg
{

	namespace ai
	{

		template <class ContextType>
		class bhSetDestinationToWaypoint : public IBehavior<ContextType>
		{
		public:

			void SetData(SetDestinationToWaypoint& data)
			{
				m_uWaypointNameHash = utl::CRC32(data.waypointName);
			}

		protected:
			virtual int Update(float fElapsed, ContextType& ctx)
			{
				NavigationWrapper* pNav = GetNavigation(ctx);
				ASSERT(pNav != NULL);
				const NavigationGrid* pNaviGrid = pNav->GetNavigationGrid();
				const Waypoint* pWaypoint = pNaviGrid->GetWaypoint(m_uWaypointNameHash);
				ASSERT(pWaypoint != NULL);
				pNav->SetDestination(Vector3f(pWaypoint->GetPosition().x, 0.0f, pWaypoint->GetPosition().y), true);
				return BH_SUCCESS;
			}
			
			uint32 m_uWaypointNameHash;
		};

	}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_SET_DESTINATIONTOWAYPOINT_BEHAVIOR__
