/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The Selector only runs the first child that returns SUCCESS.
*****************************************************************************/

#ifndef __USG_AI_SELECTOR__
#define __USG_AI_SELECTOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Composite.h"

namespace usg
{

namespace ai
{

template <class ContextType>
class Selector : public IComposite<ContextType>
{
public:
	Selector():
	m_uCurrent(0){}
	virtual ~Selector(){}

	virtual const char* Name() const { return "Selector"; }
protected:

	virtual void Init(ContextType& ctx)
	{
		m_uCurrent = 0;
	}

	virtual int Update(float fElapsed, ContextType& ctx)
	{
		ASSERT(this->GetNumChildren() != 0);

		while(1)
		{
			int iStatus = this->GetChild(m_uCurrent).Tick(fElapsed, ctx);

			if(iStatus != BH_FAILURE)
			{
				return iStatus;
			}

			if(++m_uCurrent == this->GetNumChildren())
			{
				return BH_FAILURE;
			}
		}
	}

	virtual void Shut(int iStatus)
	{
		m_uCurrent = 0;
		IComposite<ContextType>::Shut(iStatus);
	}

	uint16 m_uCurrent;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_SELECTOR__
