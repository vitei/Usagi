#include "Engine/Common/Common.h"
#include "Engine/Physics/PhysicsComponents.pb.h"
#include "Engine/Physics/Raycast.h"
#include "Engine/Framework/EventManager.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "Engine/Physics/Signals/OnRaycastHit.h"
#include "Engine/Physics/PhysicsSceneData.h"
#include "Engine/Physics/PhysicsTools.h"

namespace usg
{
	struct RaycastBitmask
	{
		static constexpr uint32 StopOnFirstHit = 1;
	};

	physx::PxQueryHitType::Enum PreFilter(physx::PxFilterData queryFilterData, physx::PxFilterData objectFilterData, const void* constantBlock, physx::PxU32 constantBlockSize, physx::PxHitFlags& hitFlags)
	{
		physx::PxQueryHitType::Enum r = physx::PxQueryHitType::eNONE;
		if (queryFilterData.word0 & objectFilterData.word0)
		{
			return (queryFilterData.word2 & RaycastBitmask::StopOnFirstHit) ? physx::PxQueryHitType::eBLOCK : physx::PxQueryHitType::eTOUCH;
		}
		return r;
	}

	namespace physics
	{
		namespace details
		{
			void RaycastAsync(Entity callbackEntity, uint32 uSystemId, const AsyncRaycastRequest& request)
			{
				// FIXME: THREADING ISSUE
				// We should make access to the raycasts queue publically accessible in a threadsafe way
				struct Getter : public UnsafeComponentGetter {		}getter ;
				Required<usg::PhysicsScene, FromSelfOrParents> sceneFromSelfOrParents;
				getter.GetComponent(callbackEntity, sceneFromSelfOrParents);
				Required<usg::PhysicsScene> scene;
				getter.GetComponent(sceneFromSelfOrParents.GetEntity(), scene);
				// FIXME: THREADING ISSUE

				ASSERT((request.vTo - request.vFrom).MagnitudeSquared() > Math::EPSILON);
				ASSERT(request.uMaxHits > 0);

				auto& sceneRtd = scene.GetRuntimeData().GetData();
				ASSERT(sceneRtd.raycastData.pendingRequests.size() < PhysicsConstants::MaxNumAsyncRaycastsPerFrame);

				const float fDistance = (request.vTo - request.vFrom).Magnitude();
				const physx::PxVec3 vDirection = ToPhysXVec3((request.vTo - request.vFrom)*(1.0f / fDistance));

				physx::PxHitFlags hitFlags = physx::PxHitFlag::ePOSITION | physx::PxHitFlag::eNORMAL;
				physx::PxQueryFilterData filterData;
				filterData.data.word0 = request.uFilter;
				filterData.data.word1 = 0;
				filterData.data.word2 = 0;
				if (request.uMaxHits == 1)
				{
					filterData.data.word2 |= RaycastBitmask::StopOnFirstHit;
				}
				filterData.data.word3 = 0;
				filterData.flags |= physx::PxQueryFlag::ePREFILTER;
				sceneRtd.raycastData.pRaycastBatchQuery->raycast(ToPhysXVec3(request.vFrom), vDirection, fDistance, request.uMaxHits, hitFlags, filterData);
				sceneRtd.raycastData.pendingRequests.emplace_back(request.uRaycastId, usg::pair<uint32, Entity>(uSystemId, callbackEntity));
			}
		}
	}

	template<typename HitSource>
	static inline RaycastHitBase AddHit(const HitSource& s)
	{
		RaycastHitBase hit;
		hit.vPosition = ToUsgVec3(s.position);
		hit.vNormal = ToUsgVec3(s.normal);
		hit.fDistance = s.distance;
		hit.uMaterialFlags = physics::details::FetchMaterialFlags(s.shape, s.faceIndex);
		hit.uGroup = s.shape->getSimulationFilterData().word0;
		return hit;
	}

