/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/stl/hash_map.h"

namespace physx
{
	class PxPhysics;
	class PxCooking;
	class PxTriangleMesh;
	class PxConvexMesh;
}

namespace usg {

	class PhysXMeshCache
	{
		physx::PxPhysics* m_pPhysics;
		physx::PxCooking* m_pCooking;
		hash_map<uint32, physx::PxTriangleMesh*> m_triangleMeshCache;
		hash_map<uint32, physx::PxConvexMesh*> m_convexMeshCache;

		physx::PxConvexMesh* GenerateCylinder(const Vector3f& vCenter, const Vector3f& vDir, float fRadius, float fHeight, uint32 uCircleVertices);

		physx::PxConvexMesh* GetConvexMesh(const char* szCollisionModelResource, uint32 uBoneNameHash);
	public:
		PhysXMeshCache(physx::PxPhysics* pPhysics, physx::PxCooking* pCooking);

		physx::PxTriangleMesh* GetTriangleMesh(const char* szCollisionModelResource, bool bFlipNormals = false);
		physx::PxConvexMesh* GetConvexMesh(const char* szCollisionModelResource, const char* szMeshName = "");
		physx::PxConvexMesh* GetCylinderMesh(const Vector3f& vCenter, const Vector3f& vDir, float fRadius, float fHeight, uint32 uCircleVertices);

		void preloadConvexMesh(const char* szCollisionModelResource);
	};

}

