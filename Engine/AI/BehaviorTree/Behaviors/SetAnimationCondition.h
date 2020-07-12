/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Modify condition variables of a ModelAnimPlayer connected to the AI entity.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_SET_ANIMCOND__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_SET_ANIMCOND__

#include "Engine/AI/BehaviorTree/Decorator.h"
#include "Engine/Scene/Model/ModelEvents.pb.h"
#include "Engine/Framework/EventManager.h"

namespace usg
{

namespace ai
{

template <class ContextType>
class bhSetAnimationCondition : public IDecorator<ContextType>
{
	static const uint32 NAME_MAX_LENGTH = 32;
	char m_szChildEntityName[NAME_MAX_LENGTH];
	uint32 m_uConditionNameHash;
	uint8 m_uValueLastTime;
	uint32 m_uEventFlags;
	Entity m_pEntity;
	EventManager* m_pEventManager;
public:
	void SetData(const SetAnimationCondition& cond, EventManager* pEventManager)
	{
		ASSERT(str::StringLength(cond.childEntityName) < NAME_MAX_LENGTH);
		ASSERT(str::StringLength(cond.conditionName) < NAME_MAX_LENGTH);
		str::Copy(m_szChildEntityName, cond.childEntityName, NAME_MAX_LENGTH);
		m_uConditionNameHash = utl::CRC32(cond.conditionName);
		m_uValueLastTime = 2;
		m_uEventFlags = cond.onAllChildrenRecursively ? (ON_ENTITY | ON_CHILDREN):(ON_ENTITY);
		m_pEntity = NULL;
		m_pEventManager = pEventManager;
	}

	virtual void Init(ContextType& ctx)
	{
		m_pEntity = m_szChildEntityName[0] != 0 ? ctx.GetEntity()->GetChildEntityByName(m_szChildEntityName) : ctx.GetEntity();
		m_pEventManager = NULL;
	}

	virtual int Update(float fElapsed, ContextType& ctx)
	{
		bool bSuccess = this->GetChild().Tick(fElapsed, ctx) == BH_SUCCESS;
		uint8 uNewValue = (uint8)bSuccess;
		if (m_uValueLastTime != uNewValue)
		{
			m_uValueLastTime = uNewValue;
			ModifyAnimationConditionEvent event;
			event.nameHash = m_uConditionNameHash;
			event.value = bSuccess;

			ASSERT(m_pEntity != NULL);
			SAFE_DEREF(m_pEventManager)->RegisterEventWithEntity(m_pEntity, event, m_uEventFlags);
		}
		return bSuccess ? BH_SUCCESS : BH_FAILURE;
	}
};

}	//	namespace ai

}	//	namespace usg

#endif
