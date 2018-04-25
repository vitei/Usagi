/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_PC_RASTERIZERSTATE_H_
#define _USG_GRAPHICS_DEVICE_PC_RASTERIZERSTATE_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/RenderState.h"
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

namespace usg {

class RasterizerState
{
public:
	RasterizerState() {};
	~RasterizerState() {};

	void Init(GFXDevice* pDevice, const RasterizerStateDecl &decl, uint32 uId);
	void Apply(bool bReverse) const;
private:
	bool	m_bPolyOffset;
	float	m_fDepthBias;
	GLenum	m_cullFace;
	bool	m_bEnabled;
	bool	m_bWireframe;
};

}

#endif