/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_PC_GFXDEVICE_
#define _USG_GRAPHICS_PC_GFXDEVICE_

#include "Engine/Graphics/Device/Display.h"
#include "Engine/Scene/RenderNode.h"
#include "Engine/Core/stl/list.h"
#include "Engine/Core/stl/queue.h"
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
class VkGPUHeap;
class VkMemAllocator;

void SetImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange, 
	VkPipelineStageFlags srcFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

class GFXDevice_ps
{
public:
	GFXDevice_ps();
	~GFXDevice_ps();

	void Init(GFXDevice* pParent);
	void Cleanup(GFXDevice* pParent);

	uint32 GetHardwareDisplayCount();
	const DisplaySettings* GetDisplayInfo(uint32 uIndex);

	void PostInit() {} 

	void Begin();
	void End();
	float GetGPUTime() const { return m_fGPUTime; }
	
	GFXContext* CreateDeferredContext(uint32 uSizeMul) { ASSERT(false); return NULL; }


	enum QueueType
	{
		QUEUE_TYPE_GRAPHICS = 0,
		QUEUE_TYPE_TRANSFER,
		QUEUE_TYPE_COUNT
	};

	VkCommandPool& GetCommandPool() { return m_cmdPool;  }
	VkCommandPool CreateCommandPool(QueueType eQueueType);
	VkDevice& GetVKDevice() { return m_vkDevice;  }
	VkInstance& GetVKInstance() { return m_instance;  }
	uint32 GetQueueFamilyCount() const { return m_uQueueFamilyCount; }

	const VkQueueFamilyProperties* GetQueueProperties(uint32 uIndex);
	const VkPhysicalDevice GetPrimaryGPU() const { return m_primaryPhysicalDevice; }
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
	VkQueue GetQueue() { return m_queue[QUEUE_TYPE_GRAPHICS]; }
	const VkPhysicalDeviceProperties* GetPhysicalProperties(uint32 uGPU = 0);

	VkFormat GetColorFormat(ColorFormat eFormat) { return m_colorFormats[(uint32)eFormat]; }
	ColorFormat GetUSGFormat(VkFormat eFormat);

	// Need to avoid raw allocations and pool them together. Should potentially explicitly declare type too (constant set, texture etc)
	bool AllocateMemory(VkMemAllocator* pAllocInOut);
	void FreeMemory(VkMemAllocator* pAllocInOut);

	void ReqDestroyBuffer(VkBuffer buffer);
	void ReqDestroyImageView(VkImageView buffer);
	void ReqDestroyImage(VkImage image);
	void ReqDestroyDescriptorSet(VkDescriptorPool pool, VkDescriptorSet set);
	void ReqDestroyDescriptorSetLayout(VkDescriptorSetLayout layout);
	void ReqDestroyDescriptorSetPool(VkDescriptorPool pool);
	void ReqDestroyFrameBuffer(VkFramebuffer frameBuffer);
	void ReqDestroyShader(VkShaderModule shader);
	void ReqDestroyPipelineLayout(VkPipelineLayout layout);
	void SetObjectDebugName(uint64 handle, VkObjectType eType, const char* szName);

	const VkPhysicalDeviceFeatures& GetEnabledFeatures() const { return m_enabledFeatures; }
	bool HasLineSmooth() const { return m_bHasLineSmooth; }
	bool IsMultiThreaded() const { return m_queue[QUEUE_TYPE_GRAPHICS] != m_queue[QUEUE_TYPE_TRANSFER]; }
private:
	void EnumerateDisplays();
	bool ColorFormatSupported(VkFormat eFormat);
	void CleanupDestroyRequests(uint32 uMaxFrameId = UINT_MAX);

	enum
	{
		MAX_GPU_COUNT = 2,
		MAX_DISPLAY_COUNT = 4,
		CALLBACK_COUNT = 2
	};


	enum ResourceType
	{
		RESOURCE_BUFFER = 0,
		RESOURCE_IMAGE_VIEW,
		RESOURCE_DESCRIPTOR_SET,
		RESOURCE_IMAGE,
		RESOURCE_DESCRIPTOR_LAYOUT,
		RESOURCE_DESCRIPTOR_POOL,
		RESOURCE_FRAME_BUFFER,
		RESOURCE_SHADER_MODULE,
		RESOURCE_PIPELINE_LAYOUT
	};


	// FIXME: Refactor to use the resource manager when we stop supporting platforms that precompile into one unit
	struct Shader
	{
		char					name[USG_MAX_PATH];
		VkMemoryPropertyFlags	shaderType;
		VkShaderModule			module;
	};

	struct MemoryPool
	{
		usg::vector<VkGPUHeap*> heaps;
	};

	struct DestroyRequest
	{
		ResourceType	eResourceType;
		uint32			uDestroyReqFrame;

		union Resource
		{
			VkBuffer				buffer;
			VkFramebuffer			frameBuffer;
			VkImageView				imageView;
			VkImage					image;
			VkDescriptorPool		pool;
			VkDescriptorSetLayout	layout;
			VkShaderModule			shader;
			VkPipelineLayout		pipelineLayout;
			struct Descriptor
			{
				// TODO: The device should probably own the pools
				VkDescriptorPool pool;
				VkDescriptorSet  set;
			} desc;
		} resource;
	};


	CriticalSection						m_criticalSection;
	usg::queue<DestroyRequest>			m_destroyQueue;
	VkFormat							m_colorFormats[ColorFormat::COUNT];

	GFXDevice*							m_pParent;
	VkFence								m_drawFence;

#ifdef USE_VK_DEBUG_EXTENSIONS
	VkDebugReportCallbackEXT			m_callbacks[CALLBACK_COUNT];
#endif
	VkPhysicalDeviceMemoryProperties	m_memoryProperites[MAX_GPU_COUNT];
	VkPhysicalDeviceProperties			m_deviceProperties[MAX_GPU_COUNT];
	VkCommandPool						m_cmdPool;
	VkDevice							m_vkDevice;
	VkInstance							m_instance;
	VkPhysicalDeviceFeatures			m_enabledFeatures = {};


	VkPhysicalDevice					m_primaryPhysicalDevice;
	VkPhysicalDevice					m_gpus[MAX_GPU_COUNT];
	uint32								m_uGPUCount;
	VkQueueFamilyProperties*			m_pQueueProps;
	uint32								m_uQueueFamilyCount;
	VkQueue								m_queue[QUEUE_TYPE_COUNT];
	VkDeviceQueueCreateInfo				m_queueInfo[QUEUE_TYPE_COUNT];


	VkPipelineCache						m_pipelineCache;
	VkAllocationCallbacks				m_allocCallbacks;

	DisplaySettings						m_diplayInfo[MAX_DISPLAY_COUNT];
	uint32								m_uDisplayCount;

	MemoryPool							m_memoryPools[VK_MAX_MEMORY_TYPES];

	float		m_fGPUTime;
	bool		m_bHasLineSmooth;
};

}

#endif