#include "Engine/Common/Common.h"
#include "Engine/Physics/PhysicsComponents.pb.h"
#include "Engine/Scene/Camera/Camera.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "Engine/Network/NetTime.h"
#include "Engine/Physics/CollisionData.pb.h"
#include <functional>

namespace usg
{
	namespace physics
	{
		static uint32 s_uFrameCount = 0;

		template<typename ColliderType, typename SearchMask>
		static Color GetMeshColor(Optional<ColliderType, SearchMask> c)
		{
			if (!c.Exists())
			{
				return Color(0.0f, 0.0f, 0.0f, 0.0f);
			}
			Required<CollisionMasks, FromSelfOrParents> masks;
			GetComponent(c.Force().GetEntity(), masks);
			if (masks->uGroup == 0 || masks->uFilter == 0)
			{
				return Color(0.0f, 0.0f, 0.0f, 0.0f);
			}
			const bool bIsTrigger = c.Force()->bIsTrigger;
			return !bIsTrigger ? Color(1, 0, 1, 0.5f) : Color(1, 0, 0, 0.25f + 0.20f*sinf(7.0f*NetTime::GetServerTime()));
		}

		template<typename SearchMask>
		static Color GetMeshColor(Optional<VehicleCollider, SearchMask> c)
		{
			if (!c.Exists())
			{
				return Color(0.0f, 0.0f, 0.0f, 0.0f);
			}
			return Color::Red;
		}

		static void RenderConvexMesh(physx::PxConvexMesh* pConvexMesh, const float fMeshScale, const Color& cColor, const physx::PxTransform& t)
		{
			physx::PxVec3 verts[256];
			physx::PxU32 nbVerts = pConvexMesh->getNbVertices();
			const physx::PxVec3* convexVerts = pConvexMesh->getVertices();
			const physx::PxU8* indexBuffer = pConvexMesh->getIndexBuffer();
			physx::PxU32 offset = 0;
			for (physx::PxU32 i = 0; i < pConvexMesh->getNbPolygons(); i++)
			{
				physx::PxHullPolygon face;
				const bool status = pConvexMesh->getPolygonData(i, face);
				PX_ASSERT(status);

				const physx::PxU8* faceIndices = indexBuffer + face.mIndexBase;
				for (physx::PxU32 j = 0; j < face.mNbVerts; j++)
				{
					verts[offset + j] = convexVerts[faceIndices[j]] * fMeshScale;
				}

				for (physx::PxU32 j = 2; j < face.mNbVerts; j++)
				{
					const auto tp1 = ToUsgVec3(t.transform(verts[physx::PxU16(offset)]));
					const auto tp2 = ToUsgVec3(t.transform(verts[physx::PxU16(offset + j)]));
					const auto tp3 = ToUsgVec3(t.transform(verts[physx::PxU16(offset + j - 1)]));
					if (s_uFrameCount % 3 == 0) Debug3D::GetRenderer()->AddLine(tp1, tp2, cColor, 0.006f);
					if (s_uFrameCount % 3 == 1) Debug3D::GetRenderer()->AddLine(tp2, tp3, cColor, 0.006f);
					if (s_uFrameCount % 3 == 2) Debug3D::GetRenderer()->AddLine(tp3, tp1, cColor, 0.006f);
				}
				offset += face.mNbVerts;
			}
		}

