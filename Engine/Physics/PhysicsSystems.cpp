/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
//
//  PhysicsSystem.cpp
//  Usagi_xcode
//
//  Created by Giles on 2/18/14.
//  Copyright (c) 2014 Vitei. All rights reserved.
//
#include "Engine/Common/Common.h"
#include "Engine/Framework/EventManager.h"
#include "Engine/Framework/FrameworkComponents.pb.h"
#include "Engine/Physics/PhysicsComponents.pb.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "Engine/Debug/GlobalDebugStats.h"
#include "Engine/Scene/Common/SceneComponents.pb.h"
#include "Engine/Debug/DebugStats.h"
#include "Engine/Physics/PhysX.h"
#include <limits>
#include "Engine/Physics/PhysXVehicle/VehicleSceneQueryData.h"
#include "Engine/Physics/PhysicsEvents.pb.h"
#include "Engine/Scene/Common/SceneEvents.pb.h"
#include "Engine/Physics/PhysicsEventsInternal.pb.h"
#include "Engine/Physics/PhysicsSceneData.h"
#include "Engine/Framework/ExclusionCheck.h"
#include "Engine/Physics/CollisionData.pb.h"
#ifndef FINAL_BUILD
#include "Engine/Scene/Common/SceneComponents.pb.h"
#include "Engine/Scene/Common/SceneComponents.h"
#include "Engine/Scene/Scene.h"
#endif
#include "Engine/Core/stl/array.h"

namespace usg
{
#ifndef FINAL_BUILD
	namespace physics
	{
		void DebugRender(const Camera& camera);
	}
#endif

	namespace Systems {

		class AlterCollisionMasks : public System
		{
		public:
			struct Inputs
			{
				Required<EventManagerHandle, FromSelfOrParents> eventManager;
				Required<PhysicsScene, FromSelfOrParents> scene;
			};

			struct Outputs
			{
				Required<CollisionMasks> collisionMasks;
				Optional<SphereCollider> sphereCollider;
				Optional<BoxCollider> boxCollider;
				Optional<MeshCollider> meshCollider;
				Optional<ConeCollider> coneCollider;
				Optional<CylinderCollider> cylinderCollider;
			};

			DECLARE_SYSTEM(SYSTEM_PHYSICS)

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const UpdateCollisionMasks& evt)
			{
				auto& masks = outputs.collisionMasks.Modify();
				if (evt.has_uFilter)
				{
					masks.uFilter = evt.uFilter;
				}
				if (evt.has_uGroup)
				{
					masks.uGroup = evt.uGroup;
				}
				usg::array<PhysXShapeRuntimeData*, 8> ptrs;
				size_t uPtrCount = 0;
				if (outputs.sphereCollider.Exists())
				{
					ASSERT(uPtrCount < ptrs.size());
					ptrs[uPtrCount++] = static_cast<PhysXShapeRuntimeData*>(&outputs.sphereCollider.Force().GetRuntimeData());
				}
				if (outputs.boxCollider.Exists())
				{
					ASSERT(uPtrCount < ptrs.size());
					ptrs[uPtrCount++] = static_cast<PhysXShapeRuntimeData*>(&outputs.boxCollider.Force().GetRuntimeData());
				}
				if (outputs.meshCollider.Exists())
				{
					ASSERT(uPtrCount < ptrs.size());
					ptrs[uPtrCount++] = static_cast<PhysXShapeRuntimeData*>(&outputs.meshCollider.Force().GetRuntimeData());
				}
				if (outputs.coneCollider.Exists())
				{
					ASSERT(uPtrCount < ptrs.size());
					ptrs[uPtrCount++] = static_cast<PhysXShapeRuntimeData*>(&outputs.coneCollider.Force().GetRuntimeData());
				}
				if (outputs.cylinderCollider.Exists())
				{
					ASSERT(uPtrCount < ptrs.size());
					ptrs[uPtrCount++] = static_cast<PhysXShapeRuntimeData*>(&outputs.cylinderCollider.Force().GetRuntimeData());
				}
				for (size_t i = 0; i < uPtrCount; i++)
				{
					physics::details::Events::MarkShapeDirty dirtyEvt;
					physics::details::Events::MarkShapeDirty_init(&dirtyEvt);
					static_assert(sizeof(decltype(dirtyEvt.pShape)) >= sizeof(PhysXShapeRuntimeData*), "Unable to store pointer into the integer type.");
					dirtyEvt.pShape = reinterpret_cast<uint64>(ptrs[i]);
					inputs.eventManager->handle->RegisterEventWithEntity(inputs.scene.GetEntity(), dirtyEvt);
				}
			}
		};


