/****************************************************************************
//	Filename: IsBaseUnderAttackBehavior.h
//	Description: Returns success if a condition variable of a ModelAnimPlayer
//               connected to the AI entity is true.
*****************************************************************************/
#ifndef __AI_BEHAVIORS_ISANIMCONDITIONTRUE_BEHAVIOR__
#define __AI_BEHAVIORS_ISANIMCONDITIONTRUE_BEHAVIOR__
#include "Engine/Common/Common.h"
#include "Engine/Scene/Model/ModelAnimPlayer.h"
#include "Engine/AI/BehaviorTree/Behaviors/Behaviors.h"
#include "Engine/AI/BehaviorTree/BehaviorCommon.pb.h"
#include "Engine/Scene/Model/ModelComponents.pb.h"

namespace usg
{

	namespace ai
	{

		template <class ContextType>
		class bhIsAnimationConditionTrue : public IBehavior<ContextType>
		{
			static const uint32 NAME_MAX_LENGTH = 32;
			char m_szChildEntityName[NAME_MAX_LENGTH];
			uint32 m_uConditionNameHash;
			Entity m_pEntity;
		public:
			void SetData(const IsAnimationConditionTrue& cond)
			{
				ASSERT(str::StringLength(cond.childEntityName) < NAME_MAX_LENGTH);
				ASSERT(str::StringLength(cond.conditionName) < NAME_MAX_LENGTH);
				str::Copy(m_szChildEntityName, cond.childEntityName, NAME_MAX_LENGTH);
				m_uConditionNameHash = utl::CRC32(cond.conditionName);
				m_pEntity = NULL;
			}
		protected:
			virtual int Update(float fElapsed, ContextType& ctx)
			{
				if (m_pEntity == NULL)
				{
					m_pEntity = m_szChildEntityName[0] != 0 ? ctx.GetEntity()->GetChildEntityByName(m_szChildEntityName) : ctx.GetEntity();
				}
				ASSERT(m_pEntity != NULL);
				Optional<usg::ModelAnimComponent> anim;
				GetComponent(m_pEntity, anim);
				if (anim.Exists())
				{
					ModelAnimPlayer* pPlayer = anim.Force().GetRuntimeData().pAnimPlayer;
					if (pPlayer->GetCondition(m_uConditionNameHash))
					{
						return usg::ai::BH_SUCCESS;
					}
				}
				return usg::ai::BH_RUNNING;
			}

		};

	}

}

#endif	

