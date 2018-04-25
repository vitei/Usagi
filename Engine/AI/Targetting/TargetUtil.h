/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Utility system and helper functions for TargetComponent.
*****************************************************************************/
#ifndef __USG_AI_TARGET_UTIL__
#define __USG_AI_TARGET_UTIL__
#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector3f.h"
#include "Engine/AI/Targetting/Target.pb.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"

namespace usg
{

namespace ai
{

class TargetUtil
{
protected:
	static void CalculateDirectionToTarget(usg::ai::Target& target, const Vector3f& vPos);
	//	targetTeam = friend or foe? (AI setting) compare targets team to passed parameter "team"
	static bool IsTeam(const usg::ai::Target& target, usg::ai::Team targetTeam, uint32 team);
	static bool IsTeam(usg::ai::Team team, uint32 a, uint32 b);
	static bool IsType(const usg::ai::Target& target, uint32 uType);
	static bool IsType(uint32 uTargetType, uint32 uCompareType);
	static void TargetCopy(const usg::ai::Target& src, usg::ai::Target& dst);
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_TARGET_UTIL__
