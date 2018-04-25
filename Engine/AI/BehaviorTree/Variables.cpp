#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Variables.h"

namespace usg
{

	namespace ai
	{
		BehaviorTreeVariables::BehaviorTreeVariables()
		{
			Clear();
		}

		bool BehaviorTreeVariables::GetVariable(uint32 uNameHash, bool bDefault) const
		{
			for (uint32 i = 0; i < MaxVariables; i++)
			{
				if (m_variables[i].uNameHash == uNameHash)
				{
					return m_variables[i].bValue;
				}
			}
			return bDefault;
		}

		void BehaviorTreeVariables::SetVariable(uint32 uNameHash, bool bValue)
		{
			uint32 uZeroIndex = MaxVariables;
			for (uint32 i = 0; i < MaxVariables; i++)
			{
				if (m_variables[i].uNameHash == uNameHash)
				{
					m_variables[i].bValue = bValue;
					return;
				}
				else if (m_variables[i].uNameHash == 0 && uZeroIndex == MaxVariables)
				{
					uZeroIndex = i;
				}
			}
			ASSERT(uZeroIndex != MaxVariables);
			ASSERT(m_variables[uZeroIndex].uNameHash == 0);
			if (uZeroIndex != MaxVariables)
			{
				m_variables[uZeroIndex].bValue = bValue;
				m_variables[uZeroIndex].uNameHash = uNameHash;
			}
		}

		void BehaviorTreeVariables::Clear()
		{
			for (uint32 i = 0; i < MaxVariables; i++)
			{
				m_variables[i].uNameHash = 0;
				m_variables[i].bValue = false;
			}
		}
	}
}
