/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/String/U8String.h"
#include "Engine/Layout/Fonts/Text.h" 
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Layout/Fonts/TextDrawer.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Layout/Global2D.h"


namespace usg
{

	// TODO: Move to a common file
	const DescriptorDeclaration g_sGlobalDescriptors3D[] =
	{
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VERTEX),
		DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL_1, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_GEOMETRY),
		DESCRIPTOR_END()
	};


	static const VertexElement g_textVertDecl[] =
	{
		VERTEX_DATA_ELEMENT_NAME(0, TextDrawer::Vertex, vPosRange, VE_FLOAT, 4, false),
		VERTEX_DATA_ELEMENT_NAME(2, TextDrawer::Vertex, vUVRange, VE_FLOAT, 4, false),
		VERTEX_DATA_ELEMENT_NAME(1, TextDrawer::Vertex, fDepth, VE_FLOAT, 1, false),
		VERTEX_DATA_ELEMENT_NAME(3, TextDrawer::Vertex, cColUpper, VE_UBYTE, 4, true),
		VERTEX_DATA_ELEMENT_NAME(4, TextDrawer::Vertex, cColLower, VE_UBYTE, 4, true),
		VERTEX_DATA_ELEMENT_NAME(5, TextDrawer::Vertex, cColBg, VE_UBYTE, 4, true),
		VERTEX_DATA_ELEMENT_NAME(6, TextDrawer::Vertex, cColFg, VE_UBYTE, 4, true),
		VERTEX_DATA_END()
	};

	TextDrawer::TextDrawer(Text* p) : m_pParent(p)
	{
		m_bufferValid = false;
		m_bOriginTL = false;
	}

	TextDrawer::~TextDrawer()
	{
		ASSERT(m_bufferValid == false);
	}

	void TextDrawer::Init(GFXDevice* pDevice, ResourceMgr* pResMgr, const RenderPassHndl& renderPass)
	{
		// Initialize vertices.
		m_charVerts.Init(
			pDevice,
			NULL,
			sizeof(Vertex),
			MAX_CHAR_VERTICES,
			"Text system",
			GPU_USAGE_DYNAMIC
		);

		PipelineStateDecl pipelineState;
		pipelineState.inputBindings[0].Init(g_textVertDecl);
		pipelineState.uInputBindingCount = 1;
		pipelineState.ePrimType = PT_POINTS;
		RasterizerStateDecl& rasDecl = pipelineState.rasterizerState;
		rasDecl.eCullFace	= CULL_FACE_NONE;

		pipelineState.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(g_sGlobalDescriptors2D);
		pipelineState.layout.descriptorSets[1] = pDevice->GetDescriptorSetLayout(Font::m_sDescriptorDecl);
		pipelineState.layout.uDescriptorSetCount = 2;
		
		AlphaStateDecl& alphaDecl = pipelineState.alphaState;
		alphaDecl.bBlendEnable = true;
		alphaDecl.SetColor0Only(); 
		alphaDecl.srcBlend = BLEND_FUNC_SRC_ALPHA;
		alphaDecl.dstBlend = BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
		alphaDecl.blendEqAlpha = usg::BLEND_EQUATION_MAX;
		alphaDecl.srcBlendAlpha = BLEND_FUNC_SRC_ALPHA;
		alphaDecl.dstBlendAlpha = BLEND_FUNC_DST_ALPHA;

		alphaDecl.uColorTargets = pDevice->GetColorTargetCount(renderPass);

		DepthStencilStateDecl& dsDecl = pipelineState.depthState;
		pipelineState.pEffect = pResMgr->GetEffect(pDevice, "Text.DistanceField");
		m_pipeline = pDevice->GetPipelineState(renderPass, pipelineState);
		dsDecl.bDepthEnable = true;
		dsDecl.eDepthFunc = DEPTH_TEST_LESS;
		pipelineState.pEffect = pResMgr->GetEffect(pDevice, "Text.Text3D");
		pipelineState.layout.descriptorSets[0] = pDevice->GetDescriptorSetLayout(SceneConsts::g_globalDescriptorDecl);
		pipelineState.layout.descriptorSets[2] = pDevice->GetDescriptorSetLayout(g_sGlobalDescriptors3D);
		pipelineState.layout.uDescriptorSetCount = 3;
		// FIXME: Need distance field on the 3D text
		//m_pipeline3D = pDevice->GetPipelineState(renderPass, pipelineState);
		
		m_bufferValid = true;
	}

	void TextDrawer::Cleanup(GFXDevice* pDevice)
	{
		m_charVerts.Cleanup(pDevice);
		m_bufferValid = false;
	}

	void TextDrawer::UpdateBuffers(GFXDevice* pDevice)
	{
		// Set up the vertex buffers if necessary.
		if (!m_bufferValid || m_pParent->GetTextLength() == 0)
			return;

		int dirtyFlags = m_pParent->GetDirtyFlags();
		if (dirtyFlags != 0)
		{
			FillVertexBuffers(pDevice);
		}
	}

	bool TextDrawer::Draw( GFXContext* pContext, bool b3D )
	{
		// Don't draw non-initialized or zero-length strings.
		if (!m_bufferValid || m_pParent->GetTextLength() == 0)
		{
			// DEBUG_PRINT("TEXT ERROR: Tried to draw text with an invalid buffer or no string.\n");
			// debugPrint();
			return false;
		}


		// TODO: Need to set up the constant buffer and shader outside of this call, it'll be shared
		// between all text
		if (m_uCharCount > 0)
		{
			if (b3D)
			{
				pContext->SetPipelineState(m_pipeline3D);
			}
			else
			{
				pContext->SetPipelineState(m_pipeline);
			}
			FontHndl font = m_pParent->GetFont();
			const DescriptorSet& desc = font->GetDescriptor();

			pContext->SetDescriptorSet(&desc, 1);
			pContext->SetVertexBuffer(&m_charVerts);
			pContext->DrawImmediate((uint32)m_uCharCount);

		}

		return true;
	}

	void TextDrawer::ApplyAlignment(LineInformation* pInfo, uint32 uLineCount, Vector2f vOrigin, float fMaxWidth)
	{
		for (uint32 i = 0; i < uLineCount; i++)
		{
			uint32 uLineStart = pInfo[i].uStartCharacter;
			if (pInfo[i].uCharacterCount > 0)
			{
				ApplyAlignment(&m_textBufferTmp[uLineStart], &pInfo[i], vOrigin, fMaxWidth);
			}
		}
	}

	void TextDrawer::ApplyAlignment(Vertex* pVerts, LineInformation* pInfo, Vector2f vOrigin, float fMaxLineWidth)
	{
		uint32 uAlignFlags = m_context.GetAlignFlags();
		Vector2f vPos = vOrigin;
		Vector2f vScale = m_context.GetScale() * m_pParent->GetFont()->GetDrawScale();
		vPos.y -= m_pParent->GetFont()->GetBaseOffset() * vScale.y;

		float fWidthDiff = fMaxLineWidth - pInfo->fWidth;

		if(uAlignFlags & kTextAlignCenter)
		{
			vPos.x += fWidthDiff / 2.f;
		}
		else if (uAlignFlags & kTextAlignRight)
		{
			vPos.x += fWidthDiff;
		}

		uint32 uVertCount = pInfo->uCharacterCount;
		for (uint32 i = 0; i < uVertCount; i++)
		{
			pVerts->vPosRange.x += vPos.x;
			pVerts->vPosRange.y += vPos.y;
			pVerts->vPosRange.z += vPos.x;
			pVerts->vPosRange.w += vPos.y;

			pVerts++;
		}
	}

	void TextDrawer::FillVertexBuffers(GFXDevice* pDevice)
	{
		// Get position to draw text at.
		// uint32 uX, uY;
		// m_pParent->GetPosition(&uX, &uY); 
		// We now set these values in applying the alignment
		float fPosX = 0.0f;//(float)uX;
		float fPosY = 0.0f;//(float)uY;

		m_context.Init(m_pParent);

		string u8Text = m_pParent->GetText();
		const FontHndl& font = m_pParent->GetFont();
		float fTmpWidth = 0.0f;
		char* szTxtTmp = (char*)u8Text.data();	// Valid on new eastl, need to cast now we've gone back to the old one
		const float fWidthLimit = m_pParent->GetWidthLimit();

		m_vMinBounds.Assign(FLT_MAX, FLT_MAX);
		m_vMaxBounds.Assign(-FLT_MAX, -FLT_MAX);

		if(fWidthLimit > 0.0f)
		{
			// Insert fake newlines
			while(*szTxtTmp != 0)
			{
				uint32 uByteCount = U8Char::GetByteCount(szTxtTmp);
				U8Char thisChar(szTxtTmp, uByteCount);
				if (uByteCount == 1 && *szTxtTmp == '\n')
				{
					fTmpWidth = 0.0f;
				}
				float fLeft, fRight, fTop, fBottom;
				Vector2f vScale = m_context.GetScale() * m_pParent->GetFont()->GetDrawScale();
				bool bFound = font->GetCharacterCoords(thisChar.GetAsUInt32(), fLeft, fRight, fTop, fBottom);
				Vector2f vDimensions = Vector2f(font->GetCharacterAspect(fLeft, fRight, fTop, fBottom), 1.0f);
				vDimensions = vDimensions * vScale;
				float fCharWidth = vDimensions.x + font->GetCharacterSpacing() * vScale.x;
				fTmpWidth += fCharWidth;
 				if (fTmpWidth > fWidthLimit)
				{
					szTxtTmp--;
					while (szTxtTmp > u8Text.c_str())
					{
						uint32 uByteCount = U8Char::GetByteCount(szTxtTmp);
						U8Char thisChar(szTxtTmp, uByteCount);
						if (uByteCount == 1 && *szTxtTmp == ' ')
						{
							*szTxtTmp = '\n';
							fTmpWidth = 0.0f;
							break;
						}
						szTxtTmp--;
					}
				}
		
				szTxtTmp += uByteCount;
			}
		}

		const char* szText = u8Text.c_str();
		uint32 uCharCount = (uint32)u8Text.size();
		const usg::Color& color = m_context.GetColor();
		const usg::Color& colorUpper = m_pParent->GetGradationStartColor();
		const usg::Color& colorLower = m_pParent->GetGradationEndColor();
		const usg::Color& colorBg = m_pParent->GetBackgroundColor();
		m_uCharCount = 0;
		
		Vertex* pVert = &m_textBufferTmp[0];//[m_uCharCount*4];
		float fZPos = m_pParent->GetDepth();
		float fLineWidth = 0.0f;

		uint32 uFoundCharCount = 0;
		uint32 uLineStart = 0;
		uint32 uCharsThisLine = 0;
		float fMaxHeight = 0.0f;
		usg::Vector2f vScale;

		LineInformation lines[MAX_LINES];
		uint32 uLineCount = 0;

		while(*szText != 0)
		{
			// ints are the new chars, dontcha know.
			//char chFoo = szString[uChar];

			bool bFound = m_context.ProcessTag(*szText, szText);
			if (bFound)
				continue;	// Run through the loop again, this was a tag

			uint32 uByteCount = U8Char::GetByteCount(szText);
			if(uByteCount == 1 && *szText == '\n')
			{
				ASSERT(uLineCount < MAX_LINES);
				lines[uLineCount].fHeight = fMaxHeight;
				lines[uLineCount].fWidth = fLineWidth;
				lines[uLineCount].uStartCharacter = uLineStart;
				lines[uLineCount].uCharacterCount = uCharsThisLine;
				uLineCount++;

				uLineStart = uFoundCharCount;
				uCharsThisLine = 0;
				fPosX = 0.0f;
				fPosY += fMaxHeight;
				fLineWidth = 0.0f;
				*szText++;
				continue;
			}
			U8Char thisChar(szText, uByteCount);
			szText += uByteCount;

			float fLeft, fRight, fTop, fBottom;
			bFound = font->GetCharacterCoords(thisChar.GetAsUInt32(), fLeft, fRight, fTop, fBottom);

			if (bFound) {
				uFoundCharCount++;
			}
			else {
				continue;
			}

			vScale = m_context.GetScale()*m_pParent->GetFont()->GetDrawScale();
			//Vector2f vDimensions = font->GetCharacterSize(fLeft, fRight, fTop, fBottom);
			Vector2f vDimensions = Vector2f(font->GetCharacterAspect(fLeft, fRight, fTop, fBottom), 1.0f);
			vDimensions = vDimensions * vScale;
			fMaxHeight = Math::Max(vDimensions.y, fMaxHeight);

			// Top right
			pVert->vPosRange.x = fPosX;
			pVert->vPosRange.y = fPosY;
			pVert->vPosRange.z = fPosX + vDimensions.x;
			pVert->vPosRange.w = fPosY + vDimensions.y;

			colorUpper.FillU8(pVert->cColUpper[0], pVert->cColUpper[1], pVert->cColUpper[2], pVert->cColUpper[3]);
			colorLower.FillU8(pVert->cColLower[0], pVert->cColLower[1], pVert->cColLower[2], pVert->cColLower[3]);
			if (m_bOriginTL)
			{
				pVert->vUVRange.Assign(fLeft, fTop, fRight, fBottom);
			}
			else
			{
				pVert->vUVRange.Assign(fLeft, fBottom, fRight, fTop);
			}
			colorBg.FillU8(pVert->cColBg[0], pVert->cColBg[1], pVert->cColBg[2], pVert->cColBg[3]);
			color.FillU8(pVert->cColFg[0], pVert->cColFg[1], pVert->cColFg[2], pVert->cColFg[3]);
			pVert->fDepth = fZPos;
	
			pVert++;

			uCharsThisLine++;
			float fCharWidth = vDimensions.x + font->GetCharacterSpacing() * vScale.x;
			fPosX += fCharWidth;
			fLineWidth += fCharWidth;
			// We can't manage anymore
			if (uFoundCharCount >= MAX_CHARS)
				break;
		}

		// Add the final line before processing the alignment
		if (uCharsThisLine)
		{
			ASSERT(uLineCount < MAX_LINES);
			lines[uLineCount].fHeight = fMaxHeight;
			lines[uLineCount].fWidth = fLineWidth;
			lines[uLineCount].uStartCharacter = uLineStart;
			lines[uLineCount].uCharacterCount = uCharsThisLine;
			uLineCount++;
		}

		float fMaxWidth = 0.0f;
		float fTotalHeight = 0.0f;
		for (uint32 i = 0; i < uLineCount; i++)
		{
			fMaxWidth = Math::Max(fMaxWidth, lines[i].fWidth);
			fTotalHeight += lines[i].fHeight;
		}

		uint32 uAlignFlags = m_context.GetAlignFlags();
		float fX, fY;
		m_pParent->GetPosition(&fX, &fY);
		Vector2f vOrigin(fX, fY);
		// Offset the origin based on the width and height of the text
		if (uAlignFlags & kTextAlignHOriginCenter)
		{
			vOrigin.x -= fMaxWidth / 2.f;
		}
		else if (uAlignFlags & kTextAlignHOriginRight)
		{
			vOrigin.x -= fMaxWidth;
		}

		if (uAlignFlags & kTextAlignVOriginMiddle)
		{
			vOrigin.y -= fTotalHeight / 2.f;
		}
		else if (uAlignFlags & kTextAlignVOriginBottom)
		{
			vOrigin.y -= fTotalHeight;
		}


		ApplyAlignment(lines, uLineCount, vOrigin, fMaxWidth);

		for(uint32 i=0; i< uFoundCharCount; i++)
		{
			pVert = &m_textBufferTmp[i];
			m_vMinBounds.Assign(Math::Min(m_vMinBounds.x, pVert->vPosRange.x), Math::Min(m_vMinBounds.y, pVert->vPosRange.y));
			m_vMaxBounds.Assign(Math::Max(m_vMaxBounds.x, pVert->vPosRange.z), Math::Max(m_vMaxBounds.y, pVert->vPosRange.w));
		}
	

		if (uFoundCharCount > 0)
		{
			m_charVerts.SetContents(pDevice, m_textBufferTmp, uFoundCharCount);
		}
		m_uCharCount = uFoundCharCount;
	}

	bool TextDrawer::Resize(memsize uStrLen)
	{
		if(uStrLen>MAX_CHARS)
		{
			DEBUG_PRINT("Resize request was larger than the OpenGL fixed buffer");
			return false;
		}
		else {
			m_uCharCount = uStrLen;
			m_bDirty = true;
			return true;
		}
	}

	void TextDrawer::GetBounds(usg::Vector2f& vMin, usg::Vector2f& vMax) const
	{
		vMin = m_vMinBounds;
		vMax = m_vMaxBounds;
	}


	memsize TextDrawer::GetMaxStringLength() const
	{
		return MAX_CHARS;
	}
}