		static void RenderDebugMesh(Optional<MeshCollider> c, const physx::PxTransform& actorTransform)
		{
			const Color cColor = GetMeshColor(c);
			if (cColor.a() > 0)
			{
				auto& rtd = c.Force().GetRuntimeData();
				const physx::PxTransform& t = actorTransform;

				if (rtd.pTriangleMesh != nullptr)
				{
					auto pMesh = rtd.pTriangleMesh;
					const physx::PxVec3* pVerts = pMesh->getVertices();
					const uint32 uTriangleCount = pMesh->getNbTriangles();

					std::function<memsize(memsize)> indexGetter;
					if (pMesh->getTriangleMeshFlags().isSet(physx::PxTriangleMeshFlag::e16_BIT_INDICES))
					{
						const uint16* pIndices = (const uint16*)pMesh->getTriangles();
						indexGetter = [pIndices](memsize i) -> memsize {
							return pIndices[i];
						};
					}
					else
					{
						const uint32* pIndices = (const uint32*)pMesh->getTriangles();
						indexGetter = [pIndices](memsize i) -> memsize {
							return pIndices[i];
						};
					}

					for (uint32 uTriangleIndex = 0; uTriangleIndex < uTriangleCount; uTriangleIndex++)
					{
						const auto tp1 = ToUsgVec3(t.transform(pVerts[indexGetter(uTriangleIndex * 3 + 0)]));
						const auto tp2 = ToUsgVec3(t.transform(pVerts[indexGetter(uTriangleIndex * 3 + 1)]));
						const auto tp3 = ToUsgVec3(t.transform(pVerts[indexGetter(uTriangleIndex * 3 + 2)]));
						if (s_uFrameCount % 3 == 0) Debug3D::GetRenderer()->AddLine(tp1, tp2, cColor, 0.01f);
						if (s_uFrameCount % 3 == 1) Debug3D::GetRenderer()->AddLine(tp2, tp3, cColor, 0.01f);
						if (s_uFrameCount % 3 == 2) Debug3D::GetRenderer()->AddLine(tp3, tp1, cColor, 0.01f);
					}

				}
				if (rtd.pConvexMesh != nullptr)
				{
					if (c.Force()->vCenter.MagnitudeSquared() > Math::EPSILON)
					{
						RenderConvexMesh(rtd.pConvexMesh, c.Force()->fMeshScale, cColor, t.transform(physx::PxTransform(ToPhysXVec3(c.Force()->vCenter))));
					}
					else
					{
						RenderConvexMesh(rtd.pConvexMesh, c.Force()->fMeshScale, cColor, t);
					}
				}
			}
		}

		static void RenderDebugMesh(Optional<CylinderCollider> c, const physx::PxTransform& actorTransform)
		{
			const Color cColor = GetMeshColor(c);
			if (cColor.a() > 0)
			{
				auto& rtd = c.Force().GetRuntimeData();
				const physx::PxTransform& t = actorTransform;

				if (rtd.pConvexMesh != nullptr)
				{
					if (c.Force()->vCenter.MagnitudeSquared() > Math::EPSILON)
					{
						RenderConvexMesh(rtd.pConvexMesh, 1, cColor, t.transform(physx::PxTransform(ToPhysXVec3(c.Force()->vCenter))));
					}
					else
					{
						RenderConvexMesh(rtd.pConvexMesh, 1, cColor, t);
					}
				}
			}
		}

		static void RenderDebugMesh(Optional<ConeCollider> c, const physx::PxTransform& actorTransform)
		{
			const Color cColor = GetMeshColor(c);
			if (cColor.a() > 0)
			{
				auto& rtd = c.Force().GetRuntimeData();
				const physx::PxTransform& t = actorTransform;

				physx::PxConvexMeshGeometry g;
				rtd.pShape->getConvexMeshGeometry(g);
				ASSERT(g.isValid());
				ASSERT(g.convexMesh != nullptr);
				if (c.Force()->vCenter.MagnitudeSquared() > Math::EPSILON)
				{
					RenderConvexMesh(g.convexMesh, 1, cColor, t.transform(physx::PxTransform(ToPhysXVec3(c.Force()->vCenter))));
				}
				else
				{
					RenderConvexMesh(g.convexMesh, 1, cColor, t);
				}
			}
		}

		static void RenderDebugMesh(Optional<SphereCollider> c, const physx::PxTransform& trans)
		{
			const Color cColor = GetMeshColor(c);
			if (cColor.a() > 0)
			{
				const SphereCollider& sc = *c.Force();
				Debug3D::GetRenderer()->AddSphere(ToUsgVec3(trans.p), sc.fRadius, cColor);
			}
		}

		static void RenderDebugMesh(Optional<BoxCollider> c, const physx::PxTransform& trans)
		{
			const Color cColor = GetMeshColor(c);
			if (cColor.a() > 0)
			{
				const BoxCollider& bc = *c.Force();
				Matrix4x4 cube;
				cube.LoadIdentity();
				cube.MakeScale(bc.vExtents);
				Matrix4x4 rotm = ToUsgQuaternionf(trans.q);
				cube = cube * rotm;
				cube.SetPos(ToUsgVec3(trans.p) + bc.vCenter*ToUsgQuaternionf(trans.q));
				Debug3D::GetRenderer()->AddCube(cube, cColor);
			}
		}

