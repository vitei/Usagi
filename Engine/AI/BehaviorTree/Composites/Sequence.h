/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A Sequence runs through all of its children until one of 
	them failes or returns RUNNING.
*****************************************************************************/

#ifndef __USG_AI_SEQUENCE__
#define __USG_AI_SEQUENCE__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Composite.h"

namespace usg
{

namespace ai
{

template <class ContextType>
class Sequence : public IComposite<ContextType>
{
public:
	Sequence():m_uCurrent(0){}
	virtual ~Sequence(){}

protected:
	virtual void Init(ContextType& ctx)
	{
		this->m_uCurrent = 0;
	}

	virtual int Update(float fElapsed, ContextType& ctx)
	{
		ASSERT(this->GetNumChildren() != 0);
		const uint32 uNumChildren = this->GetNumChildren();
		while(1)
		{
			IBehavior<ContextType>& child = this->GetChild(m_uCurrent);
			int iStatus = child.Tick(fElapsed, ctx);

			if(iStatus != BH_SUCCESS)
			{
				return iStatus;
			}

			if(++m_uCurrent == uNumChildren)
			{
				return BH_SUCCESS;
			}
		}
	}

	virtual void Shut(int iStatus)
	{
		this->m_uCurrent = 0;
		IComposite<ContextType>::Shut(iStatus);
	}

	uint16 m_uCurrent;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_SEQUENCE__
