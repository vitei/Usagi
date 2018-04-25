//	TargetListUtil.cpp
#include "Engine/Common/Common.h"
#include "Engine/Core/Utility.h"
#include "TargetListUtil.h"
#include <algorithm>
#include <float.h>

static const uint32 MaxTicks = 25;

namespace usg
{
	namespace ai
	{

		uint32 TargetListUtil::FillArray(const RuntimeData<ai::Components::TargetListComponent>& targetList, const Target** ppTargets, uint32 uSize, const TargetParams& params)
		{
			uint32 uCount = 0;
	
			auto& targets = targetList.targets;
			for(uint32 i = 0; i < (uint32)targets.size() && uCount < uSize; ++i)
			{
				const usg::ai::Target& target = targets[i].target;
				const bool bIsCorrectType = TargetUtil::IsType(params.type, target.type);
				const bool bIsCorrectTeam = TargetUtil::IsTeam(params.teamType, target.team, params.team);
				if(target.visible && bIsCorrectType	&& bIsCorrectTeam && (!params.requireDetected || target.uLineOfSight>0))
				{
					const float fDistanceSq = target.position.GetSquaredXZDistanceFrom(params.position);
					if(params.ignoreDistance || (fDistanceSq < params.minDistance))
					{
						if(params.ignoreAngle)
						{
							ppTargets[uCount++] = &target;
						}
						else
						{
							const usg::Vector3f vDirToTarget = (target.position - params.position).GetNormalised();
							const float fDot = DotProduct(vDirToTarget, params.forward);
							const float fAngle = acosf(fDot) * (180.0f / usg::Math::pi);
							if(fAngle < params.minAngle)
							{
								ppTargets[uCount++] = &target;
							}
						}
					}
				}
			}

			return uCount;
		}

		bool TargetListUtil::UpdateTarget(RuntimeData<ai::Components::TargetListComponent>& targetList, usg::ai::TargetData& targetData, usg::ai::Target& target, const usg::Vector3f& vPos)
		{
			if(targetData.target.entity == NULL)
			{
				return false;
			}
	
			uint32 uCopyIndex = (uint32)(-1);

			auto& targets = targetList.targets;

			if(targetData.index < targets.size() && targets[targetData.index].target.entityRef.entity == targetData.target.entity)
			{
				uCopyIndex = targetData.index;
			}
			else
			{
				for(uint32 i = 0; i < targets.size(); ++i)
				{
					if(targets[i].target.entityRef.entity == targetData.target.entity)
					{
						uCopyIndex = i;
						targetData.index = i;
						break;
					}
				}
			}
			if (uCopyIndex != ((uint32)(-1)))
			{
				const uint32 uOldLineOfSight = target.uLineOfSight;
				const bool bTargetWillChange = target.entityRef != targets[uCopyIndex].target.entityRef;
				TargetUtil::TargetCopy(targets[uCopyIndex].target, target);
				if (!bTargetWillChange)
				{
					target.uLineOfSight = uOldLineOfSight;
				}
				TargetUtil::CalculateDirectionToTarget(target, vPos);
				return true;
			}
			return false;
		}

		bool TargetListUtil::GetNearestTarget(const RuntimeData<Components::TargetListComponent>& targetList, const usg::ai::TargetParams& in, usg::ai::TargetData& out)
		{
			const usg::ai::Target* pTargets[usg::ai::Components::AIMaxTargets_MaxTargets];
			const uint32 uCount = TargetListUtil::FillArray(targetList, &pTargets[0], usg::ai::Components::AIMaxTargets_MaxTargets, in);
	
			if(uCount > 0)
			{
				const usg::Vector3f& vPos = in.position;
				float fDistanceSq = FLT_MAX;
				uint32 uTargetIdx = 0;
				for(uint32 i = 0; i < uCount; ++i)
				{
					const usg::ai::Target* pTarget = pTargets[i];
					const float fDSq = vPos.GetSquaredXZDistanceFrom(pTarget->position);
					if(fDSq < fDistanceSq)
					{
						fDistanceSq = fDSq;
						uTargetIdx = i;
					}
				}

				TargetListUtil::SetTargetData(out, pTargets[uTargetIdx]->entityRef.entity);

				return true;
			}

			return false;
		}

		bool TargetListUtil::GetFurthestTarget(const RuntimeData<Components::TargetListComponent>& targetList, const usg::ai::TargetParams& in, usg::ai::TargetData& out)
		{
			const usg::ai::Target* pTargets[usg::ai::Components::AIMaxTargets_MaxTargets];
			const uint32 uCount = TargetListUtil::FillArray(targetList, &pTargets[0], usg::ai::Components::AIMaxTargets_MaxTargets, in);

			if(uCount > 0)
			{
				const usg::Vector3f& vPos = in.position;
				float fDistanceSq = 0;
				uint32 uTargetIdx = 0;
				for(uint32 i = 0; i < uCount; ++i)
				{
					const usg::ai::Target* pTarget = pTargets[i];
					const float fDSq = vPos.GetSquaredXZDistanceFrom(pTarget->position);
					if(fDSq > fDistanceSq)
					{
						fDistanceSq = fDSq;
						uTargetIdx = i;
					}
				}

				TargetListUtil::SetTargetData(out, pTargets[uTargetIdx]->entityRef.entity);

				return true;
			}

			return false;
		}

