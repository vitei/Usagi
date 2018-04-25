#include "Cmdl.h"

#include "exchange/Mesh.h"
#include "exchange/Shape.h"
#include "exchange/Skeleton.h"
#include "common/Vector.h"

inline void crossProduct( float out[], const float v1[], const float v2[], const float v3[] )
{
	float vecA[] = { v2[0] - v1[0], v2[1] - v1[1], v2[2] - v1[2] };
	float vecB[] = { v3[0] - v2[0], v3[1] - v2[1], v3[2] - v2[2] };

	out[0] = ( vecA[1] * vecB[2] ) - ( vecA[2] * vecB[1] );
	out[1] = ( vecA[2] * vecB[0] ) - ( vecA[0] * vecB[2] );
	out[2] = ( vecA[0] * vecB[1] ) - ( vecA[1] * vecB[0] );
}

Cmdl::Cmdl()
{
	const int reserveNum = 32;
	m_vectorShape.reserve( reserveNum );
	m_vectorMaterial.reserve( reserveNum );
	m_vectorMesh.reserve( reserveNum );
	m_vectorStream.reserve( reserveNum );

	m_pSkeleton = NULL;
}

Cmdl::~Cmdl()
{
	DeleteAll( m_vectorShape );
	DeleteAll( m_vectorMaterial );
	DeleteAll( m_vectorMesh );
	DeleteAll( m_vectorStream );

	SAFE_DELETE( m_pSkeleton );
}

void Cmdl::AddShape(::exchange::Shape* p)
{
	m_vectorShape.push_back( p );
}

void Cmdl::AddAnimation(::exchange::Animation* p)
{
	m_vectorAnimation.push_back( p );
}

void Cmdl::AddMaterial(::exchange::Material* p)
{
	m_vectorMaterial.push_back( p );
}

void Cmdl::AddMesh(::exchange::Mesh* p)
{
	m_vectorMesh.push_back( p );
}

void Cmdl::AddStream( ::exchange::Stream* p )
{
	m_vectorStream.push_back( p );
}

void Cmdl::SetSkeleton( ::exchange::Skeleton* p )
{
	ASSERT_MSG( m_pSkeleton == NULL, "overwrite!" );
	m_pSkeleton = p;
}

void Cmdl::ReverseCoordinate()
{
	size_t size = m_vectorShape.size();
	for( size_t i = 0; i < size; ++i ) {
		ReverseCoordinateInt( m_vectorShape.at( i ) );
	}

	if( m_pSkeleton ) {
		m_pSkeleton->ReverseCoordinate();
	}
}

void Cmdl::CalculatePolygonNormal()
{
	size_t size = m_vectorShape.size();
	for( size_t i = 0; i < size; ++i ) {
		CalculatePolygonNormal( m_vectorShape.at( i ) );
	}
}

aya::string Cmdl::GetName() const
{
	return m_stringName;
}

void Cmdl::SetName(const aya::string& value)
{
	m_stringName = value;
}

template <class T>
void Cmdl::DeleteAll( std::vector< T, aya::Allocator<T> >& v )
{
	size_t size = v.size();
	for(size_t n = 0; n < size; ++n ) {
		SAFE_DELETE( v.at( n ) );
	}
	v.clear();
}


uint32_t Cmdl::GetBoneIndexCount(int materialNum)
{
	int count = -1;
	// FIXME: This makes an assumption that the number of bones will match, this isn't necesserily true
	// This whole thing needs refactoring
	for (uint32 i = 0; i < GetMeshNum(); i++)
	{
		::exchange::Mesh* pMesh = GetMeshPtr(i);
		int meshMat = pMesh->GetMaterialRefIndex();
		if (meshMat != materialNum)
		{
			continue;
		}
		int shapeNum = pMesh->GetShapeRefIndex();
		::exchange::Shape* pShape = GetShapePtr(shapeNum);
		int meshBoneCount = pShape->GetBoneIndexCount();
		ASSERT(count == (-1) || count == meshBoneCount);
		count = meshBoneCount;
	}
	ASSERT(count != (-1));

	return count;
}

