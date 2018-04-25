/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Makes a behavior return FAILURE/SUCCESS if a target is preset, or not.
*****************************************************************************/

#ifndef __USG_AI_HAS_TARGET_DECORATOR__
#define __USG_AI_HAS_TARGET_DECORATOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Decorator.h"

namespace usg
{

namespace ai
{

template <class ContextType>
class dcHasTarget : public IDecorator<ContextType>
{
protected:
	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(ctx.HasTarget())
		{
			return this->GetChild().Tick(fElapsed, ctx);
		}

		return BH_FAILURE;
	}
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_HAS_TARGET_DECORATOR__