/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include API_HEADER(Engine/Graphics/Textures, Sampler.h)

namespace usg {


static const VkFilter g_filterMap[] =
{
	VK_FILTER_NEAREST,			// TF_POINT = 0,
	VK_FILTER_LINEAR,			// TF_LINEAR,
};


static const VkSamplerMipmapMode g_mipMapMode[] =
{
	VK_SAMPLER_MIPMAP_MODE_NEAREST,			// TF_POINT = 0,
	VK_SAMPLER_MIPMAP_MODE_LINEAR,			// TF_LINEAR,
};

static const VkSamplerAddressMode g_textureClampMap[] =
{
	VK_SAMPLER_ADDRESS_MODE_REPEAT,  		 // SC_WRAP = 0,
	VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, // SC_MIRROR = 1,
	VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE 	 // SC_CLAMP = 2,
};


const VkCompareOp g_cmpFuncMap[] =
{
	VK_COMPARE_OP_NEVER,	 		// CF_TEST_NEVER
	VK_COMPARE_OP_ALWAYS,			// CF_TEST_ALWAYS
	VK_COMPARE_OP_EQUAL,			// CF_TEST_EQUAL,
	VK_COMPARE_OP_NOT_EQUAL,		// CF_TEST_NOTEQUAL
	VK_COMPARE_OP_LESS,				// CF_TEST_LESS
	VK_COMPARE_OP_LESS_OR_EQUAL,	// CF_TEST_LEQUAL
	VK_COMPARE_OP_GREATER,			// CF_TEST_GREATER
	VK_COMPARE_OP_GREATER_OR_EQUAL	// CF_TEST_GEQUAL
};


static float32 g_anisoLevel[] =
{
 	1.f,
    2.f,
    4.f,
    8.f,
    16.f
};


Sampler::Sampler()
{
	m_sampler = nullptr;
}

Sampler::~Sampler()
{
	ASSERT(m_sampler == nullptr);
}

void Sampler::Init(GFXDevice* pDevice, const SamplerDecl &decl, uint32 uId)
{
	VkResult err;

	VkDevice vkDevice = pDevice->GetPlatform().GetVKDevice();
	//const VkAllocationCallbacks* pAllocCB = pDevice->GetPlatform().GetVKAllocCB();

	VkSamplerCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
    createInfo.pNext = NULL,
	createInfo.magFilter = g_filterMap[decl.eFilterMag];
	createInfo.minFilter = g_filterMap[decl.eFilterMin];
	createInfo.mipmapMode = g_mipMapMode[decl.eMipFilter];
	createInfo.addressModeU = g_textureClampMap[decl.eClampU];
	createInfo.addressModeV = g_textureClampMap[decl.eClampV];
	createInfo.addressModeW = g_textureClampMap[decl.eClampV];//VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	createInfo.mipLodBias = decl.LodBias;
	createInfo.anisotropyEnable = decl.eAnisoLevel != ANISO_LEVEL_1;
	createInfo.maxAnisotropy = g_anisoLevel[decl.eAnisoLevel];
	createInfo.compareEnable = decl.bEnableCmp;
	createInfo.compareOp = decl.bEnableCmp ? g_cmpFuncMap[decl.eCmpFnc] : VK_COMPARE_OP_NEVER;
	createInfo.minLod = (float)decl.LodMinLevel;
	createInfo.maxLod = 1000.0f;
	createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	createInfo.unnormalizedCoordinates = VK_FALSE;

	err = vkCreateSampler(vkDevice, &createInfo, pDevice->GetPlatform().GetAllocCallbacks(), &m_sampler);

	ASSERT(!err);
}

void Sampler::Cleanup(GFXDevice* pDevice)
{
	if (m_sampler)
	{
		vkDestroySampler(pDevice->GetPlatform().GetVKDevice(), m_sampler, pDevice->GetPlatform().GetAllocCallbacks());
		m_sampler = nullptr;
	}
}

}

