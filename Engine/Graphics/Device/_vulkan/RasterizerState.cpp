/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include API_HEADER(Engine/Graphics/Device, RasterizerState.h)

namespace usg {

const VkCullModeFlagBits g_cullFaceMap[] =
{
	VK_CULL_MODE_NONE,	// CULL_FACE_NONE = 0
	VK_CULL_MODE_BACK_BIT,	// CULL_FACE_BACK = 1
	VK_CULL_MODE_FRONT_BIT,	// CULL_FACE_FRONT = 2
};


void RasterizerState::Init(GFXDevice* pDevice, const RasterizerStateDecl &decl, uint32 uId)
{
	memset(&m_createInfo, 0, sizeof(m_createInfo));
    m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    m_createInfo.polygonMode = decl.bWireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    m_createInfo.cullMode = g_cullFaceMap[decl.eCullFace];
    m_createInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    m_createInfo.depthClampEnable = VK_FALSE;
    m_createInfo.rasterizerDiscardEnable = VK_FALSE;
    m_createInfo.depthBiasEnable = decl.bUseDepthBias;
	m_createInfo.lineWidth = decl.fLineWidth;
	m_createInfo.lineWidth = 1.0f;
}


}