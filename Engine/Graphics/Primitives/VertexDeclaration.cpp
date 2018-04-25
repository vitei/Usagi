/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/Graphics/Primitives/VertexDeclaration.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Maths/MathUtil.h"

namespace usg {

static const int MAX_VERTEX_DECLARATIONS = 100;
static uint32 						guDeclCount = 0;
static VertexDeclaration			guDeclarations[MAX_VERTEX_DECLARATIONS];

VertexDeclaration::VertexDeclaration()
{
	m_uCount 		= 0;
	m_uSize			= 0;
	m_uInstanceDiv 	= 0;

	MemSet(m_elements, 0, sizeof(VertexElement)*MAX_VERTEX_ATTRIBUTES);
}


VertexDeclaration::~VertexDeclaration()
{
	
}


void VertexDeclaration::InitDecl(const VertexElement* pElements, uint32 uInstanceDiv, uint32 uSize)
{
	uint32 uCount = 0;
	while(pElements[uCount].eType != VE_INVALID)
	{
		uCount++;
	}

	Init(pElements, uCount, uInstanceDiv, uSize);
}


static memsize AlignSize(memsize uSize, memsize uAlign)
{
	memsize uMask = uAlign - 1;
	memsize uMisAlignment = (uSize & uMask);
	memsize uAdjustment = uAlign - uMisAlignment;
	if (uMisAlignment == 0)
		return uSize;

	return uSize + uAdjustment;
}


void VertexDeclaration::Init(const VertexElement* pElements, uint32 uCount, uint32 uInstanceDiv, uint32 uSize)
{
	m_uInstanceDiv = uInstanceDiv;
	m_uCount = uCount;
	for(uint32 i=0; i<uCount; i++)
	{
		MemCpy(&m_elements[i], &pElements[i], sizeof(VertexElement));
		// FIXME: THIS NEEDS TO BE PASSED IN!
		m_uSize = Math::Max(m_elements[i].uOffset + GetByteCount(pElements[i].eType)*pElements[i].uCount, m_uSize);
		ASSERT(m_uSize < 2048);
	}
	m_elements[uCount] = VERTEX_ELEMENT_CAP;
	m_uSize = AlignSize(m_uSize, 4);
	if (uSize)
	{
		// Optional use when we have paddding
		ASSERT(m_uSize <= uSize);
		m_uSize = uSize;
	}
}

uint32 VertexDeclaration::GetByteCount(VertexElementType eType)
{
	switch(eType)
	{
	case VE_BYTE:
	case VE_UBYTE:
		return 1;
	case VE_SHORT:
	case VE_USHORT:
		return 2;
	case VE_INT:
		return 4;
	case VE_FLOAT:
		return 4;
	};
	
	ASSERT(false);
	return 2;
}


bool VertexDeclaration::operator==(const VertexDeclaration& rhs)
{
	if(rhs.m_uCount != m_uCount || rhs.m_uSize != m_uSize)	
		return false;

	for(uint32 i=0; i<m_uCount; i++)
	{
		const VertexElement* pLeft = &m_elements[i];
		const VertexElement* pRight = &rhs.m_elements[i];
		if( (pLeft->uAttribId != pRight->uAttribId) ||
				(pLeft->uOffset	!= pRight->uOffset) ||
				(pLeft->eType	!= pRight->eType) ||
				(pLeft->uCount	!= pRight->uCount) ||
				(pLeft->bNormalised != pRight->bNormalised) )
		{
			return false;
		}
	}

	return true;
}

//
// STATIC FUNCTIONS
//
uint32 VertexDeclaration::GetDeclId(const VertexDeclaration& decl)
{
	for(uint32 i=0; i<guDeclCount; i++)
	{
		if(guDeclarations[i]==decl)
		{
			return i;
		}
	}

	if(guDeclCount < MAX_VERTEX_DECLARATIONS)
	{
		guDeclarations[guDeclCount] = decl;
		guDeclCount++;
		return (guDeclCount-1);
	}

	ASSERT(false);
	return 0;
}


uint32 VertexDeclaration::GetDeclId(const VertexElement* pElements)
{
	VertexDeclaration decl;
	decl.InitDecl(pElements);
	return GetDeclId(decl);
}

const VertexDeclaration* VertexDeclaration::GetDecl(uint32 uDeclId)
{
	ASSERT(uDeclId<guDeclCount);
	return &guDeclarations[uDeclId];
}

}
