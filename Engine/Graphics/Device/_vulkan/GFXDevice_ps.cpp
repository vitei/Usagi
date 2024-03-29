/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include API_HEADER(Engine/Graphics/Device, RasterizerState.h)
#include API_HEADER(Engine/Graphics/Device, AlphaState.h)
#include API_HEADER(Engine/Graphics/Device, DepthStencilState.h)
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include API_HEADER(Engine/Graphics/Device, VkGPUHeap.h)
#include API_HEADER(Engine/Graphics/Device, VkMemAllocator.h)
#include API_HEADER(Engine/Oculus, OculusVKExport.h)
#include "Engine/Graphics/Device/GFXDevice.h" 
#include "Engine/Graphics/Device/IHeadMountedDisplay.h" 
#include "Engine/Core/Modules/ModuleManager.h"
#include "Engine/Graphics/Device/GFXContext.h" 
#include <vulkan/vulkan.h>
#include "Engine/Core/stl/vector.h"

// Note disable for render doc builds, otherwise VK_EXT_validation_features extension will topple the replay
#ifdef DEBUG_BUILD
#define USE_VALIDATION
#endif

#ifndef FINAL_BUILD
#define USE_DEBUG_MARKERS
#endif

// 256MB per alloc is recommended
// Sometimes this won't be big enough for a single allocation so pools larger than this are possible
static const memsize g_sPoolAllocSize = 256 * 1024 * 1024;

