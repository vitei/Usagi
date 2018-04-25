/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "PostEffect.h"

namespace usg {

PostEffect::PostEffect():
RenderNode()
{
	m_bEnabled = false;
	SetPostEffect(true);
}


PostEffect::~PostEffect()
{

}

void PostEffect::SetEnabled(bool bEnabled)
{
	m_bEnabled = bEnabled;
}

}

