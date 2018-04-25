/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_PC_DEPTHSTENCILSTATE_H_
#define _USG_GRAPHICS_DEVICE_PC_DEPTHSTENCILSTATE_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/RenderState.h"
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

namespace usg {

class DepthStencilState
{
public:
	DepthStencilState() {};
	~DepthStencilState() {};

	void Init(GFXDevice* pDevice, const DepthStencilStateDecl &decl, uint32 uId);
	void Apply() const;

private:

	GLenum  m_depthFunc;
	bool	m_depthEnable;
	bool	m_depthEnableWrite;
	uint32	m_uMask[STENCIL_REF_TYPE];

	bool	m_stencilEnable;
	GLenum	m_stencilFunc;
	GLenum	m_stencilOps[SOP_TYPE_COUNT];
};

}


#endif