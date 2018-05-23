/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Memory/Mem.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Debug/Rendering/DebugRender.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Layout/Global2D.h"
#include "Engine/Core/String/String_Util.h"

namespace usg {

DebugRender* DebugRender::m_psRenderer = NULL;

struct TextConstants
{
	Matrix4x4	mProj;
	Vector4f	vCharSize;	
	Vector4f	vTexCoordRng;
};

struct SolidConstants
{
	Matrix4x4	mProj;
};

const DescriptorDeclaration g_sGlobalDescriptorDummy[] =
{
	DESCRIPTOR_END()
};

static const ShaderConstantDecl g_solidConstantDef[] = 
{
	SHADER_CONSTANT_ELEMENT( SolidConstants, mProj,	CT_MATRIX_44, 1 ),
	SHADER_CONSTANT_END()
};

static const DescriptorDeclaration g_solidDescriptorDecl[] =
{
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VS_GS),
	DESCRIPTOR_END()
};


static const ShaderConstantDecl g_textConstantDef[] =
{
	SHADER_CONSTANT_ELEMENT( TextConstants, mProj,			CT_MATRIX_44, 1 ),
	SHADER_CONSTANT_ELEMENT( TextConstants, vCharSize,		CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_ELEMENT( TextConstants, vTexCoordRng,	CT_VECTOR_4, 1 ),
	SHADER_CONSTANT_END()
};

static const DescriptorDeclaration g_textDescriptorDecl[] =
{
	DESCRIPTOR_ELEMENT(0,						 DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_ELEMENT(SHADER_CONSTANT_MATERIAL, DESCRIPTOR_TYPE_CONSTANT_BUFFER, 1, SHADER_FLAG_VS_GS),
	DESCRIPTOR_END()
};


DebugRender::DebugRender()
{
	m_uBarCount		= 0;
	m_uCharCount	= 0;

	m_fChrScale = 0.0f;
}

DebugRender::~DebugRender()
{
	m_psRenderer = NULL;
}

void DebugRender::Init(GFXDevice* pDevice, const RenderPassHndl& renderPass)
{
	// Setup the vertex data
	{
		uint16* puIndices;
		ScratchObj<uint16> scratchIndices(puIndices, MAX_BAR_INDICES);

		uint16* puIndicesTmp = puIndices;
		for(uint32 i=0; i<MAX_BAR_INDICES/6; i++)
		{
			uint32 uBaseIndex = i*4;
#if CCW_WINDING			
			puIndicesTmp[0] = uBaseIndex + 2;
			puIndicesTmp[1] = uBaseIndex + 1;
			puIndicesTmp[2] = uBaseIndex + 0;
			puIndicesTmp[3] = uBaseIndex + 2;
			puIndicesTmp[4] = uBaseIndex + 3;
			puIndicesTmp[5] = uBaseIndex + 1;
#else
			puIndicesTmp[0] = uBaseIndex + 0;
			puIndicesTmp[1] = uBaseIndex + 1;
			puIndicesTmp[2] = uBaseIndex + 2;
			puIndicesTmp[3] = uBaseIndex + 1;
			puIndicesTmp[4] = uBaseIndex + 3;
			puIndicesTmp[5] = uBaseIndex + 2;
#endif

			puIndicesTmp += 6;
		}
		m_indices.Init( pDevice, puIndices, MAX_BAR_INDICES );
	}

	m_barVerts.Init( pDevice, NULL, sizeof(PositionDiffuseVertex), MAX_BAR_VERTICES, "Debug Bars", GPU_USAGE_DYNAMIC);
	m_charVerts.Init( pDevice, NULL, sizeof(PositionUVColVertex), MAX_CHAR_VERTICES, "Debug Chars", GPU_USAGE_DYNAMIC);

	{
		uint16* puIndices;
		ScratchObj<uint16> scratchIndices(puIndices, MAX_CHAR_VERTICES);
		uint16* puIndicesTmp = puIndices;
		for(uint32 i=0; i<MAX_CHAR_VERTICES; i++)
		{
			*puIndicesTmp++ = i;
		}

		m_textIndices.Init( pDevice, puIndices, MAX_CHAR_VERTICES, PT_POINTS );
	}

	PipelineStateDecl pipelineState;
	pipelineState.renderPass = renderPass;
	pipelineState.uInputBindingCount = 1;
	pipelineState.ePrimType = PT_POINTS;

	DescriptorSetLayoutHndl globalDesc = pDevice->GetDescriptorSetLayout(g_sGlobalDescriptorDummy);
	DescriptorSetLayoutHndl textDescriptors = pDevice->GetDescriptorSetLayout(g_textDescriptorDecl);
	DescriptorSetLayoutHndl solidDescriptors = pDevice->GetDescriptorSetLayout(g_solidDescriptorDecl);
	pipelineState.layout.descriptorSets[0] = textDescriptors;
	pipelineState.layout.descriptorSets[1] = textDescriptors;
	pipelineState.layout.uDescriptorSetCount = 2;

	pipelineState.rasterizerState.eCullFace	= CULL_FACE_NONE;

	m_font.Load(pDevice, "B42");

	m_fChrScale = 1.0f;

	AlphaStateDecl& alphaDecl = pipelineState.alphaState;
	alphaDecl.bBlendEnable = false;
	alphaDecl.srcBlend = BLEND_FUNC_SRC_ALPHA;
	alphaDecl.dstBlend = BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
	pipelineState.inputBindings[0].Init(GetVertexDeclaration(VT_POSITION_UV_COL));
	pipelineState.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, "DebugText");
	m_textMaterial.Init(pDevice, pDevice->GetPipelineState(pipelineState), textDescriptors);
	m_textConstants.Init(pDevice, g_textConstantDef);
	m_textMaterial.SetConstantSet(SHADER_CONSTANT_MATERIAL, &m_textConstants);
	SamplerDecl pointDecl(SF_POINT, SC_CLAMP);//SF_MIN_MAG_POINT, SC_CLAMP);
	m_textMaterial.SetTexture(0, m_font.GetTexture(), pDevice->GetSampler(pointDecl));
	m_textMaterial.UpdateDescriptors(pDevice);


	pipelineState.ePrimType = PT_TRIANGLES;
	alphaDecl.bBlendEnable = true;
	pipelineState.inputBindings[0].Init(GetVertexDeclaration(VT_POSITION_DIFFUSE));
	// Set up the materials
	pipelineState.pEffect = ResourceMgr::Inst()->GetEffect(pDevice, "DebugPosCol");

	pipelineState.layout.descriptorSets[1] = solidDescriptors;

	m_posColMaterial.Init(pDevice, pDevice->GetPipelineState(pipelineState), solidDescriptors);
	m_posColConstants.Init(pDevice, g_solidConstantDef);
	m_posColMaterial.SetConstantSet(SHADER_CONSTANT_MATERIAL, &m_posColConstants);
	m_posColMaterial.UpdateDescriptors(pDevice);


	ASSERT(m_psRenderer==NULL);

	m_psRenderer = this;
}


void DebugRender::CleanUp(GFXDevice* pDevice)
{
	m_posColMaterial.Cleanup(pDevice);
	m_posColConstants.CleanUp(pDevice);
	m_textMaterial.Cleanup(pDevice);
	m_textConstants.CleanUp(pDevice);
	m_barVerts.CleanUp(pDevice);
	m_charVerts.CleanUp(pDevice);
	m_indices.CleanUp(pDevice);
	m_textIndices.CleanUp(pDevice);
}

DebugRender* DebugRender::GetRenderer()
{
	return m_psRenderer;
}

void DebugRender::SetDrawArea(float fLeft, float fTop, float fWidth, float fHeight, float fLineSpacing)
{
	m_viewport2D.InitViewport((uint32)fLeft, (uint32)fTop, (uint32)fWidth, (uint32)fHeight);
	// Assuming a viewport of 0-1
	//m_fLineHeight = fLineHeight / fHeight;
	float fChrHeight = m_font.GetChrHeight() * m_fChrScale;
	float fChrWidth = m_font.GetChrWidth() * m_fChrScale;
	m_fLineHeight = (fChrHeight*fLineSpacing/(fHeight));

	m_fBarHeight = m_fLineHeight * 0.8f;
	m_fBarOffset = m_fLineHeight * 0.1f;

	m_fCharHeight = fChrHeight/fHeight;//m_fLineHeight * 0.8f;
	m_fCharOffset = (m_fLineHeight - m_fCharHeight) / 2.0f;
	
	SolidConstants* pConsts = m_posColConstants.Lock<SolidConstants>();
	pConsts->mProj.Orthographic( 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 10.0f ); 
	m_posColConstants.Unlock();

	TextConstants* pTxtConsts = m_textConstants.Lock<TextConstants>();
	pTxtConsts->mProj.Orthographic( 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 10.0f ); 
	// FIXME: Remove hardcoding, based on the nsub debugfx
	Vector4f vCharSize(fChrWidth/fWidth, fChrHeight/fHeight, 0.0f, 0.0f);
	pTxtConsts->vCharSize = vCharSize * pTxtConsts->mProj;
	pTxtConsts->vTexCoordRng.Assign(0.0f, 0.0f, (1.0f/32.0f) * 8.0f, -((1.0f/256.0f)*6.0f));
	m_textConstants.Unlock();
}

// Values on the X range are a fraction of the debug draw area
void DebugRender::AddString(const char* szString, float fPosX, float fLineNum, const Color& color)
{
	uint32 uCharCount = str::StringLength(szString);
	if(m_uCharCount + uCharCount > MAX_CHARS )
	{
		ASSERT(false);
		return;
	}

	PositionUVColVertex* pVert = &m_textBufferTmp[m_uCharCount];
	float fPosXCache = fPosX;
	float fZPos = 0.0f;
	float fTotalWidth = 0.0f;

	float fWidth = ((float)m_font.GetChrWidth()*m_fChrScale)/(float)m_viewport2D.GetWidth();
	uint32 uValidChars = 0;

	for(uint32 uChar = 0; uChar < uCharCount; uChar++)
	{
		//m_font.GetCharacterWidth(szString[uChar], m_fCharHeight);
		//float fLeft, fRight, fTop, fBottom;
		//m_font.GetCharacterCoords(szString[uChar], fLeft, fRight, fTop, fBottom);

		float u, v;
		m_font.GetCharacterUV(szString[uChar], u, v);

		if (szString[uChar] == '\n')
		{
			fLineNum += 1.0f;
			fPosX = fPosXCache;
		}
		else
		{
			pVert->x = fPosX;
			pVert->y = (fLineNum * m_fLineHeight) + m_fCharOffset;
			pVert->z = fZPos;
			pVert->c = color;
			pVert->u = v;
			pVert->v = 1.0f - u;

			pVert++;

			fPosX += fWidth;
			fTotalWidth += fWidth;
			uValidChars++;
		}
	}
	
	m_uCharCount += uValidChars;

	Color black(0.0f, 0.0f, 0.0f, 0.0f);
	//AddBar(fLineNum, fPosXCache, fTotalWidth+(fWidth*0.3f), black);
}

void DebugRender::AddBar(float fLineNum, float fPosX, float fWidth, const Color& color)
{
	float fPosY = (fLineNum * m_fLineHeight) + m_fBarOffset;
	float fHeight = m_fBarHeight;
	AddRect( fPosX, fPosY, fWidth, fHeight, color );
}

void DebugRender::AddRect( float fPosX, float fPosY, float fWidth, float fHeight, const Color& color )
{
	if( m_uBarCount >= MAX_BARS-1 )
	{
		ASSERT( false );
		return;
	}

	PositionDiffuseVertex*	pVerts = &m_barBufferTmp[m_uBarCount * 4];

	// TODO: Massively redundant, most of this could be moved into a geometry shader
	float fZPos = 0.0f;

	// Top left
	pVerts[0].x = fPosX;
	pVerts[0].y = fPosY;
	pVerts[0].z = fZPos;

	// Top right
	pVerts[1].x = fPosX + fWidth;
	pVerts[1].y = fPosY;
	pVerts[1].z = fZPos;

	// Bottom left
	pVerts[2].x = fPosX;
	pVerts[2].y = fPosY + fHeight;
	pVerts[2].z = fZPos;

	// Bottom right
	pVerts[3].x = fPosX + fWidth;
	pVerts[3].y = fPosY + fHeight;
	pVerts[3].z = fZPos;

	for( int i = 0; i < 4; i++ )
	{
		pVerts[i].r = color.r();
		pVerts[i].g = color.g();
		pVerts[i].b = color.b();
		pVerts[i].a = color.a();
	}

	m_uBarCount++;
}

void DebugRender::Updatebuffers(GFXDevice* pDevice)
{
	if(m_uBarCount)
	{
		m_barVerts.SetContents(pDevice, m_barBufferTmp, m_uBarCount*4);
	}

	if(m_uCharCount)
	{
		m_charVerts.SetContents(pDevice, m_textBufferTmp, m_uCharCount);
	}

	m_posColConstants.UpdateData(pDevice);
	m_textConstants.UpdateData(pDevice);
}

void DebugRender::Draw(GFXContext* pContext)
{
	pContext->BeginGPUTag("DebugRender");
	if(m_uBarCount)
	{
		m_posColMaterial.Apply(pContext);
		pContext->SetVertexBuffer(&m_barVerts);
		pContext->DrawIndexedEx(&m_indices, 0, m_uBarCount*6);
	}

	if(m_uCharCount)
	{
		m_textMaterial.Apply(pContext);
		pContext->SetVertexBuffer(&m_charVerts);
		pContext->DrawImmediate(m_uCharCount);
	}

	pContext->EndGPUTag();

}


void DebugRender::Clear()
{
	m_uBarCount = 0;
	m_uCharCount = 0;	
}

}