		bool TargetListUtil::GetWeakestTarget(const RuntimeData<Components::TargetListComponent>& targetList, const usg::ai::TargetParams& in, usg::ai::TargetData& out)
		{
			const usg::ai::Target* pTargets[usg::ai::Components::AIMaxTargets_MaxTargets];
			const uint32 uCount = TargetListUtil::FillArray(targetList, &pTargets[0], usg::ai::Components::AIMaxTargets_MaxTargets, in);

			if (uCount > 0)
			{
				float fHealth = 1.0f;
				uint32 uTargetIdx = 0;
				for (uint32 i = 0; i < uCount; ++i)
				{
					const usg::ai::Target* pTarget = pTargets[i];
					if (pTarget->health <= fHealth)
					{
						fHealth = pTarget->health;
						uTargetIdx = i;
					}
				}

				TargetListUtil::SetTargetData(out, pTargets[uTargetIdx]->entityRef.entity);

				return true;
			}

			return false;
		}

		bool TargetListUtil::GetStrongestTarget(const RuntimeData<Components::TargetListComponent>& targetList, const usg::ai::TargetParams& in, usg::ai::TargetData& out)
		{
			const usg::ai::Target* pTargets[usg::ai::Components::AIMaxTargets_MaxTargets];
			const uint32 uCount = TargetListUtil::FillArray(targetList, &pTargets[0], usg::ai::Components::AIMaxTargets_MaxTargets, in);

			if (uCount > 0)
			{
				float fHealth = 0.0f;
				uint32 uTargetIdx = 0;
				for (uint32 i = 0; i < uCount; ++i)
				{
					const usg::ai::Target* pTarget = pTargets[i];
					if (pTarget->health > fHealth)
					{
						fHealth = pTarget->health;
						uTargetIdx = i;
					}
				}

				TargetListUtil::SetTargetData(out, pTargets[uTargetIdx]->entityRef.entity);

				return true;
			}

			return false;
		}

		void TargetListUtil::ApplyToTarget(usg::ai::Target& target, const usg::ai::Events::TargetEvent& targetEvt)
		{
			target.entityRef.entity = targetEvt.selfID.entity;
			target.position = targetEvt.position;
			target.health = targetEvt.health;
			target.team = targetEvt.team;
			target.type = targetEvt.type;
			target.visible = targetEvt.bVisible;
			target.right = targetEvt.vRight;
			target.forward = targetEvt.vForward;
			target.vVelocity = targetEvt.vVelocity;
			target.iNUID = targetEvt.iNUID;
			target.fTimeStamp = targetEvt.fTimeStamp;
		}

		uint32 TargetListUtil::GetTargetIndexByEntifyRef(const RuntimeData<Components::TargetListComponent>& targetList, const usg::Entity pEntity)
		{
			for (memsize i = 0; i < targetList.targets.size(); i++)
			{
				if (targetList.targets[i].target.entityRef.entity == pEntity)
				{
					return (uint32)i;
				}
			}
			return (uint32)(-1);
		}

		void TargetListUtil::UpdateTargetList(RuntimeData<Components::TargetListComponent>& targetList, const usg::ai::Events::TargetEvent& targetEvt)
		{
			uint32 i, uC = (uint32)targetList.targets.size();
			for (i = 0; i < uC; ++i)
			{
				if (targetList.targets[i].target.entityRef.entity == targetEvt.selfID.entity && targetList.targets[i].target.type == targetEvt.type)
				{
					usg::ai::Target& target = targetList.targets[i].target;
					ApplyToTarget(target, targetEvt);
					TargetUtil::TargetCopy(target, targetList.targets[i].target);
					targetList.targets[i].ticks = 0;
					return;
				}

			}

			{
				targetList.targets.push_back();
				usg::ai::Target target;
				Target_init(&target);
				ApplyToTarget(target, targetEvt);
				TargetUtil::TargetCopy(target, targetList.targets.back().target);
				targetList.targets[targetList.targets.size() - 1].ticks = 0;
			}
		}

		void TargetListUtil::Tick(RuntimeData<Components::TargetListComponent>& targetList)
		{
			targetList.targets.erase(std::remove_if(targetList.targets.begin(), targetList.targets.end(), [&](auto& t)
			{
				return t.ticks++ >= MaxTicks;
			}), targetList.targets.end());
		}

		void TargetListUtil::SetTargetData(usg::ai::TargetData& targetData, usg::Entity targetEntity)
		{
			if(targetData.target.entity != targetEntity)
			{
				targetData.target.entity = targetEntity;
				targetData.index = 0;
			}
		}

	}
}
