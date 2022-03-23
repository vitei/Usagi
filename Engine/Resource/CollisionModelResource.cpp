/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "CollisionModelResource.h"
#include "Engine/Core/File/File.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Maths/MathUtil.h"
#include "Engine/Maths/Plane.h"
#include "Engine/Debug/Rendering/DebugRender.h"

#include "Engine/Physics/CollisionQuadTree.pb.h"
#include "Engine/Physics/PhysicsComponents.pb.h"
#include "Engine/Physics/CollisionDetection.h"

namespace usg{



	CollisionModelResource::CollisionModelResource() :
		ResourceBase(StaticResType)
	{
		m_pVertices = NULL;
		m_pTriangleNormals = NULL;
		m_pTriangles = NULL;
		m_uTriangles = 0;
		m_fRadius = 0.0f;
	}

	CollisionModelResource::~CollisionModelResource()
	{
		if (m_pVertices)
		{
			mem::Free(MEMTYPE_STANDARD, m_pVertices);
			m_pVertices = NULL;
		}

		if (m_pTriangleNormals)
		{
			mem::Free(MEMTYPE_STANDARD, m_pTriangleNormals);
			m_pTriangleNormals = NULL;
		}

		if (m_pTriangles)
		{
			mem::Free(MEMTYPE_STANDARD, m_pTriangles);
			m_pTriangles = NULL;
		}
	}
#if 0 // deprecated
	bool CollisionModelResource::ClipSphere(const Vector3f &pos, float radius, Intersect &intersect) const
	{
		const float maxDistanceSquared = 10000000.0f;
		float fClosestDistanceSquared = maxDistanceSquared;
		size_t uClosestTriangleIndex = (size_t)(-1);
		for (uint32 i = 0; i < m_uTriangles; i++)
		{
			const TriangleIndices* tris = &m_pTriangles[i];
			const Vector3f &p1 = m_pVertices[tris->i1];
			const Vector3f &p2 = m_pVertices[tris->i2];
			const Vector3f &p3 = m_pVertices[tris->i3];
			const Vector3f &norm = m_pTriangleNormals[i];
			const Vector3f vClosest = ClosestPointOnTriangleToPoint(pos, p1, p2, p3);
			const float fDistanceSquared = vClosest.GetSquaredDistanceFrom(pos);
			if (fDistanceSquared < fClosestDistanceSquared)
			{
				if (fDistanceSquared <= (radius * radius))
				{
					const Vector3f vRel = vClosest - pos;
					intersect.normal = -vRel;
					uClosestTriangleIndex = i;
					intersect.point = vClosest;
					fClosestDistanceSquared = fDistanceSquared;
				}
			}
		}
		if (fClosestDistanceSquared < maxDistanceSquared)
		{
			intersect.normal.Normalise();
			intersect.distance = sqrtf(fClosestDistanceSquared) - radius;
			if (DotProduct(intersect.normal, m_pTriangleNormals[uClosestTriangleIndex]) < 0)
			{
				intersect.normal.x *= -1;
				intersect.normal.y *= -1;
				intersect.normal.z *= -1;
			}
			return true;
		}
		return false;
	}


	bool CollisionModelResource::ClipLine(const Vector3f &vFrom, const Vector3f &vTo, const Vector3f &vDir, float fClosest, Intersect &intersect) const
	{
		float fClosestStart = fClosest;

		for (uint32 i = 0; i < m_uTriangles; i++)
		{

			const TriangleIndices* tris = &m_pTriangles[i];

			const Vector3f &p1 = m_pVertices[tris->i1];
			const Vector3f &p2 = m_pVertices[tris->i2];
			const Vector3f &p3 = m_pVertices[tris->i3];

			const Vector3f &norm = m_pTriangleNormals[i];


			float Vd = DotProduct(norm, vDir);

			if (Vd < 0.0f)
			{
				float D = -DotProduct(norm, p1);

				float Vo = -(DotProduct(norm, vFrom) + D);

				float t = Vo / Vd;

				if (t >= 0.0f && t < fClosest)
				{
					Vector3f tip = vFrom + (vDir*t);

					if (InTriangle(tip, p1, p2, p3))
					{
						intersect.normal = norm;

						intersect.point = tip;

						intersect.distance = D + DotProduct(norm, vTo);

						fClosest = t;
					}
				}
			}
		}

		return fClosest < fClosestStart;
	}



