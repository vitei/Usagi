#include "CollisionStore.h"

#include <stdint.h>
#include <algorithm>
#include <float.h>
#include <set>
#include "common.h"
#include "endian.h"
#include "exchange/Shape.h"
#include "exchange/Material.h"
#include "exchange/Mesh.h"
#include "Engine/Core/Utility.h"
#include "Engine/Physics/CollisionQuadTree.pb.h"

typedef uint32_t IndexFormat;
typedef uint32_t MaterialIndexFormat;

// CollisionHeader
//-----
// IndexStream
//-----
// AdjacencyStream
//-----
// PositionStream
//-----
// NormalStream (perVertex)
//-----
// NormalStream (perTriangle)

CollisionStore::CollisionStore(bool bSwapEndian)
{
	m_bSwapEndian = bSwapEndian;
	m_contextOffset = 0;
}

size_t CollisionStore::CalcStoredBinarySize(const Cmdl &cmdl)
{
	size_t size = sizeof(usg::CollisionQuadTreeHeader) + 2*sizeof(uint32); // Header + magic number + file version

	const int meshNum = cmdl.GetMeshNum();

	size += sizeof(uint32)*(1 + (size_t)(meshNum*2)); // Submesh data (submesh count and last triangle index and namehash of each submesh)

	for( int meshIndex = 0; meshIndex < meshNum; ++meshIndex ) {
		const ::exchange::Mesh* pMesh = cmdl.GetMeshPtr( meshIndex );

		size += CalcShapeBinarySize( cmdl, cmdl.GetShapePtr( pMesh->GetShapeRefIndex() ) );
	}

	return size;
}

inline bool _isSameVertex(const usg::Vector3f& vert0, const usg::Vector3f& vert1)
{
	return vert1.GetSquaredDistanceFrom(vert0) <= FLT_EPSILON;
}


typedef std::list< uint32_t, aya::Allocator<uint32_t> > WorkType;

void onlyDoubled(WorkType& listA, WorkType& listB, uint32_t host)
{
	WorkType::iterator itrA = listA.begin();
	WorkType::iterator endA = listA.end();

	while (itrA != endA)
	{
		if (*itrA == host)
		{
			itrA = listA.erase(itrA);
			continue;
		}

		bool bUnique = true;
		WorkType::iterator itrB = listB.begin();
		WorkType::iterator endB = listB.end();
		while (itrB != endB)
		{
			if (*itrB == *itrA)
			{
				bUnique = false;
				break;
			}
			++itrB;
		}

		if (bUnique)
		{
			itrA = listA.erase(itrA);
			continue;
		}

		++itrA;
	}
}

