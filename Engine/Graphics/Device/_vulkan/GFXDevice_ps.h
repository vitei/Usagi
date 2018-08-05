/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_GFXDEVICE_
#define _USG_GRAPHICS_PC_GFXDEVICE_
#include "Engine/Common/Common.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Graphics/Device/Display.h"
#include "Engine/Scene/RenderNode.h"
#include "Engine/Core/Containers/List.h"
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

#ifdef DEBUG_BUILD
#define USE_VK_DEBUG_EXTENSIONS
#endif

namespace usg {

class AlphaState;
class RasterizerState;
class DepthStencilState;
class Viewport;
class GFXDevice;

class GFXDevice_ps
{
public:
	GFXDevice_ps();
	~GFXDevice_ps();

	void Init(GFXDevice* pParent);

	uint32 GetHardwareDisplayCount();
	const DisplaySettings* GetDisplayInfo(uint32 uIndex);

	void PostInit() {} 

	void Begin();
	void End();
	float GetGPUTime() const { return m_fGPUTime; }
	
	GFXContext* CreateDeferredContext(uint32 uSizeMul) { ASSERT(false); return NULL; }

	VkCommandPool& GetCommandPool() { return m_cmdPool;  }
	VkDevice& GetVKDevice() { return m_vkDevice;  }
	VkInstance& GetVKInstance() { return m_instance;  }
	uint32 GetQueueFamilyCount() const { return m_uQueueFamilyCount; }

	const VkQueueFamilyProperties* GetQueueProperties(uint32 uIndex);
	const VkPhysicalDevice GetGPU(uint32 uIndex) const;
	uint32 GetMemoryTypeIndex(uint32 typeBits, VkMemoryPropertyFlags properties, VkMemoryPropertyFlags prefferedProps = 0) const;
	VkPipelineCache GetPipelineCache() { return m_pipelineCache; }

	void FinishedStaticLoad() {  }
	void ClearDynamicResources() {  }
	bool Is3DEnabled() const { return false; }
	void WaitIdle();

	// FIXME: Not yet used, should be used with allocations
	VkAllocationCallbacks* GetAllocCallbacks() { return &m_allocCallbacks; }

	VkCommandBuffer CreateCommandBuffer(VkCommandBufferLevel level, bool begin);
	void FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free);
	VkQueue GetQueue() { return m_queue; }

private:
	void EnumerateDisplays();
	enum
	{
		MAX_GPU_COUNT = 2,
		MAX_DISPLAY_COUNT = 4,
		CALLBACK_COUNT = 2
	};

	// FIXME: Refactor to use the resource manager when we stop supporting platforms that precompile into one unit
	struct Shader
	{
		char					name[USG_MAX_PATH];
		VkMemoryPropertyFlags	shaderType;
		VkShaderModule			module;
	};

	GFXDevice*							m_pParent;
	VkFence								m_drawFence;

#ifdef USE_VK_DEBUG_EXTENSIONS
	VkDebugReportCallbackEXT			m_callbacks[CALLBACK_COUNT];
#endif
	VkPhysicalDeviceMemoryProperties	m_memoryProperites[MAX_GPU_COUNT];
	VkCommandPool						m_cmdPool;
	VkDevice							m_vkDevice;
	VkInstance							m_instance;

	VkPhysicalDevice					m_gpus[MAX_GPU_COUNT];
	uint32								m_uGPUCount;
	VkQueueFamilyProperties*			m_pQueueProps;
	uint32								m_uQueueFamilyCount;
	VkQueue								m_queue;

	VkPipelineCache						m_pipelineCache;
	VkAllocationCallbacks				m_allocCallbacks;

	DisplaySettings						m_diplayInfo[MAX_DISPLAY_COUNT];
	uint32								m_uDisplayCount;

	float		m_fGPUTime;
};

}

#endif