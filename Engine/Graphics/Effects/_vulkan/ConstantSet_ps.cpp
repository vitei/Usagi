/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include API_HEADER(Engine/Graphics/Effects, ConstantSet_ps.h)
#include <vulkan/vulkan.h>
#include <stdio.h>

namespace usg {

const uint32 g_uGPUFormatSize[CT_COUNT] =
{
	sizeof(Matrix4x4),	// CT_MATRIX_44 = 0,
	sizeof(Matrix4x3),	// CT_MATRIX_43,
	sizeof(Vector4f),	// CT_VECTOR_4,
	sizeof(Vector3f),	// CT_VECTOR_3,
	sizeof(Vector2f),	// CT_VECTOR_2,
	sizeof(float32),	// CT_FLOAT,
	sizeof(sint32),		// CT_INT
	sizeof(sint32)*2,	// CT_VECTOR2I
	sizeof(sint32)*4,	// CT_VECTOR4I
	sizeof(uint32)*4,	// CT_VECTOR4U
	4,					// CT_BOOL
	0,					// CT_STRUCT	(Not a valid size)
};

const uint32 g_uGPUAlignments[CT_COUNT] =
{
	sizeof(float)*4,	// CT_MATRIX_44
	sizeof(float)*4,	// CT_MATRIX_43
	sizeof(float)*4,	// CT_VECTOR_4
	sizeof(float)*4,	// CT_VECTOR_3
	sizeof(float)*2,	// CT_VECTOR_2
	sizeof(float),		// CT_FLOAT
	sizeof(int),		// CT_INT
	sizeof(int)*2,		// CT_VECTOR2I
	sizeof(int)*4,		// CT_VECTOR4I
	sizeof(int) * 4,	// CT_VECTOR4U
	4,					// CT_BOOL
	sizeof(float)*4		// CT_STRUCT (Not a valid size on it's own)
};

static_assert(ARRAY_SIZE(g_uGPUFormatSize) == CT_COUNT, "Invalid array size");
static_assert(ARRAY_SIZE(g_uGPUAlignments) == CT_COUNT, "Invalid array size");

ConstantSet_ps::ConstantSet_ps()
{
	m_bDataValid	= false;
	m_pOwner		= NULL;
	m_buffer = VK_NULL_HANDLE;
	m_uActiveBuffer	= 0;
	m_pVarData		= 0;
	m_uGPUSize = 0;
	m_uBufferCount = 0;
}

ConstantSet_ps::~ConstantSet_ps()
{
	ASSERT(m_buffer == VK_NULL_HANDLE);
	utl::SafeArrayDelete(&m_pVarData);
}


