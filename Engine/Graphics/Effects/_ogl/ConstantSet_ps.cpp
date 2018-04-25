/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "Engine/Core/Utility.h"
#include API_HEADER(Engine/Graphics/Effects, ConstantSet_ps.h)
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)
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
	sizeof(sint32)*4,	// CT_INT_4
	4,					// CT_BOOL
	0,					// CT_STRUCT	(Not a valid size)
};

static_assert(ARRAY_SIZE(g_uGPUFormatSize) == CT_COUNT, "Entries in g_GPUFormatSize don't match the constant count in the ConstantType enum, has an extra value been added?");


const uint32 g_uGPUAlignments[CT_COUNT] =
{
	sizeof(float)*4,	// CT_MATRIX_44
	sizeof(float)*4,	// CT_MATRIX_43
	sizeof(float)*4,	// CT_VECTOR_4
	sizeof(float)*2,	// CT_VECTOR_2
	sizeof(float),		// CT_FLOAT
	sizeof(int),		// CT_INT
	sizeof(int),		// CT_INT_4
	4,					// CT_BOOL
	sizeof(float)*4		// CT_STRUCT (Not a valid size on it's own)
};

static_assert(ARRAY_SIZE(g_uGPUAlignments) == CT_COUNT, "Entries in g_uGPUAlignments don't match the constant count in the ConstantType enum, has an extra value been added?");

ConstantSet_ps::ConstantSet_ps()
{
	m_bDataValid	= false;
	m_pGPUData		= NULL;
	m_pOwner		= NULL;
	m_uActiveBuffer	= 0;
	m_pVarData		= 0;
}

ConstantSet_ps::~ConstantSet_ps()
{
	ASSERT(m_bDataValid == false);
}

void ConstantSet_ps::CleanUp(GFXDevice* pDevice)
{
	if (m_pGPUData)
	{
		mem::Free(m_pGPUData);
		m_pGPUData = nullptr;
		utl::SafeArrayDelete(&m_pVarData);
		glDeleteBuffers(BUFFER_COUNT, m_buffers);
		m_pVarData = nullptr;
		m_bDataValid = false;
	}
}

void ConstantSet_ps::Init(GFXDevice* pDevice, const ConstantSet& owner)
{
	m_pOwner = &owner;	
	m_bDynamic = false;

	glGenBuffers(BUFFER_COUNT, m_buffers);
	InitOffsetsAndGPUData( m_pOwner->GetDeclaration() );
}


void ConstantSet_ps::AppendOffsets(const ShaderConstantDecl* pDecl, memsize uOffset, memsize& uSize, uint32& uVars)
{
	while(pDecl->eType != CT_INVALID)
	{
		if(pDecl->eType == CT_STRUCT)
		{
			uint32 uExOffset = 0;
			for(uint32 i=0; i<pDecl->uiCount; i++)
			{
				AppendOffsets(pDecl->pSubDecl, pDecl->uiOffset + (pDecl->uiSize*i), uSize, uVars);
			}
		}
		else
		{
			uint32 uAlign = g_uGPUAlignments[pDecl->eType];
			uint32 uItemSize = g_uGPUFormatSize[pDecl->eType];
			if (pDecl->uiCount > 1)
			{
				// When we have arrays each element takes up a full vec4 to allow address indexing
				uAlign = Math::Max((uint32)(sizeof(float) * 4), uAlign);
				uItemSize = Math::Max((uint32)(sizeof(float) * 4), uItemSize);
			}
			uItemSize *= pDecl->uiCount;
			uSize = (uSize + uAlign - 1) - ((uSize + uAlign - 1) % uAlign);
			m_pVarData[uVars].uOffsetDst= uSize;
			m_pVarData[uVars].uOffsetSrc= uOffset + pDecl->uiOffset;
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
	memsize uSize = 0;
	uint32 uVars = 0;
	m_pVarData	= vnew(ALLOC_SHADER_CONSTANTS) VariableData[m_pOwner->GetVarCount()];

	AppendOffsets(pDecl, 0, uSize, uVars);
	m_uGPUSize = Math::Roundup(uSize, 16);
	ASSERT(uVars == m_pOwner->GetVarCount());
	// Doesn't take into account alignment
	//ASSERT(uSize == m_pOwner->GetSize());
	m_pGPUData = vnew(ALLOC_SHADER_CONSTANTS) uint8[uSize];
}


void ConstantSet_ps::UpdateBuffer(GFXDevice* pDevice, bool bDoubleUpdate)
{	
	// FIXME: Assert this only called once per frame
	// we'll probably want to use standard constant setting for effects
	if(!bDoubleUpdate)
		m_uActiveBuffer = (m_uActiveBuffer+1)%BUFFER_COUNT;

	uint8* pCPUData = (uint8*)m_pOwner->GetCPUData();
	uint32 uVarCount = m_pOwner->GetVarCount();
	//const ShaderConstantDecl* pDecl = m_pOwner->GetDeclaration();
	// All of the matrices in our data to be transposed before passing them over to the GPU
	const ShaderConstantDecl* pDecl = m_pOwner->GetDeclaration();
	const VariableData* pVarData = m_pVarData;

	for(uint32 i=0; i<uVarCount; i++)
	{
		uint8* pLoc = ((uint8*)m_pGPUData) + pVarData->uOffsetDst;
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
		case CT_VECTOR4I:
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

	GLuint buffer =  m_buffers[m_uActiveBuffer];
	glBindBuffer(GL_UNIFORM_BUFFER, buffer);
	glBufferData(GL_UNIFORM_BUFFER, m_uGPUSize, m_pGPUData, GL_STREAM_DRAW);
	m_bDataValid = true;
}


void ConstantSet_ps::Bind(ShaderConstantType eConstType) const
{
	// TODO: Check not active in this slot
	{
		glBindBufferBase(GL_UNIFORM_BUFFER, eConstType+1, m_buffers[m_uActiveBuffer]);
	}
}

}