void setupAdjacencyStream(uint32_t* pAdjacencyStream,
	const uint32_t* pIndexStream, uint32_t colNum, uint32_t len, uint32_t vertexNum)
{
	WorkType* pWork = vnew(ALLOC_OBJECT) WorkType[vertexNum];

	// PASS 1
	// make a list of polygons' indices that include particular vertex
	uint32_t* pAdjacencies = pAdjacencyStream;
	const uint32_t* pIndices = pIndexStream;
	uint32_t columnNum = colNum;
	uint32_t num = len / columnNum;
	for (uint32_t poly = 0; poly < num; ++poly)
	{
		for (uint32_t vert = 0; vert < columnNum; ++vert)
		{
			uint32_t i = pIndices[poly * columnNum + vert];
			ASSERT_MSG(i < vertexNum, "error");
			pWork[i].push_back(poly);
		}
	}

	// PASS 2
	for (uint32_t poly = 0; poly < num; ++poly)
	{
		for (uint32_t vert = 0; vert < columnNum; ++vert)
		{
			uint32_t indexOffsetA = poly * columnNum + vert;
			uint32_t indexOffsetB = poly * columnNum + ((vert + 1) % columnNum);

			uint32_t vertA = pIndices[indexOffsetA];
			uint32_t vertB = pIndices[indexOffsetB];

			WorkType listA = pWork[vertA];
			WorkType listB = pWork[vertB];

			if (vertA == vertB)
			{
				// degenerate triangle
				listA.clear();
				listB.clear();
			}
			else
			{
				onlyDoubled(listA, listB, poly);
				onlyDoubled(listB, listA, poly);

				if (listA.size() != listB.size()) {
					// degenerate triangle
					listA.clear();
					listB.clear();
				}
			}

#if 0
			DEBUG_PRINTF("poly:%d\n", poly);
			DEBUG_PRINTF("vertA:%4d list:", vertA);
			WorkType::iterator itr = listA.begin();
			WorkType::iterator end = listA.end();
			while (itr != end) {
				DEBUG_PRINTF("%4d ", *itr);
				++itr;
			}
			DEBUG_PRINTF("\n");

			DEBUG_PRINTF("vertB:%4d list:", vertB);
			itr = listB.begin();
			end = listB.end();
			while (itr != end) {
				DEBUG_PRINTF("%4d ", *itr);
				++itr;
			}
			DEBUG_PRINTF("\n");
#endif
			ASSERT_MSG(listA.size() == listB.size(), "something wrong");

			if (listA.size() > 0)
			{
				// ignore index 1 or after, if it exists
				pAdjacencies[indexOffsetA] = listA.front();
			}
			else
			{
				// doesn't have adjacent polygon
				pAdjacencies[indexOffsetA] = UINT32_MAX;
			}
		}
	}

#if 0
	DEBUG_PRINTF("Adjacencies\n");
	for (uint32_t i = 0; i < pAdjacencyStream->getLength(); ++i) {
		DEBUG_PRINTF("%u\n", pAdjacencies[i]);
	}
#endif

	SAFE_DELETE_ARRAY(pWork);
}



void _merge( usg::CollisionQuadTreeHeader* pHeader, IndexFormat* pIndexStreamHead,
			 IndexFormat* pAdjacencyStreamHead, float* pPositionStreamHead )
{
	const int triNum = pHeader->uTriangles;
	const uint32_t VERTS_PER_TRIANGLE = 3;
	uint32_t indicesNum = triNum * VERTS_PER_TRIANGLE;

	std::vector<bool> skipper;
	skipper.resize(indicesNum, false);
	for( uint32_t i = 0; i < indicesNum - 1; ++i ) {
		if (skipper[i])
		{
			continue;
		}
		for( uint32_t j = i + 1; j < indicesNum; ++j )
		{
			const float32 dx = pPositionStreamHead[pIndexStreamHead[i] * 3] - pPositionStreamHead[pIndexStreamHead[j] * 3];
			if (dx*dx > FLT_EPSILON)
			{
				continue;
			}

			const float32 dy = pPositionStreamHead[pIndexStreamHead[i] * 3 + 1] - pPositionStreamHead[pIndexStreamHead[j] * 3 + 1];
			if (dy*dy > FLT_EPSILON)
			{
				continue;
			}

			const float32 dz = pPositionStreamHead[pIndexStreamHead[i] * 3 + 2] - pPositionStreamHead[pIndexStreamHead[j] * 3 + 2];
			if (dz*dz > FLT_EPSILON)
			{
				continue;
			}
			pIndexStreamHead[j] = pIndexStreamHead[i]; // Update
			skipper[j] = 1;
		}
	}

	uint32_t colNum = VERTS_PER_TRIANGLE;
	uint32_t len = pHeader->uTriangles * colNum;
	uint32_t vertexNum = pHeader->uVertices;
	setupAdjacencyStream( pAdjacencyStreamHead, pIndexStreamHead, colNum, len, vertexNum );
}

