/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Executes a behavior until the set time has been reached.
//	This decorator can be used, ideally, within a selector were you want something 
//	to run for a set amount of time and then continue to the next etc.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_DECORATORS_EXECUTE_UNTIL_TIME_DECORATOR__
#define __USG_AI_BEHAVIOR_TREE_DECORATORS_EXECUTE_UNTIL_TIME_DECORATOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Decorator.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class dcExecuteUntilTime : public IDecorator<ContextType>
{
public:
	dcExecuteUntilTime()
	{
		m_fCurrentTime = 0.0f;
	}
	void SetData(const usg::ai::ExecuteUntilTime& data)
	{
		m_data.targetTime = data.targetTime;
	}

protected:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		m_fCurrentTime += fElapsed;

		if(m_fCurrentTime < m_data.targetTime)
		{
			return this->GetChild().Tick(fElapsed, ctx);
		}

		m_fCurrentTime = 0.0f;	//	Reset and return failure
		return BH_FAILURE;
	}

	usg::ai::ExecuteUntilTime m_data;
	float m_fCurrentTime;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_DECORATORS_EXECUTE_UNTIL_TIME_DECORATOR__
