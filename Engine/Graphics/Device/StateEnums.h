/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Platform independent enums for render state values
*****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_STATE_ENUMS_H_
#define _USG_GRAPHICS_DEVICE_STATE_ENUMS_H_

#include "Engine/Graphics/Device/StateEnums.pb.h"

namespace usg {


enum RenderTargetMask	
{
	RT_MASK_RED		= (1<<0),
	RT_MASK_GREEN	= (1<<1),
	RT_MASK_BLUE	= (1<<2),
	RT_MASK_ALPHA	= (1<<3),
	RT_MASK_RGB		= (RT_MASK_RED|RT_MASK_GREEN|RT_MASK_BLUE),
	RT_MASK_ALL		= (RT_MASK_RED|RT_MASK_GREEN|RT_MASK_BLUE|RT_MASK_ALPHA),
	RT_MASK_NONE	= 0
};

enum StencilValue
{
	STENCIL_CMP_MASK = 0,
	STENCIL_WRITE_MASK,
	STENCIL_REF,
	STENCIL_CMP_MASK_BACK,
	STENCIL_WRITE_MASK_BACK,
	STENCIL_REF_BACK,
	STENCIL_REF_TYPE
};


enum StencilOpType
{
	SOP_TYPE_STENCIL_FAIL = 0,
	SOP_TYPE_DEPTH_FAIL,
	SOP_TYPE_PASS,
	SOP_TYPE_STENCIL_FAIL_BACK,
	SOP_TYPE_DEPTH_FAIL_BACK,
	SOP_TYPE_PASS_BACK,
	SOP_TYPE_COUNT
};


enum StencilTest
{
	STENCIL_TEST_NEVER = 0,
	STENCIL_TEST_ALWAYS,
	STENCIL_TEST_EQUAL,
	STENCIL_TEST_NOTEQUAL,
	STENCIL_TEST_LESS,
	STENCIL_TEST_LEQUAL,
	STENCIL_TEST_GREATER,
	STENCIL_TEST_GEQUAL
};



}


#endif
