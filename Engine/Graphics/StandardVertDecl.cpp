/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Primitives/VertexDeclaration.h"
#include "Engine/Graphics/StandardVertDecl.h"

namespace usg {

static VertexDeclaration	g_declarations[VT_NONE];
static uint32				g_mappings[VT_NONE];
static bool					g_bDeclarationsValid = false;

static const VertexElement g_vertexElements[][VT_NONE]  =
{
	// VT_POSITION
	{
		{
			0,			// szInditifier
			0,			// uOffset
			VE_FLOAT,	//eType;
			3,			//uCount;
			false		// bNormalised
		},
		VERTEX_DATA_END()
	},
	// VT_POSITION_DIFFUSE
	{
		{
			0,			// szInditifier
			0,			// uOffset
			VE_FLOAT,	//eType;
			3,			//uCount;
			false		// bNormalised
		},
		{
			1,			// szInditifier
			12,			// uOffset
			VE_FLOAT,	//eType;
			4,			//uCount;
			false		// bNormalised
		},
		VERTEX_DATA_END()
	},
	// VT_POSITION_NORMAL
	{
		{
			0,	// szInditifier
			0,			// uOffset
			VE_FLOAT,	//eType;
			3,			//uCount;
			false		// bNormalised
		},
		{
			1,	// szInditifier
			12,			// uOffset
			VE_FLOAT,	//eType;
			3,			//uCount;
			false		// bNormalised
		},
		VERTEX_DATA_END()
	},
	// VT_POSITION_UV
	{
		{
			0,	// szInditifier
			0,			// uOffset
			VE_FLOAT,	//eType;
			3,			//uCount;
			false		// bNormalised
		},
		{
			1,	// szInditifier
			12,				// uOffset
			VE_FLOAT,		//eType;
			2,				//uCount;
			false			// bNormalised
		},
		VERTEX_DATA_END()
	},
	// VT_POSITION_UV_COL
	{
		{
			0,	// szInditifier
			0,			// uOffset
			VE_FLOAT,	//eType;
			3,			//uCount;
			false		// bNormalised
		},
		{
			1,	// szInditifier
			12,				// uOffset
			VE_FLOAT,		//eType;
			2,				//uCount;
			false			// bNormalised
		},
		{
			2,		// szInditifier
			20,				// uOffset
			VE_FLOAT,		//eType;
			4,				//uCount;
			false			// bNormalised
		},
		VERTEX_DATA_END()
	},
	// VT_POSITION_NORMAL_UV
	{
		{
			0,	// szInditifier
			0,			// uOffset
			VE_FLOAT,	//eType;
			3,			//uCount;
			false		// bNormalised
		},
		{
			1,		// szInditifier
			12,				// uOffset
			VE_FLOAT,		//eType;
			3,				//uCount;
			false			// bNormalised
		},
		{
			2,	// szInditifier
			24,				// uOffset
			VE_FLOAT,		//eType;
			2,				//uCount;
			false			// bNormalised
		},
		VERTEX_DATA_END()
	},
	// VT_TANGENT
	{
		{
			0,	// szInditifier
			0,			// uOffset
			VE_FLOAT,	//eType;
			3,			//uCount;
			false		// bNormalised
		},
		{
			1,		// szInditifier
			12,				// uOffset
			VE_FLOAT,		//eType;
			3,				//uCount;
			false			// bNormalised
		},
		{
			2,	// szInditifier
			24,				// uOffset
			VE_FLOAT,		//eType;
			2,				//uCount;
			false			// bNormalised
		},
		{
			3,		// szInditifier
			32,				// uOffset
			VE_FLOAT,		//eType;
			4,				//uCount;
			false			// bNormalised
		},
		VERTEX_DATA_END()
	}
};

const VertexElement* GetVertexDeclaration(VertexType eType)
{
	return g_vertexElements[eType];
}

void InitMappings()
{
	for(uint32 i=0; i<VT_NONE; i++)
	{
		g_declarations[i].InitDecl(g_vertexElements[i]);
		g_mappings[i] = VertexDeclaration::GetDeclId(g_declarations[i]);
	}

	g_bDeclarationsValid = true;
}

const VertexDeclaration &GetStandardDeclaration(VertexType eType)
{
	if(!g_bDeclarationsValid)
		InitMappings();
	
	return g_declarations[eType];
}

uint32 GetStandardDeclarationId(VertexType eType)
{
	if(!g_bDeclarationsValid)
		InitMappings();

	return g_mappings[eType];

}


}

