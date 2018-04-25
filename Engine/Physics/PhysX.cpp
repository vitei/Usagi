#include "Engine/Common/Common.h"
#include "Engine/Physics/PhysicsComponents.h"
#include "Engine/Physics/PhysXVehicle/VehicleSceneQueryData.h"
#include "Engine/Physics/PhysXVehicle/FrictionPairs.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Framework/SystemCoordinator.h"
#include "Engine/Physics/Raycast.h"
#include "PhysX.h"
#include "PhysXMeshCache.h"
#include "Engine/Physics/CollisionData.pb.h"
#include "Engine/Physics/Signals/OnTrigger.h"
#include "Engine/Physics/Signals/OnCollision.h"
#include "Engine/Physics/PhysicsSceneData.h"
#include "Engine/Physics/PhysicsTools.h"
#include "Engine/Physics/CollisionMask.pb.h"
#include "Engine/Physics/PhysicsEvents.pb.h"
#include "Engine/Framework/EventManager.h"

namespace usg
{
#ifndef FINAL_BUILD
	namespace physics
	{
		namespace details
		{
			float g_fOnCollisionClosureCallTimes = 0;
			float g_fFoundColliderInputsTimes = 0;
		}
	}
#endif

	namespace physics
	{
		static float32 s_fTimeStep = DefaultTimeStep;

		void SetTimeStep(float32 fStep)
		{
			s_fTimeStep = fStep;
		}

		float32 GetTimeStep()
		{
			return s_fTimeStep;
		}

		Components::TransformComponent ToUsgTransform(const physx::PxTransform& t)
		{
			TransformComponent r;
			r.bInheritFromParent = true;
			r.position = ToUsgVec3(t.p);
			r.rotation = ToUsgQuaternionf(t.q);
			return r;
		}

		static PhysXAllocator s_physXAllocator;
		static PhysXErrorCallback s_physXErrorCallback;

		static bool s_bInitialized = false;

		static struct {
			physx::PxFoundation* pFoundation = nullptr;
			physx::PxPhysics* pPhysics = nullptr;
			physx::PxDefaultCpuDispatcher* pCPUDispatcher = nullptr;
			physx::PxCooking* pCooking = nullptr;
			PhysXMeshCache* pMeshCache = nullptr;
		} s_physXInit;

		void init()
		{
			ASSERT(!s_bInitialized);
			s_physXInit.pFoundation = PxCreateFoundation(PX_FOUNDATION_VERSION, s_physXAllocator, s_physXErrorCallback);
			s_physXInit.pPhysics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *s_physXInit.pFoundation, physx::PxTolerancesScale(), false);
			s_physXInit.pCPUDispatcher = physx::PxDefaultCpuDispatcherCreate(2);
			auto cookingParams = physx::PxCookingParams(physx::PxTolerancesScale());
			cookingParams.skinWidth = 0.05f;
			cookingParams.areaTestEpsilon = 0.02f;
			PxRegisterHeightFields(*s_physXInit.pPhysics);
			s_physXInit.pCooking = PxCreateCooking(PX_PHYSICS_VERSION, *s_physXInit.pFoundation, cookingParams);
			s_physXInit.pMeshCache = vnew(ALLOC_PHYSICS)PhysXMeshCache(s_physXInit.pPhysics, s_physXInit.pCooking);

			physx::PxInitVehicleSDK(*s_physXInit.pPhysics);
			physx::PxVehicleSetBasisVectors(physx::PxVec3(0, 1, 0), physx::PxVec3(0, 0, 1));
			physx::PxVehicleSetUpdateMode(physx::PxVehicleUpdateMode::eACCELERATION);
			s_bInitialized = true;
		}

