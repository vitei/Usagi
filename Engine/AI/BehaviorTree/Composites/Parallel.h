/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Parallel executes multiple children at once.
*****************************************************************************/

#ifndef __USG_AI_PARALLEL__
#define __USG_AI_PARALLEL__

#include "Engine/AI/BehaviorTree/Composite.h"

namespace usg
{

namespace ai
{

template <class ContextType>
class Parallel : public IComposite<ContextType>
{
	enum { MAX_RESULTS = 16 };
public:
	Parallel():
	m_uMinSuccess(0){}
	virtual ~Parallel(){}

	virtual const char* Name() const { return "Parallel"; }

	void SetNumSuccess(uint16 uNumSuccess)
	{
		this->m_uMinSuccess = uNumSuccess;
	}

protected:

	virtual int Update(float fElapsed, ContextType& ctx)
	{
		ASSERT(this->GetNumChildren() != 0);

		uint16 uCurrent = 0;
		ASSERT(this->m_uSize < MAX_RESULTS);
		uint16 uResults[MAX_RESULTS];

		while(1)
		{
			uResults[uCurrent] = (uint16)this->GetChild(uCurrent).Tick(fElapsed, ctx);

			if(++uCurrent == this->GetNumChildren())
			{
				uint32 uNumFailures = 0;
				uint32 uNumSuccess = 0;
				for(uint32 i = 0; i < this->GetNumChildren(); i++)
				{
					if(uResults[i] == BH_FAILURE)
					{
						uNumFailures++;
					}
					else if(uResults[i] == BH_SUCCESS)
					{
						uNumSuccess++;
					}
				}

				int iResult;

				if(m_uMinSuccess != 0)
				{
					if(uNumSuccess == this->GetNumChildren() || uNumSuccess >= m_uMinSuccess)
					{
						iResult = BH_SUCCESS;
					}
					else if(uNumFailures == this->GetNumChildren())
					{
						iResult = BH_FAILURE;
					}
					else
					{
						iResult = BH_RUNNING;
					}
				}
				else
				{
					if(uNumSuccess == this->GetNumChildren())
					{
						iResult = BH_SUCCESS;
					}
					else if(uNumFailures == this->GetNumChildren())
					{
						iResult = BH_FAILURE;
					}
					else
					{
						iResult = BH_RUNNING;
					}
				}

				return iResult;
			}
		}
	}

	virtual void Shut(int iStatus)
	{
		IComposite<ContextType>::Shut(iStatus);
	}

	uint16 m_uMinSuccess;
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_PARALLEL__
