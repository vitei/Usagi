/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: Waits for a set amount of time.
 *****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_WAIT_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_WAIT_BEHAVIOR__

#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class bhWait : public IBehavior<ContextType>
{
public:
	bhWait()
	{
		m_fCurrentTime = 0.0f;
	}

	virtual int Update(float fElapsed, ContextType& ctx)
	{
		m_fCurrentTime += fElapsed;
		if(m_fCurrentTime < m_data.targetTime)
		{
			return BH_RUNNING;
		}

		m_fCurrentTime = 0.0f;
		return BH_SUCCESS;
	}

	void SetData(const usg::ai::Wait& data)
	{
		m_data.targetTime = data.targetTime;
	}

private:
	usg::ai::Wait m_data;
	float m_fCurrentTime;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_WAIT_BEHAVIOR__