/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Used for creating behaviors for a behavior tree.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIOR_FACTORY__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIOR_FACTORY__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/BehaviorTree.h"
#include "Engine/AI/BehaviorTree/Behaviors/Behaviors.h"
#include "Engine/AI/BehaviorTree/Decorators/Decorators.h"
#include "Engine/AI/BehaviorTree/Composite.h"
#include "Engine/AI/BehaviorTree/Composites/Composites.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Framework/EventManager.h"

namespace usg
{

namespace ai
{

template <class ContextType>
class BehaviorFactory
{
public:
	BehaviorFactory(const char* szFilename, BehaviorTree<ContextType>* pBt) :
		m_file(szFilename),
		m_pBt(pBt){}
	virtual ~BehaviorFactory(){}

	IBehavior<ContextType>* Load();
	IBehavior<ContextType>* CreateBehaviorContextSpecific(usg::ProtocolBufferFile& file, BehaviorTree<ContextType>* pBT, usg::ai::BehaviorHeader& behaviorHeader){ return NULL; }
	IDecorator<ContextType>* CreateDecoratorContextSpecific(usg::ProtocolBufferFile& file, BehaviorTree<ContextType>* pBT, usg::ai::DecoratorHeader& decoratorHeader) { return NULL; }


protected:
	IBehavior<ContextType>* CreateComposite(CompositeHeader& compositeHeader);
	virtual IBehavior<ContextType>* CreateBehavior(BehaviorHeader& behaviorHeader);
	virtual IDecorator<ContextType>* CreateDecorator(DecoratorHeader& decoratorHeader, EventManager* pEventManager=NULL);

