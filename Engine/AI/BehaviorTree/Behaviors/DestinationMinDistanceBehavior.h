/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Returns SUCCESS if the destination is at least below the specified 
	distance (squared).
*****************************************************************************/

#ifndef __USG_AI_DEST_MIN_DISTANCE_BEHAVIOR__
#define __USG_AI_DEST_MIN_DISTANCE_BEHAVIOR__

#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
#include "Engine/AI/AICommon.h"

namespace usg
{

	namespace ai
	{

		template <class ContextType>
		class bhDestinationMinDistance : public IBehavior<ContextType>
		{
		public:
			virtual int Update(float fElapsed, ContextType& ctx)
			{
				const NavigationWrapper* pAgent = GetNavigation(ctx);
				ASSERT(pAgent && "This behavior must be used from a context which provides NavigationWrapper.");
				if (!pAgent->HasDestination())
				{
					return BH_FAILURE;
				}
				if (pAgent->GetPosition().GetSquaredXZDistanceFrom(pAgent->GetDestination()) <= m_fMinDistSqr)
				{
					return BH_SUCCESS;
				}
				return BH_RUNNING;
			}

			void SetData(const DestinationMinDistance& data)
			{
				m_fMinDistSqr = data.fMinDistance*data.fMinDistance;
			}
		private:
			float m_fMinDistSqr;
		};

		template <class ContextType>
		class bhDestinationMaxDistance : public IBehavior<ContextType>
		{
		public:
			virtual int Update(float fElapsed, ContextType& ctx)
			{
				const NavigationWrapper* pAgent = GetNavigation(ctx);
				ASSERT(pAgent && "This behavior must be used from a context which provides NavigationWrapper.");
				if (!pAgent->HasDestination())
				{
					return BH_FAILURE;
				}
				if (pAgent->GetPosition().GetSquaredXZDistanceFrom(pAgent->GetDestination()) >= m_fMaxDistSqr)
				{
					return BH_SUCCESS;
				}
				return BH_RUNNING;
			}

			void SetData(const DestinationMaxDistance& data)
			{
				m_fMaxDistSqr = data.fMaxDistance*data.fMaxDistance;
			}
		private:
			float m_fMaxDistSqr;
		};

	}

}

#endif