namespace usg {

static const VkFormat gColorFormatMap[]=
{
	VK_FORMAT_B8G8R8A8_UNORM,					// TF_RGBA_8888
	VK_FORMAT_A1R5G5B5_UNORM_PACK16,			// TG_RGBA_5551
	VK_FORMAT_R5G6B5_UNORM_PACK16,				// CF_RGB_565,
	VK_FORMAT_B4G4R4A4_UNORM_PACK16,			// CF_RGBA_4444,
	VK_FORMAT_R8G8B8_UNORM,						// CF_RGB_888,
	VK_FORMAT_R32_SFLOAT,						// CF_SHADOW,
	VK_FORMAT_R16G16B16A16_SFLOAT,				// CF_RGBA_16F
	VK_FORMAT_A2B10G10R10_UNORM_PACK32,			// CF_RGB_HDR,
	VK_FORMAT_R32_SFLOAT,						// CF_R_32F,
	VK_FORMAT_R32_UINT,							// CF_R_32,
	VK_FORMAT_R32G32_SFLOAT,					// CF_RG_32F,
	VK_FORMAT_R16_SFLOAT,						// CF_R_16F,
	VK_FORMAT_R16G16_SFLOAT,					// CF_RG_16F,
	VK_FORMAT_R8_UNORM,							// CF_R_8
	VK_FORMAT_R8G8_UNORM,						// CF_RG_8
	VK_FORMAT_R16G16B16A16_SNORM,				// CF_NORMAL
	VK_FORMAT_B8G8R8A8_SRGB,					// CF_SRGBA
	VK_FORMAT_UNDEFINED,						// CF_UNDEFINED	// Only makes sense for render passes
};


static_assert(ARRAY_SIZE(gColorFormatMap) == (uint32)usg::ColorFormat::COUNT, "Mismatch on color format mapping size");

static const uint32 gMaxColorFormatFallbacks = 3;

static const VkFormat gFallbackColorFormatMap[][gMaxColorFormatFallbacks] =
{
	{ VK_FORMAT_B8G8R8A8_UNORM },																			// TF_RGBA_8888
	{ VK_FORMAT_B5G5R5A1_UNORM_PACK16, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM },				// TG_RGBA_5551
	{ VK_FORMAT_B5G6R5_UNORM_PACK16, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM },					// CF_RGB_565,
	{ VK_FORMAT_R4G4B4A4_UNORM_PACK16, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM },				// CF_RGBA_4444,
	{ VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM },							// CF_RGB_888,
	{ VK_FORMAT_R32_UINT },																					// CF_SHADOW,
	{  },																									// CF_RGBA_16F
	{ VK_FORMAT_B10G11R11_UFLOAT_PACK32, VK_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT },	// CF_RGB_HDR,
	{ VK_FORMAT_R32_SFLOAT,	VK_FORMAT_R32_UINT, VK_FORMAT_R32G32_SFLOAT },									// CF_R_32F,
	{ VK_FORMAT_R32_SFLOAT },																				// CF_R_32,
	{ VK_FORMAT_R32G32B32_SFLOAT },																			// CF_RG_32F,
	{ VK_FORMAT_R32_SFLOAT,	VK_FORMAT_R32_UINT },															// CF_R_16F,
	{ VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT },												// CF_RG_16F,
	{ VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8B8_UNORM },														// CF_R_8
	{ VK_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8B8_UNORM },														// CF_RG_8
	{ VK_FORMAT_R16G16B16A16_SFLOAT },																		// CF_NORMAL
	{ VK_FORMAT_R8G8B8A8_SRGB },																									// CF_SRGBA
	{ },																									// CF_UNDEFINED	// Only makes sense for render passes
};



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

void SetImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
	VkImageSubresourceRange subresourceRange, VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags)
{
	// Create an image barrier object
	VkImageMemoryBarrier imageMemoryBarrier = {};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = NULL;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;


	// Src access mask specifies the operations we need to wait on. For example a source image which is a color attachment will need to wait on any color attachment writes
	switch (oldImageLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		imageMemoryBarrier.srcAccessMask = 0;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		break;
	}

	// The new layout for the image
	switch (newImageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		if (imageMemoryBarrier.srcAccessMask == 0)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		break;
	}

	vkCmdPipelineBarrier(cmdBuffer, srcFlags, dstFlags, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
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
	m_uDisplayCount = 0;
	m_fGPUTime = 0.0f;
}

void GFXDevice_ps::Cleanup(GFXDevice* pParent)
{
	vkDestroyCommandPool(m_vkDevice, m_cmdPool, NULL);
	vkDeviceWaitIdle(m_vkDevice);
	// Cleanup any requested destroys before destroying the device
	CleanupDestroyRequests();

	m_criticalSection.Finalize();
}

GFXDevice_ps::~GFXDevice_ps()
{
	PFN_vkDestroyDebugReportCallbackEXT DestroyReportCallback = VK_NULL_HANDLE;
	DestroyReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT");

#ifdef USE_VK_DEBUG_EXTENSIONS
	for (uint32 i = 0; i < CALLBACK_COUNT; i++)
	{
		DestroyReportCallback(m_instance, m_callbacks[i], nullptr);
	}
#endif

	vkDestroyDevice(m_vkDevice, NULL);
	vkDestroyInstance(m_instance, NULL);


	if (m_pQueueProps)
	{
		mem::Free(MEMTYPE_STANDARD, m_pQueueProps);
		m_pQueueProps = NULL;
	}
	
}

void GFXDevice_ps::CleanupDestroyRequests(uint32 uMaxFrameId)
{	
	CriticalSection::ScopedLock lock(m_criticalSection);

	// Handle case that we've wrapped around
	uint32 uCurrentFrame = m_pParent->GetFrameCount();
	
	while(!m_destroyQueue.empty())
	{
		DestroyRequest& req = m_destroyQueue.front();
		if (req.uDestroyReqFrame <= uMaxFrameId || req.uDestroyReqFrame > uCurrentFrame)
		{
			switch (req.eResourceType)
			{
				case RESOURCE_BUFFER:
					vkDestroyBuffer(m_vkDevice, req.resource.buffer, nullptr);
				break;
				case RESOURCE_FRAME_BUFFER:
					vkDestroyFramebuffer(m_vkDevice, req.resource.frameBuffer, nullptr);
				break;
				case RESOURCE_IMAGE_VIEW:
					vkDestroyImageView(m_vkDevice, req.resource.imageView, nullptr);
				break;
				case RESOURCE_IMAGE:
					vkDestroyImage(m_vkDevice, req.resource.image, nullptr);
				break;
				case RESOURCE_DESCRIPTOR_SET:
					vkFreeDescriptorSets(m_vkDevice, req.resource.desc.pool, 1, &req.resource.desc.set);
				break;
				case RESOURCE_DESCRIPTOR_LAYOUT:
					vkDestroyDescriptorSetLayout(m_vkDevice, req.resource.layout, nullptr);
				break;
				case RESOURCE_DESCRIPTOR_POOL:
					vkDestroyDescriptorPool(m_vkDevice, req.resource.pool, nullptr);
					break;
				default:
					ASSERT(false);
			}
			m_destroyQueue.pop();
		}
		else
		{
			break;
		}
	}
}

void GetHMDExtensionsForType(IHeadMountedDisplay* pHmd, IHeadMountedDisplay::ExtensionType eType, vector<const char*>& extensions)
{
	if (pHmd)
	{
		for (uint32 i = 0; i < pHmd->GetRequiredAPIExtensionCount(eType); i++)
		{
			bool bFound = false;
			const char* szExtension = pHmd->GetRequiredAPIExtension(eType, i);
			for (int j = 0; j < extensions.size(); j++)
			{
				if (strcmp(extensions[j], szExtension) == 0)
				{
					bFound = true;
					break;
				}
			}
			if (!bFound)
			{
				extensions.push_back(szExtension);
			}
		}
	}
}


ColorFormat GFXDevice_ps::GetUSGFormat(VkFormat eFormat)
{
	for (int i = 0; i < int(ColorFormat::COUNT); i++)
	{
		if (m_colorFormats[i] == eFormat)
		{
			return (ColorFormat)i;
		}
	}

	return ColorFormat::INVALID;
}

void GFXDevice_ps::Init(GFXDevice* pParent)
{
	m_pParent = pParent;

	m_criticalSection.Initialize();

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
	
	// Check to see if an HMD has been loaded and grab the extensions
	uint32 uHMDId = IHeadMountedDisplay::GetModuleTypeNameStatic();
	uint32 uHMDCount = ModuleManager::Inst()->GetNumberOfInterfacesForType(uHMDId);

	// FIXME: Confirm of type oculus
	IHeadMountedDisplay* pHmd = nullptr;
	for (uint32 i = 0; i < uHMDCount; i++)
	{
		ModuleInterface* pInterface = ModuleManager::Inst()->GetInterfaceOfType(uHMDId, i);
		IHeadMountedDisplay* pThisHMD = (IHeadMountedDisplay*)pInterface;
		if (pThisHMD)
		{
			pHmd = pThisHMD;
			break;
		}
	}

	// TODO: Should come from HMD
	typedef bool (far *GetHMDPhysicalDeviceVK)(VkInstance instance, VkPhysicalDevice* deviceOut);
	GetHMDPhysicalDeviceVK GetHMDPhysicalDeviceVKFn = nullptr;
	if (pHmd)
	{
		HMODULE oculusModule = ModuleManager::Inst()->GetModule(pHmd);
		GetHMDPhysicalDeviceVKFn = (GetHMDPhysicalDeviceVK)GetProcAddress(oculusModule, "GetHMDPhysicalDeviceVK");
	}

	GetHMDExtensionsForType(pHmd, IHeadMountedDisplay::ExtensionType::Instance, extensions);

	// initialize the VkInstanceCreateInfo structure
	VkInstanceCreateInfo inst_info = {};
	inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
#ifdef USE_VALIDATION
	const char *validationLayerNames[] =
	{
		"VK_LAYER_KHRONOS_validation" /* Enable validation layers in debug builds to detect validation errors */
	};
	int validationLayerCount = ARRAY_SIZE(validationLayerNames);

	VkValidationFeatureDisableEXT disabledValidation[] = { VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT, VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT };
	VkValidationFeaturesEXT validation = {};
	validation.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT;
	validation.disabledValidationFeatureCount = 2;
	validation.pDisabledValidationFeatures = disabledValidation;

	//extensions.push_back("VK_EXT_validation_features");
	//inst_info.pNext = &validation;

#else
	int validationLayerCount = 0;
	const char *validationLayerNames[1] = {};
#endif

	inst_info.flags = 0;
	inst_info.pApplicationInfo = &app_info;
	inst_info.enabledExtensionCount = (uint32)extensions.size();
	inst_info.ppEnabledExtensionNames = extensions.data();
	inst_info.enabledLayerCount = validationLayerCount;
	inst_info.ppEnabledLayerNames = validationLayerNames;

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
		VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT,
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
	ASSERT(gpu_count > 0);
	m_uGPUCount = Math::Min((uint32)gpu_count, (uint32)MAX_GPU_COUNT);
	res = vkEnumeratePhysicalDevices(m_instance, &m_uGPUCount, m_gpus);
	ASSERT(!res && m_uGPUCount >= 1);

	for (uint32 i = 0; i < m_uGPUCount; i++)
	{
		vkGetPhysicalDeviceMemoryProperties(m_gpus[i], &m_memoryProperites[i]);
		vkGetPhysicalDeviceProperties(m_gpus[i], &m_deviceProperties[i]);
	}

	// Init the device
	memset(&m_queueInfo, 0, sizeof(m_queueInfo));

	if (GetHMDPhysicalDeviceVKFn)
	{
		GetHMDPhysicalDeviceVKFn(m_instance, &m_primaryPhysicalDevice);
	}
	else
	{
		m_primaryPhysicalDevice = m_gpus[0];
	}

	vkGetPhysicalDeviceQueueFamilyProperties(m_primaryPhysicalDevice, &m_uQueueFamilyCount, NULL);
	FATAL_RELEASE(m_uQueueFamilyCount >= 1, "No queue families found");

	m_pQueueProps = (VkQueueFamilyProperties*)mem::Alloc(MEMTYPE_STANDARD, ALLOC_GFX_INTERNAL, sizeof(VkQueueFamilyProperties)*m_uQueueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_primaryPhysicalDevice, &m_uQueueFamilyCount, m_pQueueProps);

	//vkGetPhysicalDeviceFeatures 

	bool bFound = false;
	for (unsigned int i = 0; i < m_uQueueFamilyCount; i++)
	{
		// TODO: We could use different queue families for the queues. The loading one only needs the transfer bit
		if (m_pQueueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && m_pQueueProps[i].queueCount >= 2)
		{
			m_queueInfo.queueFamilyIndex = i;
			bFound = true;
			break;
		}
	}

	FATAL_RELEASE(bFound, "No valid queue family with enough queues");

	float queue_priorities[QUEUE_TYPE_COUNT] = { 0.0, 0.0 };	// The relative priority of work submitted to each of the queues
	m_queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	m_queueInfo.pNext = NULL;
	m_queueInfo.queueCount = QUEUE_TYPE_COUNT;
	m_queueInfo.pQueuePriorities = queue_priorities;

	VkPhysicalDeviceFeatures enabledFeatures = {};
	VkPhysicalDeviceFeatures supportedFeatures = {};

	vkGetPhysicalDeviceFeatures(m_primaryPhysicalDevice, &supportedFeatures);

	// FIXME: Set up additional enabled features
	enabledFeatures.samplerAnisotropy = VK_TRUE;
	enabledFeatures.geometryShader = VK_TRUE;
	enabledFeatures.tessellationShader = VK_TRUE;
	enabledFeatures.multiDrawIndirect = VK_TRUE;
	enabledFeatures.shaderStorageImageMultisample = VK_TRUE;
	enabledFeatures.textureCompressionBC = VK_TRUE;
	enabledFeatures.fillModeNonSolid = VK_TRUE;
	enabledFeatures.independentBlend = VK_TRUE;
	enabledFeatures.fragmentStoresAndAtomics = VK_TRUE;



	extensions.clear();
	extensions.push_back("VK_KHR_swapchain");
#ifdef USE_DEBUG_MARKERS
	
	bool bExtensionPresent = false;
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(m_primaryPhysicalDevice, nullptr, &extensionCount, nullptr);
	usg::vector<VkExtensionProperties> deviceExt(extensionCount);
	vkEnumerateDeviceExtensionProperties(m_primaryPhysicalDevice, nullptr, &extensionCount, deviceExt.data());
	for (auto extension : deviceExt) {
		if (strcmp(extension.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0) {
			bExtensionPresent = true;
			break;
		}
	}

	if(bExtensionPresent)
	{
		extensions.push_back("VK_EXT_debug_marker");
	}
#endif

	// Smooth lines are hugely helpful, but not online until 1.1.117
	//extensions.push_back("VK_EXT_line_rasterization");

	GetHMDExtensionsForType(pHmd, IHeadMountedDisplay::ExtensionType::Device, extensions);

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = NULL;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &m_queueInfo;
	device_info.enabledExtensionCount = (uint32)extensions.size();
	device_info.ppEnabledExtensionNames = extensions.data();
	device_info.enabledLayerCount = validationLayerCount;
	device_info.ppEnabledLayerNames = validationLayerNames;
	device_info.pEnabledFeatures = &enabledFeatures;

	// Issue with the allocators atm so disabling for now
	res = vkCreateDevice(m_primaryPhysicalDevice, &device_info, nullptr/*&m_allocCallbacks*/, &m_vkDevice);
	ASSERT(res == VK_SUCCESS);



	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	res = vkCreatePipelineCache(m_vkDevice, &pipelineCacheCreateInfo, nullptr, &m_pipelineCache);
	ASSERT(res == VK_SUCCESS);

	EnumerateDisplays();

	m_cmdPool = CreateCommandPool();

	for(int i=0; i<QUEUE_TYPE_COUNT; i++)
	{
		vkGetDeviceQueue(m_vkDevice, m_queueInfo.queueFamilyIndex, i, &m_queue[i]);
	}

	VkFenceCreateInfo fenceInfo;
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = NULL;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	vkCreateFence(m_vkDevice, &fenceInfo, NULL, &m_drawFence);

	//ASSERT(false);	// Need to set up the copy command buffer

	for (uint32 i = 0; i < int(ColorFormat::COUNT); i++)
	{
		if (gColorFormatMap[i] == VK_FORMAT_UNDEFINED || ColorFormatSupported(gColorFormatMap[i]))
		{
			m_colorFormats[i] = gColorFormatMap[i];
		}
		else
		{
			m_colorFormats[i] = VK_FORMAT_UNDEFINED;
			for (int j = 0; j < gMaxColorFormatFallbacks; j++)
			{
				if (gFallbackColorFormatMap[i][j] == VK_FORMAT_UNDEFINED)
				{
					break;
				}

				if (ColorFormatSupported(gFallbackColorFormatMap[i][j]))
				{
					m_colorFormats[i] = gFallbackColorFormatMap[i][j];
					break;
				}
			}
			if (m_colorFormats[i] == VK_FORMAT_UNDEFINED)
			{
				DEBUG_PRINT("Unsupported color format and all fallbacks failed\n");
				m_colorFormats[i] = VK_FORMAT_R8G8B8A8_UNORM;
			}
		}
	}
}

VkCommandPool GFXDevice_ps::CreateCommandPool()
{
	// Create a command pool to allocate our command buffer from
	VkCommandPoolCreateInfo cmd_pool_info = {};
	cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmd_pool_info.pNext = NULL;
	cmd_pool_info.queueFamilyIndex = m_queueInfo.queueFamilyIndex;
	// We will have short lived cmd buffers (for file loading), and reuse them
	// TODO: Perhaps we want multiple command pools?
	cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkCommandPool pool;
	VkResult res = vkCreateCommandPool(m_vkDevice, &cmd_pool_info, NULL, &pool);
	ASSERT(res == VK_SUCCESS);

	return pool;
}


bool GFXDevice_ps::ColorFormatSupported(VkFormat eFormat)
{
	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(m_primaryPhysicalDevice, eFormat, &props);
	return ((props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) != 0);
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
					pSettings->screenDim.x = mi.rcMonitor.left;
					pSettings->screenDim.y = mi.rcMonitor.top;
					pSettings->screenDim.width = mi.rcMonitor.right - mi.rcMonitor.left;
					pSettings->screenDim.height = Math::Abs(mi.rcMonitor.top - mi.rcMonitor.bottom);
					m_uDisplayCount++;
				}
			}

			// Get other supported resolutions
			DWORD iMode = 0;
			bool bFound = false;
			while(EnumDisplaySettingsEx(DispDev.DeviceName, iMode, &dm, 0))
			{
				if (DispDev.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)
				{
					GFXBounds bounds;
					bounds.x = 0;
					bounds.y = 0;
					bounds.width = dm.dmPelsWidth;
					bounds.height = dm.dmPelsHeight;

					bool bFound = false;
					for (auto itr : pSettings->supportedResolutions)
					{
						if (itr.width == bounds.width && itr.height == bounds.height)
						{
							bFound = true;
							break;
						}
					}

					if(!bFound)
					{
						pSettings->supportedResolutions.push_back(bounds);
					}
				}

				iMode++;

			} while (bFound == true);
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
	}
	vkResetFences(m_vkDevice, 1, &m_drawFence);
	bFirst = false;
	for (uint32 i = 0; i < m_pParent->GetValidDisplayCount(); i++)
	{
		m_pParent->GetDisplay(i)->GetPlatform().SwapBuffers(m_pParent);
	}

	// Safe to take out resources older than max dynamic buffers
	if(m_pParent->GetFrameCount() > GFX_NUM_DYN_BUFF)
	{
		CleanupDestroyRequests( m_pParent->GetFrameCount() - GFX_NUM_DYN_BUFF );
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

	VkResult res = vkQueueSubmit(m_queue[QUEUE_TYPE_GRAPHICS], 1, &submitInfo, m_drawFence);
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
	uint32 uMemoryType = 0;
	prefferedProps |= properties;

	bool bFound = false;
	bool bPrefferedFound = false;
	uint32 uCurrentHeapSize = 0;

	for (uint32_t i = 0; i < VK_MAX_MEMORY_TYPES; ++i)
	{
		if (typeBits & 0x1)
		{
			uint32 uHeapIdx = m_memoryProperites[0].memoryTypes[i].heapIndex;
			uint32 uHeapSize = (uint32)m_memoryProperites[0].memoryHeaps[uHeapIdx].size;
			bool bPreferred = ((m_memoryProperites[0].memoryTypes[i].propertyFlags & prefferedProps) == prefferedProps);

			if ((m_memoryProperites[0].memoryTypes[i].propertyFlags & properties) == properties)
			{
				if (!bFound)
				{
					uMemoryType = i;
					bFound = true;
					bPrefferedFound = (m_memoryProperites[0].memoryTypes[i].propertyFlags & prefferedProps) == prefferedProps;
				}
				else if ( !bPrefferedFound && bPreferred)
				{
					uMemoryType = i;
					bPrefferedFound = true;
				}
				else
				{
					// Compare the size
					if (uHeapSize > uCurrentHeapSize && (bPreferred || !bPrefferedFound) )
					{
						uMemoryType = i;
						bPrefferedFound = bPreferred;
					}

				}

				if (uMemoryType == i)
				{
					uCurrentHeapSize = uHeapSize;
				}
			}
		}

		typeBits >>= 1;
	}

	if (bFound)
	{
		return uMemoryType;
	}

	ASSERT(false);
	return USG_INVALID_ID;
}

