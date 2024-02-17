/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "InputBinding_ps.h"


namespace usg {

	VkFormat GetAttribFormatByte(uint32 uCount, bool bNormalised)
	{
		switch (uCount)
		{
		case 1:
			return bNormalised ? VK_FORMAT_R8_SNORM : VK_FORMAT_R8_SINT;
		case 2:
			return bNormalised ? VK_FORMAT_R8G8_SNORM : VK_FORMAT_R8G8_SINT;
		case 3:
			return bNormalised ? VK_FORMAT_R8G8B8_SNORM : VK_FORMAT_R8G8B8_SINT;
		case 4:
			return bNormalised ? VK_FORMAT_R8G8B8A8_SNORM : VK_FORMAT_R8G8B8A8_SINT;
		default:
			ASSERT(false);
		}

		return VK_FORMAT_R8G8B8A8_UNORM;
	}

	VkFormat GetAttribFormatUByte(uint32 uCount, bool bNormalised)
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
			return VK_FORMAT_R32_SINT;
		case 2:
			return VK_FORMAT_R32G32_SINT;
		case 3:
			return VK_FORMAT_R32G32B32_SINT;
		case 4:
			return  VK_FORMAT_R32G32B32A32_SINT;
		default:
			ASSERT(false);
		}

		return VK_FORMAT_R32G32B32A32_SINT;
	}

	VkFormat GetAttribFormatUInt(uint32 uCount)
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
		case VE_UBYTE:
			return GetAttribFormatUByte(uCount, bNormalised);
		case VE_SHORT:
			return GetAttribFormatShort(uCount, bNormalised);
		case VE_FLOAT:
			return GetAttribFormatFloat(uCount);
		case VE_INT:
			ASSERT(!bNormalised);
			return GetAttribFormatInt(uCount);
		case VE_UINT:
			ASSERT(!bNormalised);
			return GetAttribFormatUInt(uCount);			
		default:
			ASSERT(false);
		}

		return VK_FORMAT_R32G32B32A32_UINT;
	}

InputBinding_ps::InputBinding_ps()
{
	m_uBuffers	= 0;
	m_pInputAttribs = NULL;
}

InputBinding_ps::~InputBinding_ps()
{
	if (m_pInputAttribs != NULL)
	{
		vdelete[] m_pInputAttribs;
	}
}

void InputBinding_ps::Init(GFXDevice* pDevice, const VertexDeclaration** ppDecls, uint32 uBufferCount)
{
	uint32 uElementCount = 0;
	uint32 uElement = 0;
	m_uBuffers = uBufferCount;

	for (uint32 uBuffer = 0; uBuffer < uBufferCount; uBuffer++)
	{
		const VertexElement* pElement = ppDecls[uBuffer]->GetElements();
		for (uint32 i = 0; i < ppDecls[uBuffer]->GetElementCount(); i++)
		{
			uint32 uLoop = pElement->uCount < 4 ? 1 : pElement->uCount / 4;
			uElementCount+= uLoop;
			pElement++;
		}
	}

	m_pInputAttribs = vnew(ALLOC_OBJECT) VkVertexInputAttributeDescription[uElementCount];

	for(uint32 uBuffer=0; uBuffer<uBufferCount; uBuffer++)
	{
		m_bindingDesc[uBuffer].binding = uBuffer;
		m_bindingDesc[uBuffer].stride = (uint32)ppDecls[uBuffer]->GetSize();
		if (ppDecls[uBuffer]->IsInstanceStream())
		{
			m_bindingDesc[uBuffer].inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
			m_bindingDesc[uBuffer].stride *= ppDecls[uBuffer]->GetInstanceDiv();
		}
		else
		{
			m_bindingDesc[uBuffer].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		}

		const VertexElement* pElement = ppDecls[uBuffer]->GetElements();
		for(uint32 i=0; i < ppDecls[uBuffer]->GetElementCount(); i++)
		{
			ASSERT(uElement<MAX_VERTEX_ATTRIBUTES);
			ASSERT((pElement->uCount < 4) || (pElement->uCount % 4) == 0);
			uint32 uLoop = pElement->uCount < 4 ? 1 : pElement->uCount / 4;
			uint32 uCount = pElement->uCount < 4 ? pElement->uCount : 4;
			memsize uOffset = pElement->uOffset;
			for (uint32 j = 0; j < uLoop; j++)
			{
				m_pInputAttribs[uElement].binding = uBuffer;
				m_pInputAttribs[uElement].location = pElement->uAttribId+ j;
				m_pInputAttribs[uElement].format = GetAttribFormat(pElement->eType, uCount, pElement->bNormalised);
				m_pInputAttribs[uElement].offset = (uint32)pElement->uOffset + (j*(sizeof(float)*4));
				uOffset += sizeof(float)*uCount;				
				uElement++;
			}
			pElement++;
		}
	}

	#ifdef DEBUG_BUILD
	for (uint32 i = 0; i < uElementCount; i++)
	{
		for (uint32 j = i+1; j < uElementCount; j++)
		{
			ASSERT(m_pInputAttribs[i].location != m_pInputAttribs[j].location);
		}
	}
	#endif

	m_inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	m_inputState.pNext = nullptr;
	m_inputState.flags = 0;
	m_inputState.vertexBindingDescriptionCount = uBufferCount;
	m_inputState.pVertexBindingDescriptions = &m_bindingDesc[0];
	m_inputState.vertexAttributeDescriptionCount = uElementCount;
	m_inputState.pVertexAttributeDescriptions = m_pInputAttribs;

}


}

