/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The structs that comprise the binary 
*****************************************************************************/
#ifndef _USG_RESOURCE_CUSTOM_EFFECT_DECL_H_
#define _USG_RESOURCE_CUSTOM_EFFECT_DECL_H_
#include "Engine/Common/Common.h"

namespace usg
{
	namespace CustomEffectDecl
	{

		struct Header
		{
			uint32 uAttributeOffset;
			uint32 uAttributeCount;
			uint32 uSamplerOffset;
			uint32 uSamplerCount;
			uint32 uConstantSetDeclOffset;
			uint32 uConstantSetCount;
			char   effectName[64];
			char   shadowEffectName[64];
			char   omniShadowEffectName[64];
			char   deferredEffectName[64];
		};

		struct Sampler
		{
			char 	hint[32];
			char    texName[64];
			uint32	uIndex;
		};

		struct Attribute
		{
			char	hint[32];
			uint8	defaultData[64];
			uint32 	uIndex;
			uint32	eConstantType;
		};

		struct Constant
		{
			char			szName[32];
			uint32			uNameHash;
			uint32			eConstantType;
			uint32			uiCount;
			uint32			uiOffset;
		};

		struct ConstantSet
		{
			char	szName[32];
			uint32	uConstants;
			uint32	uDeclOffset;
			uint32	uDataOffset;
			uint32	uDataSize;
		};

	}

}

#endif	// #ifndef _USG_RESOURCE_CUSTOM_EFFECT_DECL_H_
