/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_PC_ALPHASTATE_H_
#define _USG_GRAPHICS_DEVICE_PC_ALPHASTATE_H_

#include "Engine/Graphics/Device/RenderState.h"
#include "Engine/Graphics/Color.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

namespace usg {

class AlphaState
{
public:
	AlphaState() {};
	~AlphaState() {};
	
	void Init(GFXDevice* pDevice, const AlphaStateDecl &decl, uint32 uId);
	const VkPipelineColorBlendStateCreateInfo& GetCreateInfo() { return m_createInfo; }

private:
	VkPipelineColorBlendStateCreateInfo m_createInfo;
	VkPipelineColorBlendAttachmentState m_attState[MAX_COLOR_TARGETS];
};

}


#endif