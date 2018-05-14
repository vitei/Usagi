/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_PC_DEPTHSTENCILSTATE_H_
#define _USG_GRAPHICS_DEVICE_PC_DEPTHSTENCILSTATE_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/RenderState.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

namespace usg {

class DepthStencilState
{
public:
	DepthStencilState() {};
	~DepthStencilState() {};

	void Init(GFXDevice* pDevice, const DepthStencilStateDecl &decl, uint32 uId);
	const VkPipelineDepthStencilStateCreateInfo& GetCreateInfo() { return m_createInfo; }

private:
	VkPipelineDepthStencilStateCreateInfo m_createInfo;
};

}


#endif