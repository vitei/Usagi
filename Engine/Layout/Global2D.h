#pragma once
#include "Engine/Common/Common.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Graphics/Effects/ConstantSet.h"


namespace usg {

	struct Global2DConstants
	{
		Matrix4x4	mProjMat;
	};

	static const ShaderConstantDecl g_global2DCBDecl[] =
	{
		SHADER_CONSTANT_ELEMENT(Global2DConstants, mProjMat,			CT_MATRIX_44, 1),
		SHADER_CONSTANT_END()
	};

	const DescriptorDeclaration g_sGlobalDescriptors2D[] =
	{
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_GLOBAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VS_GS),
		DESCRIPTOR_END()
	};

}