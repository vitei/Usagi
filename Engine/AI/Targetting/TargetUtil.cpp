/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
//	TargetUtil.cpp
#include "Engine/Common/Common.h"
#include "TargetUtil.h"

namespace usg
{

namespace ai
{

void TargetUtil::CalculateDirectionToTarget(usg::ai::Target& target, const Vector3f& vPos)
{
	target.normalizedDirToTarget = (target.position - vPos).GetNormalisedIfNZero(Vector3f(0.0f, 0.0f, 1.0f));
}

bool TargetUtil::IsTeam(const usg::ai::Target& target, usg::ai::Team targetTeam, uint32 team)
{
	if(targetTeam == usg::ai::Team_Friendly)
	{
		return target.team == team;
	}
	else if(targetTeam == usg::ai::Team_Enemy)
	{
		return target.team != team;
	}

	return true;
}

bool TargetUtil::IsTeam(usg::ai::Team team, uint32 a, uint32 b)
{
	if(team == usg::ai::Team_Friendly)
	{
		return a == b;
	}
	else if(team == usg::ai::Team_Enemy)
	{
		return a != b;
	}

	return true;
}

bool TargetUtil::IsType(const usg::ai::Target& target, uint32 uType)
{
	return ((target.type & uType) != 0);
}

bool TargetUtil::IsType(uint32 uTargetType, uint32 uCompareType)
{
	return uTargetType == uCompareType;
}

void TargetUtil::TargetCopy(const usg::ai::Target& src, usg::ai::Target& dst)
{
	dst.active = src.active;
	dst.entityRef.entity = src.entityRef.entity;
	dst.forward = src.forward;
	dst.health = src.health;
	dst.normalizedDirToTarget = src.normalizedDirToTarget;
	dst.position = src.position;
	dst.right = src.right;
	dst.team = src.team;
	dst.type = src.type;
	dst.visible = src.visible;
	dst.uLineOfSight = src.uLineOfSight;
	dst.vVelocity = src.vVelocity;
	dst.iNUID = src.iNUID;
	dst.fTimeStamp = src.fTimeStamp;
}

}	//	namespace ai

}	//	namespace usg
