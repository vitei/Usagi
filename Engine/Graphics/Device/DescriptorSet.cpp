/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemUtil.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Device/DescriptorSetLayout.h"
#include "Engine/Graphics/Device/DescriptorSet.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "Engine/Graphics/Device/DescriptorData.h"

namespace usg {

DescriptorSet::DescriptorSet()
	: m_platform()
	, m_pData(nullptr)
	, m_bValid(false)
	, m_uDataCount(0)
	, m_uLastUpdate(USG_INVALID_ID)
	, m_layoutHndl()
	, m_pLayoutDesc(nullptr)
{
}

DescriptorSet::~DescriptorSet()
{
	ASSERT(m_pData == nullptr);
}



void DescriptorSet::CleanUp(GFXDevice* pDevice)
{
	m_platform.CleanUp(pDevice, m_layoutHndl.GetContents());
	if (m_pData)
	{
		vdelete[] m_pData;
		m_pData = nullptr;
	}

	m_pData = nullptr;
	m_bValid = false;
	m_uDataCount = 0;
	m_pLayoutDesc = nullptr;
}

void DescriptorSet::Init(GFXDevice* pDevice, const DescriptorSetLayoutHndl& layout)
{
	if (m_pData)
	{
		if (m_layoutHndl == layout)
		{
			// Already fine
			return;
		}
		CleanUp(pDevice);
	}
	m_pLayoutDesc = layout.GetContents();
	m_layoutHndl = layout;

	m_pData = vnew(ALLOC_OBJECT) DescriptorData[m_pLayoutDesc->GetResourceCount()];
	MemClear(m_pData, sizeof(DescriptorData)*m_pLayoutDesc->GetResourceCount());
	

	m_uDataCount = 0;
	for (uint32 i = 0; i < m_pLayoutDesc->GetDeclarationCount(); i++)
	{
		const DescriptorDeclaration* pDecl = m_pLayoutDesc->GetDeclaration(i);
		for (uint32 j = 0; j < pDecl->uCount; j++)
		{
			m_pData[m_uDataCount].eDescType = pDecl->eDescriptorType;
			m_pData[m_uDataCount].uLastUpdateIdx = USG_INVALID_ID;
			m_uDataCount++;
		}
	}

	m_platform.Init(pDevice, layout.GetContents());
}

void DescriptorSet::Init(GFXDevice* pDevice, const DescriptorSet& copy)
{
	Init(pDevice, copy.m_layoutHndl);

	// Copy the initialization data across
	uint32 uDataIndex = 0;
	for (uint32 i = 0; i < m_pLayoutDesc->GetDeclarationCount(); i++)
	{
		const DescriptorDeclaration* pDecl = m_pLayoutDesc->GetDeclaration(i);
		for (uint32 j = 0; j < pDecl->uCount; j++)
		{
			switch (pDecl->eDescriptorType)
			{
			case DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			case DESCRIPTOR_TYPE_STORAGE_IMAGE:
			{
				m_pData[uDataIndex].texData.tex = copy.m_pData[uDataIndex].texData.tex;
				m_pData[uDataIndex].texData.sampler = copy.m_pData[uDataIndex].texData.sampler;
				break;
			}
			case DESCRIPTOR_TYPE_CONSTANT_BUFFER:
			case DESCRIPTOR_TYPE_CONSTANT_BUFFER_DYNAMIC:
				m_pData[uDataIndex].pConstBuffer = copy.m_pData[uDataIndex].pConstBuffer;
				break;
			default:
				ASSERT(false);
				break;
			}
			uDataIndex++;
		}
	}

	// And GPU update
	UpdateDescriptors(pDevice);
}


void DescriptorSet::UpdateTimeTags()
{
	for (uint32 i = 0; i < m_pLayoutDesc->GetResourceCount(); i++)
	{
		switch (m_pData[i].eDescType)
		{
		case DESCRIPTOR_TYPE_CONSTANT_BUFFER:
		case DESCRIPTOR_TYPE_CONSTANT_BUFFER_DYNAMIC:
		{
			if (m_pData[i].pConstBuffer)
			{
				m_pData[i].uLastUpdateIdx = m_pData[i].pConstBuffer->GetUpdateIdx();
			}
			break;
		}
		case DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
		case DESCRIPTOR_TYPE_STORAGE_IMAGE:
		{
			if (m_pData[i].texData.tex.get() != nullptr)
			{
				m_pData[i].uLastUpdateIdx = m_pData[i].texData.tex->GetUpdateIdx();
			}
			break;
		}
		default:
			ASSERT(false);
		}
	}
}

bool DescriptorSet::IsUptoDate() const
{
	for (uint32 i = 0; i < m_pLayoutDesc->GetResourceCount(); i++)
	{
		switch(m_pData[i].eDescType)
		{
			case DESCRIPTOR_TYPE_CONSTANT_BUFFER:
			{
				if (!m_pData[i].pConstBuffer || (m_pData[i].pConstBuffer->GetUpdateIdx() != m_pData[i].uLastUpdateIdx) )
				{
					return false;
				}
				break;
			}
			case DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
			case DESCRIPTOR_TYPE_STORAGE_IMAGE:
			{
				if (!m_pData[i].texData.tex.get() || (m_pData[i].texData.tex->GetUpdateIdx() != m_pData[i].uLastUpdateIdx) )
				{
					return false;
				}
				break;
			}
			default:
				break;
		}
	}
	return true;
}

void DescriptorSet::SetImage(uint32 uLayoutIndex, const TextureHndl& pTexture, const ImageViewDef& imageView)
{
	ASSERT(uLayoutIndex < m_pLayoutDesc->GetDeclarationCount());
	const DescriptorDeclaration* pDecl = m_pLayoutDesc->GetDeclaration(uLayoutIndex);

	uint32 uResourceIndex = m_pLayoutDesc->GetResourceIndex(uLayoutIndex, 0);
	ASSERT(pDecl->eDescriptorType == DESCRIPTOR_TYPE_STORAGE_IMAGE);
	ASSERT(pTexture.get() != nullptr);

	// I feel so dirty doing this but we know for a fact that we created this pointer and it saves a lot of extra hassle to get around it
	m_pData[uResourceIndex].texData.tex = pTexture;
	m_pData[uResourceIndex].texData.sampler = SamplerHndl();
	m_pData[uResourceIndex].uLastUpdateIdx = USG_INVALID_ID;
	m_pData[uResourceIndex].texData.imageView = imageView;
}

void DescriptorSet::SetImageSamplerPair(uint32 uLayoutIndex, const TextureHndl& pTexture, const SamplerHndl& sampler, uint32 uSubIndex, const ImageViewDef& imageView)
{
	ASSERT(uLayoutIndex < m_pLayoutDesc->GetDeclarationCount());
	const DescriptorDeclaration* pDecl = m_pLayoutDesc->GetDeclaration(uLayoutIndex);

	uint32 uResourceIndex = m_pLayoutDesc->GetResourceIndex(uLayoutIndex, uSubIndex);
	ASSERT(pDecl->eDescriptorType == DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	ASSERT(pTexture.get()!=nullptr);

	// I feel so dirty doing this but we know for a fact that we created this pointer and it saves a lot of extra hassle to get around it
	m_pData[uResourceIndex].texData.tex = pTexture;
	m_pData[uResourceIndex].texData.sampler = sampler;
	m_pData[uResourceIndex].uLastUpdateIdx = USG_INVALID_ID;
	m_pData[uResourceIndex].texData.imageView = imageView;
}

void DescriptorSet::SetConstantSet(uint32 uLayoutIndex, const ConstantSet* pBuffer, uint32 uSubIndex)
{
	ASSERT(uLayoutIndex < m_pLayoutDesc->GetDeclarationCount());
	const DescriptorDeclaration* pDecl = m_pLayoutDesc->GetDeclaration(uLayoutIndex);
	ASSERT(uSubIndex < pDecl->uCount);

	ASSERT(pDecl->eDescriptorType == DESCRIPTOR_TYPE_CONSTANT_BUFFER || pDecl->eDescriptorType == DESCRIPTOR_TYPE_CONSTANT_BUFFER_DYNAMIC);
	ASSERT(pBuffer!=nullptr);
	uint32 uResourceIndex = m_pLayoutDesc->GetResourceIndex(uLayoutIndex, uSubIndex);

	m_pData[uResourceIndex].pConstBuffer = pBuffer;
	m_pData[uResourceIndex].uLastUpdateIdx = USG_INVALID_ID;
}


void DescriptorSet::SetImageAtBinding(uint32 uBinding, const TextureHndl& pTexture, const ImageViewDef& imageView)
{
	for (uint32 i = 0; i < m_pLayoutDesc->GetDeclarationCount(); i++)
	{
		const DescriptorDeclaration* pDecl = m_pLayoutDesc->GetDeclaration(i);
		if (pDecl->eDescriptorType == DESCRIPTOR_TYPE_STORAGE_IMAGE && uBinding == pDecl->uBinding)
		{
			SetImage(i, pTexture, imageView);
			return;
		}
	}
	ASSERT(false);
}


void DescriptorSet::SetImageSamplerPairAtBinding(uint32 uBinding, const TextureHndl& pTexture, const SamplerHndl& sampler, uint32 uSubIndex, const ImageViewDef& imageView)
{
	for (uint32 i = 0; i < m_pLayoutDesc->GetDeclarationCount(); i++)
	{
		const DescriptorDeclaration* pDecl = m_pLayoutDesc->GetDeclaration(i);
		if (pDecl->eDescriptorType == DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER && uBinding == pDecl->uBinding)
		{
			SetImageSamplerPair(i, pTexture, sampler, uSubIndex, imageView);
			return;
		}
	}
	ASSERT(false);
}

void DescriptorSet::SetConstantSetAtBinding(uint32 uBinding, const ConstantSet* pBuffer, uint32 uSubIndex, uint32 uFlags )
{
	bool bFound = false;
	for (uint32 i = 0; i < m_pLayoutDesc->GetDeclarationCount(); i++)
	{
		const DescriptorDeclaration* pDecl = m_pLayoutDesc->GetDeclaration(i);
		if ( (pDecl->eDescriptorType == DESCRIPTOR_TYPE_CONSTANT_BUFFER || pDecl->eDescriptorType == DESCRIPTOR_TYPE_CONSTANT_BUFFER_DYNAMIC)
			&& uBinding == pDecl->uBinding && (pDecl->shaderType & uFlags)!=0 )
		{
			SetConstantSet(i, pBuffer,uSubIndex);
			bFound = true;
		}
	}

	ASSERT(bFound);
}


TextureHndl DescriptorSet::GetTextureAtBinding(uint32 uBinding) const
{
	for (uint32 i = 0; i < m_pLayoutDesc->GetDeclarationCount(); i++)
	{
		const DescriptorDeclaration* pDecl = m_pLayoutDesc->GetDeclaration(i);
		if (pDecl->eDescriptorType == DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER && uBinding == pDecl->uBinding)
		{
			uint32 uDataIndex = m_pLayoutDesc->GetResourceIndex(i, 0);
			return m_pData[uDataIndex].texData.tex;
		}
	}
	return nullptr;
}

SamplerHndl DescriptorSet::GetSamplerAtBinding(uint32 uBinding) const
{
	for (uint32 i = 0; i < m_pLayoutDesc->GetDeclarationCount(); i++)
	{
		const DescriptorDeclaration* pDecl = m_pLayoutDesc->GetDeclaration(i);
		if (pDecl->eDescriptorType == DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER && uBinding == pDecl->uBinding)
		{
			uint32 uDataIndex = m_pLayoutDesc->GetResourceIndex(i, 0);
			return m_pData[uDataIndex].texData.sampler;
		}
	}
	ASSERT(false);
	return SamplerHndl();
}


void DescriptorSet::UpdateDescriptors(GFXDevice* pDevice)
{
	// Do our best to avoid doing this at all, anything updated each frame should use dynamic
	// constants or push constants (push constants not yet available)
	if (!m_bValid || !IsUptoDate())
	{
		bool bDoubleUpdate = m_uLastUpdate == pDevice->GetFrameCount();
		m_platform.UpdateDescriptors(pDevice, m_pLayoutDesc, m_pData, bDoubleUpdate);
		m_uLastUpdate = pDevice->GetFrameCount();
		UpdateTimeTags();
		m_bValid = true;
	}
}

}