bool GFXDevice_ps::AllocateMemory(VkMemAllocator* pAllocInOut)
{
	uint32 uHeap = USG_INVALID_ID;
	uint32 uMemType = pAllocInOut->GetPoolId();
	
	for(memsize i=0; i<m_memoryPools[uMemType].heaps.size(); i++)
	{
		if (pAllocInOut->NeedsDynamicCPUMap() == m_memoryPools[uMemType].heaps[i]->IsDynamic() && m_memoryPools[uMemType].heaps[i]->CanAllocate(m_pParent, pAllocInOut))
		{
			uHeap = (uint32)i;
			break;
		}
	}

	if (uHeap == USG_INVALID_ID)
	{
		VkGPUHeap* pHeap = vnew(ALLOC_POOL) VkGPUHeap;
		uint32 uHeapIdx = m_memoryProperites[0].memoryTypes[uMemType].heapIndex;
		// Use the pool alloc size unless it's larger than 1/4 of the total memory
		memsize uSize = usg::Math::Min(m_memoryProperites[0].memoryHeaps[uHeapIdx].size / 4, g_sPoolAllocSize);
		// Force the image size to be aligned up (re-using the render target memory atm which has this requirement)
		memsize uImageSize = AlignSizeUp(pAllocInOut->GetSize(), pAllocInOut->GetAlign());
		// If this single object is larger, set it to that
		uSize = usg::Math::Max(uSize, uImageSize);
		pHeap->AllocData(m_vkDevice, uMemType, uSize, pAllocInOut->NeedsDynamicCPUMap());
		uHeap = (uint32)(m_memoryPools[uMemType].heaps.size());
		m_memoryPools[uMemType].heaps.push_back(pHeap);
	}

	if(m_memoryPools[uMemType].heaps[uHeap]->CanAllocate(m_pParent, pAllocInOut) )
	{
		m_memoryPools[uMemType].heaps[uHeap]->AddAllocator(m_pParent, pAllocInOut);
		return true;
	}

	ASSERT(false);

	return false;
}

