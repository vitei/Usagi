/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_INPUT_BINDING_H
#define _USG_GRAPHICS_INPUT_BINDING_H

#include "Engine/Graphics/Primitives/VertexDeclaration.h"
#include "Engine/Resource/ResourceDecl.h"
#include API_HEADER(Engine/Graphics/Effects, InputBinding_ps.h)

namespace usg {

class Texture;
class GFXDevice;

class InputBinding
{
public:
	InputBinding();
	~InputBinding();

	void Init(GFXDevice* pDevice, uint32 uDecl);
	void InitMultiStream(GFXDevice* pDevice, uint32* puDeclIds, uint32 uBufferCount);
	bool IsDecl(uint32* puDeclId, uint32 uDeclCount) const;

	const InputBinding_ps& GetPlatform() const { return m_platform; }
	uint32 GetId() const { return m_uBindingId; }
	uint32 GetDeclCount() const {return m_uDeclCount; }
	uint32 GetDeclId(uint32 uIndex) const { return m_uDeclId[uIndex]; }

private:
	PRIVATIZE_COPY(InputBinding)

	void InitPlatform(GFXDevice* pDevice);

	uint32				m_uDeclId[MAX_VERTEX_BUFFERS];
	uint32				m_uDeclCount;
	uint32				m_uDynamicDecls;
	InputBinding_ps		m_platform;
	uint32				m_uBindingId;
};

}

#endif
