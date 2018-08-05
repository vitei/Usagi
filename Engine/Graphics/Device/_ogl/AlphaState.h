/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_DEVICE_PC_ALPHASTATE_H_
#define _USG_GRAPHICS_DEVICE_PC_ALPHASTATE_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/RenderState.h"
#include "Engine/Graphics/Color.h"
#include OS_HEADER(Engine/Graphics/Device, OpenGLIncludes.h)

namespace usg {

class AlphaState
{
public:
	AlphaState() {};
	~AlphaState() {};
	
	void Init(GFXDevice* pDevice, const AlphaStateDecl &decl, uint32 uId);
	void Apply() const;
private:

	bool				m_bEnable;
	GLenum				m_eqRgb;
	GLenum				m_eqAlpha;
	GLenum				m_srcRgb;
	GLenum				m_srcAlpha;
	GLenum				m_dstRgb;
	GLenum				m_dstAlpha;
	uint32				m_uColorMask[MAX_COLOR_TARGETS];

	GLenum				m_eAlphaTest;
	float32				m_fAlphaRef;
};

}


#endif