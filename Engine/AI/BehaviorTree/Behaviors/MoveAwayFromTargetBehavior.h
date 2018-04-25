/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Moves the agent away from the target.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_MOVE_AWAY_FROM_TARGET_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_MOVE_AWAY_FROM_TARGET_BEHAVIOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"

namespace usg
{

	namespace ai
	{

		template <class ContextType>
		class bhMoveAwayFromTarget : public IBehavior<ContextType>
		{
		public:
			bhMoveAwayFromTarget()
			{
				m_bStopMoving = false;
			}

			virtual int Update(float fElapsed, ContextType& ctx)
			{
				if (!ctx.HasTarget())
				{
					return BH_FAILURE;
				}

				ctx.Navigation().ClearDestination();

				const Vector3f& vPos = ctx.Navigation().GetPosition();
				const Vector3f& vTarget = ctx.GetTarget().position;

				const float fDistanceToTargetSq = vPos.GetSquaredXZDistanceFrom(vTarget);

				if (fDistanceToTargetSq > m_data.maxDistance - Math::EPSILON)
				{
					m_bStopMoving = true;
				}

				if (m_bStopMoving)
				{
					if (fDistanceToTargetSq < m_data.minDistance)
					{
						m_bStopMoving = false;
					}

					return BH_SUCCESS;
				}

				const float fDistanceToTarget = sqrtf(fDistanceToTargetSq);
				const float fMax = sqrtf(m_data.maxDistance);

				ASSERT(fDistanceToTarget <= fMax);

				const float fEscapeAmount = fMax - fDistanceToTarget;
				Vector3f vWantsToEscapeTo = ctx.GetPosition() - ctx.GetTarget().normalizedDirToTarget*fEscapeAmount;

				// Change escape point if it lies inside blocking area
				const NavigationGrid* pNaviGrid = ctx.Navigation().GetNavigationGrid();
				float fAngle = 0.2f;
				if (!pNaviGrid->CanPlacePoint(ToVector2f(vWantsToEscapeTo)))
				{
					for (;;)
					{
						const Vector2f vToEscapePos = ToVector2f(vWantsToEscapeTo - ctx.GetPosition());
						Vector2f vTestPoint = vToEscapePos;
						Vector2f::RotateAboutPoint(Vector2f(0.0f, 0.0f), vTestPoint, fAngle);
						if (pNaviGrid->CanPlacePoint(ToVector2f(ctx.GetPosition() + ToVector3f(vTestPoint))))
						{
							vWantsToEscapeTo = ctx.GetPosition() + ToVector3f(vTestPoint);
							break;
						}

						vTestPoint = vToEscapePos;
						Vector2f::RotateAboutPoint(Vector2f(0.0f, 0.0f), vTestPoint, -fAngle);
						if (pNaviGrid->CanPlacePoint(ToVector2f(ctx.GetPosition() + ToVector3f(vTestPoint))))
						{
							vWantsToEscapeTo = ctx.GetPosition() + ToVector3f(vTestPoint);
							break;
						}

						fAngle += 0.2f;
						if (fAngle > 1.0f)
						{
							return BH_RUNNING;
						}
						break;
					}
				}

				ctx.Navigation().AddToTargetDirection((vWantsToEscapeTo - ctx.GetTarget().position).GetNormalisedIfNZero());
				ctx.Navigation().SetMove(true);

				const bool bIsFacingTarget = DotProduct(ctx.GetTarget().normalizedDirToTarget, ctx.GetForward()) > 0;
				ctx.Navigation().SetReverse(bIsFacingTarget);

				return BH_RUNNING;
			}

			void SetData(const usg::ai::MoveAwayFromTarget& data)
			{
				m_data.maxDistance = data.maxDistance * data.maxDistance;
				m_data.minDistance = data.minDistance * data.minDistance;
			}

		private:
			usg::ai::MoveAwayFromTarget m_data;
			bool m_bStopMoving;
		};

	}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_MOVE_AWAY_FROM_TARGET_BEHAVIOR__