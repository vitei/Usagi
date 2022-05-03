/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_VERTEXBUFFER_H
#define _USG_GRAPHICS_PC_VERTEXBUFFER_H

#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)
#include API_HEADER(Engine/Graphics/Device, VkMemAllocator.h)

namespace usg {

class Effect;
class GFXDevice;

class VertexBuffer_ps
{
public:
	VertexBuffer_ps();
	~VertexBuffer_ps();

	void Init(GFXDevice* pDevice, const void* const pVerts, uint32 uDataSize, const char* pszName, GPUUsage eUpdateType, GPULocation eLocation);
	void Cleanup(GFXDevice* pDevice);
	void SetContents(GFXDevice* pDevice, const void* const pData, uint32 uVertCount);

	void*	LockData(GFXDevice* pDevice, uint32 uElements);
	void	UnlockData(GFXDevice* pDevice, void* pData, uint32 uElements);

	VkBuffer GetBuffer() const { return m_buffer[m_uActiveVBO]; }
private:
	void CreateStagingBuffer(GFXDevice* pDevice, uint32 uDataSize);
	void CreateFinalBuffer(GFXDevice* pDevice, uint32 uDataSize, bool bHasStaging);
	void CleanupStaging(GFXDevice* pDevice);

    VkBuffer 					m_buffer[GFX_NUM_DYN_BUFF];
	VkBuffer 					m_stagingBuffer[GFX_NUM_DYN_BUFF];

	GPULocation					m_eLocation;
	VkDeviceSize				m_uBufferSize;
	VkMemAllocator				m_memoryAlloc;
	VkMemAllocator				m_stagingMemoryAlloc;
	uint32						m_uActiveVBO;
	uint32						m_uBufferCount;
	bool						m_bUseStaging;
};

}

#endif
