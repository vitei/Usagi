/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Effects/EffectBinding.h"



namespace usg {

	VkFormat GetAttribFormatByte(uint32 uCount, bool bNormalised)
	{
		switch (uCount)
		{
		case 1:
			return bNormalised ? VK_FORMAT_R8_UNORM : VK_FORMAT_R8_UINT;
		case 2:
			return bNormalised ? VK_FORMAT_R8G8_UNORM : VK_FORMAT_R8G8_UINT;
		case 3:
			return bNormalised ? VK_FORMAT_R8G8B8_UNORM : VK_FORMAT_R8G8B8_UINT;
		case 4:
			return bNormalised ? VK_FORMAT_R8G8B8A8_UNORM : VK_FORMAT_R8G8B8A8_UINT;
		default:
			ASSERT(false);
		}

		return VK_FORMAT_R8G8B8A8_UNORM;
	}

	VkFormat GetAttribFormatShort(uint32 uCount, bool bNormalised)
	{
		switch (uCount)
		{
		case 1:
			return bNormalised ? VK_FORMAT_R16_UNORM : VK_FORMAT_R16_UINT;
		case 2:
			return bNormalised ? VK_FORMAT_R16G16_UNORM : VK_FORMAT_R16G16_UINT;
		case 3:
			return bNormalised ? VK_FORMAT_R16G16B16_UNORM : VK_FORMAT_R16G16B16_UINT;
		case 4:
			return bNormalised ? VK_FORMAT_R16G16B16A16_UNORM : VK_FORMAT_R16G16B16A16_UINT;
		default:
			ASSERT(false);
		}

		return VK_FORMAT_R16G16B16A16_UNORM;
	}


	VkFormat GetAttribFormatFloat(uint32 uCount)
	{
		switch (uCount)
		{
		case 1:
			return VK_FORMAT_R32_SFLOAT;
		case 2:
			return VK_FORMAT_R32G32_SFLOAT;
		case 3:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case 4:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		default:
			ASSERT(false);
		}

		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}

	VkFormat GetAttribFormatInt(uint32 uCount)
	{
		switch (uCount)
		{
		case 1:
			return VK_FORMAT_R32_UINT;
		case 2:
			return VK_FORMAT_R32G32_UINT;
		case 3:
			return VK_FORMAT_R32G32B32_UINT;
		case 4:
			return  VK_FORMAT_R32G32B32A32_UINT;
		default:
			ASSERT(false);
		}

		return VK_FORMAT_R32G32B32A32_UINT;
	}


	VkFormat GetAttribFormat(VertexElementType eType, uint32 uCount, bool bNormalised)
	{
		switch (eType)
		{
		case VE_BYTE:
			return GetAttribFormatByte(uCount, bNormalised);
		case VE_SHORT:
			return GetAttribFormatShort(uCount, bNormalised);
		case VE_FLOAT:
			return GetAttribFormatFloat(uCount);
		case VE_INT:
			ASSERT(!bNormalised);
			return GetAttribFormatInt(uCount);
		default:
			ASSERT(false);
		}

		return VK_FORMAT_R32G32B32A32_UINT;
	}

EffectBinding_ps::EffectBinding_ps()
{
	for(uint32 i=0; i<MAX_VERTEX_BUFFERS; i++)
	{
		m_pDecl[i]		= NULL;
	}
	m_uBuffers	= 0;
	m_pInputAttribs = NULL;
}

EffectBinding_ps::~EffectBinding_ps()
{
	if (m_pInputAttribs != NULL)
	{
		vdelete[] m_pInputAttribs;
	}
}

void EffectBinding_ps::Init(GFXDevice* pDevice, EffectHndl pEffect, const VertexDeclaration** ppDecls, uint32 uCount)
{
	uint32 uElementCount = 0;
	uint32 uElement = 0;
	m_uBuffers = uCount;

	for (uint32 uBuffer = 0; uBuffer < uCount; uBuffer++)
	{
		const VertexElement* pElement = m_pDecl[uBuffer]->GetElements();
		m_pDecl[uBuffer] = ppDecls[uBuffer];
		for (uint32 i = 0; i < m_pDecl[uBuffer]->GetElementCount(); i++)
		{
			uint32 uLoop = pElement->uCount < 4 ? 1 : pElement->uCount / 4;
			uElementCount+= uLoop;
			pElement++;
		}
	}

	// FIXME: When stable this should all move over to a platofrm specific version of the vertex declaration (as we will be binding by index and not name this will become redundant)
	m_pInputAttribs = vnew(ALLOC_OBJECT) VkVertexInputAttributeDescription[uElement];

	for(uint32 uBuffer=0; uBuffer<uCount; uBuffer++)
	{
		m_pDecl[uBuffer] = ppDecls[uBuffer];

		m_bindingDesc[uBuffer].binding = uBuffer;
		m_bindingDesc[uBuffer].stride = (uint32)m_pDecl[uBuffer]->GetSize();
		if (m_pDecl[uBuffer]->IsInstanceStream())
		{
			ASSERT(false);	// Look at the documentation
		}
		else
		{
			m_bindingDesc[uBuffer].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		}

		const VertexElement* pElement = m_pDecl[uBuffer]->GetElements();
		for(uint32 i=0; i < m_pDecl[uBuffer]->GetElementCount(); i++)
		{
			ASSERT(uElement<MAX_VERTEX_STREAMS);
			ASSERT((pElement->uCount < 4) || (pElement->uCount % 4) == 0);
			uint32 uLoop = pElement->uCount < 4 ? 1 : pElement->uCount / 4;
			uint32 uCount = pElement->uCount < 4 ? pElement->uCount : 4;
			memsize uOffset = pElement->uOffset;
			for (uint32 j = 0; j < uLoop; j++)
			{
				m_pInputAttribs[uElement].binding = uBuffer;
				m_pInputAttribs[uElement].location = pElement->uAttribId;
				m_pInputAttribs[uElement].format = GetAttribFormat(pElement->eType, pElement->uCount, pElement->bNormalised);
				m_pInputAttribs[uElement].offset = (uint32)pElement->uOffset;
				uOffset += sizeof(float)*uCount;				
			}
			pElement++;
			uElement++;
		}
	}

	m_inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	m_inputState.pNext = nullptr;
	m_inputState.flags = 0;
	m_inputState.vertexBindingDescriptionCount = uCount;
	m_inputState.pVertexBindingDescriptions = &m_bindingDesc[0];
	m_inputState.vertexAttributeDescriptionCount = uElementCount;
	m_inputState.pVertexAttributeDescriptions = m_pInputAttribs;

}


}

