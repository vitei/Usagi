#ifndef _LOADER_UTIL_H_
#define _LOADER_UTIL_H_

#include <float.h>

#include "Engine/Maths/Matrix4x4.h"
#include "exchange/Stream.h"
#include "exchange/Shape.h"
#include "exchange/Mesh.h"
#include "exchange/Material.h"
#include "pugixmlUtil.h"
#include "StringUtil.h"
#include "LoaderCommon.h"

namespace LoaderUtil
{

	inline void filpUV(::exchange::Stream* pUVStream)
	{
		float* pUVStreamArray = reinterpret_cast<float*>(pUVStream->GetStreamArrayPtr());

		uint32_t length = pUVStream->GetLength();
		for (uint32_t i = 1; i < length; i += 2) {
			pUVStreamArray[i] = 1.0f - pUVStreamArray[i];
		}
	}

	inline void filpUV(Cmdl& cmdl)
	{
		const uint32_t shapeNum = cmdl.GetShapeNum();
		for (uint32_t shapeIndex = 0; shapeIndex < shapeNum; ++shapeIndex) {
			::exchange::Shape* pShape = cmdl.GetShapePtr(shapeIndex);

			const uint32_t streamNum = pShape->GetVertexStreamNum();
			for (uint32_t streamIndex = 0; streamIndex < streamNum; ++streamIndex) {

				usg::exchange::VertexStreamInfo info = pShape->GetVertexStreamInfo(streamIndex);
				if (info.attribute == usg::exchange::VertexAttribute_UV) {
					filpUV(cmdl.GetStreamPtr(info.refIndex));
				}
			}
		}
	}

inline void setupShapeAABB( Vector3& outMin, Vector3& outMax, const ::exchange::Stream* pPositionStream )
{
	ASSERT_MSG( pPositionStream->IsAllocated(), "PositionStream is not allocated." );
	const float* pPositionStreamArray = reinterpret_cast<const float*>( pPositionStream->GetStreamArrayPtr() );

	Vector3 vMin( FLT_MAX, FLT_MAX, FLT_MAX );
	Vector3 vMax( FLT_MIN, FLT_MIN, FLT_MIN );
	uint32_t vertexNum = pPositionStream->GetLength() / pPositionStream->GetColumnNum();
	for( uint32_t vertexIndex = 0; vertexIndex < vertexNum; ++vertexIndex ) {
		float x, y, z;
		x = pPositionStreamArray[vertexIndex * 3];
		y = pPositionStreamArray[vertexIndex * 3 + 1];
		z = pPositionStreamArray[vertexIndex * 3 + 2];

		if( vertexIndex == 0 ) {
			vMin.x = vMax.x = x;
			vMin.y = vMax.y = y;
			vMin.z = vMax.z = z;
		}
		else {
			vMin.x = x < vMin.x ? x : vMin.x;
			vMin.y = y < vMin.y ? y : vMin.y;
			vMin.z = z < vMin.z ? z : vMin.z;
			vMax.x = x > vMax.x ? x : vMax.x;
			vMax.y = y > vMax.y ? y : vMax.y;
			vMax.z = z > vMax.z ? z : vMax.z;
		}
	}
	outMin = vMin;
	outMax = vMax;
}

inline void setupMaterialIndex( ::exchange::Mesh** pMesh, const Cmdl& cmdl )
{
	// search material by name
	int materialNum = cmdl.GetMaterialNum();
	aya::string materialRefName( (*pMesh)->GetMaterialRefName() );

	::exchange::Material* pMaterial = NULL;
	for( int materialIndex = 0; materialIndex < materialNum; ++materialIndex ) {
		pMaterial = cmdl.GetMaterialPtr( materialIndex );
		if( materialRefName.compare( pMaterial->pb().materialName ) == 0 ) {
			(*pMesh)->SetMaterialRefIndex( materialIndex );
			break;
		}
		else {
			pMaterial = NULL;
		}
	}

}

inline usg::Vector3f getVector3NodeAttribute( const pugi::xpath_node& node, const char* pChildNodeName ) {
	pugi::xml_node transNode = evaluateXpathQuery( node, pChildNodeName ).first().node();

	return usg::Vector3f( transNode.attribute( "X" ).as_float(), transNode.attribute( "Y" ).as_float(), transNode.attribute( "Z" ).as_float() );
}


inline void setupTransformMatrix( usg::Matrix4x4& out, const usg::Vector3f& scale, const usg::Vector3f& rotate, const usg::Vector3f& translate )
{
	usg::Matrix4x4 scaleMtx = usg::Matrix4x4::Identity();
	scaleMtx.MakeScale( scale );

	usg::Matrix4x4 rotateMtx = usg::Matrix4x4::Identity();
	rotateMtx.MakeRotate( rotate.x, rotate.y, rotate.z );

	out = scaleMtx * rotateMtx;
	out.SetTranslation(translate);
}

inline void setupTransformMatrix( usg::Matrix4x4& out, const pugi::xpath_node& transformNode )
{
	usg::Vector3f scale     = getVector3NodeAttribute( transformNode, "Scale" );
	usg::Vector3f rotate    = getVector3NodeAttribute( transformNode, "Rotate" );
	usg::Vector3f translate = getVector3NodeAttribute( transformNode, "Translate" );

	setupTransformMatrix( out, scale, rotate, translate );
}

inline void copyStringAttribute( char dst[], int size, const pugi::xml_node& node, const char* pAttributeName )
{
	strncpy( dst, node.attribute( pAttributeName ).as_string(), size - 1 );
}

inline usg::exchange::VertexAttribute checkVectorUsage( const char* pUsage, uint32_t columnNum )
{
	usg::exchange::VertexAttribute attr = usg::exchange::VertexAttribute_NONE;
	if( strcmp( pUsage, "Position" ) == 0 )
	{
		attr = usg::exchange::VertexAttribute_POSITION;
	}
	else if( strcmp( pUsage, "Normal" ) == 0 )
	{
		attr = usg::exchange::VertexAttribute_NORMAL;
	}
	else if( strcmp( pUsage, "Color" ) == 0 ) 
	{
			attr = usg::exchange::VertexAttribute_COLOR;
	}
	else if( startsWith( aya::string( pUsage ), "TextureCoordinate" ) )
	{
		attr = usg::exchange::VertexAttribute_UV;
	}
	else if( strcmp( pUsage, "Tangent" ) == 0 )
	{
		attr = usg::exchange::VertexAttribute_TANGENT;
	}
	else if( strcmp( pUsage, "BoneIndex" ) == 0 )
	{
		attr = usg::exchange::VertexAttribute_BONE_INDEX;
	}
	else if( strcmp( pUsage, "BoneWeight" ) == 0 )
	{
		attr = usg::exchange::VertexAttribute_BONE_WEIGHT;
	}
	return attr;
}

inline void uniteSphere( usg::Vector3f& centerOut, float& radiusOut, const usg::Vector3f& centerA, float radiusA, const usg::Vector3f& centerB, float radiusB )
{
	// Center corresponded
	if( centerA == centerB ) {
		if( radiusA == radiusB ) {
			// Completely same
			centerOut = centerA;
			radiusOut = radiusA;
			return;
		}
		else{
			centerOut = centerA;
			radiusOut = std::max( radiusA, radiusB ); // Larger radius
			return;
		}
	}


	usg::Vector3f sub = centerA - centerB;
	float magn = sub.Magnitude();

	if( magn + radiusB <= radiusA || radiusB == 0.0f ) {
		// A
		centerOut = centerA;
		radiusOut = radiusA;
		return;
	}
	else if( magn + radiusA <= radiusB || radiusA == 0.0f ) {
		// B
		centerOut = centerB;
		radiusOut = radiusB;
		return;
	}

	sub.Normalise();
	radiusOut = ( magn + radiusA + radiusB ) / 2.0f;
	centerOut = ( centerB - ( sub * radiusB ) ) + ( sub * radiusOut ); // The edge of SphereB + The vector that directs to center.
}

}

#endif // _LOADER_UTIL_H_
