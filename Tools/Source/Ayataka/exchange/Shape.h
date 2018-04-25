#ifndef SHAPE_H
#define SHAPE_H

#include <stdio.h>
#include <math.h>
#include <limits>

#include "common.h"
#include "common/Vector.h"

#include "Engine/Scene/Model/Shape.pb.h"
#include "Stream.h"

using namespace usg;

namespace exchange {

// 

typedef std::vector< usg::exchange::VertexStreamInfo, aya::Allocator<usg::exchange::VertexStreamInfo> > VectorVertexStreamInfo;

// FIXME: No sub-meshes for shapes, that's a 3DS limitation
struct PrimitiveInfo
{
	uint32_t indexStreamRefIndex;
	uint32_t indexStreamFormatSize;
	uint32_t rootBoneIndex;
	char rootBoneName[40];

	usg::exchange::SkinningType eSkinType;
	uint32_t adjacencyStreamRefIndex;
	uint32_t lodLevel;
};
inline void PrimitiveInfo_init( PrimitiveInfo& r ) {
	r.indexStreamRefIndex = UINT32_MAX;
	r.indexStreamFormatSize = UINT32_MAX;
	memset( r.rootBoneName, 0, sizeof( r.rootBoneName) );
	r.adjacencyStreamRefIndex = UINT32_MAX;
	r.lodLevel = 0;
}

class Shape
{
public:
	Shape() {
		Shape_init( &m_shape );
	}

	virtual ~Shape() {
	}

	Stream& RefCalculatedNormalStream( void ) { return m_calculatedNormalStream; }
	const Stream& RefCalculatedNormalStream( void ) const { return m_calculatedNormalStream; }

	uint32_t GetVertexAttributeMask() const;
	usg::exchange::SkinningType GetSkinningType() const;

	::Vector3 GetAABBMin( void ) const;
	::Vector3 GetAABBMax( void ) const;
	void SetAABB( ::Vector3 min, ::Vector3 max );

	uint32_t SearchStream( usg::exchange::VertexAttribute attrType ) const;
	uint32_t GetPositionStreamRefIndex( void ) const;
	uint32_t GetNormalStreamRefIndex( void ) const;
	
	static void Copy( Shape& rDest, const Shape& rSrc, uint32_t attrFlag );
	bool HasVertexAlpha() const;
	uint32 GetBoneIndexCount() const;

	PrimitiveInfo& GetPrimitiveInfo() { return m_primitiveInfo; }
	const PrimitiveInfo& GetPrimitiveInfo() const { return m_primitiveInfo; }

	void AddVertexStreamInfo( const usg::exchange::VertexStreamInfo& info );
	const usg::exchange::VertexStreamInfo& GetVertexStreamInfo( uint32_t i ) const;
	usg::exchange::VertexStreamInfo& GetVertexStreamInfo(uint32_t i);
	usg::exchange::VertexStreamInfo* GetVertexStreamInfoOfType(usg::exchange::VertexAttribute attribType);
	uint32_t GetVertexStreamNum( void ) const;
	uint32_t CalcTextureUVCoordinateNum( void ) const;

	uint32_t SearchSingleAttribute( usg::exchange::VertexAttribute attrType ) const;


	usg::exchange::Shape& pb() { return m_shape; }
	const usg::exchange::Shape& pb() const { return m_shape; }
private:
	usg::exchange::Shape m_shape;

	Stream m_calculatedNormalStream;

	PrimitiveInfo m_primitiveInfo;
	VectorVertexStreamInfo m_vertexStreamInfos;

	Vector3 m_aabbMin, m_aabbMax;
};

}

#endif // SHAPE_H
