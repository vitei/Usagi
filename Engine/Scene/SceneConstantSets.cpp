#include "Engine/Common/Common.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Scene/SceneConstantSets.h"

namespace usg
{
	namespace SceneConsts
	{
		// TODO: Also want a pre depth
		const DescriptorDeclaration g_globalDescriptorDecl[] =
		{
			DESCRIPTOR_ELEMENT(SHADER_CONSTANT_GLOBAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_ALL),
			DESCRIPTOR_ELEMENT(SHADER_CONSTANT_LIGHTING, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_PIXEL),
			DESCRIPTOR_ELEMENT(14,		DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),	// The linear depth texture
			DESCRIPTOR_ELEMENT(15,		DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),	// The shadow cascades
			DESCRIPTOR_END()
		};

		const DescriptorDeclaration g_shadowGlobalDescriptorDecl[] =
		{
			DESCRIPTOR_ELEMENT(SHADER_CONSTANT_GLOBAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VERTEX),
			DESCRIPTOR_END()
		};

		const DescriptorDeclaration g_omniShadowGlobalDescriptorDecl[] =
		{
			DESCRIPTOR_ELEMENT(SHADER_CONSTANT_GLOBAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_ALL),
			DESCRIPTOR_END()
		};

		const DescriptorDeclaration g_boneDescriptorDecl[] =
		{
			DESCRIPTOR_ELEMENT(SHADER_CONSTANT_BONES, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VERTEX),
			DESCRIPTOR_END()
		};

		const ShaderConstantDecl g_instanceCBDecl[] =
		{
			SHADER_CONSTANT_ELEMENT(ModelTransform, mModelMat,		CT_MATRIX_43, 1),
			SHADER_CONSTANT_END()
		};

		
	}
}

