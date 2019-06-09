/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Ticks the child if a dest is present and it is less
//	than the given distance away
*****************************************************************************/
#ifndef __USG_AI_DEST_MIN_DISTANCE_DECORATOR__
#define __USG_AI_DEST_MIN_DISTANCE_DECORATOR__

#include "Engine/AI/BehaviorTree/Decorator.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
#include "Engine/AI/BehaviorTree/Behaviors/DestinationMinDistanceBehavior.h"

namespace usg
{

	namespace ai
	{

		template <class ContextType>
		class dcDestinationMinDistance : public IDecorator<ContextType>
		{
			bhDestinationMinDistance<ContextType> m_bhNode;
		public:

			void SetData(const usg::ai::DestinationMinDistance& data)
			{
				m_bhNode.SetData(data);
			}

		protected:
			virtual int Update(float fElapsed, ContextType& ctx)
			{
				if (m_bhNode.Update(fElapsed, ctx) == BH_SUCCESS)
				{
					return this->GetChild().Tick(fElapsed, ctx);
				}
				return BH_FAILURE;
			}
		};

		template <class ContextType>
		class dcDestinationMaxDistance : public IDecorator<ContextType>
		{
			bhDestinationMaxDistance<ContextType> m_bhNode;
		public:

			void SetData(const usg::ai::DestinationMaxDistance& data)
			{
				m_bhNode.SetData(data);
			}

		protected:
			virtual int Update(float fElapsed, ContextType& ctx)
			{
				if (m_bhNode.Update(fElapsed, ctx) == BH_SUCCESS)
				{
					return this->GetChild().Tick(fElapsed, ctx);
				}
				return BH_FAILURE;
			}
		};

	}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_DECORATORS_TARGET_MIN_DISTANCE_DECORATOR__