		static void RenderDebugMesh(Optional<VehicleCollider> c, const physx::PxTransform& trans)
		{
			const Color cColor = GetMeshColor(c);
			if (cColor.a() > 0)
			{
				const VehicleCollider& vc = *c.Force();
				auto& rtd = c.Force().GetRuntimeData();

				// Chassis
				{
					physx::PxConvexMeshGeometry g;
					if (rtd.pChassisShape->getConvexMeshGeometry(g))
					{
						auto pConvexMesh = g.convexMesh;
						RenderConvexMesh(pConvexMesh, 1.0f, cColor, trans.transform(rtd.pChassisShape->getLocalPose()));
					}
				}

				// Wheels
				{
					for (memsize i = 0; i < vc.uNumWheels; i++)
					{
						auto pWheelShape = rtd.wheelsData.wheel[i].rtd.pShape;
						ASSERT(pWheelShape != nullptr);
						physx::PxConvexMeshGeometry g;
						if (pWheelShape->getConvexMeshGeometry(g))
						{
							auto pConvexMesh = g.convexMesh;
							RenderConvexMesh(pConvexMesh, 1.0f, cColor, trans.transform(pWheelShape->getLocalPose()));
						}
					}
				}

			}
		}

		void DebugRender(const Camera& camera)
		{
			s_uFrameCount++;

			constexpr float VisibilityRadius = 60;
			const Vector3f vCamPos = camera.GetPos().v3();
			uint32 uCount = 0;
			for (auto it = GameComponents<RigidBody>::GetIterator(); !it.IsEnd(); ++it)
			{
				Entity e = (*it)->GetEntity();
				Required<RigidBody, FromSelf> rb;
				GetComponent(e, rb);
				const auto vRigidBodyPos = Vector4f(ToUsgVec3(rb.GetRuntimeData().pRigidActor->getGlobalPose().p),1);
				if (camera.GetFrustum().ArePointsInFrustum(&vRigidBodyPos, 1) && vRigidBodyPos.v3().GetSquaredDistanceFrom(vCamPos) < VisibilityRadius*VisibilityRadius)
				{
					Optional<MeshCollider> meshCollider;
					GetComponent(e, meshCollider);
					RenderDebugMesh(meshCollider, rb.GetRuntimeData().pRigidActor->getGlobalPose());

					Optional<BoxCollider> boxCollider;
					GetComponent(e, boxCollider);
					RenderDebugMesh(boxCollider, rb.GetRuntimeData().pRigidActor->getGlobalPose());					

					Optional<VehicleCollider> vehicleCollider;
					GetComponent(e, vehicleCollider);
					RenderDebugMesh(vehicleCollider, rb.GetRuntimeData().pRigidActor->getGlobalPose());

					Optional<CylinderCollider> cylinderCollider;
					GetComponent(e, cylinderCollider);
					RenderDebugMesh(cylinderCollider, rb.GetRuntimeData().pRigidActor->getGlobalPose());

					Optional<ConeCollider> coneCollider;
					GetComponent(e, coneCollider);
					RenderDebugMesh(coneCollider, rb.GetRuntimeData().pRigidActor->getGlobalPose());
				}

				Debug3D::GetRenderer()->AddSphere(vRigidBodyPos.v3(), 0.15f, Color(0.0f,1.0f,0.0f,0.33f));
			}

			for (auto it = GameComponents<SphereCollider>::GetIterator(); !it.IsEnd(); ++it)
			{
				Required<RigidBody, FromSelfOrParents> rb;
				GetComponent((*it)->GetEntity(), rb);
				const auto& rbPhysXTrans = rb.GetRuntimeData().pRigidActor->getGlobalPose();
				const auto& vRigidBodyPos = Vector4f(ToUsgVec3(rbPhysXTrans.p), 1);
				if (camera.GetFrustum().ArePointsInFrustum(&vRigidBodyPos, 1) && vRigidBodyPos.v3().GetSquaredDistanceFrom(vCamPos) < VisibilityRadius*VisibilityRadius)
				{
					Optional<SphereCollider> sphereCollider;
					GetComponent((*it)->GetEntity(), sphereCollider);
					const auto& sphereLocalTrans = sphereCollider.Force().GetRuntimeData().pShape->getLocalPose();
					RenderDebugMesh(sphereCollider, rbPhysXTrans.transform(sphereLocalTrans));
				}
			}
		}
	}
}