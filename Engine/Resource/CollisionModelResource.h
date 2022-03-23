/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The data for a collidable model
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_COLLISION_MODEL_RESOURCE_H_
#define _USG_GRAPHICS_SCENE_COLLISION_MODEL_RESOURCE_H_

#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Scene/TransformNode.h"
#include "Engine/Resource/ResourceBase.h"
#include "Engine/Maths/AABB.h"
#include "Engine/Core/stl/vector.h"

namespace vmdc {
struct Header;
}

namespace usg{

class CollisionModelResource : public ResourceBase
{
public:
	using IndexType = uint32;

	struct TriangleIndices
	{
		IndexType i1;
		IndexType i2;
		IndexType i3;
	};


	struct Intersect
	{
		Vector3f point;
		Vector3f normal;
		float distance;
	};

	struct SubmeshData
	{
		uint32 uNameHash;
		uint32 uTrianglesBegin;
		uint32 uTrianglesEnd;
		AABB aabb;
	};

	CollisionModelResource();
	virtual ~CollisionModelResource();
	void Init(const char* szName);
	bool Init(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileHeader, const class FileDependencies* pDependencies, const void* pData);

#if 0 // deprecated
	bool ClipLine(const Vector3f &p1, const Vector3f &p2, Intersect &intersect) const;
	bool ClipLine(const Vector3f &vFrom, const Vector3f &vTo, const Vector3f &vDir, float fClosest, Intersect &intersect) const;
	bool ClipSphere(const Vector3f &pos, float radius, Intersect &intersect) const;
#endif
	const Vector3f& GetCenter() const { return m_vCenter; }
	float GetRadius() const { return m_fRadius;  }
	AABB GetAABB() const
	{
		AABB r;
		r.SetMinMax(m_vMin, m_vMax);
		return r;
	}

	uint32 GetVertexCount() const { return m_uVertices; }
	const Vector3f* GetVertices() const { return m_pVertices; }

	uint32 GetTriangleCount() const { return m_uTriangles; }
	const TriangleIndices* GetTriangles() const { return m_pTriangles; }

	struct BoneDataSource
	{
		uint32 uBoneNameHash;
		const CollisionModelResource* pCollisionModel;
		BoneDataSource(uint32 uBoneNameHash, const CollisionModelResource& collisionModel);
		struct Iterator
		{
		private:
			void IncreaseMeshIndex();
		public:
			static constexpr size_t EndTerminator = 0xffffffff;
			const BoneDataSource* pDataSource;
			size_t uMeshIndex;
			CollisionModelResource::SubmeshData operator*();
			Iterator(const BoneDataSource* pDataSource, size_t uMeshIndex);
			Iterator& operator++();
			bool IsValid() const;
			bool operator!=(const Iterator& o) { return !(*this == o); }
			bool operator==(const Iterator& o) { return pDataSource == o.pDataSource && uMeshIndex == o.uMeshIndex;	}
		};
		Iterator begin() const;
		Iterator end() const;
		bool IsValid() const;
	};

	const vector<SubmeshData>& GetSubmeshData() const { return m_submeshData;  }

	BoneDataSource GetBoneData(const char* szBoneName) const;
	BoneDataSource GetBoneData(const uint32 uBoneNameHash) const;

	const static ResourceType StaticResType = ResourceType::COLLISION;

private:
	void Init(const uint32* pData);
	const SubmeshData* GetSubmeshData(const char* szName) const;
	const SubmeshData* GetSubmeshData(const uint32 uMeshNameHash) const;
	void UpdateSubmeshAABBs();

	usg::string			m_name;
	uint32				m_uTriangles;
	TriangleIndices*	m_pTriangles;
	float				m_fRadius;
	Vector3f			m_vMin;
	Vector3f			m_vMax;
	Vector3f			m_vCenter;
	Vector3f*			m_pVertices;
	uint32				m_uVertices;
	Vector3f*			m_pTriangleNormals;
	vector<SubmeshData>	m_submeshData;
};

}

#endif	// #ifndef _USG_GRAPHICS_SCENE_COLLISION_MODEL_RESOURCE_H_
