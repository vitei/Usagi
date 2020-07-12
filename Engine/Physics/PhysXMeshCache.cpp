/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Framework/GameComponents.h"
#include "Engine/Resource/CollisionModelResource.h"
#include "Engine/Resource/ResourceMgr.h"
#include "PhysXMeshCache.h"
#include "Engine/Core/stl/set.h"
#include "Engine/Core/stl/memory.h"
#include "Engine/Physics/PhysX.h"
#include "Engine/Network/NetVectorPack.h"
#include <EASTL/string.h>

namespace usg
{
	static const uint32 EmptyHash = utl::CRC32("");

	PhysXMeshCache::PhysXMeshCache(physx::PxPhysics* pPhysics, physx::PxCooking* pCooking) : 
		m_pPhysics(pPhysics),
		m_pCooking(pCooking)
	{

	}

	physx::PxTriangleMesh* PhysXMeshCache::GetTriangleMesh(ComponentLoadHandles& handles, const char* szCollisionModelResource, bool bFlipNormals)
	{
		const uint32 uHash = utl::CRC32(szCollisionModelResource) ^ (const uint32)bFlipNormals;
		auto it = m_triangleMeshCache.find(uHash);
		if (it == m_triangleMeshCache.end())
		{
			CollisionModelResHndl handle = handles.pResourceMgr->GetCollisionModel(szCollisionModelResource);
			ASSERT(handle != nullptr);
			const Vector3f* pVerts = handle->GetVertices();
			const uint32 uVertexCount = handle->GetVertexCount();
			const uint32 uTriangleCount = handle->GetTriangleCount();

			physx::PxTriangleMeshDesc meshDesc;

			if (bFlipNormals)
			{
				meshDesc.flags |= physx::PxMeshFlag::eFLIPNORMALS;
			}
			meshDesc.flags &= ~physx::PxMeshFlag::e16_BIT_INDICES;

			meshDesc.points.count = uVertexCount;
			meshDesc.points.stride = sizeof(physx::PxVec3);
			meshDesc.points.data = reinterpret_cast<const physx::PxVec3*>(pVerts);

			meshDesc.triangles.count = uTriangleCount;
			meshDesc.triangles.stride = 3 * sizeof(physx::PxU32);
			meshDesc.triangles.data = reinterpret_cast<const physx::PxU32*>(handle->GetTriangles());

			physx::PxDefaultMemoryOutputStream buf;
			if (!m_pCooking->cookTriangleMesh(meshDesc, buf))
			{
				ASSERT(false && "Failed to generate triangle mesh.");
				return nullptr;
			}

			physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
			physx::PxTriangleMesh* pTriangleMesh = m_pPhysics->createTriangleMesh(input);
			m_triangleMeshCache[uHash] = pTriangleMesh;
			return pTriangleMesh;
		}
		return it->second;
	}

	void PhysXMeshCache::preloadConvexMesh(ComponentLoadHandles& handles, const char* szCollisionModelResource)
	{
		CollisionModelResHndl handle = handles.pResourceMgr->GetCollisionModel(szCollisionModelResource);
		ASSERT(handle != nullptr);
		if (handle != nullptr)
		{
			for (auto& bd : handle->GetSubmeshData())
			{
				GetConvexMesh(handles, szCollisionModelResource, bd.uNameHash);
			}
		}
	}

	physx::PxConvexMesh* PhysXMeshCache::GetConvexMesh(ComponentLoadHandles& handles, const char* szCollisionModelResource, uint32 uBoneNameHash)
	{
		const bool bFetchSubmesh = uBoneNameHash != EmptyHash;
		const uint32 uNameHash = utl::CRC32(szCollisionModelResource) ^ uBoneNameHash;
		auto it = m_convexMeshCache.find(uNameHash);
		if (it == m_convexMeshCache.end())
		{
			CollisionModelResHndl handle = handles.pResourceMgr->GetCollisionModel(szCollisionModelResource);
			ASSERT(handle != nullptr);

			unique_ptr<vector<Vector3f>> boneVertices;
			physx::PxConvexMeshDesc convexDesc;
			convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
			convexDesc.points.stride = sizeof(physx::PxVec3);
			if (!bFetchSubmesh)
			{
				convexDesc.points.count = handle->GetVertexCount();
				convexDesc.points.data = reinterpret_cast<const physx::PxVec3*>(handle->GetVertices());
			}
			else
			{
				set<uint32> boneVerticexIndices;
				const auto boneDataSource = handle->GetBoneData(uBoneNameHash);
				for (const auto& smd : boneDataSource)
				{
					for (size_t uTriangleIndex = smd.uTrianglesBegin; uTriangleIndex < smd.uTrianglesEnd; uTriangleIndex++)
					{
						const auto& tri = handle->GetTriangles()[uTriangleIndex];
						boneVerticexIndices.insert(tri.i1);
						boneVerticexIndices.insert(tri.i2);
						boneVerticexIndices.insert(tri.i3);
					}
				}
				boneVertices.reset(vnew(ALLOC_PHYSICS)vector<Vector3f>());
				for (auto uVertexIndex : boneVerticexIndices)
				{
					boneVertices->push_back(handle->GetVertices()[uVertexIndex]);
				}
				convexDesc.points.count = static_cast<uint32>(boneVertices->size());
				convexDesc.points.data = reinterpret_cast<const physx::PxVec3*>(&(*boneVertices)[0]);
			}
			ASSERT(convexDesc.points.count >= 4 && convexDesc.points.count <= 256 && "These limits are imposed by PhysX");
			unique_ptr<physx::PxDefaultMemoryOutputStream> buf(vnew(ALLOC_PHYSICS) physx::PxDefaultMemoryOutputStream());
			physx::PxConvexMeshCookingResult::Enum result;
			convexDesc.flags |= physx::PxConvexFlag::eCHECK_ZERO_AREA_TRIANGLES;
			if (!m_pCooking->cookConvexMesh(convexDesc, *buf.get(), &result))
			{
				DEBUG_PRINT("Warning: failed to cook convex hull, using AABB as fallback\n");
				buf.reset(vnew(ALLOC_PHYSICS) physx::PxDefaultMemoryOutputStream());

				AABB boundingBox;
				boundingBox.Invalidate();
				const auto pVerts = reinterpret_cast<const physx::PxVec3*>(convexDesc.points.data);
				for (size_t i = 0; i < convexDesc.points.count; i++)
				{
					boundingBox.Apply(ToUsgVec3(pVerts[i]));
				}

				convexDesc.points.count = 8;
				physx::PxVec3 points[8];
				for (memsize i = 0; i < 8; i++)
				{
					points[i].x = ((i & 1) == 0) ? boundingBox.GetMin().x : boundingBox.GetMax().x;
					points[i].y = ((i & 2) == 0) ? boundingBox.GetMin().y : boundingBox.GetMax().y;
					points[i].z = ((i & 4) == 0) ? boundingBox.GetMin().z : boundingBox.GetMax().z;
				}
				convexDesc.points.data = points;
				convexDesc.flags &= ~physx::PxConvexFlag::eCHECK_ZERO_AREA_TRIANGLES;
				if (!m_pCooking->cookConvexMesh(convexDesc, *buf.get(), &result))
				{
					ASSERT(false && "Failed to generate convex mesh.");
					return nullptr;
				}
			}
			physx::PxDefaultMemoryInputData input(buf->getData(), buf->getSize());
			physx::PxConvexMesh* pConvexMesh = m_pPhysics->createConvexMesh(input);
			m_convexMeshCache[uNameHash] = pConvexMesh;
			return pConvexMesh;
		}
		return it->second;
	}

