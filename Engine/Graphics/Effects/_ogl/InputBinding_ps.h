/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_EFFECT_BINDING_PS_H_
#define _USG_GRAPHICS_EFFECT_BINDING_PS_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Primitives/VertexDeclaration.h"
#include "Engine/Resource/ResourceDecl.h"
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

namespace usg {

class Texture;
class Effect;
class GFXDevice;

class InputBinding_ps
{
public:
	InputBinding_ps();
	~InputBinding_ps();

	void Init(GFXDevice* pDevice, const VertexDeclaration** ppDecls, uint32 uCount);
	void UpdateVBOMapping(uint32 uBuffer) const;

private:
	PRIVATIZE_COPY(InputBinding_ps)

	const VertexDeclaration*	m_pDecl[MAX_VERTEX_BUFFERS];
	uint32						m_uMapOffset[MAX_VERTEX_BUFFERS];
	uint32						m_uBuffers;
	GLuint						m_mapping[MAX_VERTEX_ATTRIBUTES];
};

}

#endif
