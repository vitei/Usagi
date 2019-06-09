/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Utility vertex declarations for common types
*****************************************************************************/
#ifndef _USG_STANDARD_VERT_DECL_
#define _USG_STANDARD_VERT_DECL_

#include "Engine/Graphics/Color.h"

namespace usg {

class VertexDeclaration;

enum VertexType
{
	VT_POSITION = 0,		// Vertices with simple x, y, z data
	VT_POSITION_DIFFUSE,	// Vertices with x,y,z and diffuse
	VT_POSITION_NORMAL,		// Vertices with x, y, z and normal
	VT_POSITION_UV,			// Vertices with x,y,z and texture co-ords
	VT_POSITION_UV_COL,		// Vertices with x,y,z, diffuse and uv
	VT_POSITION_NORMAL_UV,	// Vertices with x, y, z, normal and u, v
	VT_TANGENT,				// Vertices with x, y, z, normal, u, v and tangent
	VT_NONE,
	VT_CUSTOM = VT_NONE
};


struct PositionVertex
{
	float x, y, z; // The transformed position for the vertex.
};

struct PositionDiffuseVertex
{
    float x, y, z;			// The transformed position for the vertex.
    float r, g, b, a;       // The vertex color.
};

struct PositionNormalVertex
{
    float x, y, z;			// The transformed position for the vertex.
    float nx, ny, nz;		// The vertex normal.
};

struct PositionUVVertex
{
	float x, y, z;			// The transformed position for the vertex.
	float u, v;				//texture co-ordinates
};


struct PositionUVColVertex
{
	float x, y, z;			// The transformed position for the vertex.
	float u, v;				//texture co-ordinates
	Color c;				// float4
};


struct PositionNormalUVVertex
{
	float x, y, z;			// The transformed position for the vertex.
	float nx, ny, nz;		// The vertex normal.
	float u, v;				//texture co-ordinates
};

struct TangentVertex
{
	float x, y, z;			// The transformed position for the vertex.
	float nx, ny, nz;		// The vertex normal.
	float u, v;				// Texture co-ordinates.
	float tx, ty, tz, tw;	// The vertex tangent.
};


inline uint32 GetVertexSize(VertexType type)
{
	switch(type)
	{
	case VT_POSITION:
		return sizeof(PositionVertex);
	case VT_POSITION_DIFFUSE:
		return sizeof(PositionDiffuseVertex);
	case VT_POSITION_NORMAL:
		return sizeof(PositionNormalVertex);
	case VT_POSITION_UV:
		return sizeof(PositionUVVertex);
	case VT_POSITION_NORMAL_UV:
		return sizeof(PositionNormalUVVertex);
	case VT_POSITION_UV_COL:
		return sizeof(PositionUVColVertex);
	case VT_TANGENT:
		return sizeof(TangentVertex);
	default:
		// Unrecognized vertex type
		ASSERT(false);
	}
	return 0;
}

const VertexDeclaration &GetStandardDeclaration(VertexType eType);
const struct VertexElement* GetVertexDeclaration(VertexType eType);
uint32 GetStandardDeclarationId(VertexType eType);

}

#endif
