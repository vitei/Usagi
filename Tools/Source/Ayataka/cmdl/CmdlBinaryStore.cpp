#include "CmdlBinaryStore.h"

#include <algorithm>

#include "PbUtil.h"
#include "exchange/Mesh.h"
#include "exchange/Material.h"
#include "exchange/Skeleton.h"
#include "common//Vector.h"

#include "Engine/Scene/Model/Model.pb.h"

#include "Engine/Scene/Model/Skeleton.pb.h"


void _calcBoundingSphereOfEntireModel( usg::exchange::ModelHeader* pHeader, const Cmdl& cmdl )
{
	Vector3 vMin = cmdl.GetShapePtr( 0 )->GetAABBMin();
	Vector3 vMax = cmdl.GetShapePtr( 0 )->GetAABBMax();

	int shapeNum = cmdl.GetShapeNum();
	for( int shapeIndex = 1; shapeIndex < shapeNum; ++shapeIndex )
	{
		const ::exchange::Shape* pShape = cmdl.GetShapePtr( shapeIndex );
		vMin.x = std::min( vMin.x, pShape->GetAABBMin().x );
		vMin.y = std::min( vMin.y, pShape->GetAABBMin().y );
		vMin.z = std::min( vMin.z, pShape->GetAABBMin().z );
		vMax.x = std::max( vMax.x, pShape->GetAABBMax().x );
		vMax.y = std::max( vMax.y, pShape->GetAABBMax().y );
		vMax.z = std::max( vMax.z, pShape->GetAABBMax().z );
	}

	// AABB -> BoundingSphere
	Vector3 vCenter = ( vMax + vMin ) / 2;
	pHeader->boundingSphere.center.Assign( vCenter.x, vCenter.y, vCenter.z );
	Vector3 vRadius = vMax - vCenter;
	pHeader->boundingSphere.radius = vRadius.calcLength();
}

CmdlBinaryStore::CmdlBinaryStore( bool bSwapEndian )
{
	mbSwapEndian = bSwapEndian;
}

size_t CmdlBinaryStore::calcStoredBinarySize(const Cmdl& cmdl, size_t alignment)
{
	size_t wholeSize = 0;

	// header size
	wholeSize = calcAlignedSize( sizeof(usg::exchange::ModelHeader), alignment );

	// shapes section size
	size_t alignedShapeSectionSize = calcAlignedSize( sizeof(usg::exchange::Shape), alignment );
	int shapeNum = cmdl.GetShapeNum();
	for( int n = 0; n < shapeNum; ++n )
	{
		const ::exchange::Shape* pShape = cmdl.GetShapePtr( n );

		// Shape struct itself
		wholeSize += alignedShapeSectionSize;

		// Primitives
		const ::exchange::PrimitiveInfo& primInfo = pShape->GetPrimitiveInfo();
		::exchange::Stream* pIndexStream = cmdl.GetStreamPtr( primInfo.indexStreamRefIndex );
		size_t indexStreamSize = pIndexStream->GetLength() * primInfo.indexStreamFormatSize;
		wholeSize += calcAlignedSize( indexStreamSize, alignment ) * 2; // Twice size, because an adjacency stream is included too

		// Vertex streams
		size_t vertexStreamStride = 0;
		uint32_t streamNum = pShape->GetVertexStreamNum();
		uint32_t uStreamAlign = 4;
		for( uint32_t i = 0; i < streamNum; ++i )
		{
			vertexStreamStride += cmdl.GetStreamPtr( pShape->GetVertexStreamInfo(i).refIndex )->GetSingleByteSize();
		}
		::exchange::Stream* pPositionStream = cmdl.GetStreamPtr(pShape->GetPositionStreamRefIndex());
		uint32 uVertexCount = pPositionStream->GetLength() / pPositionStream->GetColumnNum();
		vertexStreamStride = calcAlignedSize(vertexStreamStride, uStreamAlign);
		wholeSize += calcAlignedSize(vertexStreamStride*uVertexCount, alignment );
	}

	// materials section size
	ASSERT_MSG( sizeof(usg::exchange::Material) % 4 == 0, "error" ); // must be 4 byte alignment for packing
	wholeSize += sizeof( usg::exchange::Material ) * cmdl.GetMaterialNum();

	// meshes section size
	ASSERT_MSG( sizeof(usg::exchange::Mesh) % 4 == 0, "error" ); // must be 4 byte alignment for packing
	wholeSize += sizeof( usg::exchange::Mesh ) * cmdl.GetMeshNum();

	// skeleton section size
	if( cmdl.GetSkeleton() )
	{
		wholeSize += sizeof( usg::exchange::Skeleton );
		wholeSize += cmdl.GetSkeleton()->pb().bonesNum * sizeof( usg::exchange::Bone );
	}

	return wholeSize;
}

