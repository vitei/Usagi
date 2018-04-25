/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Executes a behavior until the set time has been reached. The
//	target time is selected randomly based on input.
//	This decorator can be used, ideally, within a selector were you want something 
//	to run for a set amount of time and then continue to the next etc.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_DECORATORS_FUZZY_EXECUTE_UNTIL_TIME_DECORATOR__
#define __USG_AI_BEHAVIOR_TREE_DECORATORS_FUZZY_EXECUTE_UNTIL_TIME_DECORATOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Decorator.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
#include "Engine/Maths/MathUtil.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class dcFuzzyExecuteUntilTime : public IDecorator<ContextType>
{
public:
	dcFuzzyExecuteUntilTime()
	{
		m_fCurrentTime = 0.0f;
		m_fTargetTime = 0.0f;
	}
	void SetData(const usg::ai::FuzzyExecuteUntilTime& data)
	{
		m_data.min = data.min;
		m_data.max = data.max;
	}

protected:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		m_fCurrentTime += fElapsed;

		if(m_fCurrentTime < m_fTargetTime)
		{
			return this->GetChild().Tick(fElapsed, ctx);
		}

		m_fCurrentTime = 0.0f;
		Randomize();
		return BH_FAILURE;
	}

	virtual void Init(ContextType& ctx)
	{
		Randomize();
	}

	void Randomize()
	{
		m_fTargetTime = usg::Math::RangedRandom(m_data.min, m_data.max);
	}

	usg::ai::FuzzyExecuteUntilTime m_data;
	float m_fCurrentTime;
	float m_fTargetTime;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_DECORATORS_FUZZY_EXECUTE_UNTIL_TIME_DECORATOR__
