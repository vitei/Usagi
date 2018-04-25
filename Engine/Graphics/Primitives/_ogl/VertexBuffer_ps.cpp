/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Effects/Effect.h"
#include API_HEADER(Engine/Graphics/Primitives, VertexBuffer_ps.h)
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

namespace usg {

VertexBuffer_ps::VertexBuffer_ps()
{
	m_uBufferCount = 0;
	m_uActiveVBO = 0;
}

VertexBuffer_ps::~VertexBuffer_ps()
{
	ASSERT(m_uBufferCount == 0);
}

void VertexBuffer_ps::Init(GFXDevice* pDevice, const void* const pVerts, uint32 uDataSize, GPUUsage eUpdateType, GPULocation eLocation)
{
	m_uBufferCount = eUpdateType == GPU_USAGE_STATIC ? 1 : GFX_NUM_DYN_BUFF;
	glGenBuffers(m_uBufferCount, m_VBO);
	for(uint32 i = 0; i < m_uBufferCount; i++)
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO[i]);
		glBufferData(GL_ARRAY_BUFFER, uDataSize, pVerts, eUpdateType == GPU_USAGE_STATIC ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
	}
	CHECK_OGL_ERROR();
}

void VertexBuffer_ps::CleanUp(GFXDevice* pDevice)
{
	if (m_uBufferCount)
	{
		glDeleteBuffers(m_uBufferCount, m_VBO);
	}
	m_uBufferCount = 0;
}


void* VertexBuffer_ps::LockData(GFXDevice* pDevice, uint32 uSize)
{
	m_uActiveVBO = (m_uActiveVBO + 1) % m_uBufferCount;
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO[m_uActiveVBO]);
	void* pData = glMapBufferRange(GL_ARRAY_BUFFER, 0, uSize, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT); 
	return pData;
}

void VertexBuffer_ps::UnlockData(GFXDevice* pDevice, void* pData, uint32 uSize)
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
}


void VertexBuffer_ps::SetContents(GFXDevice* pDevice, const void* const pData, uint32 uSize)
{
	// Could consider using glMapBufferRange and avoiding these copies, but we shouldn't be updating many VBOs
	m_uActiveVBO = (m_uActiveVBO + 1) % m_uBufferCount;
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO[m_uActiveVBO]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, uSize, pData);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	CHECK_OGL_ERROR();
}

}
