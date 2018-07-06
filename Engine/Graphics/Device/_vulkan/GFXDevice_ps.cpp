/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include API_HEADER(Engine/Graphics/Device, RasterizerState.h)
#include API_HEADER(Engine/Graphics/Device, AlphaState.h)
#include API_HEADER(Engine/Graphics/Device, DepthStencilState.h)
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include "Engine/Graphics/Device/GFXDevice.h" 
#include "Engine/Graphics/Device/GFXContext.h" 
#include <vulkan/vulkan.h>
#include "Engine/Core/stl/vector.h"


namespace usg {

#ifdef USE_VK_DEBUG_EXTENSIONS
static VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugString(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData)
{
	(void)msgFlags; (void)objType; (void)srcObject; (void)location; (void)pUserData; (void)msgCode;
	DEBUG_PRINT("%s: %s\n", pLayerPrefix, pMsg);
	return 1;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugBreak(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg, void *pUserData)
{
	(void)msgFlags; (void)objType; (void)srcObject; (void)location; (void)pUserData; (void)msgCode;
	ASSERT_MSG(false, "%s: %s\n", pLayerPrefix, pMsg);
	return 1;
}
#endif


void* VKAPI_CALL VkAllocation(void* pUserData, size_t size, size_t align, VkSystemAllocationScope eScope)
{
	return mem::Alloc(MEMTYPE_STANDARD, ALLOC_GFX_INTERNAL, size, (uint32)align);
}

void* VKAPI_CALL VkReallocation(void* pUserData, void* pOriginal, size_t size, size_t align, VkSystemAllocationScope eScope)
{
	void* pRet = mem::Alloc(MEMTYPE_STANDARD, ALLOC_GFX_INTERNAL, size, (uint32)align);
	MemCpy(pRet, pOriginal, size);
	mem::Free(MEMTYPE_STANDARD, pOriginal);
	return pRet;
}

void VKAPI_CALL VkFree(void* pUserData, void* pMem)
{
	return mem::Free(MEMTYPE_STANDARD, pMem);
}

GFXDevice_ps::GFXDevice_ps()
{
	m_pParent = NULL;
	m_uQueueFamilyCount = 0;
	m_uGPUCount = 0;
	m_pQueueProps = NULL;
	m_uStockCount = 0;
	m_uDisplayCount = 0;
	m_fGPUTime = 0.0f;
}

GFXDevice_ps::~GFXDevice_ps()
{
	PFN_vkDestroyDebugReportCallbackEXT DestroyReportCallback = VK_NULL_HANDLE;
	DestroyReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT");

	for (uint32 i = 0; i < CALLBACK_COUNT; i++)
	{
		DestroyReportCallback(m_instance, m_callbacks[i], nullptr);
	}

	vkDestroyCommandPool(m_vkDevice, m_cmdPool, NULL);
	vkDeviceWaitIdle(m_vkDevice);
	vkDestroyDevice(m_vkDevice, NULL);
	vkDestroyInstance(m_instance, NULL);


	if (m_pQueueProps)
	{
		mem::Free(MEMTYPE_STANDARD, m_pQueueProps);
		m_pQueueProps = NULL;
	}
	
}


void GFXDevice_ps::Init(GFXDevice* pParent)
{  
	m_pParent = pParent;

	m_allocCallbacks.pfnAllocation = VkAllocation;
	m_allocCallbacks.pfnFree = VkFree;
	m_allocCallbacks.pfnReallocation = VkReallocation;
	m_allocCallbacks.pUserData = nullptr;
	m_allocCallbacks.pfnInternalFree = nullptr;
	m_allocCallbacks.pfnInternalAllocation = nullptr;

	// This is application specific, not unique to a single device
	// We are assuming just one device for our purposes
	VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = NULL;
    app_info.pApplicationName = "UsagiEngine";
    app_info.applicationVersion = 1;
    app_info.pEngineName = "Usagi_Engine";
    app_info.engineVersion = 1;
	app_info.apiVersion = VK_API_VERSION_1_0;

	vector<const char*> extensions;
	extensions.push_back("VK_KHR_surface");
	extensions.push_back("VK_KHR_win32_surface");
	extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);


    // initialize the VkInstanceCreateInfo structure
    VkInstanceCreateInfo inst_info = {};
    inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_info.pNext = NULL;
    inst_info.flags = 0;
    inst_info.pApplicationInfo = &app_info;
    inst_info.enabledExtensionCount = (uint32)extensions.size();
    inst_info.ppEnabledExtensionNames = extensions.data();
    inst_info.enabledLayerCount = 0;
    inst_info.ppEnabledLayerNames = NULL;

    VkResult res;

    res = vkCreateInstance(&inst_info, NULL, &m_instance);
    if (res == VK_ERROR_INCOMPATIBLE_DRIVER)
    {
        ASSERT_MSG(false, "cannot find a compatible Vulkan ICD\n");
        return;
    }
    else if (res)
    {
        ASSERT_MSG(false, "unknown error\n");
        return;
    }

	PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
	CreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT");


#ifdef USE_VK_DEBUG_EXTENSIONS
	VkDebugReportCallbackCreateInfoEXT callback = {
		VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,    // sType
		NULL,                                                       // pNext
		VK_DEBUG_REPORT_WARNING_BIT_EXT| VK_DEBUG_REPORT_INFORMATION_BIT_EXT|VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT|VK_DEBUG_REPORT_DEBUG_BIT_EXT,
		VkDebugString,                                        // pfnCallback
		NULL                                                        // pUserData
	};
	res = CreateDebugReportCallback(m_instance, &callback, nullptr, &m_callbacks[0]);
	
	callback.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT;
	callback.pfnCallback = VkDebugBreak;
	callback.pUserData = NULL;
	res = CreateDebugReportCallback(m_instance, &callback, nullptr, &m_callbacks[1]);
#endif

    // Enumerate the available GPUs
    uint32 gpu_count = 1;
    res = vkEnumeratePhysicalDevices(m_instance, &gpu_count, NULL);
    ASSERT(gpu_count>0);
	m_uGPUCount = Math::Min((uint32)gpu_count, (uint32)MAX_GPU_COUNT);
    res = vkEnumeratePhysicalDevices(m_instance, &m_uGPUCount, m_gpus);
    ASSERT(!res && m_uGPUCount >= 1);

	for (uint32 i = 0; i < m_uGPUCount; i++)
	{ 
		vkGetPhysicalDeviceMemoryProperties(m_gpus[i], &m_memoryProperites[i]);
	}
    
    // Init the device
    VkDeviceQueueCreateInfo queue_info = {};

    vkGetPhysicalDeviceQueueFamilyProperties(m_gpus[0], &m_uQueueFamilyCount, NULL);
    ASSERT(m_uQueueFamilyCount >= 1);

	m_pQueueProps = (VkQueueFamilyProperties*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_GFX_INTERNAL, sizeof(VkQueueFamilyProperties)*m_uQueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_gpus[0], &m_uQueueFamilyCount, m_pQueueProps);

	//vkGetPhysicalDeviceFeatures 

    bool bFound = false;
    for (unsigned int i = 0; i < m_uQueueFamilyCount; i++)
    {
        if (m_pQueueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            queue_info.queueFamilyIndex = i;
            bFound = true;
            break;
        }
    }
    
    ASSERT(bFound);

    float queue_priorities[1] = {0.0};	// The relative priority of work submitted to each of the queues
    queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_info.pNext = NULL;
    queue_info.queueCount = 1;
    queue_info.pQueuePriorities = queue_priorities;

	VkPhysicalDeviceFeatures enabledFeatures = {};
	VkPhysicalDeviceFeatures supportedFeatures = {};

	vkGetPhysicalDeviceFeatures(m_gpus[0], &supportedFeatures);

	// FIXME: Set up additional enabled features
	enabledFeatures.geometryShader = VK_TRUE;
	enabledFeatures.tessellationShader = VK_TRUE;
	enabledFeatures.multiDrawIndirect = VK_TRUE;
	
#ifdef DEBUG_BUILD
	int validationLayerCount = 1;
	const char *validationLayerNames[] =
	{
		"VK_LAYER_LUNARG_standard_validation" /* Enable validation layers in debug builds to detect validation errors */
	};
#else
	int validationLayerCount = 0;
	const char *validationLayerNames[];
#endif


	extensions.clear();
	extensions.push_back("VK_KHR_swapchain");

    VkDeviceCreateInfo device_info = {};
    device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_info.pNext = NULL;
    device_info.queueCreateInfoCount = 1;
    device_info.pQueueCreateInfos = &queue_info;
    device_info.enabledExtensionCount = (uint32)extensions.size();
    device_info.ppEnabledExtensionNames = extensions.data();
    device_info.enabledLayerCount = validationLayerCount;
    device_info.ppEnabledLayerNames = validationLayerNames;
    device_info.pEnabledFeatures = &enabledFeatures;

	// Issue with the allocators atm so disabling for now
    res = vkCreateDevice(m_gpus[0], &device_info, nullptr/*&m_allocCallbacks*/, &m_vkDevice);
    ASSERT(res == VK_SUCCESS);

	// Create a command pool to allocate our command buffer from
    VkCommandPoolCreateInfo cmd_pool_info = {};
    cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_info.pNext = NULL;
    cmd_pool_info.queueFamilyIndex = queue_info.queueFamilyIndex;
	// We will have short lived cmd buffers (for file loading), and reuse them
	// TODO: Perhaps we want multiple command pools?
    cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;	

    res = vkCreateCommandPool(m_vkDevice, &cmd_pool_info, NULL, &m_cmdPool);
    ASSERT(res == VK_SUCCESS);

	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	res = vkCreatePipelineCache(m_vkDevice, &pipelineCacheCreateInfo, nullptr, &m_pipelineCache);
	ASSERT(res == VK_SUCCESS);
	
	EnumerateDisplays();

	vkGetDeviceQueue(m_vkDevice, queue_info.queueFamilyIndex, 0, &m_queue);

	VkFenceCreateInfo fenceInfo;
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = NULL;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	vkCreateFence(m_vkDevice, &fenceInfo, NULL, &m_drawFence);

	//ASSERT(false);	// Need to set up the copy command buffer
}


void GFXDevice_ps::EnumerateDisplays()
{
	m_uDisplayCount = 0;

	// TODO: Move this out into platform specific code
	DISPLAY_DEVICE DispDev;
	ZeroMemory(&DispDev, sizeof(DISPLAY_DEVICE));
	DispDev.cb = sizeof(DISPLAY_DEVICE);

	// Pass in a value over than zero to the second parameter to enumerate graphics cards
	for (DWORD uDeviceIndex = 0; m_uDisplayCount < MAX_DISPLAY_COUNT && EnumDisplayDevices(NULL, uDeviceIndex, &DispDev, 0); uDeviceIndex++)
	{
		if (!(DispDev.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
		{
			DisplaySettings* pSettings = &m_diplayInfo[m_uDisplayCount];
			str::Copy(pSettings->name, DispDev.DeviceString, USG_IDENTIFIER_LEN);
			DEVMODE dm;
			ZeroMemory(&dm, sizeof(dm));
			dm.dmSize = sizeof(dm);
			if (EnumDisplaySettingsEx(DispDev.DeviceName, ENUM_CURRENT_SETTINGS, &dm, 0) == FALSE)
			{
				BOOL bReturn = EnumDisplaySettingsEx(DispDev.DeviceName, ENUM_REGISTRY_SETTINGS, &dm, 0);
				ASSERT(bReturn == TRUE);
			}

			HMONITOR hm = 0;
			MONITORINFO mi;
			ZeroMemory(&mi, sizeof(mi));
			mi.cbSize = sizeof(mi);
			if (DispDev.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
			{
				// display is enabled. only enabled displays have a monitor handle
				POINT pt = { dm.dmPosition.x, dm.dmPosition.y };
				hm = MonitorFromPoint(pt, MONITOR_DEFAULTTONULL);
				if (hm)
				{
					GetMonitorInfo(hm, &mi);
					pSettings->uX = mi.rcMonitor.left;
					pSettings->uY = mi.rcMonitor.top;
					pSettings->uWidth = mi.rcMonitor.right - mi.rcMonitor.left;
					pSettings->uHeight = Math::Abs(mi.rcMonitor.top - mi.rcMonitor.bottom);
					pSettings->bWindowed = false;	// No meaning for init data
					pSettings->hardwareHndl = NULL;	// This represents a display, so there is no parent window
					m_uDisplayCount++;
				}
			}
		}
	}
}

uint32 GFXDevice_ps::GetHardwareDisplayCount()
{
	return m_uDisplayCount;
}

const DisplaySettings* GFXDevice_ps::GetDisplayInfo(uint32 uIndex)
{
	if (uIndex < m_uDisplayCount)
	{
		return &m_diplayInfo[uIndex];
	}

	return NULL;
}

void GFXDevice_ps::Begin()
{
	static bool bFirst = true;
	if (!bFirst)
	{
		VkResult res;
		do {
			res = vkWaitForFences(m_vkDevice, 1, &m_drawFence, VK_TRUE, 100000);
		} while (res == VK_TIMEOUT);
		vkResetFences(m_vkDevice, 1, &m_drawFence);
	}
	bFirst = false;
	for (uint32 i = 0; i < m_pParent->GetValidDisplayCount(); i++)
	{
		m_pParent->GetDisplay(i)->GetPlatform().SwapBuffers(m_pParent);
	}

	// TODO: Update GPU time
}

void GFXDevice_ps::End()
{
	// For now just submit our immediate context
	VkSubmitInfo submitInfo = {};
	VkCommandBuffer buffers[] = { m_pParent->GetImmediateCtxt()->GetPlatform().GetVkCmdBuffer() };
	VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = buffers;
	// FIXME: Semaphores for all displays
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_pParent->GetDisplay(0)->GetPlatform().GetImageAcquired();
	submitInfo.pWaitDstStageMask = &pipe_stage_flags;

	VkResult res = vkQueueSubmit(m_queue, 1, &submitInfo, m_drawFence);
	ASSERT(res == VK_SUCCESS);

	// TODO: Not the right place to wait on the GPU
	res = vkQueueWaitIdle(m_queue);
	ASSERT(res == VK_SUCCESS);
}

 

const VkQueueFamilyProperties* GFXDevice_ps::GetQueueProperties(uint32 uIndex)
{
	if (uIndex < m_uQueueFamilyCount)
	{
		return &m_pQueueProps[uIndex];
	}
	ASSERT(false);
	return NULL;
}


const VkPhysicalDevice GFXDevice_ps::GetGPU(uint32 uIndex) const
{
	if (uIndex < m_uGPUCount)
	{
		return m_gpus[uIndex];
	}
	ASSERT(false);
	return NULL;
}

uint32 GFXDevice_ps::GetMemoryTypeIndex(uint32 typeBits, VkMemoryPropertyFlags properties, VkMemoryPropertyFlags prefferedProps) const
{
	prefferedProps |= properties;
	
	for (uint32 uMemoryType = 0; uMemoryType < VK_MAX_MEMORY_TYPES; ++uMemoryType)
	{
		if (((typeBits >> uMemoryType) && 1) == 1)
		{
			const VkMemoryType& type = m_memoryProperites[0].memoryTypes[uMemoryType];

			if ((type.propertyFlags & prefferedProps) == prefferedProps)
			{
				return uMemoryType;
			}
		}
	}


	for (uint32 uMemoryType = 0; uMemoryType < VK_MAX_MEMORY_TYPES; ++uMemoryType)
	{
		if ( ((typeBits >> uMemoryType) && 1) == 1)
		{
			const VkMemoryType& type = m_memoryProperites[0].memoryTypes[uMemoryType];

			if ((type.propertyFlags & properties) == properties)
			{
				return uMemoryType;
			}
		}
	}

	ASSERT(false);
	return ~0U;
}


VkShaderModule GFXDevice_ps::GetShaderFromStock(const U8String &name, VkShaderStageFlagBits shaderType)
{
	for (uint32 uId = 0; uId < m_uStockCount; uId++)
	{
		if (m_stockShaders[uId].shaderType == shaderType && str::Compare(m_stockShaders[uId].name, name.CStr()))
		{
			return m_stockShaders[uId].module;
		}
	}

	if (m_uStockCount < MAX_STOCK_SHADERS)
	{
		U8String fullName = name;
		switch (shaderType)
		{
		case VK_SHADER_STAGE_VERTEX_BIT:
			fullName += ".vert.spv";
			break;
		case VK_SHADER_STAGE_GEOMETRY_BIT:
			fullName += ".geom.spv";
			break;
		case VK_SHADER_STAGE_FRAGMENT_BIT:
			fullName += ".frag.spv";
			break;
		default:
			ASSERT(false);
		}
		Shader* pNext = &m_stockShaders[m_uStockCount];
		pNext->shaderType = shaderType;
		str::Copy(pNext->name, fullName.CStr(), USG_MAX_PATH);

		size_t size;

		FILE *fp = NULL;
		fopen_s(&fp, pNext->name, "rb");
		ASSERT(fp!=NULL);

		fseek(fp, 0L, SEEK_END);
		size = ftell(fp);

		fseek(fp, 0L, SEEK_SET);

		//shaderCode = malloc(size);
		char *shaderCode = new char[size];
		size_t retval = fread(shaderCode, size, 1, fp);
		ASSERT(retval == 1);
		ASSERT(size > 0);

		fclose(fp);

		VkShaderModule shaderModule;
		VkShaderModuleCreateInfo moduleCreateInfo;
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.pNext = NULL;
		moduleCreateInfo.codeSize = size;
		moduleCreateInfo.pCode = (uint32_t*)shaderCode;
		moduleCreateInfo.flags = 0;

		VkResult result = vkCreateShaderModule(m_vkDevice, &moduleCreateInfo, NULL, &shaderModule);
		ASSERT(result == VK_SUCCESS);

		delete[] shaderCode;

		return shaderModule;

		m_uStockCount++;
		return pNext->module;
	}

	ASSERT(false);
	return NULL;
}

void GFXDevice_ps::WaitIdle()
{
	vkDeviceWaitIdle(m_vkDevice);
}

VkCommandBuffer GFXDevice_ps::CreateCommandBuffer(VkCommandBufferLevel level, bool begin)
{
	VkCommandBuffer cmdBuffer;

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = m_cmdPool;
	commandBufferAllocateInfo.level = level;
	commandBufferAllocateInfo.commandBufferCount = 1;

	VkResult res = vkAllocateCommandBuffers(m_vkDevice, &commandBufferAllocateInfo, &cmdBuffer);
	ASSERT(res == VK_SUCCESS);

	// If requested, also start the new command buffer
	if (begin)
	{
		VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		// TODO: Really if a one time buffer we should flag it with VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		cmdBufferBeginInfo.flags = 0;
		cmdBufferBeginInfo.pNext = NULL;

		res = vkBeginCommandBuffer(cmdBuffer, &cmdBufferBeginInfo);
		ASSERT(res == VK_SUCCESS);
	}

	return cmdBuffer;
}

void GFXDevice_ps::FlushCommandBuffer(VkCommandBuffer commandBuffer, bool free)
{
	if (commandBuffer == VK_NULL_HANDLE)
	{
		return;
	}

	VkResult res = vkEndCommandBuffer(commandBuffer);
	ASSERT(res == VK_SUCCESS);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	res = vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
	ASSERT(res == VK_SUCCESS);
	res = vkQueueWaitIdle(m_queue);
	ASSERT(res == VK_SUCCESS);

	if (free)
	{
		vkFreeCommandBuffers(m_vkDevice, m_cmdPool, 1, &commandBuffer);
	}
}

}