	template<typename HitSource>
	static inline Entity AddEntity(const HitSource& s)
	{
		ASSERT(s.shape != nullptr);
		ASSERT(s.shape->userData != nullptr);
		const PhysXShapeRuntimeData& shapeRtd = *(const PhysXShapeRuntimeData*)s.shape->userData;
		ASSERT(shapeRtd.entity != nullptr);
		return shapeRtd.entity;
	}

	void ExecuteRaycasts(Required<usg::PhysicsScene> scene, EventManager& eventManager, SystemCoordinator& systemCoordinator)
	{
		auto& sceneRtd = scene.GetRuntimeData().GetData();

		vector<RaycastHitBase>& hits = sceneRtd.raycastData.workData.hits;
		vector<Entity> entities = sceneRtd.raycastData.workData.entities;
		vector<uint8> workBuffer = sceneRtd.raycastData.workData.workBuffer;
		if (workBuffer.size() < PhysicsConstants::RaycastWorkBufferSize)
		{
			workBuffer.resize(PhysicsConstants::RaycastWorkBufferSize);
		}

		sceneRtd.raycastData.pRaycastBatchQuery->execute();
		auto& userMem = sceneRtd.raycastData.pRaycastBatchQuery->getUserMemory();
		const uint32 uResultCount = (uint32)sceneRtd.raycastData.pendingRequests.size();
		for (uint32 i = 0; i < uResultCount; i++)
		{
			auto& result = userMem.userRaycastResultBuffer[i];

			hits.clear();
			entities.clear();
			if (result.hasBlock)
			{
				hits.push_back(AddHit(result.block));
				entities.push_back(AddEntity(result.block));
			}
			else
			{
				for (uint32 j = 0; j < result.nbTouches; j++)
				{
					const auto& touch = result.touches[j];
					hits.push_back(AddHit(touch));
					entities.push_back(AddEntity(touch));
				}
			}

			ASSERT(entities.size() == hits.size());
			Entity callbackEntity = sceneRtd.raycastData.pendingRequests[i].second.second;
			const uint32 uUserProvidedRaycastId = sceneRtd.raycastData.pendingRequests[i].first;
			const uint32 uSystemId = sceneRtd.raycastData.pendingRequests[i].second.first;
			const uint32 uHitCount = (uint32)hits.size();

			OnRaycastHitSignal onRaycastHitSignal(uSystemId, uUserProvidedRaycastId, &hits[0], &entities[0], uHitCount, &workBuffer[0], workBuffer.size());
			systemCoordinator.Trigger(callbackEntity, onRaycastHitSignal, ON_ENTITY);
		}
		sceneRtd.raycastData.pendingRequests.clear();
	}

	void InitRaycasting(Required<usg::PhysicsScene> scene, usg::PhysXAllocator& allocator)
	{
		const uint32 uTouchBufferSize = sizeof(physx::PxRaycastHit)*PhysicsConstants::MaxNumAsyncRaycastsPerFrame * 2;

		auto& rtd = scene.GetRuntimeData().GetData();
		physx::PxBatchQueryDesc raycastBatchQueryDesc(PhysicsConstants::MaxNumAsyncRaycastsPerFrame, 0, 0);
		physx::PxBatchQueryMemory raycastBatchQueryMem(PhysicsConstants::MaxNumAsyncRaycastsPerFrame, 0, 0);
		raycastBatchQueryMem.userRaycastResultBuffer = (physx::PxRaycastQueryResult*)allocator.allocate(sizeof(physx::PxRaycastQueryResult)*PhysicsConstants::MaxNumAsyncRaycastsPerFrame * 2, "", 0, 0);
		raycastBatchQueryMem.raycastTouchBufferSize = uTouchBufferSize;
		raycastBatchQueryMem.userRaycastTouchBuffer = (physx::PxRaycastHit*)allocator.allocate(uTouchBufferSize, "", 0, 0);
		raycastBatchQueryDesc.queryMemory = raycastBatchQueryMem;
		raycastBatchQueryDesc.preFilterShader = PreFilter;
		rtd.raycastData.pRaycastBatchQuery = rtd.pScene->createBatchQuery(raycastBatchQueryDesc);
	}

}