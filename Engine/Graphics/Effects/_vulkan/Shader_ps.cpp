/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/PakDecl.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include API_HEADER(Engine/Graphics/Effects, Shader_ps.h)


namespace usg {



	Shader_ps::Shader_ps()
	{
	}
	
	Shader_ps::~Shader_ps()
	{

	}


	bool Shader_ps::Init(GFXDevice* pDevice, const PakFileDecl::FileInfo* pFileHeader, const class FileDependencies* pDependencies, const void* pData)
	{	
		VkDevice vkDevice = pDevice->GetPlatform().GetVKDevice();
		VkShaderModuleCreateInfo moduleCreateInfo = {};
		moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		moduleCreateInfo.pNext = NULL;
		moduleCreateInfo.codeSize = pFileHeader->uDataSize;
		moduleCreateInfo.pCode = (uint32_t*)pData;
		moduleCreateInfo.flags = 0;

		VkResult result = vkCreateShaderModule(vkDevice, &moduleCreateInfo, NULL, &m_shaderModule);
		ASSERT(result == VK_SUCCESS);

#ifndef FINAL_BUILD
		PFN_vkSetDebugUtilsObjectNameEXT pfnSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(pDevice->GetPlatform().GetVKInstance(), "vkSetDebugUtilsObjectNameEXT");

		VkDebugUtilsObjectNameInfoEXT name = {};
		name.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		name.objectType = VK_OBJECT_TYPE_SHADER_MODULE;
		name.objectHandle = (uint64_t)m_shaderModule;
		name.pObjectName = pFileHeader->szName;
		pfnSetDebugUtilsObjectNameEXT(vkDevice, &name);
#endif

		return result == VK_SUCCESS && m_shaderModule != VK_NULL_HANDLE;

	}

	void Shader_ps::Cleanup(GFXDevice* pDevice)
	{
		if (m_shaderModule)
		{
			pDevice->GetPlatform().ReqDestroyShader(m_shaderModule);
			m_shaderModule = nullptr;
		}
	}

}

