/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Finds the closest obstacle object and adds avoidance to it it 
//	close enough.
*****************************************************************************/
#ifndef __USG_AI_BEHAVIOR_TREE_BEHAVIORS_OBJECT_AVOIDANCE_BEHAVIOR__
#define __USG_AI_BEHAVIOR_TREE_BEHAVIORS_OBJECT_AVOIDANCE_BEHAVIOR__
#include "Engine/Common/Common.h"
#include "Engine/AI/BehaviorTree/Behaviors/AvoidanceBehavior.h"
#include "Engine/AI/Pathfinding/NavigationGrid.h"
#include "Engine/Memory/MemUtil.h"

// ObjectAvoidance should be used to avoid small objects, not to avoid colliding with walls in a city (for that, use pathfinding). Therefore, we do not apply Avoidance to large objects.
static const float MaxObjectScale = 10.0f;

namespace usg
{

namespace ai
{

template <class ContextType>
class bhObjectAvoidance : public bhAvoidance<ContextType>
{
	typedef bhAvoidance<ContextType> Inherited;
public:
	bhObjectAvoidance(){}

	virtual ~bhObjectAvoidance() {}

	void SetObjectBuffer(uint32 uNumObjects)
	{
		ASSERT(uNumObjects <= NumObjects);
		m_uNumObjects = uNumObjects;
	}

	void SetData(const ObjectAvoidance& data)
	{
		SetObjectBuffer(data.numObjects);
	}

protected:
	static const uint32 NumObjects = 4;

	virtual int Update(float fElapsed, ContextType& ctx)
	{
		if(!ctx.Navigation().IsMoving())
		{
			return BH_FAILURE;
		}

		NavigationWrapper& nav = ctx.Navigation();

		usg::MemSet(m_pBuffer, 0, NumObjects * sizeof(usg::ai::OBB*));
		const NavigationGrid* pNavGrid = ctx.Navigation().GetNavigationGrid();
		const Vector3f vPos = ctx.GetPosition();
		const uint32 uCount = pNavGrid->GetNearestOBBs(vPos, &m_pBuffer[0], m_uNumObjects);
		if(uCount > 0)
		{
			for(uint32 i = 0; i < uCount; ++i)
			{
				const usg::ai::OBB* pOBB = m_pBuffer[i];
				const float fMaxScale = Math::Max(pOBB->GetScale().x, pOBB->GetScale().y);
				if (fMaxScale > MaxObjectScale)
				{
					continue;
				}
				const float fRatio = Math::Max(pOBB->GetScale().x/pOBB->GetScale().y, pOBB->GetScale().y / pOBB->GetScale().x);
				if (fRatio < 1.5f)
				{
					const usg::ai::AABB aabb = pOBB->GetAABB();
					const usg::Vector2f vExtents = aabb.GetExtents();
					const float fTargetRad = vExtents.x > vExtents.y ? vExtents.x : vExtents.y;

					const usg::Vector2f vT = pOBB->GetCenter();
					const usg::Vector3f vTarget(vT.x, 0.0f, vT.y);

					Inherited::Avoid(ctx, vTarget, fTargetRad);
				}
				else
				{
					Inherited::Avoid(ctx, *pOBB);
				}
			}
		}

		return BH_RUNNING;
	}

	usg::ai::OBB* m_pBuffer[NumObjects];
	uint32 m_uNumObjects;	//	number of objects to get from the navigation grid.
};

}	//	namespace ai

}	//	namespace usg

#endif	//	__USG_AI_BEHAVIOR_TREE_BEHAVIORS_OBJECT_AVOIDANCE_BEHAVIOR__