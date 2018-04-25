/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include API_HEADER(Engine/Graphics/Device, RasterizerState.h)

namespace usg {

const GLenum g_cullFaceMap[] =
{
	GL_NONE,	// CULL_FACE_NONE = 0
	GL_BACK,	// CULL_FACE_BACK = 1
	GL_FRONT,	// CULL_FACE_FRONT = 2
};


void RasterizerState::Init(GFXDevice* pDevice, const RasterizerStateDecl &decl, uint32 uId)
{
	m_bEnabled = decl.eCullFace != CULL_FACE_NONE;
	m_cullFace = g_cullFaceMap[decl.eCullFace];
	m_bWireframe = decl.bWireframe;
	m_bPolyOffset = decl.bUseDepthBias;
	if(decl.bUseDepthBias)
	{
		m_fDepthBias = decl.fDepthBias/128.f;
	}
	else
	{
		m_fDepthBias = 0.0f;
	}
	
	
	// TODO: Multisample and depth bias
}


void RasterizerState::Apply(bool bReverseWinding) const
{
	if(m_bEnabled)
	{
		glEnable(GL_CULL_FACE);
#if CCW_WINDING		
		glFrontFace(bReverseWinding ? GL_CW : GL_CCW );
#else
		glFrontFace(bReverseWinding ? GL_CCW : GL_CW );
#endif
		glCullFace(m_cullFace);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	if(m_bPolyOffset)
	{
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(m_fDepthBias, 0.0f);
	}
	else
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
	
#if 1//def _DEBUG
	if( m_bWireframe ) {
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	}
	else {
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	}
#endif
}

}