 void ConstantSet_ps::Init(GFXDevice* pDevice, const ConstantSet& owner, GPUUsage eUsage)
{
	// TODO: Obey usage so we don't allocate extra memory for static buffers
	GFXDevice_ps& devicePS = pDevice->GetPlatform();
	memsize uPerBufferAlign = devicePS.GetPhysicalProperties(0)->limits.minUniformBufferOffsetAlignment;
	uPerBufferAlign = Math::Max(uPerBufferAlign, sizeof(usg::Matrix4x4));
	m_pOwner = &owner;	

	m_uBufferCount = GFX_NUM_DYN_BUFF;

	InitOffsets( m_pOwner->GetDeclaration() );
	memsize uUnAlignedSize = m_uGPUSize;
	m_uGPUSize = AlignSizeUp(m_uGPUSize, uPerBufferAlign);

	VkMemoryRequirements memReqs;
	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

	VkBufferCreateInfo bufCreateInfo = {};
	bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufCreateInfo.pNext = NULL;
	bufCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufCreateInfo.size = m_uGPUSize * m_uBufferCount;
	bufCreateInfo.flags = 0;

	VkResult eResult = vkCreateBuffer(devicePS.GetVKDevice(), &bufCreateInfo, nullptr, &m_buffer);
	ASSERT(eResult == VK_SUCCESS);
	pDevice->GetPlatform().SetObjectDebugName((uint64)m_buffer, VK_OBJECT_TYPE_BUFFER, "Constant set buffer");

	vkGetBufferMemoryRequirements(devicePS.GetVKDevice(), m_buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = pDevice->GetPlatform().GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	m_memoryAlloc.Init(memAlloc.memoryTypeIndex, (uint32)memAlloc.allocationSize, usg::Math::Max((uint32)uPerBufferAlign, (uint32)memReqs.alignment), true);
	pDevice->GetPlatform().AllocateMemory(&m_memoryAlloc);

	m_pBoundGPUData = m_memoryAlloc.GetMappedMemory();

	eResult = vkBindBufferMemory(devicePS.GetVKDevice(), m_buffer, m_memoryAlloc.GetMemory(), m_memoryAlloc.GetMemOffset());
	ASSERT(eResult == VK_SUCCESS);

	// Used for dynamic uniform buffers
	memsize uOffset = 0;
	for (uint32 i = 0; i < m_uBufferCount; i++)
	{
		m_uOffsets[i] = (uint32)uOffset;
		uOffset += m_uGPUSize;
	}

	// Only 8 dynamic uniform buffers are guaranteed across all stages (the no is 15 on my 1070) so need to support
	// buffer flipping for standard constant buffers too
	for (uint32 i = 0; i < m_uBufferCount; i++)
	{
		m_descriptor[i].offset = i * m_uGPUSize;
		m_descriptor[i].buffer = m_buffer;
		m_descriptor[i].range = uUnAlignedSize;
	}
}

void ConstantSet_ps::Cleanup(GFXDevice* pDevice)
{
	// Not valid if the owner isn't
	GFXDevice_ps& devicePS = pDevice->GetPlatform();
	if (m_buffer != VK_NULL_HANDLE)
	{
		pDevice->GetPlatform().ReqDestroyBuffer(m_buffer);
		m_buffer = VK_NULL_HANDLE;
	}

	pDevice->GetPlatform().FreeMemory(&m_memoryAlloc);
}

uint32 ConstantSet_ps::GetGPUAlignment(ConstantType eType, uint32 uCount)
{
	memsize uAlign =  (memsize)g_uGPUAlignments[eType];
	if (uCount > 1)
	{
		uAlign = usg::AlignSizeUp(uAlign, 16);
	}
	return (uint32)uAlign;
}

uint32 ConstantSet_ps::GetGPUSize(ConstantType eType, uint32 uCount)
{
	memsize uSize = (memsize)g_uGPUFormatSize[eType];
	if (uCount > 1)
	{
		uSize = usg::AlignSizeUp(uSize, 16);
	}
	return (uint32)uSize;
}


void ConstantSet_ps::AppendOffsets(const ShaderConstantDecl* pDecl, uint32 uOffset, uint32& uSize, uint32& uVars)
{
	while(pDecl->eType != CT_INVALID)
	{
		if(pDecl->eType == CT_STRUCT)
		{
			for(uint32 i=0; i<pDecl->uiCount; i++)
			{
				AppendOffsets(pDecl->pSubDecl, (uint32)(pDecl->uiOffset + (pDecl->uiSize*i)), uSize, uVars);
				// Structs aligned to a register
				uSize = (uSize + 16 - 1) - ((uSize + 16 - 1) % 16);
			}
		}
		else
		{
			uint32 uAlign = GetGPUAlignment(pDecl->eType, pDecl->uiCount);
			uint32 uItemSize = GetGPUSize(pDecl->eType, pDecl->uiCount);
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

void ConstantSet_ps::InitOffsets(const ShaderConstantDecl* pDecl)
{
	// TODO: Proper allocation, alignment issues, etc
	uint32 uSize = 0;
	uint32 uVars = 0;
	m_pVarData	= vnew(ALLOC_SHADER_CONSTANTS) VariableData[m_pOwner->GetVarCount()];

	AppendOffsets(pDecl, 0, uSize, uVars);
	m_uGPUSize = uSize;

	ASSERT(uVars == m_pOwner->GetVarCount());
}

void* ConstantSet_ps::GetGPUData(uint32 uActiveBuffer)
{
	void* pGPUData = (void*)((uint8*)(m_pBoundGPUData) +(m_uGPUSize * uActiveBuffer));
	return pGPUData;
}


void ConstantSet_ps::UpdateBuffer(GFXDevice* pDevice, bool bDoubleUpdate)
{	
	if(!bDoubleUpdate)
		m_uActiveBuffer = (m_uActiveBuffer+1)% m_uBufferCount;

	GFXDevice_ps& devicePS = pDevice->GetPlatform();

	uint8* pCPUData = (uint8*)m_pOwner->GetCPUData();
	uint32 uVarCount = m_pOwner->GetVarCount();
	//const ShaderConstantDecl* pDecl = m_pOwner->GetDeclaration();
	// All of the matrices in our data to be transposed before passing them over to the GPU
	const ShaderConstantDecl* pDecl = m_pOwner->GetDeclaration();
	const VariableData* pVarData = m_pVarData;

	void* pGPUData = GetGPUData(m_uActiveBuffer);

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
		case CT_VECTOR_3:
			WriteVector3((Vector3f*)(pCPUData + pVarData->uOffsetSrc), pVarData->uCount, pLoc);
			break;
		case CT_VECTOR_2:
		case CT_VECTOR2I:
			WriteVector2((Vector2f*)(pCPUData + pVarData->uOffsetSrc), pVarData->uCount, pLoc);
			break;
		case CT_FLOAT:
			WriteFloat((float*)(pCPUData + pVarData->uOffsetSrc), pVarData->uCount, pLoc);
			break;
		case CT_INT:
			WriteInt((int*)(pCPUData + pVarData->uOffsetSrc), pVarData->uCount, pLoc);
			break;
		case CT_VECTOR4I:
		case CT_VECTOR4U:
			WriteVector4((Vector4f*)(pCPUData + pVarData->uOffsetSrc), pVarData->uCount, pLoc);
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

	m_bDataValid = true;
}



}