void GFXDevice_ps::ReqDestroyFrameBuffer(VkFramebuffer frameBuffer)
{
	CriticalSection::ScopedLock lock(m_criticalSection);
	DestroyRequest req;
	req.eResourceType = RESOURCE_FRAME_BUFFER;
	req.resource.frameBuffer = frameBuffer;
	req.uDestroyReqFrame = m_pParent->GetFrameCount();
	m_destroyQueue.push(req);
}

void GFXDevice_ps::ReqDestroyBuffer(VkBuffer buffer)
{
	CriticalSection::ScopedLock lock(m_criticalSection);
	DestroyRequest req;
	req.eResourceType = RESOURCE_BUFFER;
	req.resource.buffer = buffer;
	req.uDestroyReqFrame = m_pParent->GetFrameCount();
	m_destroyQueue.push(req);
}

void GFXDevice_ps::ReqDestroyImageView(VkImageView imageView)
{
	CriticalSection::ScopedLock lock(m_criticalSection);
	DestroyRequest req;
	req.eResourceType = RESOURCE_IMAGE_VIEW;
	req.resource.imageView = imageView;
	req.uDestroyReqFrame = m_pParent->GetFrameCount();
	m_destroyQueue.push(req);
}

void GFXDevice_ps::ReqDestroyImage(VkImage image)
{
	CriticalSection::ScopedLock lock(m_criticalSection);
	DestroyRequest req;
	req.eResourceType = RESOURCE_IMAGE;
	req.resource.image = image;
	req.uDestroyReqFrame = m_pParent->GetFrameCount();
	m_destroyQueue.push(req);
}

