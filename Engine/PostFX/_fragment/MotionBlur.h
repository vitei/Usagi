
#if 0
/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: Velocity buffer based motion blur post process effect
*****************************************************************************/
#ifndef _USG_POSTFX_MOTIONBLUR_H_
#define _USG_POSTFX_MOTIONBLUR_H_

#include "Engine/Graphics/Materials/Material.h"
#include "Engine/PostFX/PostEffect.h"

class PostFXSys;

class MotionBlur : public PostEffect
{
public:
	MotionBlur();
	~MotionBlur();

	virtual void Init(GFXDevice* pDevice, PostFXSys* pSys);
	virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);

private:
	PostFXSys*				m_pSys;

	Material				m_material;
	ColorBuffer				m_prevVelTex;
	RenderTarget			m_prevVel;
	bool					m_bFirstFrame;
};


#endif

#endif