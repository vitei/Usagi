/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Effects, ConstantSet_ps.h)
#include <vulkan/vulkan.h>
#include <stdio.h>

namespace usg {

const uint32 g_uGPUFormatSize[CT_COUNT] =
{
	sizeof(Matrix4x4),	// CT_MATRIX_44 = 0,
	sizeof(Matrix4x3),	// CT_MATRIX_43,
	sizeof(Vector4f),	// CT_VECTOR_4,
	sizeof(Vector2f),	// CT_VECTOR_2,
	sizeof(float32),	// CT_FLOAT,
	sizeof(sint32),		// CT_INT
	4,					// CT_BOOL
	0,					// CT_STRUCT	(Not a valid size)
};

const uint32 g_uGPUAlignments[CT_COUNT] =
{
	sizeof(float)*4,	// CT_MATRIX_44
	sizeof(float)*4,	// CT_MATRIX_43
	sizeof(float)*4,	// CT_VECTOR_4
	sizeof(float)*2,	// CT_VECTOR_2
	sizeof(float),		// CT_FLOAT
	sizeof(int),		// CT_INT
	4,					// CT_BOOL
	sizeof(float)*4		// CT_STRUCT (Not a valid size on it's own)
};

ConstantSet_ps::ConstantSet_ps()
{
	m_bDataValid	= false;
	m_pOwner		= NULL;
	m_uActiveBuffer	= 0;
	m_pVarData		= 0;
}

ConstantSet_ps::~ConstantSet_ps()
{
	utl::SafeArrayDelete(&m_pVarData);
}

void ConstantSet_ps::Init(GFXDevice* pDevice, const ConstantSet& owner)
{
	GFXDevice_ps& devicePS = pDevice->GetPlatform();
	m_pOwner = &owner;	

	InitOffsetsAndGPUData( m_pOwner->GetDeclaration() );

	uint32 uOffset = 0;
	for (uint32 i = 0; i < GFX_NUM_DYN_BUFF; i++)
	{
		m_uOffsets[i] = uOffset;
		uOffset += m_uGPUSize;
	}


	VkMemoryRequirements memReqs;
	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkBufferCreateInfo bufCreateInfo = {};
	bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufCreateInfo.pNext = NULL;
	bufCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufCreateInfo.size = m_uGPUSize * GFX_NUM_DYN_BUFF;
	bufCreateInfo.flags = 0;

	VkResult eResult = vkCreateBuffer(devicePS.GetVKDevice(), &bufCreateInfo, devicePS.GetAllocCallbacks(), &m_buffer);
	ASSERT(eResult == VK_SUCCESS);

	vkGetBufferMemoryRequirements(devicePS.GetVKDevice(), m_buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = pDevice->GetPlatform().GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	eResult = vkAllocateMemory(devicePS.GetVKDevice(), &memAlloc, devicePS.GetAllocCallbacks(), &m_memory);
	ASSERT(eResult == VK_SUCCESS);


	eResult = vkBindBufferMemory(devicePS.GetVKDevice(), m_buffer, m_memory, 0);
	ASSERT(eResult == VK_SUCCESS);

	m_descriptor.offset = 0;
	m_descriptor.buffer = m_buffer;
	m_descriptor.range = m_uGPUSize;
}

void ConstantSet_ps::CleanUp(GFXDevice* pDevice)
{
	// Not valid if the owner isn't
	if (m_pOwner)
	{
		GFXDevice_ps& devicePS = pDevice->GetPlatform();

		vkDestroyBuffer(devicePS.GetVKDevice(), m_buffer, devicePS.GetAllocCallbacks());
	}
}


void ConstantSet_ps::AppendOffsets(const ShaderConstantDecl* pDecl, uint32 uOffset, uint32& uSize, uint32& uVars)
{
	while(pDecl->eType != CT_INVALID)
	{
		if(pDecl->eType == CT_STRUCT)
		{
			uint32 uExOffset = 0;
			for(uint32 i=0; i<pDecl->uiCount; i++)
			{
				AppendOffsets(pDecl->pSubDecl, (uint32)(pDecl->uiOffset + (pDecl->uiSize*i)), uSize, uVars);
			}
		}
		else
		{
			uint32 uAlign = g_uGPUAlignments[pDecl->eType];
			uint32 uItemSize = g_uGPUFormatSize[pDecl->eType];
			if (pDecl->uiCount > 1)
			{
				// When we have arrays each element takes up a full vec4 to allow address indexing
				uAlign = Math::Max((uint32)sizeof(float) * 4, uAlign);
				uItemSize = Math::Max((uint32)sizeof(float) * 4, uItemSize);
			}
			uItemSize *= pDecl->uiCount;
			uSize = (uSize + uAlign - 1) - ((uSize + uAlign - 1) % uAlign);
			m_pVarData[uVars].uOffsetDst= uSize;
			m_pVarData[uVars].uOffsetSrc= uOffset + (uint32)pDecl->uiOffset;
			m_pVarData[uVars].eType		= pDecl->eType;
			m_pVarData[uVars].uCount 	= pDecl->uiCount;
			uSize += (uItemSize);
			uVars++;
		}
		pDecl++;
	}	
}

void ConstantSet_ps::InitOffsetsAndGPUData(const ShaderConstantDecl* pDecl)
{
	// TODO: Proper allocation, alignment issues, etc
	uint32 uSize = 0;
	uint32 uVars = 0;
	m_pVarData	= vnew(ALLOC_SHADER_CONSTANTS) VariableData[m_pOwner->GetVarCount()];

	AppendOffsets(pDecl, 0, uSize, uVars);
	m_uGPUSize = Math::Roundup(uSize, 16);
	ASSERT(uVars == m_pOwner->GetVarCount());
}


void ConstantSet_ps::UpdateBuffer(GFXDevice* pDevice, bool bDoubleUpdate)
{	
	// FIXME: Assert this only called once per frame
	// we'll probably want to use standard constant setting for effects
	if(!bDoubleUpdate)
		m_uActiveBuffer = (m_uActiveBuffer+1)%GFX_NUM_DYN_BUFF;

	uint8* pCPUData = (uint8*)m_pOwner->GetCPUData();
	uint32 uVarCount = m_pOwner->GetVarCount();
	//const ShaderConstantDecl* pDecl = m_pOwner->GetDeclaration();
	// All of the matrices in our data to be transposed before passing them over to the GPU
	const ShaderConstantDecl* pDecl = m_pOwner->GetDeclaration();
	const VariableData* pVarData = m_pVarData;

	void* pGPUData;
	VkResult err = vkMapMemory(pDevice->GetPlatform().GetVKDevice(), m_memory, GetActiveBufferOffset(), m_uGPUSize, 0, &pGPUData);
	ASSERT(err == VK_SUCCESS);

	for(uint32 i=0; i<uVarCount; i++)
	{
		uint8* pLoc = ((uint8*)pGPUData) + pVarData->uOffsetDst;
		switch( pVarData->eType )
		{
		case CT_MATRIX_44:
			WriteMatrix44((Matrix4x4*)(pCPUData+pVarData->uOffsetSrc), pVarData->uCount, pLoc);
			break;
		case CT_MATRIX_43:
			WriteMatrix43((Matrix4x3*)(pCPUData+pVarData->uOffsetSrc), pVarData->uCount, pLoc);
			break;
		case CT_VECTOR_4:
			WriteVector4((Vector4f*)(pCPUData+pVarData->uOffsetSrc), pVarData->uCount, pLoc);
			break;
		case CT_VECTOR_2:
			WriteVector2((Vector2f*)(pCPUData + pVarData->uOffsetSrc), pVarData->uCount, pLoc);
			break;
		case CT_FLOAT:
			WriteFloat((float*)(pCPUData + pVarData->uOffsetSrc), pVarData->uCount, pLoc);
			break;
		case CT_INT:
			WriteInt((int*)(pCPUData + pVarData->uOffsetSrc), pVarData->uCount, pLoc);
			break;
		case CT_BOOL:
			WriteBool((bool*)(pCPUData + pVarData->uOffsetSrc), pVarData->uCount, pLoc);
			break;
		default:
			ASSERT(false);
			break;
		}
		pVarData++;
	}

	vkUnmapMemory(pDevice->GetPlatform().GetVKDevice(), m_memory);

	m_bDataValid = true;
}



}