		// FIXME: This system can't be run on a thread - we need some way of signalling this
		class BeginPhysicsSimulationSystem : public System, public UnsafeComponentGetter
		{
		public:

			struct Inputs
			{
				Required<usg::Components::SimulationActive, FromSelfOrParents> simactive;
				Required<PhysicsScene, FromSelf> scene;
				Required<EventManagerHandle, FromSelfOrParents> eventManager;
			};

			struct Outputs
			{
				Required<PhysicsScene, FromSelf> scene;
			};

			DECLARE_SYSTEM(SYSTEM_PHYSICS)

			static void ProcessDirtyActors(const Inputs& inputs, Outputs& outputs)
			{
				const auto& rtd = inputs.scene.GetRuntimeData().GetData();
				if (rtd.dirtyActorList.size())
				{
					for (auto pActorRtd : rtd.dirtyActorList)
					{
						rtd.pScene->resetFiltering(*pActorRtd->pActor);
					}
					auto& mutableRtd = outputs.scene.GetRuntimeData().GetData();
					mutableRtd.dirtyActorList.clear();
				}
			}

			static void ProcessDirtyShapes(const Inputs& inputs, Outputs& outputs)
			{
				const auto& rtd = inputs.scene.GetRuntimeData().GetData();
				if (rtd.dirtyShapeList.size())
				{
					// Update collision filters / callbacks for dirty shapes
					auto& mutableRtd = outputs.scene.GetRuntimeData().GetData();
					for (auto& dirtyShapeData : mutableRtd.dirtyShapeList)
					{
						Required<CollisionMasks, FromSelfOrParents> collisionMasks;
						GetComponentImpl(dirtyShapeData->entity, collisionMasks);

						Optional<RigidBody, FromSelfOrParents> rigidBody;
						GetComponentImpl(dirtyShapeData->entity, rigidBody);

						const CollisionMasks* pMasks = &(*collisionMasks);
						const RigidBody* pBody = rigidBody.Exists() ? &(*rigidBody.Force()) : nullptr;

						UpdateSimulationFilter(dirtyShapeData->pShape, pMasks, pBody, dirtyShapeData->shapeAggregateEntity);
					}
					mutableRtd.dirtyShapeList.clear();
				}
			}

			static void ProcessNewActors(const Inputs& inputs, Outputs& outputs)
			{
				const auto& rtd = inputs.scene.GetRuntimeData().GetData();
				if (rtd.addActorList.size())
				{
					// Add a list of actors in one batch
					auto& mutableRtd = outputs.scene.GetRuntimeData().GetData();
					vector<physx::PxActor*> newActors;
					newActors.reserve(mutableRtd.addActorList.size());
					for (auto pActorRtd : mutableRtd.addActorList)
					{
						if (pActorRtd->pActor->getAggregate())
						{
							// Will be added as an aggregate shortly
							continue;
						}
						newActors.push_back(pActorRtd->pActor);
						mutableRtd.dirtyActorList.erase(pActorRtd);
						pActorRtd->uBitmask |= static_cast<decltype(pActorRtd->uBitmask)>(ActorRuntimeData::Bitmask::InScene);
					}
					rtd.pScene->addActors(&newActors[0], (uint32)newActors.size());
					mutableRtd.addActorList.clear();
				}
				if (rtd.addAggregateList.size())
				{
					auto& mutableRtd = outputs.scene.GetRuntimeData().GetData();
					for (auto pAggregate : mutableRtd.addAggregateList)
					{
						rtd.pScene->addAggregate(*pAggregate);
					}
					mutableRtd.addAggregateList.clear();
				}
			}

