/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_PC_RASTERIZERSTATE_H_
#define _USG_GRAPHICS_DEVICE_PC_RASTERIZERSTATE_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/RenderState.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

namespace usg {

class RasterizerState
{
public:
	RasterizerState() {};
	~RasterizerState() {};

	void Init(GFXDevice* pDevice, const RasterizerStateDecl &decl);
	const VkPipelineRasterizationStateCreateInfo& GetCreateInfo() { return m_createInfo; }

private:
	VkPipelineRasterizationStateCreateInfo m_createInfo;
};

}

#endif