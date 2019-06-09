/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Constants required for the rendering class
*****************************************************************************/
#ifndef _USG_MESH_UTILITY
#define _USG_MESH_UTILITY

#include "RenderConsts.h"

namespace usg {

namespace MeshUtl
{

// Really should be loading mesh data than calculating this on the fly
template<class VertexType, class IndexType>
NO_INLINE_TEMPL void CalculateNormals( uint32 vertexCount, uint32 indexCount, VertexType* pVertices, const IndexType* pIndices )
{
	Vector3f* pNormals;
	ScratchObj<Vector3f> scratchNormals(pNormals, vertexCount, 4);
	uint32				triangleCount	= (uint32)(indexCount/3);
	const IndexType*	currentTriangle	= pIndices;

	for(uint32 i=0; i<vertexCount; i++)
	{
		pNormals[i].Assign(0.0f, 0.0f, 0.0f);
	}

	for( uint32 triId = 0; triId < triangleCount; triId++ )
	{
		// Get the indices of the triangle
		uint16 i0 = currentTriangle[0];
		uint16 i1 = currentTriangle[1];
		uint16 i2 = currentTriangle[2];

		// Grab the vertices of the triangle
		VertexType& p0 = pVertices[i0];
		VertexType& p1 = pVertices[i1];
		VertexType& p2 = pVertices[i2];

		// Calculate the components of vectors v1 and v2
		Vector3f v1( p1.x - p0.x, p1.y - p0.y, p1.z - p0.z );
		Vector3f v2( p2.x - p0.x, p2.y - p0.y, p2.z - p0.z );

		Vector3f normal = CrossProduct(v2, v1);
		//normal.Normalise();
		
		pNormals[i0] += normal;
		pNormals[i1] += normal;
		pNormals[i2] += normal;

		currentTriangle += 3;
	}


	for( uint32 vertexId = 0; vertexId < vertexCount; vertexId++ )
	{
		VertexType		&vertex = pVertices[vertexId];
		Vector3f		&normal = pNormals[vertexId];
		normal.Normalise();
		vertex.nx = normal.x;
		vertex.ny = normal.y;
		vertex.nz = normal.z;

		// Ensure there are no nans
		ASSERT(normal.x == normal.x);
		ASSERT(normal.y == normal.y);
		ASSERT(normal.z == normal.z);

	}
}

}

}

#endif
