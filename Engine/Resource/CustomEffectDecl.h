/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: The structs that comprise the binary 
*****************************************************************************/
#ifndef _USG_RESOURCE_CUSTOM_EFFECT_DECL_H_
#define _USG_RESOURCE_CUSTOM_EFFECT_DECL_H_


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
		};

		struct Sampler
		{
			char 	hint[32];
			char    texName[64];
			uint32	eTexType;
			uint32	uIndex;
			uint32	uShaderSets;
		};

		struct Attribute
		{
			char	name[32];
			char	hint[32];
			uint8	defaultData[64];
			uint32 	uIndex;
			uint32	eConstantType;
			uint32	uCount;
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
			uint32	uShaderSets;
			uint32  uBinding;
		};

	}

}

#endif	// #ifndef _USG_RESOURCE_CUSTOM_EFFECT_DECL_H_