			static void ClearCollisions(const Inputs& inputs, Outputs& outputs)
			{
				const uint32 uCurrentTick = inputs.scene.GetRuntimeData().GetData().uTick;

				auto& mutableRtd = outputs.scene.GetRuntimeData().GetData();
				vector< pair<Entity, Entity>> cleanList;
				for (auto& cd : mutableRtd.collisions)
				{
					cd.second.contacts.clear();
					if (uCurrentTick - cd.second.uTick > 5)
					{
						cleanList.push_back(cd.first);
					}
				}
				if (cleanList.size())
				{
					for (const auto& key : cleanList)
					{
						mutableRtd.collisions.erase(key);
					}
				}
			}

			static void CleanRemovedActors(const Inputs& inputs, Outputs& outputs)
			{
				// From PhysX documentation:
				// "If you intend to delete both the PxAggregate and its actors, it is most efficient to release the actors first, then release the PxAggregate when it is empty."
				const auto& rtd = inputs.scene.GetRuntimeData().GetData();
				if (rtd.removeActorList.size())
				{
					auto& mutableRtd = outputs.scene.GetRuntimeData().GetData();
					for (auto pActor : rtd.removeActorList)
					{
						rtd.pScene->removeActor(*pActor);
						pActor->release();
					}
					mutableRtd.removeActorList.clear();
				}
				if (rtd.removeAggregateList.size())
				{
					auto& mutableRtd = outputs.scene.GetRuntimeData().GetData();
					for (auto pAggregate : rtd.removeAggregateList)
					{
						rtd.pScene->removeAggregate(*pAggregate);
						pAggregate->release();
					}
					mutableRtd.removeAggregateList.clear();
				}
			}

			static void PreSimulation(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				auto& mutableRtd = outputs.scene.GetRuntimeData().GetData();
				mutableRtd.bSimulationRunning = true;

				CleanRemovedActors(inputs, outputs);
				ProcessDirtyShapes(inputs, outputs);
				ProcessNewActors(inputs, outputs);
				ProcessDirtyActors(inputs, outputs);
				ClearCollisions(inputs, outputs);
				VehicleUpdate(inputs, outputs, fDelta);
			}