void CollisionStore::Store(void *pBuffer, size_t size, const Cmdl &cmdl)
{
	constexpr uint32 FileVersion = 2;

#ifndef FINAL_BUILD
	// estimated end point
	void* pEndOfBinary = offsetAddress( pBuffer, size );
#endif

	memset( pBuffer, 0, size );
	void* p = pBuffer;

	// magic number + file version
	uint32* pMagicNumber = reinterpret_cast<uint32*>(p);
	*pMagicNumber = utl::CRC32("AyatakaCollisionModel");
	p = offsetAddress(p, sizeof(uint32));
	uint32* pFileVersion = reinterpret_cast<uint32*>(p);
	*pFileVersion = FileVersion;
	p = offsetAddress(p, sizeof(uint32));

	// submesh data
	uint32* submeshData = reinterpret_cast<uint32*>(p);
	submeshData[0] = (uint32)cmdl.GetMeshNum();
	p = offsetAddress(p, sizeof(uint32)*(1 + cmdl.GetMeshNum()*2));

	// header
	usg::CollisionQuadTreeHeader* pHeader = reinterpret_cast<usg::CollisionQuadTreeHeader*>( p );
	usg::CollisionQuadTreeHeader_init( pHeader );
	pHeader->vMin.Assign( FLT_MAX, FLT_MAX, FLT_MAX );
	pHeader->vMax.Assign( FLT_MIN, FLT_MIN, FLT_MIN );
	pHeader->uMaterials = cmdl.GetMaterialNum();
	p = offsetAddress( p, sizeof( usg::CollisionQuadTreeHeader ) );

	// traverse all meshes
	int meshNum = cmdl.GetMeshNum();

	IndexFormat* pIndexStreamHead = reinterpret_cast<IndexFormat*>( p );

	uint32_t indexOffset = 0;
	for( int meshIndex = 0; meshIndex < meshNum; ++meshIndex ) {
		const ::exchange::Shape* pShape = cmdl.GetShapePtr( cmdl.GetMeshPtr( meshIndex )->GetShapeRefIndex() );

		const ::exchange::Stream* pPositionStream = cmdl.GetStreamPtr( pShape->GetPositionStreamRefIndex() );
		uint32_t vertexNum = pPositionStream->GetLength() / pPositionStream->GetColumnNum();

		const ::exchange::PrimitiveInfo& primInfo = pShape->GetPrimitiveInfo();
		const ::exchange::Stream* pIndexStream = cmdl.GetStreamPtr( primInfo.indexStreamRefIndex );

		// store index
		p = StoreIndexStream( p, pIndexStream, indexOffset );
		indexOffset += vertexNum;

		// update AABB size
		pHeader->vMin.x = std::min( pHeader->vMin.x, pShape->GetAABBMin().x );
		pHeader->vMin.y = std::min( pHeader->vMin.y, pShape->GetAABBMin().y );
		pHeader->vMin.z = std::min( pHeader->vMin.z, pShape->GetAABBMin().z );

		pHeader->vMax.x = std::max( pHeader->vMax.x, pShape->GetAABBMax().x );
		pHeader->vMax.y = std::max( pHeader->vMax.y, pShape->GetAABBMax().y );
		pHeader->vMax.z = std::max( pHeader->vMax.z, pShape->GetAABBMax().z );

		// accumulate the number of triangles and vertices
		const uint32 uTriangleNum = pIndexStream->GetLength() / 3;
		pHeader->uTriangles += uTriangleNum;
		pHeader->uVertices += vertexNum;

		submeshData[2 * meshIndex + 1] = uTriangleNum;
		const uint32 uNameHash = utl::CRC32(cmdl.GetMeshPtr(meshIndex)->GetName());
		submeshData[2 * meshIndex + 1 + 1] = uNameHash;
	}


	m_contextOffset = 0;
	IndexFormat* pAdjacencyStreamHead = reinterpret_cast<IndexFormat*>( p );
	p = TraverseMesh( &CollisionStore::StoreAdjacencyStream, p, cmdl );
	float* pPositionStreamHead = reinterpret_cast<float*>( p );
	p = TraverseMesh( &CollisionStore::StorePositionStream, p, cmdl );

	_merge( pHeader, pIndexStreamHead, pAdjacencyStreamHead, pPositionStreamHead );

	p = TraverseMesh( &CollisionStore::StoreNormalStreamPerVertex, p, cmdl );
	p = TraverseMesh( &CollisionStore::StoreNormalStreamPerTriangle, p, cmdl );
	p = TraverseMesh( &CollisionStore::StoreMaterialIndexStream, p, cmdl );

	// swap endian if need
	if( m_bSwapEndian ) {
		swapEndian<uint32_t>( pBuffer, size );
	}

	// final check
	ASSERT_MSG( p == pEndOfBinary, "End point is not corresponding" );
}

