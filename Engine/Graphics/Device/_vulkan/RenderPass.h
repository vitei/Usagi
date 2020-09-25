/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_PC_RENDERPASS_H_
#define _USG_GRAPHICS_DEVICE_PC_RENDERPASS_H_

#include "Engine/Graphics/Device/RenderState.h"
#include "Engine/Core/stl/vector.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

namespace usg {

class RenderPass
{
public:
	RenderPass() {};
	~RenderPass() {};
	
	void Init(GFXDevice* pDevice, const class RenderPassInitData &decl, uint32 uId);
	void Cleanup(GFXDevice* pDevice);
	const VkRenderPass& GetPass() const { return m_renderPass; }
	uint32 GetCRC() const { return m_uCRCForPass; }

private:

	
	VkRenderPass m_renderPass;
	uint32		 m_uCRCForPass;
};

}


#endif