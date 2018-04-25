/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_PC_RENDERPASS_H_
#define _USG_GRAPHICS_DEVICE_PC_RENDERPASS_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/RenderState.h"

namespace usg {

class RenderPass
{
public:
	RenderPass() {};
	~RenderPass() {};
	
	void Init(GFXDevice* pDevice, const class RenderPassInitData &decl, uint32 uId) {}

private:
	
};

}


#endif