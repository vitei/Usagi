/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Waits for a set amount of time, resets and changes randomly 
	between two values.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_FUZZY_WAIT_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_FUZZY_WAIT_BEHAVIOR__

#include "Engine/AI/BehaviorTree/Behaviors/WaitBehavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class bhFuzzyWait : public IBehavior<ContextType>
{
public:
	bhFuzzyWait()
	{
		m_fTargetTime = 0.0f;
		m_fCurrentTime = 0.0f;
	}

	virtual void Init(ContextType& ctx)
	{
		Randomize();
	}


	virtual int Update(float fElapsed, ContextType& ctx)
	{
		m_fCurrentTime += fElapsed;
		if(m_fCurrentTime < m_fTargetTime)
		{
			return BH_RUNNING;
		}

		Randomize();
		m_fCurrentTime = 0.0f;
		return BH_SUCCESS;
	}

	void SetData(const usg::ai::FuzzyWait& data)
	{
		m_data.maxTime = data.maxTime;
		m_data.minTime = data.minTime;
	}

private:
	void Randomize()
	{
		m_fTargetTime = usg::Math::RangedRandom(m_data.minTime, m_data.maxTime);
	}

	usg::ai::FuzzyWait m_data;
	float m_fTargetTime;
	float m_fCurrentTime;
};

}	//	namespace ai

}	//	namespace usg


#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_FUZZY_WAIT_BEHAVIOR__