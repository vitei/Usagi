/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_VERTEXBUFFER_H
#define _USG_GRAPHICS_PC_VERTEXBUFFER_H
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Effects/EffectBinding.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

namespace usg {

class Effect;

class VertexBuffer_ps
{
public:
	VertexBuffer_ps();
	~VertexBuffer_ps();

	void Init(GFXDevice* pDevice, void* pVerts, uint32 uDataSize, GPUUsage eUpdateType, GPULocation eLocation);
	void SetContents(GFXDevice* pDevice, void *pData, uint32 uVertCount);

	void*	LockData(GFXDevice* pDevice, uint32 uElements);
	void	UnlockData(GFXDevice* pDevice, void* pData, uint32 uElements);

	VkBuffer GetBuffer() const { return m_buffer[m_uActiveVBO]; }
private:
    VkBuffer 					m_buffer[GFX_NUM_DYN_BUFF];
    VkDeviceMemory				m_mem[GFX_NUM_DYN_BUFF];
	uint32						m_uActiveVBO;
	uint32						m_uBufferCount;
};

}

#endif
