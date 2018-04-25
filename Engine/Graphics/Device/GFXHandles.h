/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Interface for creating the various render states
//  FIXME: Move all of this into the graphics render context
*****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_GFX_HANDLES_H_
#define _USG_GRAPHICS_DEVICE_GFX_HANDLES_H_
#include "Engine/Common/Common.h"

namespace usg {

class AlphaState;
class RasterizerState;
class DepthStencilState;
class PipelineState;
class Sampler;
class DescriptorSetLayout;
class PipelineLayout;
class GFXDevice;
class InputBinding;
class RenderPass;

typedef PointerHandle<AlphaState, uint8>			AlphaStateHndl;
typedef PointerHandle<RasterizerState, uint8>		RasterizerStateHndl;
typedef PointerHandle<DepthStencilState, uint8>		DepthStencilStateHndl;
typedef PointerHandle<Sampler, uint16>				SamplerHndl;
typedef PointerHandle<PipelineState, uint32>		PipelineStateHndl;
typedef PointerHandle<DescriptorSetLayout, uint32>	DescriptorSetLayoutHndl;
typedef PointerHandle<PipelineLayout, uint32>		PipelineLayoutHndl;
typedef PointerHandle<InputBinding, uint32>			InputBindingHndl;
typedef PointerHandle<RenderPass, uint32>			RenderPassHndl;


}


#endif