	usg::ProtocolBufferFile m_file;
	BehaviorTree<ContextType>* m_pBt;
};

template <class ContextType>
IBehavior<ContextType>* BehaviorFactory<ContextType>::Load()
{
	usg::ai::CompositeHeader header;
	bool bOk = m_file.Read(&header);
	ASSERT(bOk);
	return this->CreateComposite(header);
}

template <class ContextType>
IBehavior<ContextType>* BehaviorFactory<ContextType>::CreateComposite(CompositeHeader& compositeHeader)
{
	IComposite<ContextType>* pComposite = NULL;
	int32_t iNumChildren = 0;
	bool bOk = false;

	switch(compositeHeader.compositeType)
	{
	case CompositeType_Sequence:
	{
		SequenceHeader seqHeader;
		bOk = m_file.Read(&seqHeader);
		ASSERT(bOk);
		iNumChildren = seqHeader.numChildren;
		pComposite = m_pBt->template Alloc<Sequence<ContextType> >();
		break;
	}
	case CompositeType_Selector:
	{
		SelectorHeader selHeader;
		bOk = m_file.Read(&selHeader);
		ASSERT(bOk);
		iNumChildren = selHeader.numChildren;
		pComposite = this->m_pBt->template Alloc<Selector<ContextType> >();
		break;
	}
	case CompositeType_Parallel:
	{
		ParallelHeader parHeader;
		bOk = m_file.Read(&parHeader);
		ASSERT(bOk);
		iNumChildren = parHeader.numChildren;
		pComposite = this->m_pBt->template Alloc<Parallel<ContextType> >();
		static_cast<Parallel<ContextType>*>(pComposite)->SetNumSuccess(parHeader.numSuccess);
		break;
	}
	case CompositeType_RandomSelector:
	{
		RandomSelectorHeader selHeader;
		bOk = m_file.Read(&selHeader);
		ASSERT(bOk);
		iNumChildren = selHeader.numChildren;
		pComposite = this->m_pBt->template Alloc<RandomSelector<ContextType> >();
		break;
	}
	case CompositeType_Decorator:
	{
		DecoratorHeader decoratorHeader;
		bOk = m_file.Read(&decoratorHeader);
		ASSERT(bOk);
		IDecorator<ContextType>* pDecorator = this->CreateDecorator(decoratorHeader);
		ASSERT(pDecorator != NULL);
		CompositeHeader header;
		bOk = m_file.Read(&header);
		ASSERT(bOk);
		IBehavior<ContextType>* pBehavior = this->CreateComposite(header);
		ASSERT(pBehavior != NULL);
		pDecorator->SetChild(pBehavior);
		return pDecorator;
	}
	case CompositeType_Behavior:
	{
		BehaviorHeader bhHeader;
		bOk = m_file.Read(&bhHeader);
		ASSERT(bOk);
		return this->CreateBehavior(bhHeader);
	}
	default: ASSERT(false); break;
	}

	ASSERT(pComposite != NULL);
	uint16* pChunk = this->m_pBt->template AllocChunk<uint16>(iNumChildren);
	ASSERT(pChunk != NULL);
	pComposite->InitChunk(pChunk, iNumChildren);

	for(int32_t i = 0; i < iNumChildren; i++)
	{
		CompositeHeader header;
		bOk = m_file.Read(&header);
		ASSERT(bOk);
		IBehavior<ContextType>* pBehavior = this->CreateComposite(header);
		ASSERT(pBehavior != NULL);
		pComposite->AddChild(pBehavior);
	}

	return pComposite;
}

template <class ContextType>
IBehavior<ContextType>* BehaviorFactory<ContextType>::CreateBehavior(usg::ai::BehaviorHeader& behaviorHeader)
{
	bool bOk = false;
	switch(behaviorHeader.behaviorType)
	{
	case BehaviorType_IsHealthAbove:
	{
		IsHealthAbove moreThan;
		bOk = m_file.Read(&moreThan);
		ASSERT(bOk);
		bhIsHealthAbove<ContextType>* pB = this->m_pBt->template Alloc<bhIsHealthAbove<ContextType> >();
		pB->SetData(moreThan);
		return pB;
	}
	case BehaviorType_IsHealthBelow:
	{
		IsHealthBelow lessThan;
		bOk = m_file.Read(&lessThan);
		ASSERT(bOk);
		bhIsHealthBelow<ContextType>* pB = this->m_pBt->template Alloc<bhIsHealthBelow<ContextType> >();
		pB->SetData(lessThan);
		return pB;
	}
	case BehaviorType_Wait:
	{
		Wait wait;
		bOk = m_file.Read(&wait);
		ASSERT(bOk);
		bhWait<ContextType>* pB = this->m_pBt->template Alloc<bhWait<ContextType> >();
		pB->SetData(wait);
		return pB;
	}
	case BehaviorType_GetVariable:
	{
		GetVariable data;
		bOk = m_file.Read(&data);
		ASSERT(bOk);
		bhGetVariable<ContextType>* pB = this->m_pBt->template Alloc<bhGetVariable<ContextType> >();
		BehaviorTreeVariables* pToVars = m_pBt->GetVariables();
		pB->SetData(data, pToVars);
		return pB;
	}
	case BehaviorType_SetVariable:
	{
		SetVariable data;
		bOk = m_file.Read(&data);
		ASSERT(bOk);
		bhSetVariable<ContextType>* pB = this->m_pBt->template Alloc<bhSetVariable<ContextType> >();
		BehaviorTreeVariables* pToVars = m_pBt->GetVariables();
		pB->SetData(data, pToVars);
		return pB;
	}
	case BehaviorType_FuzzyWait:
	{
		FuzzyWait fuzzyWait;
		bOk = m_file.Read(&fuzzyWait);
		ASSERT(bOk);
		bhFuzzyWait<ContextType>* pB = this->m_pBt->template Alloc<bhFuzzyWait<ContextType> >();
		pB->SetData(fuzzyWait);
		return pB;
	}
	case BehaviorType_HasNoTarget:
		return this->m_pBt->template Alloc<bhHasNoTarget<ContextType> >();
	case BehaviorType_HasTarget:
		return this->m_pBt->template Alloc<bhHasTarget<ContextType> >();
	case BehaviorType_IsTargetAlive:
		return this->m_pBt->template Alloc<bhIsTargetAlive<ContextType> >();
	case BehaviorType_IsTargetNotAlive:
		return this->m_pBt->template Alloc<bhIsTargetNotAlive<ContextType> >();
	case BehaviorType_DestinationMinDistance:
	{
		DestinationMinDistance minDist;
		bOk = m_file.Read(&minDist);
		ASSERT(bOk);
		bhDestinationMinDistance<ContextType>* pB = this->m_pBt->template Alloc<bhDestinationMinDistance<ContextType> >();
		pB->SetData(minDist);
		return pB;
	}
	case BehaviorType_DestinationMaxDistance:
	{
		DestinationMaxDistance maxDist;
		bOk = m_file.Read(&maxDist);
		ASSERT(bOk);
		bhDestinationMaxDistance<ContextType>* pB = this->m_pBt->template Alloc<bhDestinationMaxDistance<ContextType> >();
		pB->SetData(maxDist);
		return pB;
	}
	case BehaviorType_IsTargetWithinDistance:
	{
		IsTargetWithinDistance isTargetWithinDistance;
		bOk = m_file.Read(&isTargetWithinDistance);
		ASSERT(bOk);
		bhIsTargetWithinDistance<ContextType>* pB = this->m_pBt->template Alloc<bhIsTargetWithinDistance<ContextType> >();
		pB->SetData(isTargetWithinDistance);
		return pB;
	}
	case BehaviorType_IsAnimationConditionTrue:
	{
		IsAnimationConditionTrue isAnimCondTrue;
		bOk = m_file.Read(&isAnimCondTrue);
		ASSERT(bOk);
		bhIsAnimationConditionTrue<ContextType>* pB = this->m_pBt->template Alloc<bhIsAnimationConditionTrue<ContextType> >();
		pB->SetData(isAnimCondTrue);
		return pB;
	}
	case BehaviorType_TargetMinDistance:
	{
		TargetMinDistance targetMinDistance;
		bOk = m_file.Read(&targetMinDistance);
		ASSERT(bOk);
		bhTargetMinDistance<ContextType>* pB = this->m_pBt->template Alloc<bhTargetMinDistance<ContextType> >();
		pB->SetData(targetMinDistance);
		return pB;
	}
	case BehaviorType_SetDestinationToWaypoint:
	{
		SetDestinationToWaypoint data;
		bOk = m_file.Read(&data);
		ASSERT(bOk);
		bhSetDestinationToWaypoint<ContextType>* pB = this->m_pBt->template Alloc<bhSetDestinationToWaypoint<ContextType> >();
		pB->SetData(data);
		return pB;
	}
	case BehaviorType_TargetMaxDistance:
	{
		TargetMaxDistance targetMaxDistance;
		bOk = m_file.Read(&targetMaxDistance);
		ASSERT(bOk);
		bhTargetMaxDistance<ContextType>* pB = this->m_pBt->template Alloc<bhTargetMaxDistance<ContextType> >();
		pB->SetData(targetMaxDistance);
		return pB;
	}
	case BehaviorType_CanSeeTarget:
		return this->m_pBt->template Alloc<bhCanSeeTarget<ContextType> >();
	case BehaviorType_CanNotSeeTarget:
		return this->m_pBt->template Alloc<bhCanNotSeeTarget<ContextType> >();
	case BehaviorType_RemoveTarget:
		return this->m_pBt->template Alloc<bhRemoveTarget<ContextType> >();
	case BehaviorType_IsTargetSameTeam:
		return this->m_pBt->template Alloc<bhIsTargetSameTeam<ContextType> >();
	case BehaviorType_IsTargetNotSameTeam:
		return this->m_pBt->template Alloc<bhIsTargetNotSameTeam<ContextType> >();
	default:
		return this->CreateBehaviorContextSpecific(m_file, m_pBt, behaviorHeader);
	}
}

template <class ContextType>
IDecorator<ContextType>* BehaviorFactory<ContextType>::CreateDecorator(DecoratorHeader& decoratorHeader, EventManager* pEventManager)
{
	bool bOk = false;
	switch(decoratorHeader.decoratorHeader)
	{
	case DecoratorType_Control:
		return m_pBt->template Alloc<dcControl<ContextType> >();
	case DecoratorType_HasTarget:
		return m_pBt->template Alloc<dcHasTarget<ContextType> >();
	case DecoratorType_HasNoTarget:
		return m_pBt->template Alloc<dcHasNoTarget<ContextType> >();
	case DecoratorType_IsTargetAlive:
		return m_pBt->template Alloc<dcIsTargetAlive<ContextType> >();
	case DecoratorType_IsTargetNotAlive:
		return m_pBt->template Alloc<dcIsTargetNotAlive<ContextType> >();
	case DecoratorType_TargetMaxDistance:
	{
		TargetMaxDistance targetMaxDistance;
		bOk = m_file.Read(&targetMaxDistance);
		ASSERT(bOk);
		dcTargetMaxDistance<ContextType>* pD = m_pBt->template Alloc<dcTargetMaxDistance<ContextType> >();
		pD->SetData(targetMaxDistance);
		return pD;
	}
	case DecoratorType_TargetMinDistance:
	{
		TargetMinDistance data;
		bOk = m_file.Read(&data);
		ASSERT(bOk);
		dcTargetMinDistance<ContextType>* pD = m_pBt->template Alloc<dcTargetMinDistance<ContextType> >();
		pD->SetData(data);
		return pD;
	}
	case DecoratorType_GetVariable:
	{
		GetVariable data;
		bOk = m_file.Read(&data);
		ASSERT(bOk);
		BehaviorTreeVariables* pToVars = m_pBt->GetVariables();
		dcGetVariable<ContextType>* pD = m_pBt->template Alloc<dcGetVariable<ContextType> >();
		pD->SetData(data, pToVars);
		return pD;
	}
	case DecoratorType_SetAnimationCondition:
	{
		SetAnimationCondition setAnimCond;
		bOk = m_file.Read(&setAnimCond);
		ASSERT(bOk);
		bhSetAnimationCondition<ContextType>* pB = this->m_pBt->template Alloc<bhSetAnimationCondition<ContextType> >();
		pB->SetData(setAnimCond, pEventManager);
		return pB;
	}
	case DecoratorType_IsHealthAbove:
	{
		IsHealthAbove above;
		bOk = m_file.Read(&above);
		ASSERT(bOk);
		dcIsHealthAbove<ContextType>* pD = m_pBt->template Alloc<dcIsHealthAbove<ContextType> >();
		pD->SetData(above);
		return pD;
	}
	case DecoratorType_IsHealthBelow:
	{
		IsHealthBelow below;
		bOk = m_file.Read(&below);
		ASSERT(bOk);
		dcIsHealthBelow<ContextType>* pD = m_pBt->template Alloc<dcIsHealthBelow<ContextType> >();
		pD->SetData(below);
		return pD;
	}
	case DecoratorType_ExecuteUntilTime:
	{
		ExecuteUntilTime data;
		bOk = m_file.Read(&data);
		ASSERT(bOk);
		dcExecuteUntilTime<ContextType>* pD = m_pBt->template Alloc<dcExecuteUntilTime<ContextType> >();
		pD->SetData(data);
		return pD;
	}
	case DecoratorType_FuzzyExecuteUntilTime:
	{
		FuzzyExecuteUntilTime data;
		bOk = m_file.Read(&data);
		ASSERT(bOk);
		dcFuzzyExecuteUntilTime<ContextType>* pD = m_pBt->template Alloc<dcFuzzyExecuteUntilTime<ContextType> >();
		pD->SetData(data);
		return pD;
	}
	case DecoratorType_DestinationMinDistance:
	{
		DestinationMinDistance data;
		bOk = m_file.Read(&data);
		ASSERT(bOk);
		dcDestinationMinDistance<ContextType>* pD = m_pBt->template Alloc<dcDestinationMinDistance<ContextType> >();
		pD->SetData(data);
		return pD;
	}
	case DecoratorType_DestinationMaxDistance:
	{
		DestinationMaxDistance data;
		bOk = m_file.Read(&data);
		ASSERT(bOk);
		dcDestinationMaxDistance<ContextType>* pD = m_pBt->template Alloc<dcDestinationMaxDistance<ContextType> >();
		pD->SetData(data);
		return pD;
	}
	case DecoratorType_CanSeeTarget:
		return m_pBt->template Alloc<dcCanSeeTarget<ContextType> >();
	case DecoratorType_CanNotSeeTarget:
		return m_pBt->template Alloc<dcCanNotSeeTarget<ContextType> >();
	case DecoratorType_IsMoving:
		return m_pBt->template Alloc<dcIsMoving<ContextType> >();
	case DecoratorType_IsNotMoving:
		return m_pBt->template Alloc<dcIsNotMoving<ContextType> >();
	case DecoratorType_IsTargetSameTeam:
		return m_pBt->template Alloc<dcIsTargetSameTeam<ContextType> >();
	case DecoratorType_IsTargetNotSameTeam:
		return m_pBt->template Alloc<dcIsTargetNotSameTeam<ContextType> >();
	default:
		return this->CreateDecoratorContextSpecific(m_file, m_pBt, decoratorHeader);
	}
}

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIOR_FACTORY__
