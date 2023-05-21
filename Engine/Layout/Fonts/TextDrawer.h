/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_FONT_TEXT_DRAWER_H_
#define _USG_GRAPHICS_FONT_TEXT_DRAWER_H_

#include "Engine/Layout/Fonts/Font.h"
#include "Engine/Layout/Fonts/TextEnums.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "TextContext.h"

namespace usg
{
	class GFXContext;

	class TextDrawer
	{
	public:
		TextDrawer(class Text* pOwner);
		~TextDrawer();

		void Init(GFXDevice* pDevice, ResourceMgr* pResMgr, const RenderPassHndl& renderPass);
		void Cleanup(GFXDevice* pDevice);
		void UpdateBuffers(GFXDevice* pDevice);
		void SetOriginTL(bool bTL) { m_bOriginTL = bTL; }
		void GetBounds(usg::Vector2f& vMin, usg::Vector2f& vMax) const;
		bool Draw(GFXContext* context, bool b3D);


		struct Vertex
		{
			Vector4f vPosRange;
			Vector4f vUVRange;
			float	 fDepth;
			uint8	 cColUpper[4];
			uint8	 cColLower[4];
			uint8	 cColBg[4];
			uint8	 cColFg[4];
		};

	private:

		enum
		{
			MAX_LINES = 20
		};

		struct LineInformation
		{
			float	fWidth;
			float	fHeight;
			uint32	uStartCharacter;
			uint32	uCharacterCount;
		};

		void FillVertexBuffers(GFXDevice* pDevice);
		void ApplyAlignment(LineInformation* pInfo, uint32 uLineCount, Vector2f vOrigin, float fMaxWidth);
		void ApplyAlignment(Vertex* pVerts, LineInformation* pInfo, Vector2f vOrigin, float fMaxLineWidth);
		// FIXME: Something more dynamic at a later date

		const class Text* m_pParent;

		TextContext				m_context;
		PipelineStateHndl		m_pipeline;
		PipelineStateHndl		m_pipeline3D;
		VertexBuffer			m_charVerts;

		// # of characters currently being drawn
		memsize			m_uCharCount;

		Vector2f		m_vPosition;
		Vector2f		m_vMinBounds;
		Vector2f		m_vMaxBounds;
		int				m_alignFlags;
		float			m_fWidthLimit;
		bool			m_bufferValid;
		bool			m_bOriginTL;
		bool			m_bDirty;	// Regenerate is anything is dirty

		usg::vector<Vertex>	m_textBufferTmp;
	};
}

#endif /* _USG_GRAPHICS_FONT_TEXT_DRAWER_H_ */
