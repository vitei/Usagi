/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A Decorator is used to modify a Behavior dynamically
*****************************************************************************/

#ifndef __USG_AI_DECORATOR__
#define __USG_AI_DECORATOR__

#include "Behavior.h"

namespace usg
{

namespace ai
{

template <class ContextType>
class IDecorator : public IBehavior<ContextType>
{
public:

	void SetChild(IBehavior<ContextType>* pChild)
	{
		ptrdiff_t p = ((uintptr_t)pChild - (uintptr_t)this);
		m_uChild = static_cast<uint16>(p);
	}

	IBehavior<ContextType>& GetChild()
	{
		return *(IBehavior<ContextType>*)((uintptr_t)this + m_uChild);
	}

	virtual ~IDecorator()
	{
		this->GetChild().~IBehavior();
	}
protected:
	virtual void Shut(int iStatus)
	{
		this->GetChild().Shut(iStatus);
	}
private:
	uint16 m_uChild;
};

}	//	namespace ai

}	//	namespace usg



#endif	//	__USG_AI_DECORATOR__