#include "Engine/Common/Common.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/AI/Targetting/TargetComponents.pb.h"
#include "Engine/AI/Targetting/TargetEvents.pb.h"
#include "Engine/Physics/PhysicsEvents.pb.h"
#include "Engine/Framework/System.h"
#include "Engine/Framework/GameComponents.h"
#include "Engine/Framework/SystemCategories.h"
#include "Engine/AI/Targetting/TargetListUtil.h"
#include "Engine/AI/AgentNavigationUtil.h"
#include "Engine/Scene/Common/SceneComponents.pb.h"
#include "Engine/Physics/Raycast.h"
#include "Engine/Physics/CollisionQuadTree.h"
#include "Engine/AI/AgentNavigation.h"

namespace usg
{
	namespace ai
	{
		namespace Systems
		{

			class TargetListSystem : public usg::System, public usg::ai::TargetListUtil
			{
			public:

				struct Inputs
				{
					Required<usg::EntityID> self;
					Required<usg::MatrixComponent> mtx;
					Required<usg::ai::TargetListComponent> targetList;
					Required<usg::ai::TargetVisibilityComponent> visibility;
				};

				struct Outputs
				{
					Required<usg::ai::TargetListComponent> targetList;
					Required<usg::ai::TargetComponent> target;
				};

				DECLARE_SYSTEM(usg::SYSTEM_DEFAULT_PRIORITY)
				static void Run(const Inputs& in, Outputs& out, float fDelta);
				static void OnEvent(const Inputs& in, Outputs& out, const usg::ai::TargetEvent& evt);
			};

			class TargetListRaycastSystem : public usg::System, public usg::ai::TargetListUtil
			{
				static const uint32 RaycastId;

				static uint32 ComputeRaycastHash(const Vector3f& vFrom, const Vector3f& vTo, const Scene& scene, const Entity pEntity);

			public:

				struct Inputs
				{
					Required<usg::ai::TargetListComponent> targetList;
					Required<usg::ai::TargetComponent> target;
					Required<usg::ai::TargetListRaycastComponent> targetRaycast;
					Required<usg::ai::TargetVisibilityComponent> visibility;
					Required<usg::SceneComponent, FromParents> scene;
					Required<usg::MatrixComponent, FromSelfOrParents> mtx;
					Required<EntityID> self;
				};

				struct Outputs
				{
					Required<usg::ai::TargetListComponent> targetList;
					Required<usg::ai::TargetComponent> target;
					Required<usg::ai::TargetListRaycastComponent> targetRaycast;
				};

				struct RaycastHitInputs
				{

				};

				// Run this system while physics simulation is active. It is totally fine to use the last frame's matrix here.
				DECLARE_SYSTEM(usg::SYSTEM_PHYSICS_INDEPENDENT_GAMEPLAY) RAYCASTER

				static void Run(const Inputs& in, Outputs& out, float fDelta);
				static void OnRaycastHit(const Inputs& in, Outputs& out, const RaycastResult& result);
			};

		}
	}
}

#include "Engine/Physics/CollisionMask.pb.h"
#include "Engine/Scene/Scene.h"
#include "Engine/AI/AICommon.h"
#include "Engine/Core/Utility.h"

using namespace usg;
using namespace usg::ai;

void usg::ai::TargetListSystem::Run(const Inputs& in, Outputs& out, float fDelta)
{
	TargetListUtil::Tick(out.targetList.GetRuntimeData());
	const Vector3f& vOwnPos = in.mtx->matrix.TransformVec3(in.visibility->vPositionOffset, 1.0f);
	if(!TargetListUtil::UpdateTarget(out.targetList.GetRuntimeData(), out.target.Modify().targetData, out.target.Modify().target, vOwnPos))
	{
		out.target.Modify().active = false;
	}
	else
	{
		out.target.Modify().active = true;
	}
}

void usg::ai::TargetListSystem::OnEvent(const Inputs& in, Outputs& out, const usg::ai::TargetEvent& evt)
{
	if(evt.selfID.entity != in.self->id)
	{
		TargetListUtil::UpdateTargetList(out.targetList.GetRuntimeData(), evt);
	}
}

const uint32 usg::ai::TargetListRaycastSystem::RaycastId = utl::CRC32("TargetListRaycastSystem_Raycast");

