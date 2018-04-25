#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Behaviors/AvoidanceBehavior.h"
#include "Engine/AI/AICommon.h"

namespace usg
{

	namespace ai
	{
		static inline Vector2f GetCollisionNormal(const IShape& shape, const Line& line)
		{
			const uint32 uNumVerts = shape.GetNumVerts();
			for (uint32 i = 0; i < uNumVerts; i++)
			{
				const Vector2f vEdgeFrom = shape.GetVerts()[i] + shape.GetCenter();
				const Vector2f vEdgeTo = shape.GetVerts()[(i+1)%uNumVerts] + shape.GetCenter();
				const Vector2f vEdgeDir = vEdgeTo - vEdgeFrom;
				const Vector2f vEdgeNormal(-vEdgeDir.y,vEdgeDir.x);
				const float fDotStart = (line.m_vFrom - vEdgeFrom).DotProduct(vEdgeNormal);
				if (fDotStart > 0)
				{
					const float fDotEnd = (line.m_vTo - vEdgeFrom).DotProduct(vEdgeNormal);
					if (fDotEnd < 0)
					{
						return vEdgeNormal.GetNormalised();
					}
				}
			}
			return Vector2f();
		}

		Vector3f AvoidanceTools::GetAvoidance(const OBB& obb, const Vector3f& vPos, const Vector3f& vForward, float fAvoidanceRadius, float fAvoidanceForce, float fAhead)
		{
			usg::Vector3f vAvoidance(0.0f);
			Line line;
			line.m_vFrom = ToVector2f(vPos);
			line.m_vTo = ToVector2f(vPos + vForward*(fAvoidanceRadius + fAhead));
			if (obb.Intersects(line))
			{
				const Vector2f vCollisionNormal = GetCollisionNormal(obb, line);
				if (vCollisionNormal.MagnitudeSquared() <= Math::EPSILON)
				{
					// Perhaps we are already inside a blocking area?
					vAvoidance += -vForward * fAvoidanceForce;
				}
				else
				{
					const Vector2f vDesiredMoveDir = (line.m_vTo - line.m_vFrom).GetNormalised();
					const float fDot = vDesiredMoveDir.DotProduct(vCollisionNormal);
					const Vector2f vAvoidanceDir = fDot < -0.98f ? -vDesiredMoveDir : (vDesiredMoveDir - vCollisionNormal*vDesiredMoveDir.DotProduct(vCollisionNormal)).GetNormalised();
					vAvoidance += ToVector3f(vAvoidanceDir) * fAvoidanceForce;
				}
			}
			return vAvoidance;
		}

	}
}