		void deinit()
		{
			ASSERT(s_bInitialized);
			
			physx::PxCloseVehicleSDK();	
			PxCloseExtensions();

			vdelete s_physXInit.pMeshCache;
			s_physXInit.pMeshCache = nullptr;

			s_physXInit.pCooking->release();
			s_physXInit.pCooking = nullptr;
			s_physXInit.pCPUDispatcher->release();
			s_physXInit.pCPUDispatcher = nullptr;

			s_physXInit.pPhysics->release();
			s_physXInit.pPhysics = nullptr;
			
			s_physXInit.pFoundation->release();
			s_physXInit.pFoundation = nullptr;

			s_bInitialized = false;
		}
	}

	void InitRaycasting(Required<usg::PhysicsScene> scene, usg::PhysXAllocator& allocator);

	static usg::pair<Entity,Entity> GenerateCollisionPair(Entity e1, Entity e2)
	{
		const physx::PxU64 uId1 = reinterpret_cast<physx::PxU64>(e1);
		const physx::PxU64 uId2 = reinterpret_cast<physx::PxU64>(e2);
		return uId1 < uId2 ? usg::pair<Entity, Entity>(e1, e2) : usg::pair<Entity, Entity>(e2,e1);
	}

	class ContactReportCallback : public physx::PxSimulationEventCallback
	{
		Required<EventManagerHandle, FromSelfOrParents> m_eventManagerHandle;
		Component<usg::Components::PhysicsScene>& m_sceneComponent;
		vector<physx::PxContactPairPoint> m_contactPoints;
	public:
		ContactReportCallback(Component<usg::Components::PhysicsScene>& sceneComponent) : m_sceneComponent(sceneComponent)
		{
			GetComponent(sceneComponent.GetEntity(), m_eventManagerHandle);
		}

		void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 uCount)
		{
			OnJointBroken evt;
			OnJointBroken_init(&evt);
			for (physx::PxU32 i = 0; i < uCount; i++)
			{
				physx::PxJoint* pJoint = reinterpret_cast<physx::PxJoint*>(constraints[i].externalReference);
				Entity e = (Entity)pJoint->userData;
				m_eventManagerHandle->handle->RegisterEventWithEntity(e,evt,ON_ENTITY);
			}
		}

		void onWake(physx::PxActor** actors, physx::PxU32 count)
		{
			for (uint32 i = 0; i < count; i++)
			{
				auto pActor = actors[i];
				Component<RigidBody>* pRigidBody = (Component<RigidBody>*)pActor->userData;
				Entity entity = pRigidBody->GetEntity();
				ASSERT(entity != nullptr);
				Optional<SleepTag> sleepTag;
				GetComponent(entity, sleepTag);
				if (sleepTag.Exists())
				{
					ComponentLoadHandles handles;	// Not needed for this component
					GameComponents<SleepTag>::Free(entity, handles);
					entity->SetChanged();
				}
			}
		}

		void onSleep(physx::PxActor** actors, physx::PxU32 count)
		{
			for (uint32 i = 0; i < count; i++)
			{
				auto pActor = actors[i];
				Component<RigidBody>* pRigidBody = (Component<RigidBody>*)pActor->userData;
				Entity entity = pRigidBody->GetEntity();
				ASSERT(entity != nullptr);
				Optional<SleepTag> sleepTag;
				GetComponent(entity, sleepTag);
				if (!sleepTag.Exists())
				{
					GameComponents<SleepTag>::Create(entity);
					entity->SetChanged();
				}
			}
		}

		void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count)
		{
			for (uint32 i = 0; i < count; i++)
			{
				const auto& pair = pairs[i];
				if (pair.triggerShape->userData == nullptr || pair.otherShape->userData == nullptr)
				{
					continue;
				}

				if (pair.flags.isSet(physx::PxTriggerPairFlag::eREMOVED_SHAPE_TRIGGER) || pair.flags.isSet(physx::PxTriggerPairFlag::eREMOVED_SHAPE_OTHER))
				{
					continue;
				}

				const PhysXShapeRuntimeData& triggerRtd = *static_cast<PhysXShapeRuntimeData*>(pair.triggerShape->userData);
				const PhysXShapeRuntimeData& triggererRtd = *static_cast<PhysXShapeRuntimeData*>(pair.otherShape->userData);

				auto& sceneRtd = m_sceneComponent.GetRuntimeData().GetData();

				sceneRtd.triggerSignals.push_back();
				auto& td = sceneRtd.triggerSignals.back();
				td.triggerEntity = triggerRtd.entity; // Note here that unlike for collisions, trigger signals are sent to the entities which have the shape, not the shape aggregate entity (entity with the rigidbody)
				td.triggererEntity = triggererRtd.entity;
				td.bEntered = pair.status == physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
			}
		}

		void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs)
		{
			auto& sceneRtd = m_sceneComponent.GetRuntimeData().GetData();

			for (physx::PxU32 i = 0; i < nbPairs; i++)
			{
				if (pairs[i].shapes[0]->userData == nullptr || pairs[i].shapes[1]->userData == nullptr)
				{
					continue;
				}
				const PhysXShapeRuntimeData& rtd1 = *physics::details::GetUserDataFromPhysXShape(pairs[i].shapes[0]);
				const PhysXShapeRuntimeData& rtd2 = *physics::details::GetUserDataFromPhysXShape(pairs[i].shapes[1]);

				const physx::PxU32 contactCount = pairs[i].contactCount;
				if (contactCount)
				{
					const bool bFirstContact = pairs[i].flags.isSet(physx::PxContactPairFlag::eACTOR_PAIR_HAS_FIRST_TOUCH);
					if (m_contactPoints.size() < contactCount)
					{
						m_contactPoints.resize(contactCount);
					}
					pairs[i].extractContacts(&m_contactPoints[0], contactCount);

					usg::pair<Entity, Entity> collisionPair = GenerateCollisionPair(rtd1.shapeAggregateEntity, rtd2.shapeAggregateEntity);
					ASSERT(collisionPair.first != nullptr && collisionPair.second != nullptr && "Destroyed entity has not been cleaned up from the PhysX scene?");
					auto& collisionDataForPair = sceneRtd.collisions[collisionPair];

					bool bFirstContactForPair = false;
					if (bFirstContact)
					{
						const uint32 uSinceLastCollision = sceneRtd.uTick - collisionDataForPair.uTick;
						if (uSinceLastCollision >= 2)
						{
							bFirstContactForPair = true;

						}
					}
					collisionDataForPair.bFirstContact = bFirstContactForPair;
					collisionDataForPair.uTick = sceneRtd.uTick;

					float fNormalMul = 1.0f;
					bool bSwap = false;
					usg::pair<decltype(&rtd1), decltype(&rtd2)> rtdPair(&rtd1,&rtd2);
					if (collisionPair.first == rtd2.shapeAggregateEntity)
					{
						fNormalMul = -1.0f;
						bSwap = true;
						eastl::swap(rtdPair.first, rtdPair.second);
					}
					if (rtdPair.first->shapeAggregateEntity->IsCollisionListener())
					{
						if (rtdPair.first->pShape->getSimulationFilterData().word1 & rtdPair.second->pShape->getSimulationFilterData().word0)
						{
							if (rtdPair.second->pShape->getSimulationFilterData().word0 & rtdPair.first->shapeAggregateEntity->GetOnCollisionMask())
							{
								collisionDataForPair.uNotifyFirstTick = sceneRtd.uTick;
							}
						}
					}
					if (rtdPair.second->shapeAggregateEntity->IsCollisionListener())
					{
						if (rtdPair.second->pShape->getSimulationFilterData().word1 & rtdPair.first->pShape->getSimulationFilterData().word0)
						{
							if (rtdPair.first->pShape->getSimulationFilterData().word0 & rtdPair.second->shapeAggregateEntity->GetOnCollisionMask())
							{
								collisionDataForPair.uNotifySecondTick = sceneRtd.uTick;
							}
						}
					}

#ifdef DEBUG_BUILD
					if (!(collisionDataForPair.uNotifyFirstTick == sceneRtd.uTick || collisionDataForPair.uNotifySecondTick == sceneRtd.uTick))
					{
						// This should not happen: we should not generate OnCollision signal unless really needed
						//DEBUG_PRINT("WARNING: unnecessary collision detection going on!\n");
						//DEBUG_PRINT("Collider 1: %s\n", rtdPair.first->pShape->getName() != nullptr ? rtdPair.first->pShape->getName() : "[unknown]");
						//DEBUG_PRINT("Mask 1: %u\n", rtdPair.first->pShape->getSimulationFilterData().word0);
						//DEBUG_PRINT("Collider 2: %s\n", rtdPair.second->pShape->getName() != nullptr ? rtdPair.second->pShape->getName() : "[unknown]");
						//DEBUG_PRINT("Mask 2: %u\n", rtdPair.second->pShape->getSimulationFilterData().word0);
					}
#endif

					for (physx::PxU32 j = 0; j < contactCount; j++)
					{
						collisionDataForPair.contacts.push_back();
						auto& newCollision = collisionDataForPair.contacts.back();
						newCollision.fDepth = m_contactPoints[j].separation;
						newCollision.vNormal = ToUsgVec3(m_contactPoints[j].normal)*fNormalMul;
						newCollision.vIntersectionPoint = ToUsgVec3(m_contactPoints[j].position);
						newCollision.uMaterialFlags[bSwap ? 1 : 0] = physics::details::FetchMaterialFlags(pairs[i].shapes[0], m_contactPoints[j].internalFaceIndex0);
						newCollision.uMaterialFlags[bSwap ? 0 : 1] = physics::details::FetchMaterialFlags(pairs[i].shapes[1], m_contactPoints[j].internalFaceIndex1);
					}
				}
			}
		}

		void onAdvance(const physx::PxRigidBody*const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override
		{

		}

	};

	static physx::PxFilterFlags UsagiPhysXContactReportFilterShader(physx::PxFilterObjectAttributes attributes0, physx::PxFilterData filterData0,
		physx::PxFilterObjectAttributes attributes1, physx::PxFilterData filterData1,
		physx::PxPairFlags& pairFlags, const void* constantBlock, physx::PxU32 constantBlockSize)
	{
		const bool bFiltersMatch = ((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1));
		if (physx::PxFilterObjectIsTrigger(attributes0) || physx::PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = physx::PxPairFlag::eTRIGGER_DEFAULT;
			return bFiltersMatch ? physx::PxFilterFlag::eDEFAULT : physx::PxFilterFlag::eKILL;
		}

		pairFlags = physx::PxPairFlags(0);

		const bool bResolveContacts = bFiltersMatch;
		if (bResolveContacts)
		{
			const bool bDisableImpulses = (filterData0.word3 & static_cast<physx::PxU32>(ShapeBitmask::DisableImpulses)) != 0 || (filterData1.word3 & static_cast<physx::PxU32>(ShapeBitmask::DisableImpulses)) != 0;
			if (bDisableImpulses)
			{
				pairFlags |= physx::PxPairFlag::eDETECT_DISCRETE_CONTACT;
			}
			else
			{
				pairFlags |= physx::PxPairFlag::eSOLVE_CONTACT | physx::PxPairFlag::eDETECT_DISCRETE_CONTACT;
			}
			if (((filterData0.word3 | filterData1.word3) & static_cast<physx::PxU32>(ShapeBitmask::CCD)) != 0)
			{
				pairFlags |= physx::PxPairFlag::eDETECT_CCD_CONTACT;
			}
		}
		else
		{
			return physx::PxFilterFlag::eKILL;
		}

		const bool bReportCollisions = ((filterData0.word0 & filterData1.word2) || (filterData1.word0 & filterData0.word2)) != 0;
		if (bReportCollisions)
		{
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= physx::PxPairFlag::eNOTIFY_TOUCH_PERSISTS;
			pairFlags |= physx::PxPairFlag::eNOTIFY_CONTACT_POINTS;
		}

		const bool bEnableContactModifications = (((filterData0.word3 | filterData1.word3) & static_cast<physx::PxU32>(ShapeBitmask::EnableContactModification)) != 0);
		if (bEnableContactModifications)
		{
			pairFlags |= physx::PxPairFlag::eMODIFY_CONTACTS;
		}

		if (!bReportCollisions && !bResolveContacts)
		{
			return physx::PxFilterFlag::eKILL;
		}

		return physx::PxFilterFlag::eDEFAULT;
	}

	physx::PxTransform ToPhysXTransform(const TransformComponent& trans)
	{
		physx::PxTransform pxt;
		pxt.p = ToPhysXVec3(trans.position);
		pxt.q = ToPhysXQuaternion(trans.rotation);
		return pxt;
	}

	class MyContactModification : public physx::PxContactModifyCallback
	{
		void onContactModify(physx::PxContactModifyPair* const pairs, physx::PxU32 count)
		{
			ASSERT(pairs->actor[0]->userData != nullptr);
			ASSERT(pairs->actor[1]->userData != nullptr);
			Component<RigidBody>* pRigids[] = { (Component<RigidBody>*)pairs->actor[0]->userData,(Component<RigidBody>*)pairs->actor[1]->userData };
			
			const float fMaxImpulse = Math::Min( pRigids[0]->GetData().fMaxImpulse > Math::EPSILON ? pRigids[0]->GetData().fMaxImpulse : FLT_MAX , pRigids[1]->GetData().fMaxImpulse > Math::EPSILON ? pRigids[1]->GetData().fMaxImpulse : FLT_MAX);
			for (uint32 i = 0; i < pairs->contacts.size(); i++)
			{
				pairs->contacts.setMaxImpulse(i,fMaxImpulse);
			}
		}
	} s_contactModifyCallback;

	void InitPhysicsScene(Component<usg::Components::PhysicsScene>& p)
	{
		p.GetRuntimeData().pSceneData = vnew(ALLOC_PHYSICS)physics::PhysicsSceneData();
		auto& rtd = p.GetRuntimeData().GetData();

		rtd.uTick = 0;

		rtd.pPhysicsFoundation = &PxGetFoundation();
		ASSERT(rtd.pPhysicsFoundation != nullptr);

		rtd.pPhysics = &PxGetPhysics();
		ASSERT(rtd.pPhysics != nullptr);

		rtd.pCooking = physics::s_physXInit.pCooking;
		ASSERT(rtd.pCooking != nullptr);

		rtd.pMeshCache = physics::s_physXInit.pMeshCache;
		
		// PhysX Scene
		physx::PxSceneDesc sceneDesc(rtd.pPhysics->getTolerancesScale());
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_KINEMATIC_STATIC_PAIRS;
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_KINEMATIC_PAIRS;
		sceneDesc.flags |= physx::PxSceneFlag::eENABLE_CCD;
		sceneDesc.gravity = ToPhysXVec3(p->vGravity);
		sceneDesc.cpuDispatcher = physics::s_physXInit.pCPUDispatcher;
		sceneDesc.filterShader = UsagiPhysXContactReportFilterShader;
 		sceneDesc.simulationEventCallback = vnew(ALLOC_PHYSICS)ContactReportCallback(p);
		sceneDesc.contactModifyCallback = &s_contactModifyCallback;
		rtd.pScene = rtd.pPhysics->createScene(sceneDesc);
		InitRaycasting(Required<PhysicsScene>(p), physics::s_physXAllocator);

		// Init vehicle related data
		rtd.vehicleData.pVehicleSceneQueryData = VehicleSceneQueryData::Allocate(p->uMaxNumberOfVehicles, PX_MAX_NB_WHEELS, p->uMaxNumberOfVehicles, physics::s_physXAllocator);
		rtd.vehicleData.pVehicleBatchQuery = VehicleSceneQueryData::SetUpBatchedSceneQuery(0, *rtd.vehicleData.pVehicleSceneQueryData, rtd.pScene);
		rtd.vehicleData.pFrictionPairs = CreateFrictionPairs(Required<PhysicsScene>(p));
	}

	void DeinitPhysicsScene(Component<usg::Components::PhysicsScene>& p)
	{
		auto& rtd = p.GetRuntimeData().GetData();
		rtd.addActorList.clear();
		rtd.dirtyShapeList.clear();

		rtd.vehicleData.pVehicleSceneQueryData->Free(physics::s_physXAllocator);
		rtd.vehicleData.pVehicleSceneQueryData = nullptr;

		physics::s_physXAllocator.deallocate(rtd.raycastData.pRaycastBatchQuery->getUserMemory().userRaycastResultBuffer);
		physics::s_physXAllocator.deallocate(rtd.raycastData.pRaycastBatchQuery->getUserMemory().userRaycastTouchBuffer);
		rtd.raycastData.pRaycastBatchQuery = nullptr;
		rtd.raycastData.pendingRequests.clear();

		rtd.pScene->release();
		rtd.pScene = nullptr;

		vdelete(p.GetRuntimeData().pSceneData);
		p.GetRuntimeData().pSceneData = nullptr;
	}

	void UpdateSimulationFilter(physx::PxShape* pShape, Entity entityWithShape, Entity aggregateEntity)
	{
		// word0 = my own collision group mask
		// word1 = collide against filter
		// word2 = report collisions filter (based on systems' collision masks)
		// word3 last bits = custom bitmask with information such as whether we should generate contact reports, etc
		// word3 = material flags (up to 16 bits reserved)

		Required<CollisionMasks, FromSelfOrParents> collisionMasks;
		GetComponent(entityWithShape, collisionMasks);

		ASSERT(collisionMasks.IsValid() && "Shapes must have CollisionMasks component.");
		ASSERT(!aggregateEntity->IsChildOf(collisionMasks.GetEntity()) && "Picked up CollisionMasks from the scene");

		Optional<RigidBody, FromSelfOrParents> rigidBody;
		GetComponent(entityWithShape, rigidBody);

		physx::PxFilterData filterData = pShape->getSimulationFilterData();
		filterData.word0 = collisionMasks->uGroup;
		filterData.word1 = collisionMasks->uFilter;
		filterData.word2 = aggregateEntity->GetOnCollisionMask();
		filterData.word3 = 0;

		if (rigidBody.Exists() && rigidBody.Force()->bDisableImpulses)
		{
			filterData.word3 |= static_cast<physx::PxU32>(ShapeBitmask::DisableImpulses);
		}
		if (rigidBody.Exists() && rigidBody.Force()->bEnableCCD)
		{
			filterData.word3 |= static_cast<physx::PxU32>(ShapeBitmask::CCD);
		}
		if (rigidBody.Exists() && rigidBody.Force()->fMaxImpulse > Math::EPSILON)
		{
			filterData.word3 |= static_cast<physx::PxU32>(ShapeBitmask::EnableContactModification);
		}

		physx::PxMaterial* pMat;
		pShape->getMaterials(&pMat, 1);
		ASSERT(pMat != nullptr);
		const auto& m = *(PhysicMaterial*)pMat->userData;
		filterData.word3 |= (m.uFlags & 0xffff);

		pShape->setSimulationFilterData(filterData);
		pShape->setQueryFilterData(filterData);
	}

	void GenerateOnTriggerSignals(SystemCoordinator& systemCoordinator, Required<usg::PhysicsScene> scene)
	{
		auto& rtd = scene.GetRuntimeData().GetData();
		for (const auto& trig : rtd.triggerSignals)
		{
			const TriggerEventType eventType = trig.bEntered ? TriggerEventType::Enter : TriggerEventType::Exit;
			OnTriggerSignal onTriggerSignal(trig.triggererEntity, eventType);
			systemCoordinator.Trigger(trig.triggerEntity, onTriggerSignal, ON_ENTITY);
		}
		rtd.triggerSignals.clear();
	}

	void GenerateOnCollisionSignals(SystemCoordinator& systemCoordinator, Required<usg::PhysicsScene> scene)
	{
		auto& rtd = scene.GetRuntimeData().GetData();
		const uint32 uColCount = (uint32)rtd.collisions.size();
		if (rtd.collisions.size() == 0)
		{
			return;
		}
		for (const auto& c : rtd.collisions)
		{
			const auto& pair = c.first;
			const auto& data = c.second;
			for (const auto& contact : data.contacts)
			{
				Collision usgCol;
				usgCol.fDepth = contact.fDepth;
				usgCol.vIntersectionPoint = contact.vIntersectionPoint;
				usgCol.bFirstContact = data.bFirstContact;
				if (data.uNotifyFirstTick == rtd.uTick - 1)
				{
					usgCol.vNormal = -contact.vNormal;
					usgCol.uMaterialFlags = contact.uMaterialFlags[1];
					CollisionMasks* pMasks = GameComponents<CollisionMasks>::GetComponentData(pair.second);
					OnCollisionSignal onCollisionSignal(pair.second, usgCol, pMasks->uGroup);
					systemCoordinator.Trigger(pair.first, onCollisionSignal, ON_ENTITY);
				}

				if (data.uNotifySecondTick == rtd.uTick - 1)
				{
					usgCol.vNormal = contact.vNormal;
					usgCol.uMaterialFlags = contact.uMaterialFlags[0];
					CollisionMasks* pMasks = GameComponents<CollisionMasks>::GetComponentData(pair.first);
					OnCollisionSignal onCollisionSignal(pair.first, usgCol, pMasks->uGroup);
					systemCoordinator.Trigger(pair.second, onCollisionSignal, ON_ENTITY);
				}
			}
		}
	}
}