void Cmdl::ReverseCoordinateInt(::exchange::Shape* pShape)
{
	uint32_t streamNum = pShape->GetVertexStreamNum();
	for( uint32_t i = 0; i < streamNum; ++i ) {
		usg::exchange::VertexStreamInfo info = pShape->GetVertexStreamInfo( i );
		usg::exchange::VertexAttribute attrType = (usg::exchange::VertexAttribute)info.attribute;
		if( attrType == usg::exchange::VertexAttribute_POSITION ||
			attrType == usg::exchange::VertexAttribute_NORMAL ||
			attrType == usg::exchange::VertexAttribute_TANGENT ||
			attrType == usg::exchange::VertexAttribute_BINORMAL ) {
			ReverseCoordinateInt( *m_vectorStream.at( info.refIndex ) );
		}
	}

	ReverseCoordinateInt( pShape->RefCalculatedNormalStream() );

	// Single attributes
	uint32_t attrIndex = 0;
	if( ( attrIndex = pShape->SearchSingleAttribute( usg::exchange::VertexAttribute_NORMAL ) ) != UINT32_MAX ) {
		usg::exchange::SingleAttribute& attr = pShape->pb().singleAttributes[attrIndex];
		attr.value.z *= -1.0f;
	}
	if( ( attrIndex = pShape->SearchSingleAttribute( usg::exchange::VertexAttribute_TANGENT ) ) != UINT32_MAX ) {
		usg::exchange::SingleAttribute& attr = pShape->pb().singleAttributes[attrIndex];
		attr.value.z *= -1.0f;
	}
	if ((attrIndex = pShape->SearchSingleAttribute(usg::exchange::VertexAttribute_BINORMAL)) != UINT32_MAX) {
		usg::exchange::SingleAttribute& attr = pShape->pb().singleAttributes[attrIndex];
		attr.value.z *= -1.0f;
	}

	// switch direction and exchange each other
	Vector3 min = pShape->GetAABBMin();
	Vector3 max = pShape->GetAABBMax();
	float newMaxZ = min.z *= -1.0f;
	float newMinZ = max.z *= -1.0f;
	min.z = newMinZ;
	max.z = newMaxZ;
	pShape->SetAABB( min, max );

	pShape->pb().boundingSphere.center.z *= -1.0f;
}

void Cmdl::ReverseCoordinateInt(::exchange::Stream& stream)
{
	if( stream.IsAllocated() ) {
		size_t step = stream.GetColumnNum();
		float* p = reinterpret_cast<float*>( stream.GetStreamArrayPtr() );

		size_t length = stream.GetSize() / sizeof( float );

		ASSERT_MSG( length % step == 0, "error" );
		length /= step;

		for( size_t i = 0; i < length; ++i ) {
			p[(i * step) + 2] *= -1.0f;
		}
	}
}

void Cmdl::CalculatePolygonNormal(::exchange::Shape* pShape)
{
	const ::exchange::Stream* pIndexStream = m_vectorStream.at( pShape->GetPrimitiveInfo().indexStreamRefIndex );
	const int indexNum = pIndexStream->GetLength();//pShape->getIndexNum();
	const uint32_t* pIndexStreamArray = reinterpret_cast<const uint32_t*>( pIndexStream->GetStreamArrayPtr() );

	const ::exchange::Stream* pPositionStream = m_vectorStream.at( pShape->GetPositionStreamRefIndex() );
	const float* pPositionStreamArray = reinterpret_cast<const float*>( pPositionStream->GetStreamArrayPtr() );

	pShape->RefCalculatedNormalStream().allocate<float>( indexNum, 3 ); // 1 triangle has 1 normal.
	float* pCalculatedNormalStreamArray = reinterpret_cast<float*>( pShape->RefCalculatedNormalStream().GetStreamArrayPtr() );


	for( int i = 0; i < indexNum; i += 3 ) {
		unsigned int index1 = pIndexStreamArray[i];
		unsigned int index2 = pIndexStreamArray[i+1];
		unsigned int index3 = pIndexStreamArray[i+2];

		const float* pV1 = &pPositionStreamArray[index1 * 3];
		const float* pV2 = &pPositionStreamArray[index2 * 3];
		const float* pV3 = &pPositionStreamArray[index3 * 3];
//		printf( "V1 index:%3d x:%5.2f y:%5.2f z:%5.2f\n", index1, pV1[0], pV1[1], pV1[2] );
//		printf( "V2 index:%3d x:%5.2f y:%5.2f z:%5.2f\n", index2, pV2[0], pV2[1], pV2[2] );
//		printf( "V3 index:%3d x:%5.2f y:%5.2f z:%5.2f\n", index3, pV3[0], pV3[1], pV3[2] );

		float cross[3];
		crossProduct( cross, pV1, pV2, pV3 );

		Vector3 normal( cross[0], cross[1], cross[2] );
		normal.normalize();

		pCalculatedNormalStreamArray[i] = normal.x;
		pCalculatedNormalStreamArray[i + 1] = normal.y;
		pCalculatedNormalStreamArray[i + 2] = normal.z;
//		printf( "N i:%d x:%5.2f y:%5.2f z:%5.2f\n", i, normal.x, normal.y, normal.z );
	}
}