void GFXDevice_ps::ReqDestroyDescriptorSet(VkDescriptorPool pool, VkDescriptorSet set)
{
	CriticalSection::ScopedLock lock(m_criticalSection);
	DestroyRequest req;
	req.eResourceType = RESOURCE_DESCRIPTOR_SET;
	req.resource.desc.set = set;
	req.resource.desc.pool = pool;
	req.uDestroyReqFrame = m_pParent->GetFrameCount();
	m_destroyQueue.push(req);
}

void GFXDevice_ps::ReqDestroyDescriptorSetLayout(VkDescriptorSetLayout layout)
{
	CriticalSection::ScopedLock lock(m_criticalSection);

	DestroyRequest req;
	req.eResourceType = RESOURCE_DESCRIPTOR_LAYOUT;
	req.resource.layout = layout;
	req.uDestroyReqFrame = m_pParent->GetFrameCount();
	m_destroyQueue.push(req);
}

void GFXDevice_ps::ReqDestroyDescriptorSetPool(VkDescriptorPool pool)
{
	CriticalSection::ScopedLock lock(m_criticalSection);

	DestroyRequest req;
	req.eResourceType = RESOURCE_DESCRIPTOR_POOL;
	req.resource.pool = pool;
	req.uDestroyReqFrame = m_pParent->GetFrameCount();
	m_destroyQueue.push(req);
}