size_t CollisionStore::CalcNamesSectionBinarySize( const Cmdl& cmdl )
{
	size_t size = 0;
	int matNum = cmdl.GetMaterialNum();
	for( int matIndex = 0; matIndex < matNum; ++matIndex ) {
		const usg::exchange::Material& material = cmdl.GetMaterialPtr( matIndex )->pb();
		size_t matNameSize = ARRAY_SIZE( material.materialName );
		ASSERT_MSG( matNameSize == usg::CollisionQuadTreeConstants_MATERIAL_NAME_LENGTH, "Material name length is not correspond" );
		size += matNameSize;
	}
	return size;
}

void CollisionStore::StoreNamesSection( void* p, size_t size, const Cmdl& cmdl )
{
#ifndef FINAL_BUILD
	void* pEndOfBinary = offsetAddress( p, size );
#endif

	memset( p, 0, size );
	p = TraverseMaterial( &CollisionStore::StoreMaterialAttributes, p, cmdl );

	// final check
	ASSERT_MSG( p == pEndOfBinary, "End point is not corresponding" );
}

size_t CollisionStore::CalcShapeBinarySize( const Cmdl& cmdl, const ::exchange::Shape* pShape )
{
	size_t size = 0;

	const ::exchange::PrimitiveInfo& primInfo = pShape->GetPrimitiveInfo();
	const ::exchange::Stream* pIndexStream = cmdl.GetStreamPtr( primInfo.indexStreamRefIndex );
	const ::exchange::Stream* pAdjacencyStream = cmdl.GetStreamPtr( primInfo.adjacencyStreamRefIndex );
	const ::exchange::Stream* pPositionStream = cmdl.GetStreamPtr( pShape->GetPositionStreamRefIndex() );

	size += pIndexStream->GetLength() * sizeof( uint32_t );
	size += pAdjacencyStream->GetLength() * sizeof( uint32_t );
	size += pPositionStream->GetSize();
	size += pPositionStream->GetSize();
	size += pShape->RefCalculatedNormalStream().GetSize();
	size += ( pIndexStream->GetLength() / 3 ) * sizeof( MaterialIndexFormat );

	return size;
}

void* CollisionStore::TraverseMesh( MeshProcessFunc func, void* p, const Cmdl& cmdl )
{
	const int meshNum = cmdl.GetMeshNum();
	for( int meshIndex = 0; meshIndex < meshNum; ++meshIndex ) {
		p = (this->*func)( p, cmdl, meshIndex );
	}
	return p;
}

void* CollisionStore::TraverseMaterial( MaterialProcessFunc func, void* p, const Cmdl& cmdl )
{
	const int materialNum = cmdl.GetMaterialNum();
	for( int materialIndex = 0; materialIndex < materialNum; ++materialIndex ) {
		const ::exchange::Material* pMaterial = cmdl.GetMaterialPtr( materialIndex );
		p = ( this->*func )( p, pMaterial );
	}
	return p;
}

void* CollisionStore::StoreIndexStream( void* p, const ::exchange::Stream* pIndexStream, uint32_t indexOffset )
{
	IndexFormat* pIndexStreamStore = reinterpret_cast<IndexFormat*>( p );

	uint32_t size = pIndexStream->GetLength();
	const uint32_t* pIndexStreamArray = reinterpret_cast<const uint32_t*>( pIndexStream->GetStreamArrayPtr() );
	for( uint32_t i = 0; i < size; ++i ) {
		*pIndexStreamStore = pIndexStreamArray[i] + indexOffset;
		++pIndexStreamStore;
	}

	return offsetAddress( p, sizeof( IndexFormat ) * pIndexStream->GetLength() );
}

void* CollisionStore::StoreAdjacencyStream( void* p, const Cmdl& cmdl, uint32_t meshIndex )
{
	const ::exchange::Shape* pShape = cmdl.GetShapePtr( cmdl.GetMeshPtr( meshIndex )->GetShapeRefIndex() );
	const ::exchange::PrimitiveInfo& primInfo = pShape->GetPrimitiveInfo();

	const ::exchange::Stream* pAdjacencyStream = cmdl.GetStreamPtr( primInfo.adjacencyStreamRefIndex );

	// Adjacency stream is virtually index stream of triangles
	p = StoreIndexStream( p, pAdjacencyStream, m_contextOffset );

	m_contextOffset += pAdjacencyStream->GetLength() / pAdjacencyStream->GetColumnNum();
	return p;
}