void CmdlBinaryStore::store(void* pBuffer, size_t size, const Cmdl& cmdl, size_t alignment)
{
	// buffer's alignment check
	ASSERT_MSG( reinterpret_cast<size_t>( pBuffer ) % alignment == 0, "error" );

#ifndef FINAL_BUILD
	// estimated end point
	void* pEndOfBinary = offsetAddress( pBuffer, size );
#endif

	memset( pBuffer, 0, size );

	void* p = pBuffer;
	usg::exchange::ModelHeader* pHeader = reinterpret_cast<usg::exchange::ModelHeader*>( p );
	usg::exchange::ModelHeader_init( pHeader );

	// header
	pHeader->alignment = (uint32_t)alignment;

	ASSERT_MSG( cmdl.GetName().length() < ARRAY_SIZE( pHeader->name ), "too long name"  );
	strncpy( pHeader->name, cmdl.GetName().c_str(), cmdl.GetName().length() );

	pHeader->shapeNum = cmdl.GetShapeNum();
	pHeader->materialNum = cmdl.GetMaterialNum();
	pHeader->meshNum = cmdl.GetMeshNum();
	ASSERT_MSG(cmdl.GetRigidIndices().size() <= usg::exchange::ModelHeader::rigidBoneIndices_max_count, "Too many rigid bone indices");
	ASSERT_MSG(cmdl.GetSmoothIndices().size() <= usg::exchange::ModelHeader::skinnedBoneIndices_max_count, "Too many rigid bone indices");
	pHeader->rigidBoneIndices_count = std::min((pb_size_t)usg::exchange::ModelHeader::rigidBoneIndices_max_count, (pb_size_t)cmdl.GetRigidIndices().size());
	pHeader->skinnedBoneIndices_count = std::min((pb_size_t)usg::exchange::ModelHeader::skinnedBoneIndices_max_count, (pb_size_t)cmdl.GetSmoothIndices().size());

	for (pb_size_t i = 0; i < pHeader->rigidBoneIndices_count; i++)
	{
		pHeader->rigidBoneIndices[i] = (uint32)cmdl.GetRigidIndices()[i];
	}

	for (pb_size_t i = 0; i < pHeader->skinnedBoneIndices_count; i++)
	{
		pHeader->skinnedBoneIndices[i] = (uint32)cmdl.GetSmoothIndices()[i];
	}
	
	p = offsetAndAlignAddress( p, sizeof(usg::exchange::ModelHeader), alignment );


	// shapes section
	pHeader->shapeOffset = (uint32_t)calcOffset( p, pBuffer );
	p = StoreShapes( p, cmdl, alignment );

	// materials section
	pHeader->materialOffset = (uint32_t)calcOffset( p, pBuffer );
	p = StoreMaterials( p, cmdl );

	// meshes section
	pHeader->meshOffset = (uint32_t)calcOffset( p, pBuffer );
	p = StoreMeshes( p, cmdl );

	// skeleton section
	if( cmdl.GetSkeleton() )
	{
		pHeader->skeletonOffset = (uint32_t)calcOffset( p, pBuffer );
		p = StoreSkeleton( p, cmdl );
	}

	_calcBoundingSphereOfEntireModel( pHeader, cmdl );

	// should be at last
	swapEndian_Struct( pHeader, mbSwapEndian );

	// final check
	ASSERT_MSG( (p == pEndOfBinary), "End point is not corresponding" );
}

