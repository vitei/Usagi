/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_VERTEXDECLARATION_H
#define _USG_GRAPHICS_VERTEXDECLARATION_H

#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Memory/MemUtil.h"

namespace usg {


class VertexDeclaration
{
public:
	VertexDeclaration();
	~VertexDeclaration();

	void InitDecl(const VertexElement* pElements, uint32 uInstanceDiv = 0, uint32 uSize=0);

	memsize GetSize() const { return m_uSize; }
	uint32 GetElementCount() const { return m_uCount; }
	bool IsInstanceStream() const { return m_uInstanceDiv != USG_INVALID_ID; }
	uint32 GetInstanceDiv() const { return m_uInstanceDiv; }

	const VertexElement* GetElements() const { return m_elements; }

	bool operator==(const VertexDeclaration& rhs);

	static uint32 GetDeclId(const VertexDeclaration& decl);
	static uint32 GetDeclId(const VertexElement* pElements);
	static const VertexDeclaration* GetDecl(uint32 uDeclId);
	static uint32 GetByteCount(VertexElementType eType);

private:
	void Init(const VertexElement* pElements, uint32 uCount, uint32 uInstanceDiv, uint32 uSize);
	
	// We don't want these getting copied, we want a very specific list so we
	// can cheaply confirm they match
	VertexDeclaration(const VertexDeclaration& decl) {}
	const VertexDeclaration& operator=(const VertexDeclaration& decl)
	{	
		MemCpy(this, &decl, sizeof(VertexDeclaration));
		return *this;
	}


	VertexElement	m_elements[MAX_VERTEX_ATTRIBUTES+1];
	memsize			m_uSize;
	uint32			m_uCount;
	uint32			m_uInstanceDiv;
};

}

#endif