void GFXDevice_ps::FreeMemory(VkMemAllocator* pAllocInOut)
{
	uint32 uMemType = pAllocInOut->GetPoolId();
	for (uint32 i = 0; i < m_memoryPools[uMemType].heaps.size(); i++)
	{
		if (m_memoryPools[uMemType].heaps[i]->GetMemory() == pAllocInOut->GetMemory())
		{
			m_memoryPools[uMemType].heaps[i]->RemoveAllocator(m_pParent, pAllocInOut);
			return;
		}
	}
}


void GFXDevice_ps::WaitIdle()
{
	vkDeviceWaitIdle(m_vkDevice);
	// Safe to clean up any destroy requests
	CleanupDestroyRequests();
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

	res = vkQueueSubmit(m_queue[QUEUE_TYPE_TRANSFER], 1, &submitInfo, VK_NULL_HANDLE);
	ASSERT(res == VK_SUCCESS);

	// TODO: Remove these waits
	res = vkQueueWaitIdle(m_queue[QUEUE_TYPE_TRANSFER]);
	ASSERT(res == VK_SUCCESS);

	if (free)
	{
		vkFreeCommandBuffers(m_vkDevice, m_cmdPool, 1, &commandBuffer);
	}
}


const VkPhysicalDeviceProperties* GFXDevice_ps::GetPhysicalProperties(uint32 uGPU)
{
	if (uGPU < m_uGPUCount)
	{
		return &m_deviceProperties[uGPU];
	}

	return nullptr;
}

}