	bool CollisionModelResource::ClipLine(const Vector3f &vFrom, const Vector3f &vTo, Intersect &intersect) const
	{
		Vector3f vDir = vTo - vFrom;

		float fClosest = vDir.Magnitude();

		vDir *= 1.0f / fClosest;

		return ClipLine(vFrom, vTo, vDir, fClosest, intersect);

	}
#endif
	CollisionModelResource::BoneDataSource::BoneDataSource(uint32 uBoneNameHash, const CollisionModelResource& collisionModel) : uBoneNameHash(uBoneNameHash), pCollisionModel(&collisionModel)
	{

	}

	bool CollisionModelResource::BoneDataSource::IsValid() const
	{
		return pCollisionModel->GetSubmeshData(uBoneNameHash) != nullptr;
	}

	CollisionModelResource::BoneDataSource::Iterator CollisionModelResource::BoneDataSource::begin() const
	{
		return CollisionModelResource::BoneDataSource::Iterator(this,0);
	}

	CollisionModelResource::BoneDataSource::Iterator CollisionModelResource::BoneDataSource::end() const
	{
		return CollisionModelResource::BoneDataSource::Iterator(this, CollisionModelResource::BoneDataSource::Iterator::EndTerminator);
	}

	CollisionModelResource::BoneDataSource::Iterator::Iterator(const CollisionModelResource::BoneDataSource* pDataSource, size_t uMeshIndex) : pDataSource(pDataSource), uMeshIndex(uMeshIndex)
	{
		while (uMeshIndex != EndTerminator && !IsValid())
		{
			++(*this);
		}
	}

	bool CollisionModelResource::BoneDataSource::Iterator::IsValid() const
	{
		if (uMeshIndex == EndTerminator)
		{
			return true;
		}
		const auto& submeshData = pDataSource->pCollisionModel->GetSubmeshData();
		ASSERT(uMeshIndex < submeshData.size());
		const uint32 uHash1 = submeshData[uMeshIndex].uNameHash;
		const uint32 uHash2 = pDataSource->uBoneNameHash;
		return uHash1 == uHash2;
	}

	void CollisionModelResource::BoneDataSource::Iterator::IncreaseMeshIndex()
	{
		uMeshIndex++;
		const size_t uMaxMeshIndex = pDataSource->pCollisionModel->GetSubmeshData().size();
		if (uMeshIndex == uMaxMeshIndex)
		{
			uMeshIndex = EndTerminator;
		}
	}

	CollisionModelResource::BoneDataSource::Iterator& CollisionModelResource::BoneDataSource::Iterator::operator++()
	{
		if (uMeshIndex == EndTerminator)
		{
			return *this;
		}
		IncreaseMeshIndex();
		while (!(uMeshIndex == EndTerminator || IsValid()))
		{
			IncreaseMeshIndex();
		}
		return *this;
	}

	CollisionModelResource::SubmeshData CollisionModelResource::BoneDataSource::Iterator::operator*()
	{
		const auto& submeshData = pDataSource->pCollisionModel->GetSubmeshData();
		return submeshData[uMeshIndex];
	}

	CollisionModelResource::BoneDataSource CollisionModelResource::GetBoneData(const char* szBoneName) const
	{
		return GetBoneData(utl::CRC32(szBoneName));
	}

	CollisionModelResource::BoneDataSource CollisionModelResource::GetBoneData(const uint32 uBoneNameHash) const
	{
		return BoneDataSource(uBoneNameHash, *this);
	}

