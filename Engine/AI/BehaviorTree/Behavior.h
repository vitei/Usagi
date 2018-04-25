/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Base class for everything Behavior, Composite or Decorator
*****************************************************************************/

#ifndef __USG_AI_BEHAVIOR__
#define __USG_AI_BEHAVIOR__

#include "Engine/Common/Common.h"

namespace usg
{

namespace ai
{

enum
{
	BH_INVALID = 0,
	BH_SUCCESS,
	BH_FAILURE,
	BH_RUNNING
};

template < typename T > class BehaviorTree;

template <class ContextType>
class IBehavior
{
public:
	IBehavior() : m_iStatus(BH_INVALID) {}
	virtual ~IBehavior(){}

	int Tick(float fElapsed, ContextType& ctx)
	{
		if(m_iStatus == BH_INVALID)
		{
			Init(ctx);
		}

		m_iStatus = Update(fElapsed, ctx);

		if(m_iStatus != BH_RUNNING)
		{
			Shut(m_iStatus);
		}

		return m_iStatus;
	}

	virtual const char* Name() const { return "BH_Empty"; }
	virtual void Shut(int iStatus) {}
protected:
	virtual int Update(float fElapsed, ContextType& ctx) = 0;
	virtual void Init(ContextType& ctx) {}

	int m_iStatus;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR__