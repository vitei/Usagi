/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/

#ifndef __USG_AI_VARIABLE_DECORATOR__
#define __USG_AI_VARIABLE_DECORATOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Decorator.h"

namespace usg
{
	namespace ai
	{
		template <class ContextType>
		class dcGetVariable: public IDecorator<ContextType>
		{
			uint32 m_uVariableNameHash;
			GetVariable m_data;
			const BehaviorTreeVariables* m_pVariables;
		public:
			dcGetVariable() : 
				m_uVariableNameHash(0), m_pVariables(NULL)
			{

			}

			void SetData(const GetVariable& data, const BehaviorTreeVariables* pVariables)
			{
				m_data = data;
				m_pVariables = pVariables;
				m_uVariableNameHash = utl::CRC32(data.szVariableName);
			}

			virtual int Update(float fElapsed, ContextType& ctx)
			{
				ASSERT(m_pVariables != NULL);
				if (m_pVariables->GetVariable(m_uVariableNameHash, m_data.bDefaultValue) == m_data.bRequiredValue)
				{
					return this->GetChild().Tick(fElapsed, ctx);
				}
				return BH_FAILURE;
			}
		};
	}
}

#endif	//	__USG_AI_CONTROL_DECORATOR__