	void CollisionModelResource::Init(const uint32* pData)
	{
		static const uint32 AyatakaMagicNumber = utl::CRC32("AyatakaCollisionModel");

		const uint32 uMagicNumber = pData[0];
		const uint32 uFileVersion = pData[1];
		const uint32 uSubmeshCount = pData[2];
		ASSERT(uMagicNumber == AyatakaMagicNumber && "Not a proper Ayataka-generated collision model file");
		ASSERT(uFileVersion >= 2);

		uint32 uBegin = 0;
		m_submeshData.resize(uSubmeshCount);
		for (uint32 uSubmeshIndex = 0; uSubmeshIndex < uSubmeshCount; uSubmeshIndex++)
		{
			const uint32 uSubmeshTriangles = pData[3 + uSubmeshIndex * 2];
			const uint32 uSubmeshNameHash = pData[3 + uSubmeshIndex * 2 + 1];
			const uint32 uEnd = uBegin + uSubmeshTriangles;
			SubmeshData& data = m_submeshData[uSubmeshIndex];
			data.uTrianglesBegin = uBegin;
			data.uTrianglesEnd = uEnd;
			data.aabb.Invalidate();
			data.uNameHash = uSubmeshNameHash;
			uBegin = uEnd;
		}

		const uint8* pRawData = (const uint8*)pData;
		size_t uPos = sizeof(uint32) * (2 + 1 + uSubmeshCount * 2);

		ALIGNED_VAR(CollisionQuadTreeHeader, FILE_READ_ALIGN, header);
		usg::MemCpy(&header, pRawData + uPos, sizeof(CollisionQuadTreeHeader));

		uPos += sizeof(CollisionQuadTreeHeader);

		m_vMin = header.vMin;
		m_vMax = header.vMax;
		m_vCenter = (m_vMin + m_vMax) * 0.5f;
		m_fRadius = (m_vMax - m_vMin).Magnitude() * 0.5f;

		m_uTriangles = header.uTriangles;
		m_pTriangles = (TriangleIndices*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_COLLISION, sizeof(TriangleIndices) * header.uTriangles, FILE_READ_ALIGN);
		m_pTriangleNormals = (Vector3f*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_COLLISION, sizeof(Vector3f) * header.uTriangles, FILE_READ_ALIGN);
		m_pVertices = (Vector3f*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_COLLISION, sizeof(Vector3f) * header.uVertices, FILE_READ_ALIGN);
		m_uVertices = header.uVertices;

		/*if (header.uTriangles >= 32)
		{
			DEBUG_PRINT("WARNING: using a very complex collision model (%s) with %u triangles, %u vertices.\n", szName, header.uTriangles, header.uVertices);
		}*/

		size_t uNextBlockSize = sizeof(TriangleIndices) * header.uTriangles;
		usg::MemCpy(m_pTriangles, pRawData + uPos, uNextBlockSize);
		uPos += uNextBlockSize;

		uNextBlockSize = sizeof(TriangleIndices) * header.uTriangles;
		// Skip these bytes
		uPos += uNextBlockSize;

		uNextBlockSize = sizeof(Vector3f) * header.uVertices;
		usg::MemCpy(m_pVertices, pRawData + uPos, uNextBlockSize);
		uPos += uNextBlockSize;

		uNextBlockSize = sizeof(Vector3f) * header.uVertices;
		// Skip these bytes
		uPos += uNextBlockSize;

		uNextBlockSize = sizeof(TriangleIndices) * header.uTriangles;
		usg::MemCpy(m_pTriangleNormals, pRawData + uPos, uNextBlockSize);
		uPos += uNextBlockSize;

		SetReady(true);
		UpdateSubmeshAABBs();
	}

	bool CollisionModelResource::Init(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileHeader, const class FileDependencies* pDependencies, const void* pData)
	{
		SetupHash(pFileHeader->szName);

		Init((uint32*)pData);

		return true;
	}

	void CollisionModelResource::Init(const char* szName)
	{

		char name[256];
		str::ParseVariableArgsC(name, 256, "Models/%s", szName);

		File dataFile(name);

		ScratchRaw dataBuffer;
		const size_t uFileSize = dataFile.GetSize();

		dataBuffer.Init(uFileSize, 4);
		dataFile.Read(uFileSize, dataBuffer.GetRawData());

		m_name = szName;
		SetupHash(m_name.c_str());

		uint32* pData = reinterpret_cast<uint32*>(dataBuffer.GetRawData());
		Init(pData);
	}

	const CollisionModelResource::SubmeshData* CollisionModelResource::GetSubmeshData(const char* szName) const
	{
		const uint32 uNameHash = utl::CRC32(szName);
		return GetSubmeshData(uNameHash);
	}

	const CollisionModelResource::SubmeshData* CollisionModelResource::GetSubmeshData(const uint32 uMeshNameHash) const
	{
		for (auto& submeshData : m_submeshData)
		{
			if (submeshData.uNameHash == uMeshNameHash)
			{
				return &submeshData;
			}
		}
		return nullptr;
	}

	void CollisionModelResource::UpdateSubmeshAABBs()
	{
		if (m_submeshData.size() == 1)
		{
			m_submeshData[0].aabb = GetAABB();
		}
		else
		{
			const size_t uSubmeshCount = m_submeshData.size();
			for (auto& data : m_submeshData)
			{
				for (uint32 uTriangleIndex = data.uTrianglesBegin; uTriangleIndex < data.uTrianglesEnd; uTriangleIndex++)
				{
					const auto& tri = m_pTriangles[uTriangleIndex];
					const auto& v1 = m_pVertices[tri.i1];
					const auto& v2 = m_pVertices[tri.i2];
					const auto& v3 = m_pVertices[tri.i3];
					data.aabb.Apply(v1);
					data.aabb.Apply(v2);
					data.aabb.Apply(v3);
				}
			}
		}
	}

}

