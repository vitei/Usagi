/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: Interface for avoidance behaviors
 *****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_AVOIDANCE_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_AVOIDANCE_BEHAVIOR__

#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/Pathfinding/NavigationGrid.h"
#include "Engine/Maths/Vector3f.h"
namespace usg
{

namespace ai
{

	class AvoidanceTools
	{
	public:
		static Vector3f GetAvoidance(const OBB& obb, const Vector3f& vPos, const Vector3f& vForward, float fAvoidanceRadius, float fAvoidanceForce, float fAhead);
	};

template <class ContextType>
class bhAvoidance : public IBehavior<ContextType>
{
public:
	bhAvoidance()
	{
		m_vAvoidanceDiff = Vector3f(0.0f);
	}

	void SetRadius(float fRadius) { m_fRadius = fRadius; }
	void SetAhead(float fAhead) { m_fAhead = fAhead; }
	void SetAheadFar(float fAheadFar) { m_fAheadFar = fAheadFar; }
	void SetAvoidanceForce(float fAvoidanceForce) { m_fAvoidanceForce = fAvoidanceForce; }
protected:
	bool CircleCollisionXZ(const Vector3f& v1, const Vector3f& v2, const float fRadA, const float fRadB)
	{
		const float fX = v2.x - v1.x;
		const float fY = v2.z - v1.z;
		const float fR = fRadA + fRadB;
		return (((fX * fX) + (fY * fY)) <= (fR * fR));
	}

	bool PointCircleCollisionXZ(const Vector3f& vTarget, const Vector3f& vPos, const float fRad)
	{
		return (vTarget.GetSquaredXZDistanceFrom(vPos) <= (fRad * fRad));
	}

	int Avoid(ContextType& ctx, const OBB& obb)
	{
		const Vector3f& vPos = ctx.GetPosition();
		const Vector3f& vForward = ctx.Navigation().GetForward();
		Vector3f vAvoidance = AvoidanceTools::GetAvoidance(obb, vPos, vForward, m_fRadius, m_fAvoidanceForce, m_fAhead);
		const bool bAvoid = vAvoidance.MagnitudeSquared() > 0.0f;
		if (bAvoid)
		{
			ctx.Navigation().AddToAvoidance(vAvoidance);
			m_vAvoidanceDiff += vAvoidance;
			return BH_RUNNING;
		}
		else
		{
			m_vAvoidanceDiff = Vector3f(0.0f);
		}

		return BH_FAILURE;
	}

	int Avoid(ContextType& ctx, const Vector3f& vTarget, const float fTargetRad)
	{
		const Vector3f& vPos = ctx.GetPosition();
		const Vector3f& vForward = ctx.Navigation().GetForward();
		Vector3f vAvoidance = GetAvoidance(vTarget, vPos, fTargetRad, vForward);
		const bool bAvoid = vAvoidance.MagnitudeSquared() > 0.0f;
		if(bAvoid)
		{
			ctx.Navigation().AddToAvoidance(vAvoidance);
			m_vAvoidanceDiff += vAvoidance;
			return BH_RUNNING;
		}
		else
		{
			m_vAvoidanceDiff = Vector3f(0.0f);
		}

		return BH_FAILURE;
	}

	Vector3f GetAvoidance(const Vector3f& vTarget, const Vector3f& vPos, float fTargetRad, const Vector3f& vForward)
	{
		usg::Vector3f vAvoidance(0.0f);
		const usg::Vector3f vPos2D(vPos.x, 0.0f, vPos.z);
		const usg::Vector3f vAhead = vPos2D + (vForward * m_fAhead);
		const usg::Vector3f vAheadFar = vPos2D + (vForward * m_fAheadFar);

		if(PointCircleCollisionXZ(vTarget, vPos2D, fTargetRad))
		{
			vAvoidance += (vPos2D - vTarget).GetNormalised() * m_fAvoidanceForce;
		}
		else if(CircleCollisionXZ(vTarget, vAhead, m_fRadius, fTargetRad))
		{
			vAvoidance += (vAhead - vTarget).GetNormalised() * m_fAvoidanceForce;
		}
		else if(CircleCollisionXZ(vTarget, vAheadFar, m_fRadius, fTargetRad))
		{
			vAvoidance += (vAheadFar - vTarget).GetNormalised() * m_fAvoidanceForce;
		}

		return vAvoidance;
	}

	Vector3f m_vAvoidanceDiff;
	float m_fRadius;	//	the size of this agent
	float m_fAhead;
	float m_fAheadFar;
	float m_fAvoidanceForce;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_AVOIDANCE_BEHAVIOR__