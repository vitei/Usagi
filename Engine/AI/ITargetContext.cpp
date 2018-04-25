#include "Engine/Common/Common.h"
#include "ITargetContext.h"

namespace usg
{
	namespace ai
	{

		void TargetContext::RemoveTarget()
		{
			m_target.active = false;
			m_target.targetData.target = NULL;
		}

		const Target* TargetContext::GetNearestTarget(const TargetParams& in) const
		{
			TargetData out;
			const bool bFoundTarget = TargetListUtil::GetNearestTarget(m_targetList, in, out);
			if (bFoundTarget)
			{
				for (uint32 i = 0; i < Components::AIMaxTargets_MaxTargets; ++i)
				{
					if (m_targetList.targets[i].target.entityRef.entity == out.target.entity)
					{
						return &m_targetList.targets[i].target;
					}
				}
			}
			return NULL;
		}

		bool ChooseTargetContext::SetTarget(const TargetParams& in)
		{
			bool bSuccess = false;
			switch (in.pickTarget)
			{
			case PickTarget_Nearest:
				bSuccess = TargetListUtil::GetNearestTarget(m_targetList, in, m_target.targetData);
				break;
			case PickTarget_Furthest:
				bSuccess = TargetListUtil::GetFurthestTarget(m_targetList, in, m_target.targetData);
				break;
			case PickTarget_Weakest:
				bSuccess = TargetListUtil::GetWeakestTarget(m_targetList, in, m_target.targetData);
				break;
			case PickTarget_Strongest:
				bSuccess = TargetListUtil::GetStrongestTarget(m_targetList, in, m_target.targetData);
				break;
			}
			if (bSuccess)
			{
				m_target.target.active = true;
			}
			return bSuccess;
		}

	}
}
