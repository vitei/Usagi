/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Selects a random child.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_COMPOSITES_RANDOM_SELECTOR__
#define __USG_AI_BEHAVIOR_TREE_COMPOSITES_RANDOM_SELECTOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Composite.h"
#include "Engine/Core/Utility.h"
namespace usg
{

namespace ai
{

template <class ContextType>
class RandomSelector : public IComposite<ContextType>
{
public:
	RandomSelector():
	m_uCurrent(0){}
	virtual ~RandomSelector(){}

	virtual const char* Name() const { return "RandomSelector"; }
protected:

	virtual void Init(ContextType& ctx)
	{
		this->Shuffle();
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
		this->Shuffle();
		m_uCurrent = 0;
		IComposite<ContextType>::Shut(iStatus);
	}

private:
	void Shuffle()
	{
		utl::RandomShuffle<uint16>(this->GetPointerToChildren(), static_cast<sint32>(this->m_uNumChildren));
	}

	uint16 m_uCurrent;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_COMPOSITES_RANDOM_SELECTOR__
