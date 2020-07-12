/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#ifndef _USG_GRAPHICS_SCENE_CONSTANTSETS_H_
#define _USG_GRAPHICS_SCENE_CONSTANTSETS_H_

#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Maths/Matrix4x3.h"


namespace usg
{
	namespace SceneConsts
	{
		// FIXME: We should be caching the handles for all of these rather than re-requesting them each time
		extern const DescriptorDeclaration g_globalDescriptorDecl[]; 
		extern const DescriptorDeclaration g_shadowGlobalDescriptorDecl[];
		extern const DescriptorDeclaration g_omniShadowGlobalDescriptorDecl[];
		extern const DescriptorDeclaration g_boneDescriptorDecl[];
		extern const ShaderConstantDecl g_instanceCBDecl[];


		struct ModelTransform
		{
			Matrix4x3	mModelMat;
		};

	}
}

#endif
