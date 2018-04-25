/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Effects/Effect.h"
#include "InputBinding_ps.h"


#define BUFFER_OFFSET(i) ((void*)(i))

namespace usg {

const int VARTYPE_MAP[] =
{
	GL_BYTE,			//VE_BYTE = 0,
	GL_UNSIGNED_BYTE,	//VE_UBYTE
	GL_SHORT,			//VE_SHORT,
	GL_UNSIGNED_SHORT,	//VE_USHORT,
	GL_FLOAT,			//VE_FLOAT,
	GL_UNSIGNED_INT		//VE_INT,
};

InputBinding_ps::InputBinding_ps()
{
	for(uint32 i=0; i<MAX_VERTEX_BUFFERS; i++)
	{
		m_pDecl[i]		= NULL;
	}
	m_uBuffers	= 0;
}

InputBinding_ps::~InputBinding_ps()
{

}

void InputBinding_ps::Init(GFXDevice* pDevice, const VertexDeclaration** ppDecls, uint32 uBufferCount)
{
	uint32 uElement = 0;
	m_uBuffers = uBufferCount;

	for(uint32 uBuffer=0; uBuffer<uBufferCount; uBuffer++)
	{
		m_pDecl[uBuffer] = ppDecls[uBuffer];
		m_uMapOffset[uBuffer] = uElement;

		const VertexElement* pElement = m_pDecl[uBuffer]->GetElements();
		for(uint32 i=0; i < m_pDecl[uBuffer]->GetElementCount(); i++)
		{
			ASSERT(uElement<MAX_VERTEX_ATTRIBUTES);
			m_mapping[uElement] = pElement->uAttribId;

			ASSERT((pElement->uCount < 4) || (pElement->uCount % 4) == 0);
			pElement++;
			uElement++;
		}
	}
}


void InputBinding_ps::UpdateVBOMapping(uint32 uBuffer) const
{
	uint32 uElement = 0;

	const VertexDeclaration* pDecl = m_pDecl[uBuffer];
	if (!pDecl)
		return;	// Sometimes depth passes don't use all the input buffers
	const VertexElement* pElement = pDecl->GetElements();
	memsize uStride = pDecl->GetSize();

	for(uint32 i=0; i<pDecl->GetElementCount(); i++)
	{
		uint32 uStream = m_mapping[m_uMapOffset[uBuffer]+uElement];
		uint32 uInstanceDiv = pDecl->GetInstanceDiv();
		if( uStream != (-1) )
		{
			uint32 uLoop = pElement->uCount < 4 ? 1 : pElement->uCount/4;
			uint32 uCount = pElement->uCount < 4 ? pElement->uCount : 4;
			memsize uOffset = pElement->uOffset;
			for(uint32 j=0; j<uLoop; j++)
			{
				glEnableVertexAttribArray(uStream+j); 
				if( pElement->eType != VE_FLOAT && !pElement->bNormalised && pElement->bIntegerReg )
				{
					glVertexAttribIPointer(uStream+j, uCount, VARTYPE_MAP[pElement->eType], (GLsizei)uStride, BUFFER_OFFSET(uOffset));
				}
				else
				{
					glVertexAttribPointer(uStream+j, uCount, VARTYPE_MAP[pElement->eType], pElement->bNormalised, (GLsizei)uStride, BUFFER_OFFSET(uOffset));	
				}
				
				glVertexAttribDivisor(uStream+j, uInstanceDiv);
				// FIXME: Handle arrays greater than 4 of types other than float
				uOffset += sizeof(float)*uCount;
			}
		}
		uElement++;
		pElement++;
	}
	ASSERT(uElement<MAX_VERTEX_ATTRIBUTES);
}

}