void usg::ai::TargetListRaycastSystem::OnRaycastHit(const Inputs& in, Outputs& out, const RaycastResult& result)
{
	if (result.uRaycastId != RaycastId)
	{
		return;
	}
	const TargetListRaycastComponent& raycast = *in.targetRaycast;

	const bool bHasLineOfSight = result.uNumHits == 0 || (result.hits[0].uMaterialFlags & CollisionQuadTree::MF_NOFIRE) != 0;
	const uint32 uLastRaycastIndex = TargetListUtil::GetTargetIndexByEntifyRef(in.targetList.GetRuntimeData(), raycast.pLastRaycastEntity.entity);
	if (uLastRaycastIndex != (uint32)(-1))
	{
		usg::ai::Target& target = out.targetList.GetRuntimeData().targets[uLastRaycastIndex].target;
		if (!bHasLineOfSight)
		{
			if (target.uLineOfSight > 0)
			{
				target.uLineOfSight--;
			}
		}
		else
		{
			target.uLineOfSight = 10;
		}
		if (out.target->target.entityRef == target.entityRef)
		{
			out.target.Modify().target.uLineOfSight = target.uLineOfSight;
		}
	}	
}

uint32 usg::ai::TargetListRaycastSystem::ComputeRaycastHash(const Vector3f& vFrom, const Vector3f& vTo, const Scene& scene, Entity pEntity)
{
	const Vector3f& vMax = scene.GetWorldBounds().GetMax();
	const Vector3f& vMin = scene.GetWorldBounds().GetMin();
	struct { uint16 m_pPositions[6]; Entity m_pEntity; } positionData = { { FloatToUint16(vFrom.x,vMin.x,vMax.x),FloatToUint16(vFrom.y,vMin.y,vMax.y),FloatToUint16(vFrom.z,vMin.z,vMax.z),	FloatToUint16(vTo.x,vMin.x,vMax.x),FloatToUint16(vTo.y,vMin.x,vMax.x),FloatToUint16(vTo.z,vMin.x,vMax.x), } , pEntity};
	const uint32 uPosHash = utl::CRC32(&positionData, sizeof(positionData));
	return uPosHash;
}

void usg::ai::TargetListRaycastSystem::Run(const Inputs& in, Outputs& out, float fDelta)
{
	const float fRaycastInterval = 0.05f;
	const auto& targets = in.targetList.GetRuntimeData().targets;
	const uint32 uNumTargets = (uint32)targets.size();
	if (uNumTargets == 0)
	{
		return;
	}

	TargetListRaycastComponent& raycast = out.targetRaycast.Modify();
	if (raycast.fTimer < -0.5f)
	{
		// Uninitialized
		raycast.fTimer = Math::RangedRandom(0, fRaycastInterval);
	}
	raycast.fTimer += fDelta;
	if (raycast.fTimer > fRaycastInterval)
	{
		raycast.uTick++;
		raycast.fTimer -= fRaycastInterval;

		const uint32 uIndexNow = raycast.uLastRaycastIndex % uNumTargets;

		while (true)
		{
			raycast.uLastRaycastIndex = (raycast.uLastRaycastIndex + 1) % uNumTargets;
			if (raycast.uLastRaycastIndex == uIndexNow)
			{
				break;
			}

			const usg::ai::Target& target = targets[raycast.uLastRaycastIndex].target;

			raycast.pLastRaycastEntity.entity = target.entityRef.entity;

			const Vector3f& vOwnPos = in.mtx->matrix.TransformVec3(in.visibility->vPositionOffset, 1.0f);
			const Vector3f& vTargetPos = target.position;
			const uint32 uPosHash = ComputeRaycastHash(vOwnPos, vTargetPos, *in.scene.GetRuntimeData().pScene, target.entityRef.entity);
			const uint32 uOldHash = in.targetRaycast->pRaycastHash[raycast.uLastRaycastIndex];
			if (uPosHash != uOldHash)
			{
				out.targetRaycast.Modify().pRaycastHash[raycast.uLastRaycastIndex] = uPosHash;

				Vector3f vOffset;
				vOffset.SphericalRandomVector(1.0f);
				vOffset.y = Math::Abs(vOffset.y);

				AsyncRaycastRequest req;
				req.uFilter = in.targetRaycast->uVisibilityBlockingCollisionMask;
				req.uMaxHits = 1;
				req.uRaycastId = RaycastId;
				req.vFrom = vOwnPos;
				req.vTo = vTargetPos + vOffset;
				RaycastAsync(in, req);
				break;
			}
		}
	}
}

#include GENERATED_SYSTEM_CODE(Engine/AI/Targetting/TargetListSystems.cpp)