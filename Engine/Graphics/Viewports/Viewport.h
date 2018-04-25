/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Creates right-handed view matrices	
*****************************************************************************/
#ifndef _USG_VIEWPORT_H
#define _USG_VIEWPORT_H
#include "Engine/Common/Common.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Graphics/RenderConsts.h"

namespace usg {

class GFXContext;

class Viewport
{
public:
	Viewport(void): m_bIsSet(false) { }

	Viewport(uint32 uLeft, uint32 uBottom, uint32 uWidth, uint32 uHeight)
	{
		InitViewport(uLeft, uBottom, uWidth, uHeight);
	}

	~Viewport();

	void		InitViewport( uint32 uLeft, uint32 uBottom, uint32 uWidth, uint32 uHeight);
	void		SetPos(uint32 uLeft, uint32 uBottom);
	void		SetSize(uint32 uWidth, uint32 uHeight);
	//void		InitPerspective(float32 fFov, float32 fAspect, float32 fNearClip, float32 fFarClip);

	float		GetAspect() const	{ return m_fAspect; }
	int			GetHeight() const	{ return m_uHeight; }
	int			GetWidth() const	{ return m_uWidth; }
	int			GetBottom() const	{ return m_uBottom; }
	int			GetLeft() const		{ return m_uLeft; }

private:
	float32		m_fAspect;

	// Viewport variables
	uint32		m_uBottom;
	uint32		m_uLeft;
	uint32		m_uWidth;
	uint32		m_uHeight;

	bool		m_bIsSet;
	
};


}


#endif
