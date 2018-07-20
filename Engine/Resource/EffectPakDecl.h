/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The structs that comprise the binary 
*****************************************************************************/
#ifndef _USG_RESOURCE_EFFECT_PAK_H_
#define _USG_RESOURCE_EFFECT_PAK_H_
#include "Engine/Common/Common.h"

namespace usg
{
	namespace EffectPakDecl
	{

		// Shaders come first as we need them to process effects
		struct ShaderEntry
		{
			char			szName[USG_MAX_PATH];
			usg::ShaderType eType;
			uint32			CRC;
			uint32			uBinaryOffset;	// Offset into the shader binary area
			uint32			uBinarySize;
		};

		struct EffectEntry
		{
			char		name[USG_MAX_PATH] = {};
			uint32		CRC[(uint32)usg::ShaderType::COUNT] = {};
			uint32		uBinaryOffset;	
			uint32		uBinarySize;	// 0 on platforms which do not compile complete effect combinations
		};

		struct Header
		{
			uint32 uShaderDeclOffset; // Not valid on platforms which do compile complete effect combinations
			uint32 uShaderBinaryOffset; // Not valid on platforms which do compile complete effect combinations
			uint32 uShaderCount;
			uint32 uEffectDefinitionOffset;
			uint32 uEffectBinaryOffset;	// Not valid on platforms which don't compile complete effect combinations
			uint32 uEffectCount;
		};

	}

}

#endif	// #ifndef _USG_RESOURCE_EFFECT_PAK_H_