void* CmdlBinaryStore::StoreShapes(void* p, const Cmdl& cmdl, size_t alignment)
{
	int shapeNum = cmdl.GetShapeNum();
	for( int n = 0; n < shapeNum; ++n ) {
		const ::exchange::Shape* pInterShape = cmdl.GetShapePtr( n );
		p = StoreShape( p, cmdl, pInterShape, alignment );
	}
	return p;
}

void *CmdlBinaryStore::StoreShape(void *p, const Cmdl& cmdl, const ::exchange::Shape *pShape, size_t alignment)
{
	usg::exchange::Shape* pOutputShape = reinterpret_cast<usg::exchange::Shape*>( p );

	// first, copy simply
	*pOutputShape = pShape->pb();

	// primitives info

	const ::exchange::PrimitiveInfo& primInfo = pShape->GetPrimitiveInfo( );
	::exchange::Stream* pIndexStream = cmdl.GetStreamPtr( primInfo.indexStreamRefIndex );

	uint32_t sizeAligned = (uint32_t)calcAlignedSize( pIndexStream->GetLength() * primInfo.indexStreamFormatSize, alignment );
	pOutputShape->primitive.indexStream.sizeAligned = sizeAligned;
	pOutputShape->primitive.indexStream.formatSize = primInfo.indexStreamFormatSize;
	pOutputShape->primitive.indexStream.indexNum = pIndexStream->GetLength();

	pOutputShape->primitive.adjacencyStream.sizeAligned = sizeAligned; // Adjacency stream size is same as index stream

	pOutputShape->primitive.rootBone = primInfo.rootBoneIndex;

		
	pOutputShape->primitive.lodLevel = primInfo.lodLevel;

	int vertexStreamStep = 0;
	size_t vertexStreamSize = 0;

	pOutputShape->attributeMask = pShape->GetVertexAttributeMask();
	pOutputShape->skinningType = (uint32)pShape->GetSkinningType();

	const uint32_t registerAlign = 4;
	pOutputShape->streamInfo_count = 0;
	const uint32_t streamNum = pShape->GetVertexStreamNum();
	for( uint32_t i = 0; i < streamNum; ++i )
	{
		//TODO: I had to put this in as a hack to get Ayataka to build models without crashing
		//      in nsub.  The if statement below was originally an ASSERT_MSG, which failed on
		//      any vertex streams Ayataka doesn't currently support (notable anything to do
		//      with bones/animations).  Now it'll simply skip those streams and carry on,
		//      but eventually we're going to need to add support for it.
		const usg::exchange::VertexStreamInfo info = pShape->GetVertexStreamInfo( i );
		if( info.attribute != usg::exchange::VertexAttribute_NONE )
		{
			const ::exchange::Stream* pStream = cmdl.GetStreamPtr( info.refIndex );

			// FIXME: Confirm this
			vertexStreamStep += pStream->GetSingleByteSize();//pShape->getStreamPtr( i )->getColumnNum();

			uint32 uStreamIndex = pOutputShape->streamInfo_count;

			pOutputShape->streamInfo[uStreamIndex] = info;

			pOutputShape->streamInfo_count++;
		}
	}

	vertexStreamStep = (int)calcAlignedSize(vertexStreamStep, registerAlign);

	::exchange::Stream* pPositionStream = cmdl.GetStreamPtr( pShape->GetPositionStreamRefIndex() );
	const uint32_t vertexNum = pPositionStream->GetLength() / pPositionStream->GetColumnNum();

	vertexStreamSize = vertexStreamStep * vertexNum;

	pOutputShape->vertexStreamSizeAligned = (uint32_t)calcAlignedSize( vertexStreamSize, alignment );
	pOutputShape->vertexNum = vertexNum;
	pOutputShape->streamOffset = (uint32_t)calcAlignedSize( sizeof(usg::exchange::Shape), alignment );

	p = offsetAndAlignAddress( p, sizeof(usg::exchange::Shape), alignment );

	::exchange::Stream* pAdjacencyStream = cmdl.GetStreamPtr( primInfo.adjacencyStreamRefIndex );

	uint32_t indexStreamSizeAligned = pOutputShape->primitive.indexStream.sizeAligned;
	uint32_t adjacencyStreamSizeAligned = pOutputShape->primitive.adjacencyStream.sizeAligned;
	ASSERT_MSG( indexStreamSizeAligned == adjacencyStreamSizeAligned, "error" ); // They should be same

	switch( primInfo.indexStreamFormatSize )
	{
		case sizeof( uint8_t ) :
		{
			CopyIndexStream<uint8_t>( p, pIndexStream );
			p = offsetAddress( p, indexStreamSizeAligned );

			CopyIndexStream<uint8_t>( p, pAdjacencyStream );
			p = offsetAddress( p, adjacencyStreamSizeAligned );
		}
		break;

		case sizeof( uint16_t ) :
		{
			CopyIndexStream<uint16_t>( p, pIndexStream );
			swapEndian_Stream<uint16_t>( p, pIndexStream->GetSize(), mbSwapEndian );
			p = offsetAddress( p, indexStreamSizeAligned );

			CopyIndexStream<uint16_t>( p, pAdjacencyStream );
			swapEndian_Stream<uint16_t>( p, pAdjacencyStream->GetSize(), mbSwapEndian );
			p = offsetAddress( p, adjacencyStreamSizeAligned );
		}
		break;

		case sizeof( uint32_t ) :
		{
			CopyIndexStream<uint32_t>( p, pIndexStream );
			swapEndian_Stream<uint32_t>( p, pIndexStream->GetSize(), mbSwapEndian );
			p = offsetAddress( p, indexStreamSizeAligned );

			CopyIndexStream<uint32_t>( p, pAdjacencyStream );
			swapEndian_Stream<uint32_t>( p, pAdjacencyStream->GetSize(), mbSwapEndian );
			p = offsetAddress( p, adjacencyStreamSizeAligned );
		}
		break;

		default:
			ASSERT_MSG( 0, "error" );
			break;
	}
	//p = offsetAddress( p, pOutputShape->primitives[i].indexStream.sizeAligned );

	// vertex stream itself
	uint8* pOutputVertexStream = reinterpret_cast<uint8*>( p );

	for( uint32_t i = 0; i < streamNum; ++i ) {
		const usg::exchange::VertexStreamInfo info = pShape->GetVertexStreamInfo( i );
		const ::exchange::Stream* pStream = cmdl.GetStreamPtr( info.refIndex );

		uint32_t itemSize = pStream->GetSingleByteSize();
		const uint8* pStreamArray = reinterpret_cast<const uint8*>( pStream->GetStreamArrayPtr() );
		switch (info.elementType)
		{
		case VE_FLOAT:
			CopyVertexStream<float>(pOutputVertexStream, vertexStreamStep, pStreamArray, itemSize, vertexNum);
			break;
		case VE_INT:
			CopyVertexStream<int>(pOutputVertexStream, vertexStreamStep, pStreamArray, itemSize, vertexNum);
			break;
		case VE_BYTE:
			CopyVertexStream<sint8>(pOutputVertexStream, vertexStreamStep, pStreamArray, itemSize, vertexNum);
			break;
		case VE_UBYTE:
			CopyVertexStream<uint8>(pOutputVertexStream, vertexStreamStep, pStreamArray, itemSize, vertexNum);
			break;
		default:
			ASSERT(false);
		}
		pOutputVertexStream += itemSize;
	}

	p = offsetAddress( p, pOutputShape->vertexStreamSizeAligned );

	swapEndian_Struct( pOutputShape, mbSwapEndian );

	return p;
}