void* CollisionStore::StorePositionStream( void* p, const Cmdl& cmdl, uint32_t meshIndex )
{
	const ::exchange::Shape* pShape = cmdl.GetShapePtr( cmdl.GetMeshPtr( meshIndex )->GetShapeRefIndex() );

	const ::exchange::Stream* pPosStream = cmdl.GetStreamPtr( pShape->GetPositionStreamRefIndex() );
	memcpy( p, pPosStream->GetStreamArrayPtr(), pPosStream->GetSize() );
	return offsetAddress( p, pPosStream->GetSize() );
}

void* CollisionStore::StoreNormalStreamPerVertex( void* p, const Cmdl& cmdl, uint32_t meshIndex )
{
	const ::exchange::Shape* pShape = cmdl.GetShapePtr( cmdl.GetMeshPtr( meshIndex )->GetShapeRefIndex() );

	uint32_t index = pShape->GetNormalStreamRefIndex();
	if( index == UINT32_MAX ) {
		index = pShape->SearchSingleAttribute( usg::exchange::VertexAttribute_NORMAL );
		ASSERT_MSG( index != UINT32_MAX, "Normal attribute doesn't exist" );
		usg::Vector4f normal = pShape->pb().singleAttributes[index].value;
		size_t unitSize = sizeof( float ) * 3; // Normal has only 3 float-values
		
		// Copy same size as position stream
		const ::exchange::Stream* pPosStream = cmdl.GetStreamPtr( pShape->GetPositionStreamRefIndex() );
		uint32_t normalNum = pPosStream->GetLength() / pPosStream->GetColumnNum();
		for( uint32_t i = 0; i < normalNum; ++i ) {
			memcpy( p, normal.m_xyzw, unitSize );
			p = offsetAddress( p, unitSize );
		}
	}
	else {
		const ::exchange::Stream* pNormalStream = cmdl.GetStreamPtr( index );
		memcpy( p, pNormalStream->GetStreamArrayPtr(), pNormalStream->GetSize() );
		p = offsetAddress( p, pNormalStream->GetSize() );
	}
	return p;
}

void* CollisionStore::StoreNormalStreamPerTriangle( void* p, const Cmdl& cmdl, uint32_t meshIndex )
{
	const ::exchange::Shape* pShape = cmdl.GetShapePtr( cmdl.GetMeshPtr( meshIndex )->GetShapeRefIndex() );

	const ::exchange::Stream& normalStream = pShape->RefCalculatedNormalStream();
	memcpy( p, normalStream.GetStreamArrayPtr(), normalStream.GetSize() );
	return offsetAddress( p, normalStream.GetSize() );
}

void* CollisionStore::StoreMaterialIndexStream( void* p, const Cmdl& cmdl, uint32_t meshIndex )
{
	const ::exchange::Mesh* pMesh = cmdl.GetMeshPtr( meshIndex );
	const ::exchange::Shape* pShape = cmdl.GetShapePtr( pMesh->GetShapeRefIndex() );

	uint32_t matIndex = pMesh->GetMaterialRefIndex();
	uint32_t triangleNum = cmdl.GetStreamPtr( pShape->GetPrimitiveInfo().indexStreamRefIndex )->GetLength() / 3;

	MaterialIndexFormat* pStream = reinterpret_cast<uint32_t*>( p );
	for( uint32_t i = 0; i < triangleNum; ++i ) {
		*pStream = matIndex;
		++pStream;
	}
	return offsetAddress( p, sizeof( MaterialIndexFormat ) * triangleNum );
}

void* CollisionStore::StoreMaterialAttributes( void* p, const ::exchange::Material* pMaterial )
{
	size_t size = ARRAY_SIZE( pMaterial->pb().materialName );

	memset( p, 0, size );
	strcpy( reinterpret_cast<char*>( p ), pMaterial->pb().materialName );

	return offsetAddress( p, size );
}