	physx::PxConvexMesh* PhysXMeshCache::GetConvexMesh(ComponentLoadHandles& handles, const char* szCollisionModelResource, const char* szMeshName)
	{
		return GetConvexMesh(handles, szCollisionModelResource, utl::CRC32(szMeshName));
	}

	static uint32 GetCylinderHash(const Vector3f& vCenter, const Vector3f& vDir, float fRadius, float fHeight, uint32 uCircleVertices)
	{
		struct
		{
			Vector3f vCenter;
			Vector3f vDir;
			float fRadius;
			float fHeight;
			uint32 uCircleVertices;
		} data;
		data.vCenter = vCenter;
		data.vDir = vDir;
		data.fRadius = fRadius;
		data.fHeight = fHeight;
		data.uCircleVertices = uCircleVertices;
		return utl::CRC32(&data, sizeof(data), 0);
	}

	physx::PxConvexMesh* PhysXMeshCache::GenerateCylinder(const Vector3f& vCenter, const Vector3f& vDir, float fRadius, float fHeight, uint32 uCircleVertices)
	{
		Vector3f vToCircle;
		Vector3f vOtherVec = Vector3f::Z_AXIS;
		if (DotProduct(vDir, Vector3f::Z_AXIS) > 0.99f)
		{
			vOtherVec = Vector3f::X_AXIS;
		}
		vToCircle = CrossProduct(vDir, vOtherVec).GetNormalised()*fRadius;

		vector<physx::PxVec3> vertices;

		vertices.reserve(2 * uCircleVertices);
		physx::PxConvexMeshDesc convexDesc;
		convexDesc.flags = physx::PxConvexFlag::eCOMPUTE_CONVEX;
		convexDesc.points.stride = sizeof(physx::PxVec3);

		for (memsize uVertexIndex = 0; uVertexIndex < uCircleVertices; uVertexIndex++)
		{
			const float fAngle = (float)uVertexIndex / (float)uCircleVertices*Math::two_pi;
			const Vector3f vOffset = vToCircle*Quaternionf(vDir, fAngle);
			const Vector3f vTopVertex = vCenter + vDir*fHeight*0.5f + vOffset;
			const Vector3f vBottomVertex = vTopVertex - vDir*fHeight;
			vertices.push_back(ToPhysXVec3(vTopVertex));
			vertices.push_back(ToPhysXVec3(vBottomVertex));
		}

		convexDesc.points.count = static_cast<uint32>(vertices.size());
		convexDesc.points.data = &vertices[0];

		physx::PxDefaultMemoryOutputStream buf;
		physx::PxConvexMeshCookingResult::Enum result;
		if (!m_pCooking->cookConvexMesh(convexDesc, buf, &result))
		{
			ASSERT(false && "Failed to generate cylinder.");
			return nullptr;
		}
		physx::PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
		physx::PxConvexMesh* pConvexMesh = m_pPhysics->createConvexMesh(input);
		return pConvexMesh;
	}

	physx::PxConvexMesh* PhysXMeshCache::GetCylinderMesh(const Vector3f& vCenter, const Vector3f& vDir, float fRadius, float fHeight, uint32 uCircleVertices)
	{
		const uint32 uCylinderHash = GetCylinderHash(vCenter, vDir, fRadius, fHeight, uCircleVertices);
		auto it = m_convexMeshCache.find(uCylinderHash);
		if (it != m_convexMeshCache.end())
		{
			return it->second;
		}
		auto pConvexMesh = GenerateCylinder(vCenter, vDir, fRadius, fHeight, uCircleVertices);
		m_convexMeshCache[uCylinderHash] = pConvexMesh;
		return pConvexMesh;
	}

}

