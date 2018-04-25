/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PARTICLE_GENERIC_PARTICLE_H_
#define _USG_PARTICLE_GENERIC_PARTICLE_H_
#include "Engine/Common/Common.h"
#include "Engine/Maths/Vector3f.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/RenderConsts.h"


namespace usg {

namespace Particle
{

	struct GenericVertex
	{
		Vector3f	vPos;
		Color		col;
		float		fLifeStart, fLifeEnd;
		float		fRotStart, fRotEnd;
		Vector3f	vVelocity;
		float	 	vUVRange[4];	// x, y, width, height
	};

	static const VertexElement g_genericVertexElements[] =
	{
		VERTEX_DATA_ELEMENT_NAME(0, GenericVertex, vPos, VE_FLOAT, 3, false),
		VERTEX_DATA_ELEMENT_NAME(1, GenericVertex, col, VE_FLOAT, 4, false),
		VERTEX_DATA_ELEMENT_NAME(2, GenericVertex, fLifeStart, VE_FLOAT, 2, false),
		VERTEX_DATA_ELEMENT_NAME(3, GenericVertex, fRotStart, VE_FLOAT, 2, false),
		VERTEX_DATA_ELEMENT_NAME(4, GenericVertex, vVelocity, VE_FLOAT, 3, false),
		VERTEX_DATA_ELEMENT_NAME(5, GenericVertex, vUVRange[0], VE_FLOAT, 4, false),
		VERTEX_DATA_END()
	};



	const uint32 g_uGenericVertexSize = (sizeof(g_genericVertexElements)/sizeof(VertexElement))-1;

	struct ParticleConstants
	{
		Vector4f	vConstsTest;
		Vector4f	vConsts2;
	};

	static const ShaderConstantDecl g_constantDecl[] =
	{
		SHADER_CONSTANT_ELEMENT(ParticleConstants, vConstsTest, CT_VECTOR_4, 1),
		SHADER_CONSTANT_ELEMENT(ParticleConstants, vConsts2, CT_VECTOR_4, 1),
		SHADER_CONSTANT_END()
	};

	static const DescriptorDeclaration g_genericDescriptorDecl[] =
	{
		DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VS_GS),
		DESCRIPTOR_END()
	};

}

}


#endif