/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Utility function for usage with TargetListComponent
****************************************************************************/
#ifndef __USG_AI_TARGET_SYSTEM_TARGET_LIST_UTIL__
#define __USG_AI_TARGET_SYSTEM_TARGET_LIST_UTIL__

#include "Engine/AI/Targetting/TargetComponents.pb.h"
#include "Engine/AI/Targetting/Target.pb.h"
#include "Engine/AI/Targetting/TargetUtil.h"
#include "Engine/AI/Targetting/TargetEvents.pb.h"
namespace usg
{
namespace ai
{
class TargetListUtil : public usg::ai::TargetUtil
{
public:
	static void UpdateTargetList(RuntimeData<Components::TargetListComponent>& targetList, const usg::ai::Events::TargetEvent& targetEvt);
	static void Tick(RuntimeData<Components::TargetListComponent>& targetList);
protected:
	static bool UpdateTarget(RuntimeData<Components::TargetListComponent>& targetList, usg::ai::TargetData& in, usg::ai::Target& out, const usg::Vector3f& vPos);

	static bool GetNearestTarget(const RuntimeData<Components::TargetListComponent>& targetList, const usg::ai::TargetParams& in, usg::ai::TargetData& out);
	static bool GetFurthestTarget(const RuntimeData<Components::TargetListComponent>& targetList, const usg::ai::TargetParams& in, usg::ai::TargetData& out);
	static bool GetWeakestTarget(const RuntimeData<Components::TargetListComponent>& targetList, const usg::ai::TargetParams& in, usg::ai::TargetData& out);
	static bool GetStrongestTarget(const RuntimeData<Components::TargetListComponent>& targetList, const usg::ai::TargetParams& in, usg::ai::TargetData& out);

	static uint32 GetTargetIndexByEntifyRef(const RuntimeData<Components::TargetListComponent>& targetList, const usg::Entity pEntity);
private:
	static void SetTargetData(usg::ai::TargetData& targetData, usg::Entity targetEntity);
	static uint32 FillArray(const RuntimeData<Components::TargetListComponent>& targetList, const Target** ppTargets, uint32 uSize, const TargetParams& params);
	static void ApplyToTarget(usg::ai::Target& target, const usg::ai::Events::TargetEvent& targetEvt);
};
}	//	namespace ai
}	//	namespace usg
#endif	//	__USG_AI_TARGET_SYSTEM_TARGET_LIST_UTIL__
