#include "Shape.h"

namespace exchange {

uint32_t Shape::GetVertexAttributeMask() const
{
	uint32_t attr = 0;
	uint32_t offset = 0;

	uint32_t size = GetVertexStreamNum();
	for( uint32_t i = 0; i < size; ++i )
	{
		attr |= 1 << m_vertexStreamInfos.at( i ).attribute;
	}

	return attr;
}


usg::exchange::SkinningType Shape::GetSkinningType() const
{
	usg::exchange::SkinningType eType = GetPrimitiveInfo().eSkinType;

	return eType;
}


Vector3 Shape::GetAABBMin( void ) const
{
	return m_aabbMin;
}

Vector3 Shape::GetAABBMax( void ) const
{
	return m_aabbMax;
}

void Shape::SetAABB( ::Vector3 min, ::Vector3 max )
{
	m_aabbMin = min;
	m_aabbMax = max;
}

uint32_t Shape::SearchStream( usg::exchange::VertexAttribute attrType ) const
{
	uint32_t ret = UINT32_MAX;

	uint32_t size = GetVertexStreamNum();
	for( uint32_t i = 0; i < size; ++i )
	{
		if( m_vertexStreamInfos.at(i).attribute == attrType )
		{
			ret = m_vertexStreamInfos.at(i).refIndex;
			break;
		}
	}

	return ret;
}

uint32 Shape::GetBoneIndexCount() const
{
	uint32_t size = GetVertexStreamNum();
	for (uint32_t i = 0; i < size; ++i)
	{
		if (usg::exchange::VertexAttribute_BONE_INDEX == m_vertexStreamInfos.at(i).attribute)
		{
			return m_vertexStreamInfos.at(i).columns;
		}
	}
	return 0;
}


bool Shape::HasVertexAlpha() const
{
	uint32_t size = GetVertexStreamNum();
	for (uint32_t i = 0; i < size; ++i)
	{
		if (usg::exchange::VertexAttribute_COLOR == m_vertexStreamInfos.at(i).attribute)
		{
			return m_vertexStreamInfos.at(i).columns >= 4;
		}
	}

	for (uint32_t i = 0; i < m_shape.singleAttributes_count; ++i)
	{
		if (usg::exchange::VertexAttribute_COLOR == m_shape.singleAttributes[i].attribute)
		{
			return m_shape.singleAttributes[i].columns >= 4;
		}
	}

	return false;
}

uint32_t Shape::GetPositionStreamRefIndex(void) const
{
	return SearchStream( usg::exchange::VertexAttribute_POSITION );
}

uint32_t Shape::GetNormalStreamRefIndex(void) const
{
	return SearchStream( usg::exchange::VertexAttribute_NORMAL );
}

void Shape::Copy( Shape& rDest, const Shape& rSrc, uint32_t attrFlag )
{
#if 1
	ASSERT_MSG( 0, "Shape::copy is not supported now." );
#else
	memcpy( &rDest.m_shape, &rSrc.m_shape, sizeof( rDest.m_shape ) );

	// index stream
	rDest.mIndexStreamRefIndex = rSrc.mIndexStreamRefIndex;

	// vertex stream
	uint32_t size = rSrc.m_vertexStreamInfos.size();
	for( uint32_t i = 0; i < size; ++i ) {
		VertexStreamInfo info = rSrc.m_vertexStreamInfos.at( i );
		if( 1 << info.attribute & attrFlag ) {
			rDest.AddVertexStreamInfo( info );
		}
	}

	// calculated normal stream
	if( rSrc.m_calculatedNormalStream.IsAllocated() ) {
		::exchange::Stream& rDestStream = rDest.m_calculatedNormalStream;
		const ::exchange::Stream& rSrcStream = rSrc.m_calculatedNormalStream;
		rDestStream.allocate<float>( rSrcStream.GetLength(), rSrcStream.GetColumnNum() );
		memcpy( rDestStream.GetStreamArrayPtr(), rSrcStream.GetStreamArrayPtr(), rDestStream.GetSize() );
	}
#endif
}

void Shape::AddVertexStreamInfo( const usg::exchange::VertexStreamInfo& info )
{
	DEBUG_PRINTF( "[%s] attr:%d, index:0x%x\n", __FUNCTION__, info.attribute, info.refIndex );
	m_vertexStreamInfos.push_back( info );
}

const usg::exchange::VertexStreamInfo& Shape::GetVertexStreamInfo( uint32_t i ) const
{
	return m_vertexStreamInfos.at(i);
}

usg::exchange::VertexStreamInfo& Shape::GetVertexStreamInfo(uint32_t i)
{
	return m_vertexStreamInfos.at(i);
}

usg::exchange::VertexStreamInfo* Shape::GetVertexStreamInfoOfType(usg::exchange::VertexAttribute attribType)
{
	uint32_t size = GetVertexStreamNum();
	for (uint32_t i = 0; i < size; ++i)
	{
		if (attribType == m_vertexStreamInfos.at(i).attribute)
		{
			return &m_vertexStreamInfos.at(i);
		}
	}
	return nullptr;
}

uint32_t Shape::GetVertexStreamNum( void ) const
{
	return (uint32_t)m_vertexStreamInfos.size();
}

uint32_t Shape::CalcTextureUVCoordinateNum( void ) const
{
	uint32_t num = 0;
	uint32_t size = GetVertexStreamNum();
	for( uint32_t i = 0; i < size; ++i )
	{
		if( m_vertexStreamInfos.at( i ).attribute == usg::exchange::VertexAttribute_UV )
		{
			++num;
		}
	}
	return num;
}

uint32_t Shape::SearchSingleAttribute( usg::exchange::VertexAttribute attrType ) const
{
	for( uint32_t i = 0; i < m_shape.singleAttributes_count; ++i )
	{
		if( attrType == m_shape.singleAttributes[i].attribute )
		{
			return i;
		}
	}

	// not found
	return UINT32_MAX;
}

}
