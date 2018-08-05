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
	RenderPass() { m_uClearFlags = 0; };
	~RenderPass() {};
	
	void Init(GFXDevice* pDevice, const class RenderPassInitData &decl, uint32 uId);
	uint32 GetClearFlags() { return m_uClearFlags; }

private:
	uint32 m_uClearFlags;
	
};

}


#endif