void* CmdlBinaryStore::StoreMaterials(void* p, const Cmdl& cmdl)
{
	for( uint32_t matIndex = 0; matIndex < cmdl.GetMaterialNum(); ++matIndex ) {
		const ::exchange::Material* pMaterial = cmdl.GetMaterialPtr( matIndex );
		p = StoreMaterial( p, pMaterial );
	}

	return p;
}

void *CmdlBinaryStore::StoreMaterial(void *p, const ::exchange::Material *pMaterial)
{
	memcpy( p, &pMaterial->pb(), sizeof( usg::exchange::Material ) );


	//swapEndian_Struct( pOutMaterial, mbSwapEndian );
	if( mbSwapEndian )
	{
		swapEndianPB( p, usg::exchange::Material_fields );
	}

	p = offsetAddress( p, sizeof( usg::exchange::Material ) );

	return p;
}

void*CmdlBinaryStore::StoreMeshes(void* p, const Cmdl& cmdl)
{
	for( uint32_t meshIndex = 0; meshIndex < cmdl.GetMeshNum(); ++meshIndex ) {
		const ::exchange::Mesh* pMesh = cmdl.GetMeshPtr( meshIndex );
		p = StoreMesh( p, pMesh );
	}

	return p;
}

void *CmdlBinaryStore::StoreMesh(void *p, const ::exchange::Mesh *pMesh)
{
	memcpy( p, &pMesh->pb(), sizeof( usg::exchange::Mesh ) );

	//swapEndian_Struct( pOutMesh, mbSwapEndian );
	if( mbSwapEndian ) {
		swapEndianPB( p, usg::exchange::Mesh_fields );
	}

	p = offsetAddress( p, sizeof( usg::exchange::Mesh ) );

	return p;
}

