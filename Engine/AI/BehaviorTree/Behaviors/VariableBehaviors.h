/****************************************************************************
 //	Usagi Engine, Copyright © Vitei, Inc. 2013
 //	Description: Behaviors related to getting/setting boolean variables.
 *****************************************************************************/

#ifndef __USG_AI_BEHAVIOR_TREE_VARIABLE_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_VARIABLE_BEHAVIOR__

#include "Engine/AI/BehaviorTree/Behavior.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
#include "Engine/AI/BehaviorTree/Variables.h"

namespace usg
{

	namespace ai
	{

		template <class ContextType>
		class bhGetVariable : public IBehavior<ContextType>
		{
			uint32 m_uVariableNameHash;
			GetVariable m_data;
			const BehaviorTreeVariables* m_pVariables;
		public:
			bhGetVariable() :
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
				if (m_pVariables->GetVariable(m_uVariableNameHash, m_data.bDefaultValue) == m_data.bRequiredValue)
				{
					return BH_SUCCESS;
				}
				return BH_RUNNING;
			}
		};

		template <class ContextType>
		class bhSetVariable : public IBehavior<ContextType>
		{
			uint32 m_uVariableNameHash;
			SetVariable m_data;
			BehaviorTreeVariables* m_pVariables;
		public:
			void SetData(const SetVariable& data, BehaviorTreeVariables* pVariables)
			{
				m_data = data;
				m_pVariables = pVariables;
				m_uVariableNameHash = utl::CRC32(data.szVariableName);
			}

			virtual int Update(float fElapsed, ContextType& ctx)
			{
				m_pVariables->SetVariable(m_uVariableNameHash, m_data.bValue);
				return BH_SUCCESS;
			}
		};

	}

}

#endif
