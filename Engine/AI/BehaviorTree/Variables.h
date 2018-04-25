/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIOR_VARS_TREE__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIOR_VARS_TREE__
#include "Engine/Common/Common.h"

namespace usg
{

	namespace ai
	{
		class BehaviorTreeVariables
		{
			static const uint32 MaxVariables = 16;
			struct Entry {
				uint32 uNameHash;
				bool bValue;
			};
			Entry m_variables[MaxVariables];

			void Clear();
		public:
			BehaviorTreeVariables();
			bool GetVariable(uint32 uNameHash, bool bDefault = false) const;
			void SetVariable(uint32 uNameHash, bool bValue);
		};
	}
}

#endif
