/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include API_HEADER(Engine/Graphics/Primitives, IndexBuffer_ps.h)

namespace usg {


IndexBuffer_ps::IndexBuffer_ps()
	: m_IBO{}
	, m_uActiveIBO(0)
	, m_uBufferCount(0)
	, m_type(0)
	, m_uElementCount(0)
	, m_uCount(0)
	, m_uIndexSize(0)
{
}

IndexBuffer_ps::~IndexBuffer_ps()
{
	ASSERT(m_uBufferCount == 0);
}


void IndexBuffer_ps::Init(GFXDevice* pDevice, const void* const pIndices, uint32 uCount, uint32 uIndexSize, bool bStatic, GPULocation eLocation)
{
	m_uBufferCount  = bStatic ? 1 : GFX_NUM_DYN_BUFF;
	m_uCount        = uCount;
	m_uElementCount = uCount;
	m_uIndexSize    = uIndexSize;

	glGenBuffers(m_uBufferCount, m_IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, uIndexSize * uCount, pIndices, bStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);

  switch( uIndexSize )
  {
    case 1:
    {
      m_type = GL_UNSIGNED_BYTE;
      break;
    }

    case 2:
    {
      m_type = GL_UNSIGNED_SHORT;
      break;
    }

    case 4:
    {
    	m_type = GL_UNSIGNED_INT;
    	break;
    }

    default:
    {
    	// Not a valid index size.
    	ASSERT(false);
    	break;
    }
  }
}

void IndexBuffer_ps::CleanUp(GFXDevice* pDevice)
{
	if (m_uBufferCount)
	{
		glDeleteBuffers(m_uBufferCount, m_IBO);
	}
	m_uBufferCount = 0;
}

void IndexBuffer_ps::SetContents(GFXDevice* pDevice, const void* const pData, uint32 uIndexCount)
{
	m_uActiveIBO = (m_uActiveIBO + 1) % m_uBufferCount;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IBO[m_uActiveIBO]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_uIndexSize * uIndexCount, pData, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

}