void* CmdlBinaryStore::StoreSkeleton( void* p, const Cmdl& cmdl )
{
	// Skeleton
	::exchange::Skeleton* pSkeleton = cmdl.GetSkeleton();
	ASSERT_MSG( pSkeleton != NULL, "Skeleton not found" );

	size_t size = sizeof( usg::exchange::Skeleton );
	memcpy( p, &pSkeleton->pb(), size );

	if( mbSwapEndian ) {
		swapEndianPB( p, usg::exchange::Skeleton_fields );
	}
	p = offsetAddress( p, size );

	// Bones
	size = sizeof( usg::exchange::Bone );
	::exchange::Skeleton::VectorBone::iterator itr = pSkeleton->Bones().begin();
	::exchange::Skeleton::VectorBone::iterator end = pSkeleton->Bones().end();
	while( itr != end ) {
		usg::exchange::Bone& bone = *itr;
		memcpy( p, &bone, size );

		if( mbSwapEndian ) {
			swapEndianPB( p, usg::exchange::Bone_fields );
		}

		p = offsetAddress( p, size );
		++itr;
	}

	return p;
}

template <typename T>
void CmdlBinaryStore::CopyIndexStream( void* p, const ::exchange::Stream* pIndexStream )
{
	T* pTemp = reinterpret_cast<T*>( p );

	size_t size = pIndexStream->GetLength();
	const uint32_t* pIndexStreamArray = reinterpret_cast<const uint32_t*>( pIndexStream->GetStreamArrayPtr() );
	for( size_t i = 0; i < size; ++i ) {
		pTemp[i] = static_cast<T>( pIndexStreamArray[i] );
	}
}

template <typename T>
void CmdlBinaryStore::CopyVertexStream(uint8* pDest, size_t destStep, const uint8* pSrc, size_t vertexElementSize, int vertexNum)
{
	for( int vertexIndex = 0; vertexIndex < vertexNum; ++vertexIndex )
	{
		memcpy( pDest, pSrc, vertexElementSize);
		if(mbSwapEndian)
			swapEndian<T>(pDest, vertexElementSize);
		pDest += (destStep);
		pSrc += vertexElementSize;
	}
}
