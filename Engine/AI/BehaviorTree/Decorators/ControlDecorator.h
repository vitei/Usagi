/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: This is used to reverse the return value of a Behavior, eg:
//	SUCCESS becomes FAILURE but ignores RUNNING
*****************************************************************************/

#ifndef __USG_AI_CONTROL_DECORATOR__
#define __USG_AI_CONTROL_DECORATOR__

#include "Engine/AI/BehaviorTree/Decorator.h"

namespace usg
{

namespace ai
{

template <class ContextType>
class dcControl : public IDecorator<ContextType>
{
protected:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		int iStatus = this->GetChild().Tick(fElapsed, ctx);

		if(iStatus == BH_FAILURE)
		{
			return BH_SUCCESS;
		}
		else if(iStatus == BH_SUCCESS)
		{
			return BH_FAILURE;
		}

		return iStatus;
	}
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_CONTROL_DECORATOR__