			static void VehicleUpdate(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				if (inputs.scene.GetRuntimeData().GetData().vehicleData.vehicleList.size() == 0)
				{
					return;
				}
				auto& mutableRtd = outputs.scene.GetRuntimeData().GetData();

				physx::PxVehicleWheels** vehicles = &mutableRtd.vehicleData.vehicleList[0];
				const uint32 uVehicleCount = (uint32)mutableRtd.vehicleData.vehicleList.size();

				const physx::PxU32 raycastQueryResultsSize = mutableRtd.vehicleData.pVehicleSceneQueryData->GetRaycastQueryResultBufferSize();
				physx::PxRaycastQueryResult* raycastQueryResults = mutableRtd.vehicleData.pVehicleSceneQueryData->GetRaycastQueryResultBuffer(0);
				PxVehicleSuspensionRaycasts(mutableRtd.vehicleData.pVehicleBatchQuery, uVehicleCount, vehicles, raycastQueryResultsSize, raycastQueryResults);

				if (mutableRtd.vehicleData.vehicleWheelQueryResults.size() != uVehicleCount)
				{
					mutableRtd.vehicleData.wheelQueryResults.resize(PX_MAX_NB_WHEELS*uVehicleCount);
					mutableRtd.vehicleData.vehicleWheelQueryResults.resize(uVehicleCount);
				}
				for (uint32 i = 0; i < uVehicleCount; i++)
				{
					mutableRtd.vehicleData.vehicleWheelQueryResults[i] = { &mutableRtd.vehicleData.wheelQueryResults[0] + PX_MAX_NB_WHEELS*i, vehicles[i]->mWheelsSimData.getNbWheels() };
				}
				PxVehicleUpdates(fDelta, ToPhysXVec3(inputs.scene->vGravity), *mutableRtd.vehicleData.pFrictionPairs, uVehicleCount, vehicles, &mutableRtd.vehicleData.vehicleWheelQueryResults[0]);
			}

			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				if (!inputs.simactive->bActive)
				{
					return;
				}
				PreSimulation(inputs, outputs, fDelta);
				inputs.scene.GetRuntimeData().GetData().pScene->simulate(fDelta);// physics::GetTimeStep());
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const physics::details::Events::MarkShapeDirty& evt)
			{
				auto& mutableRtd = outputs.scene.GetRuntimeData().GetData();
				PhysXShapeRuntimeData* pShapeRtd = reinterpret_cast<PhysXShapeRuntimeData*>(evt.pShape);
				mutableRtd.dirtyShapeList.insert(pShapeRtd);
				
				physics::details::Events::MarkActorDirty actorEvt;
				physics::details::Events::MarkActorDirty_init(&actorEvt);
				actorEvt.pActor = 0;
				inputs.eventManager->handle->RegisterEventWithEntity(pShapeRtd->shapeAggregateEntity, actorEvt);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const physics::details::Events::MarkActorDirty& evt)
			{
				auto& mutableRtd = outputs.scene.GetRuntimeData().GetData();
				ActorRuntimeData* pActorRtd = reinterpret_cast<ActorRuntimeData*>(evt.pActor);
				mutableRtd.dirtyActorList.insert(pActorRtd);
			}
		};

		class SetPhysicsActorDirty : public System
		{
		public:
			struct Inputs
			{
				Required<EventManagerHandle, FromSelfOrParents> eventManager;
				Required<PhysicsScene, FromSelfOrParents> scene;
			};

			struct Outputs
			{
				Required<RigidBody> rigidBody;
			};

			DECLARE_SYSTEM(SYSTEM_POST_PHYSICS)

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const physics::details::Events::MarkActorDirty& evt)
			{
				ASSERT(evt.pActor == 0);
				physics::details::Events::MarkActorDirty forwardedEvt;
				physics::details::Events::MarkActorDirty_init(&forwardedEvt);
				forwardedEvt.pActor = reinterpret_cast<uint64>((ActorRuntimeData*)&outputs.rigidBody.GetRuntimeData());
				inputs.eventManager->handle->RegisterEventWithEntity(inputs.scene.GetEntity(), forwardedEvt);
			}
		};

		class FetchPhysicsResultsSystem : public System
		{
		public:

			struct Inputs
			{
				Required<EventManagerHandle, FromSelfOrParents> eventManager;
				Required<usg::Components::SimulationActive, FromSelfOrParents> simactive;
				Required<SceneComponent, FromSelfOrParents> visualScene;
				Required<usg::PhysicsScene, FromSelf> scene;
			};

			struct Outputs
			{
				Required<usg::PhysicsScene, FromSelf> scene;
			};

			DECLARE_SYSTEM(SYSTEM_POST_PHYSICS)

			static void PostSimulation(const Inputs& inputs, Outputs& outputs)
			{
				auto& mutableRtd = outputs.scene.GetRuntimeData().GetData();
				mutableRtd.uTick++;
				mutableRtd.bSimulationRunning = false;
			}

			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				if (!inputs.simactive->bActive)
				{
					return;
				}
				ProfilingTimer timer;
				timer.Start();
				inputs.scene.GetRuntimeData().GetData().pScene->fetchResults(true);
				PostSimulation(inputs, outputs);
				timer.Stop();
#ifndef FINAL_BUILD
				auto& mutableRtd = outputs.scene.GetRuntimeData().GetData();
				mutableRtd.diagnostics.fFetchResultsTime.Add(timer.GetTotalMilliSeconds());
				bool bPhysicsDebug = DebugStats::Inst()->GetPage() == GlobalDebugStats::PAGE_PHYSICS;
				if (mutableRtd.diagnostics.bDebugRenderOnNextFrame || bPhysicsDebug)
				{
					mutableRtd.diagnostics.bDebugRenderOnNextFrame = false;
					physics::DebugRender(*inputs.visualScene.GetRuntimeData().pScene->GetSceneCamera(0));
				}
#endif
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ShiftWorldOrigin& evt)
			{
 				outputs.scene.GetRuntimeData().pSceneData->pScene->shiftOrigin(ToPhysXVec3(evt.vShift));

				PhysicsSceneDirty dirtyEvt;
				inputs.eventManager->handle->RegisterEvent(dirtyEvt);
			}
		};

		class TransformKinematicBody : public System
		{
		public:

			struct Inputs
			{
				Required<RigidBody> rigidBody;
				Required<TransformComponent> transform;
				Required<KinematicBodyTag> kinematicTag;
			};

			struct Outputs
			{

			};

			DECLARE_SYSTEM(SYSTEM_PRE_PHYSICS)

			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				const auto& rtd = inputs.rigidBody.GetRuntimeData();
				ASSERT(inputs.rigidBody->bKinematic);
				ASSERT(rtd.pActor->is<physx::PxRigidDynamic>() != nullptr);
				if ((rtd.uBitmask & static_cast<decltype(rtd.uBitmask)>(RuntimeData<RigidBody>::Bitmask::InScene)) == 0)
				{
					return;
				}
				physx::PxRigidDynamic* pRigidDynamic = (physx::PxRigidDynamic*)rtd.pActor;
				const physx::PxTransform kinematicTargetTransform = ToPhysXTransform(*inputs.transform);
				pRigidDynamic->setKinematicTarget(kinematicTargetTransform);
			}
		};


		class HandleShapeEvents : public System
		{
		public:

			struct Outputs
			{
				Required<SphereCollider> sphereCollider;
			};

			DECLARE_SYSTEM(SYSTEM_TRANSFORM_RIGIDBODIES)

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ResizeSphere& evt)
			{
				outputs.sphereCollider.Modify().fRadius = evt.fNewRadius;
				outputs.sphereCollider.GetRuntimeData().pShape->setGeometry(physx::PxSphereGeometry(evt.fNewRadius));
			}
		};

		class HandleRigidBodyEvents : public System
		{
		public:

			struct Inputs
			{
				Required<RigidBody> rigidBody;
				Required<DynamicBodyTag> dynamicTag;
				Required<SceneComponent, FromSelfOrParents> visualScene;
			};

			DECLARE_SYSTEM(SYSTEM_TRANSFORM_RIGIDBODIES)
			EXCLUSION(usg::pair<KinematicBodyTag, FromSelf>)

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ApplyForce& evt)
			{
				if (!inputs.rigidBody->bDynamic || inputs.rigidBody->bKinematic)
				{
					return;
				}
				physx::PxRigidDynamic* pRigidDynamic = inputs.rigidBody.GetRuntimeData().pActor->is<physx::PxRigidDynamic>();
				if (pRigidDynamic != nullptr)
				{
					const physx::PxForceMode::Enum eForceMode = (physx::PxForceMode::Enum)(evt.eForceMode);
					switch (eForceMode)
					{
					case physx::PxForceMode::Enum::eVELOCITY_CHANGE:
					case physx::PxForceMode::Enum::eACCELERATION:
						pRigidDynamic->addForce(ToPhysXVec3(evt.vForce), eForceMode);
						break;
					default:
						if (!evt.has_vWorldPosition)
						{
							pRigidDynamic->addForce(ToPhysXVec3(evt.vForce), eForceMode);
						}
						else
						{
							physx::PxRigidBodyExt::addForceAtPos(*pRigidDynamic, ToPhysXVec3(evt.vForce), ToPhysXVec3(evt.vWorldPosition), eForceMode);
						}
					}
				}
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const FullPhysicsSync& msg)
			{
				physx::PxRigidDynamic* pRigidDynamic = inputs.rigidBody.GetRuntimeData().pActor->is<physx::PxRigidDynamic>();
				ASSERT(inputs.rigidBody->bDynamic);

				physx::PxTransform globalPose;
				physx::PxTransform prevPos = pRigidDynamic->getGlobalPose();
				globalPose.p = ToPhysXVec3(msg.vPosition);
				globalPose.q = ToPhysXQuaternion(msg.qRotation);
				if (msg.bGlobalSpace)
				{
					globalPose.p -= ToPhysXVec3(inputs.visualScene->vOriginOffset);
				}

				pRigidDynamic->setGlobalPose(globalPose);
				physx::PxTransform newPose = pRigidDynamic->getGlobalPose();
				pRigidDynamic->clearForce();
				pRigidDynamic->clearTorque();
				pRigidDynamic->setLinearVelocity(ToPhysXVec3(msg.vVelocity));
				pRigidDynamic->setAngularVelocity(ToPhysXVec3(msg.vTorque));

			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const ApplyTorque& evt)
			{
				if (!inputs.rigidBody->bDynamic || inputs.rigidBody->bKinematic)
				{
					return;
				}
				ASSERT(inputs.rigidBody->bDynamic);
				physx::PxRigidDynamic* pRigidDynamic = inputs.rigidBody.GetRuntimeData().pActor->is<physx::PxRigidDynamic>();
				ASSERT(pRigidDynamic != nullptr);
				pRigidDynamic->addTorque(ToPhysXVec3(evt.vTorque), (physx::PxForceMode::Enum)evt.eForceMode);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const SetLinearVelocity& evt)
			{
				if (!inputs.rigidBody->bDynamic || inputs.rigidBody->bKinematic)
				{
					return;
				}
				ASSERT(inputs.rigidBody->bDynamic);
				physx::PxRigidDynamic* pRigidDynamic = inputs.rigidBody.GetRuntimeData().pActor->is<physx::PxRigidDynamic>();
				ASSERT(pRigidDynamic != nullptr);
				pRigidDynamic->setLinearVelocity(ToPhysXVec3(evt.vVelocity));
			}


			static void OnEvent(const Inputs& inputs, Outputs& outputs, const OverrideLinearDamping& evt)
			{
				if (!inputs.rigidBody->bDynamic || inputs.rigidBody->bKinematic)
				{
					return;
				}
				ASSERT(inputs.rigidBody->bDynamic);
				physx::PxRigidDynamic* pRigidDynamic = inputs.rigidBody.GetRuntimeData().pActor->is<physx::PxRigidDynamic>();
				ASSERT(pRigidDynamic != nullptr);
				pRigidDynamic->setLinearDamping(evt.fDamping);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const RestoreLinearDamping& evt)
			{
				if (!inputs.rigidBody->bDynamic || inputs.rigidBody->bKinematic)
				{
					return;
				}
				ASSERT(inputs.rigidBody->bDynamic);
				physx::PxRigidDynamic* pRigidDynamic = inputs.rigidBody.GetRuntimeData().pActor->is<physx::PxRigidDynamic>();
				ASSERT(pRigidDynamic != nullptr);
				pRigidDynamic->setLinearDamping(inputs.rigidBody->fLinearDamping);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const AddLinearVelocity& evt)
			{
				if (!inputs.rigidBody->bDynamic || inputs.rigidBody->bKinematic)
				{
					return;
				}
				ASSERT(inputs.rigidBody->bDynamic);
				physx::PxRigidDynamic* pRigidDynamic = inputs.rigidBody.GetRuntimeData().pActor->is<physx::PxRigidDynamic>();
				ASSERT(pRigidDynamic != nullptr);
				pRigidDynamic->setLinearVelocity(ToPhysXVec3(evt.vVelocity) + pRigidDynamic->getLinearVelocity());
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const SetAngularVelocity& evt)
			{
				if (!inputs.rigidBody->bDynamic || inputs.rigidBody->bKinematic)
				{
					return;
				}
				ASSERT(inputs.rigidBody->bDynamic);
				physx::PxRigidDynamic* pRigidDynamic = inputs.rigidBody.GetRuntimeData().pActor->is<physx::PxRigidDynamic>();
				ASSERT(pRigidDynamic != nullptr);
				pRigidDynamic->setAngularVelocity(ToPhysXVec3(evt.vVelocity));
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const SetMaxAngularVelocity& evt)
			{
				if (!inputs.rigidBody->bDynamic || inputs.rigidBody->bKinematic)
				{
					return;
				}
				ASSERT(inputs.rigidBody->bDynamic);
				physx::PxRigidDynamic* pRigidDynamic = inputs.rigidBody.GetRuntimeData().pActor->is<physx::PxRigidDynamic>();
				ASSERT(pRigidDynamic != nullptr);
				pRigidDynamic->setMaxAngularVelocity(evt.fMax);
			} 

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const SetMaxLinearVelocity& evt)
			{
				if (!inputs.rigidBody->bDynamic || inputs.rigidBody->bKinematic)
				{
					return;
				}
				ASSERT(inputs.rigidBody->bDynamic);
				physx::PxRigidDynamic* pRigidDynamic = inputs.rigidBody.GetRuntimeData().pActor->is<physx::PxRigidDynamic>();
				ASSERT(pRigidDynamic != nullptr);
				pRigidDynamic->setMaxLinearVelocity(evt.fMax);
			}

		};
		
		class RigidBodyUpdateWorldMatrix : public System
		{
		public:
			struct Inputs
			{
				Optional<MatrixComponent, FromParents>      parentMtx; // Optional parent matrix
				Required<TransformComponent>				tran;
				Required<RigidBodyTransformUpdate>			rigidBodyUpdate;
			};

			struct Outputs
			{
				Required<MatrixComponent>      worldMtx;
			};

			DECLARE_SYSTEM(SYSTEM_TRANSFORM_NESTED_RIGIDBODIES)

			static  void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				Matrix4x4& mOut = outputs.worldMtx.Modify().matrix;
				mOut = inputs.tran->rotation;
				mOut.Translate(inputs.tran->position.x, inputs.tran->position.y, inputs.tran->position.z);

				if (inputs.parentMtx.Exists() && inputs.tran->bInheritFromParent)
				{
					mOut = mOut * inputs.parentMtx.Force()->matrix;
				}
			}
		};

		class HandlePhysicsDirty : public System
		{
		public:

			struct Inputs
			{
				Required<RigidBody> rigidBody;
				Required<TransformComponent> transform;
				Optional<MatrixComponent, FromParents> parentMatrix;
			};

			struct Outputs
			{
				Required<TransformComponent> transform;
			};

			DECLARE_SYSTEM(SYSTEM_TRANSFORM_RIGIDBODIES)

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const PhysicsSceneDirty& evt)
			{
				const auto& rtd = inputs.rigidBody.GetRuntimeData();
				if (!inputs.transform->bInheritFromParent || !inputs.parentMatrix.Exists())
				{
					TransformComponent& transOut = outputs.transform.Modify();
					transOut.position = ToUsgVec3(rtd.pRigidActor->getGlobalPose().p);
					transOut.rotation = ToUsgQuaternionf(rtd.pRigidActor->getGlobalPose().q);
					return;
				}

				const physx::PxTransform& globalTransform = rtd.pRigidActor->getGlobalPose();
				physx::PxTransform cumulativeParentTransform = ToPhysXTransform(*inputs.parentMatrix.Force());

				const physx::PxTransform localTransform = cumulativeParentTransform.transformInv(globalTransform);
				outputs.transform.Modify().position = ToUsgVec3(localTransform.p);
				outputs.transform.Modify().rotation = ToUsgQuaternionf(localTransform.q);
			}
		};

		class TransformRigidBody : public System
		{
		public:

			struct Inputs
			{
				Required<RigidBody> rigidBody;
				Required<DynamicBodyTag> dynamicTag;
			};

			struct Outputs
			{
				Required<TransformComponent> transform;
			};

			DECLARE_SYSTEM(SYSTEM_TRANSFORM_RIGIDBODIES)
			EXCLUSION(IfHas<SleepTag, FromSelf>, IfHas<TransformComponent, FromParents>)

			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				ASSERT(inputs.rigidBody->bDynamic);
				const auto& rtd = inputs.rigidBody.GetRuntimeData();
				TransformComponent& transOut = outputs.transform.Modify();
				transOut.position = ToUsgVec3(rtd.pRigidActor->getGlobalPose().p);
				transOut.rotation = ToUsgQuaternionf(rtd.pRigidActor->getGlobalPose().q);
			}
		};


		class TransformRigidBodyWithParent : public System
		{
		public:

			struct Inputs
			{
				Required<RigidBody> rigidBody;
				Required<DynamicBodyTag> dynamicTag;
				Required<TransformComponent> transform;
				Required<MatrixComponent, FromParents> parentMatrix;
			};

			struct Outputs
			{
				Required<TransformComponent> transform;
			};

			DECLARE_SYSTEM(SYSTEM_TRANSFORM_RIGIDBODIES)
			EXCLUSION(IfHas<SleepTag, FromSelf>)

			static void Run(const Inputs& inputs, Outputs& outputs, float fDelta)
			{
				// Note that the performance of this solution is very slow. You should generally avoid nested rigid bodies.
				ASSERT(inputs.rigidBody->bDynamic);
				const auto& rtd = inputs.rigidBody.GetRuntimeData();
				if (!inputs.transform->bInheritFromParent)
				{
					TransformComponent& transOut = outputs.transform.Modify();
					transOut.position = ToUsgVec3(rtd.pRigidActor->getGlobalPose().p);
					transOut.rotation = ToUsgQuaternionf(rtd.pRigidActor->getGlobalPose().q);
					return;
				}

				const physx::PxTransform& globalTransform = rtd.pRigidActor->getGlobalPose();
				physx::PxTransform cumulativeParentTransform = ToPhysXTransform(*inputs.parentMatrix);
				
				const physx::PxTransform localTransform = cumulativeParentTransform.transformInv(globalTransform);
				outputs.transform.Modify().position = ToUsgVec3(localTransform.p);
				outputs.transform.Modify().rotation = ToUsgQuaternionf(localTransform.q);
			}
		};

		namespace joint_details
		{
			template<typename JointType>
			void DestroyJoint(Required<JointType> c)
			{
				auto pJoint = c.GetRuntimeData().pJoint;
				if (pJoint != nullptr)
				{
					pJoint->release();
					c.GetRuntimeData().pJoint = nullptr;
				}
			}
		}

		class RevoluteJointSystem : public System
		{
		public:

			struct Inputs
			{
				Required<RevoluteJoint> joint;
			};

			struct Outputs
			{
				Required<RevoluteJoint> joint;
			};

			DECLARE_SYSTEM(SYSTEM_DEFAULT_PRIORITY)

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const SetRevoluteJointAngleLimits& evt)
			{
				auto pJoint = inputs.joint.GetRuntimeData().pJoint;
				const bool bLimitEnabled = evt.has_fMin || evt.has_fMax;
				if (bLimitEnabled)
				{
					const physx::PxJointAngularLimitPair limits(evt.has_fMin ? evt.fMin : -physx::PxPi, evt.has_fMax ? evt.fMax : physx::PxPi);
					pJoint->setLimit(limits);
				}
				pJoint->setRevoluteJointFlag(physx::PxRevoluteJointFlag::eLIMIT_ENABLED, bLimitEnabled);
			}

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const DestroyJoint& evt)
			{
				joint_details::DestroyJoint(outputs.joint);
			}
		};

		class FixedJointSystem : public System
		{
		public:

			struct Inputs
			{
				Required<FixedJoint> joint;
			};

			struct Outputs
			{
				Required<FixedJoint> joint;
			};

			DECLARE_SYSTEM(SYSTEM_DEFAULT_PRIORITY)

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const DestroyJoint& evt)
			{
				joint_details::DestroyJoint(outputs.joint);
			}
		};

		class PrismaticJointSystem : public System
		{
		public:

			struct Inputs
			{
				Required<PrismaticJoint> joint;
			};

			struct Outputs
			{
				Required<PrismaticJoint> joint;
			};

			DECLARE_SYSTEM(SYSTEM_DEFAULT_PRIORITY)

			static void OnEvent(const Inputs& inputs, Outputs& outputs, const DestroyJoint& evt)
			{
				joint_details::DestroyJoint(outputs.joint);
			}
		};
	}
}

#include GENERATED_SYSTEM_CODE(Engine/Physics/PhysicsSystems.cpp)