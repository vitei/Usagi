/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Data context for AI's that want to target other game objects.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_TARGET_CONTEXT__
#define __USG_AI_BEHAVIOR_TREE_TARGET_CONTEXT__
#include "Engine/Common/Common.h"
#include "Engine/Framework/GameComponents.h"
#include "Engine/AI/Targetting/TargetListUtil.h"
#include "Engine/AI/Targetting/Target.pb.h"
#include "Engine/AI/Targetting/TargetComponents.pb.h"
namespace usg
{
namespace ai
{
class TargetContext : public TargetListUtil
{
public:
	TargetContext(TargetComponent& target, const RuntimeData<TargetListComponent>& targetList) : m_target(target), m_targetList(targetList)
	{
	
	}

	bool HasTarget() const { return m_target.active; }
	const Target& GetTarget() const { return m_target.target; }
	const RuntimeData<TargetListComponent>& GetTargetList() const { return m_targetList; }
	void RemoveTarget();

	const Target* GetNearestTarget(const TargetParams& in) const;

protected:
	TargetComponent& m_target;
	const RuntimeData<TargetListComponent>& m_targetList;
};

class TargetContextReadOnly
{
public:
	TargetContextReadOnly(const TargetComponent& target) :m_target(target) {}

	bool HasTarget() const { return m_target.active; }
	const usg::ai::Target& GetTarget() const { return m_target.target; }
	void RemoveTarget() { ASSERT(false); }	//	not implemented, shouldn't be implemented..

protected:
	const TargetComponent& m_target;
};

class ChooseTargetContext : public TargetContext
{
public:
	ChooseTargetContext(const RuntimeData<TargetListComponent>& targetList, TargetComponent& target) : TargetContext(target, targetList), m_targetList(targetList) { }

	bool SetTarget(const TargetParams& in);

private:
	const RuntimeData<TargetListComponent>& m_targetList;
};
}	//	namespace ai
}	//	namespace usg
#endif	//	__USG_AI_BEHAVIOR_TREE_TARGET_CONTEXT__
