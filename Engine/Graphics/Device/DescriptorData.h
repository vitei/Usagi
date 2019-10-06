/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
*****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_DESCRIPTOR_DATA_H_
#define _USG_GRAPHICS_DEVICE_DESCRIPTOR_DATA_H_

#include "Engine/Graphics/Device/GFXHandles.h"
#include "Engine/Resource/ResourceDecl.h"

namespace usg
{

class Texture;
class ConstantSet;

struct DescriptorData
{
	DescriptorType eDescType = DESCRIPTOR_TYPE_INVALID;

	// We can't (shouldn't) unionise smart pointers
	struct TexData
	{
		TextureHndl 		tex;
		SamplerHndl			sampler;
	} texData;

	const ConstantSet*		pConstBuffer = nullptr;

	uint32					uLastUpdateIdx = USG_INVALID_ID;

	
	// Other types use a raw pointer to the appropriate type
};

}


#endif

