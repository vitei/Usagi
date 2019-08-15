/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: System for rendering debug output
*****************************************************************************/
#ifndef __USG_DEBUG_RENDER_H__
#define __USG_DEBUG_RENDER_H__

#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Primitives/IndexBuffer.h"
#include "Engine/Graphics/Viewports/Viewport.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Debug/Rendering/DebugFont.h"
#include "Engine/Debug/Rendering/Debug3D.h"
#include "Engine/Graphics/StandardVertDecl.h"

namespace usg {

class DebugRender
{
public:
	DebugRender();
	~DebugRender();

	void Init(GFXDevice* pDevice, const RenderPassHndl& renderPass);
	void CleanUp(GFXDevice* pDevice);
	void SetDrawArea(float fLeft, float fTop, float fWidth, float fHeight, float fLineSpacing = 1.25f);

	// Values on the X range are a fraction of the debug draw area
	void AddString(const char* szString, float fPosX, float fLineNum, const Color& color);
	void AddBar(float fLineNum, float fPosX, float fWidth, const Color& color);
	void AddRect( float fPosX, float fPosY, float fWidth, float fHeight, const Color& color );

	void Draw(GFXContext* pContext);
	void Clear();
	void Updatebuffers(GFXDevice* pDevice);

	float GetLineHeight( void ) const { return m_fLineHeight; }

	static DebugRender* GetRenderer();

private:

	static DebugRender* 	m_psRenderer;

	enum
	{
		MAX_BARS			= 64,
		MAX_BAR_VERTICES	= MAX_BARS * 4,
		MAX_BAR_INDICES		= MAX_BARS * 6,

		MAX_CHARS			= 2048,
		MAX_CHAR_VERTICES	= MAX_CHARS,

		MAX_PAGES			= 20
	};

	DebugFont				m_font;

	Viewport				m_viewport2D;

	Material				m_posColMaterial;
	ConstantSet				m_posColConstants;
	Material				m_textMaterial;
	ConstantSet				m_textConstants;

	VertexBuffer			m_barVerts;
	VertexBuffer			m_charVerts;
	IndexBuffer				m_indices;
	IndexBuffer				m_textIndices;

	float32					m_fLineHeight;
	float32					m_fBarHeight;
	float32					m_fBarOffset;
	
	float32					m_fCharHeight;
	float32					m_fCharOffset;

	float32					m_fChrScale;

	uint32					m_uBarCount;
	uint32					m_uCharCount;

	PositionDiffuseVertex	m_barBufferTmp[MAX_BAR_VERTICES];
	PositionUVColVertex		m_textBufferTmp[MAX_CHAR_VERTICES];
};

}

#endif
