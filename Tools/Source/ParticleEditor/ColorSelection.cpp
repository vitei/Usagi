#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Memory/MemUtil.h"
#include "Engine/Graphics/Device/Display.h"
#include "Engine/Layout/Global2D.h"
#include "Engine/HID/Mouse.h"
#include "Engine/HID/Input.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/RenderConsts.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Scene/ViewContext.h"
#include "Engine/Layout/Global2D.h"
#include "Engine/Scene/SceneConstantSets.h"
#include "Engine/Scene/Scene.h"
#include "ColorSelection.h"

static const float32 g_fBarLeft = 0.85f;
static const float32 g_fBarRight = 1.0f;

static const float32 g_fHSTop = 0.2f;
static const float32 g_fHSBottom = 1.0f;
static const float32 g_fHSLeft = 0.1f;
static const float32 g_fHSRight = 0.8f;

static const float32 g_fPreviewTop = 0.0f;
static const float32 g_fPreviewBottom = 0.15f;
static const float32 g_fPreviewLeft = 0.1f;
static const float32 g_fPreviewRight = 0.5f;

static const float32 g_fPreviousTop = 0.0f;
static const float32 g_fPreviousBottom = 0.15f;
static const float32 g_fPreviousLeft = 0.50f;
static const float32 g_fPreviousRight = 0.65f;

static const float32 g_fHistorySize = 0.08f;
static const float32 g_fHistoryGap = 0.01f;


struct SolidConstants
{
	usg::Matrix4x4	mProj;
};

static const usg::ShaderConstantDecl g_solidConstantDef[] = 
{
	SHADER_CONSTANT_ELEMENT( SolidConstants, mProj,	usg::CT_MATRIX_44, 1 ),
	SHADER_CONSTANT_END()
};

static const usg::DescriptorDeclaration g_descriptorDecl[] =
{
	DESCRIPTOR_ELEMENT(0,						 usg::DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, usg::SHADER_FLAG_PIXEL),
	DESCRIPTOR_END()
};

static const usg::DescriptorDeclaration g_solidDescriptorDecl[] =
{
	DESCRIPTOR_END()
};


ColorSelection::ColorSelection()
{
	m_color.Assign(1.0f, 1.0f, 1.0f, 1.0f);
	m_prevColor = m_color;
	m_saturatedColor.Assign(1.0f, 0.0f, 0.0f, 1.0f);
	m_bVertsDirty = false;
}

ColorSelection::~ColorSelection()
{

}

void FillOutVert(float fX, float fY, float fR, float fG, float fB, usg::PositionDiffuseVertex* pVertOut)
{
	pVertOut->x = fX;
	pVertOut->y = fY;
	pVertOut->z = 0.0f;

	pVertOut->r = fR;
	pVertOut->g = fG;
	pVertOut->b = fB;
	pVertOut->a = 1.0f;
}

void ColorSelection::CleanUp(usg::GFXDevice* pDevice)
{
	m_globalDescriptor.CleanUp(pDevice);
	m_hueSatVertices.CleanUp(pDevice);
	m_rgbVertices.CleanUp(pDevice);
	m_indices.CleanUp(pDevice);
	m_hueSatIndices.CleanUp(pDevice);
	m_imageIndices.CleanUp(pDevice);
	m_material.Cleanup(pDevice);
	m_constants.CleanUp(pDevice);

	for (uint32 i = 0; i < CURSOR_COUNT; i++)
	{
		m_cursors[i].material.Cleanup(pDevice);
		m_cursors[i].vertices.CleanUp(pDevice);
	}
}

void ColorSelection::Init(usg::GFXDevice* pDevice, usg::Scene& scene)
{
	uint32 uHSVertices = VERTEX_COUNT;
	uint32 uRGBVertices = HUE_RGB_STACKS * 2;
	

	usg::SamplerDecl samplerDecl(usg::SF_LINEAR, usg::SC_WRAP);
	m_sampler = pDevice->GetSampler(samplerDecl);
	
	{
		uint32 uIndexCount = 6*(HUE_RGB_STACKS-1);
		uint16* puIndices;
		usg::ScratchObj<uint16> scratchIndices(puIndices, uIndexCount);
		// Bar 1
		for(uint32 i=0; i<(HUE_RGB_STACKS-1); i++)
		{
			uint32 uIndexOff = 6*i;
			uint32 uVertOff = 2*i;
			puIndices[0+uIndexOff] = 2 + uVertOff;
			puIndices[1+uIndexOff] = 1 + uVertOff;
			puIndices[2+uIndexOff] = 0 + uVertOff;
			puIndices[3+uIndexOff] = 2 + uVertOff;
			puIndices[4+uIndexOff] = 3 + uVertOff;
			puIndices[5+uIndexOff] = 1 + uVertOff;
		}
		m_indices.Init(pDevice, puIndices, uIndexCount, true);
	}

	{
		uint32 uIndexCount = INDEX_COUNT;
		uint16* puIndices;
		usg::ScratchObj<uint16> scratchIndices(puIndices, uIndexCount);
		// Bar 1
		for(uint32 i=0; i<INDEX_COUNT/6; i++)
		{
			uint32 uIndexOff = 6*i;
			uint32 uVertOff = 4*i;
			puIndices[0+uIndexOff] = 2 + uVertOff;
			puIndices[1+uIndexOff] = 1 + uVertOff;
			puIndices[2+uIndexOff] = 0 + uVertOff;
			puIndices[3+uIndexOff] = 2 + uVertOff;
			puIndices[4+uIndexOff] = 3 + uVertOff;
			puIndices[5+uIndexOff] = 1 + uVertOff;
		}
		m_hueSatIndices.Init(pDevice, puIndices, uIndexCount, true);
	}


	{
		// A single image
		uint32 uIndexCount = 6;
		uint16* puIndices;
		usg::ScratchObj<uint16> scratchIndices(puIndices, uIndexCount);
	
		puIndices[0] = 2;
		puIndices[1] = 1;
		puIndices[2] = 0;
		puIndices[3] = 2;
		puIndices[4] = 3;
		puIndices[5] = 1;

		m_imageIndices.Init(pDevice, puIndices, uIndexCount, true);
	}

	m_cursors[CURSOR_HUE].vSize.Assign(0.04f, 0.04f);
	m_cursors[CURSOR_HUE].vPosition.Assign(g_fBarLeft-0.02f, 0.0f);
	
	m_cursors[CURSOR_SATURATION].vSize.Assign(0.02f, 0.1f);
	m_cursors[CURSOR_SATURATION].vPosition.Assign(g_fHSLeft, g_fHSBottom);

	m_cursors[CURSOR_POINTER].vPosition.Assign(g_fHSLeft, g_fHSTop);
	m_cursors[CURSOR_POINTER].vSize.Assign(0.05f, 0.05f);
	for (uint32 uVert = 0; uVert < 4; uVert++)
	{
		m_cursors[CURSOR_POINTER].verts[uVert].c.Assign(0.0f, 0.0f, 0.0f, 0.0f);
		m_cursors[CURSOR_HUE].verts[uVert].c.Assign(1.0f, 1.0f, 1.0f, 1.0f);
	}

	for (uint32 i = 0; i < CURSOR_COUNT; i++)
	{
		for (uint32 uVert = 0; uVert < 4; uVert++)
		{
			m_cursors[i].verts[uVert].z = 0.0f;
		}

		m_cursors[i].verts[0].u = 0.0f;
		m_cursors[i].verts[0].v = 0.0f;
		m_cursors[i].verts[1].u = 1.0f;
		m_cursors[i].verts[1].v = 0.0f;
		m_cursors[i].verts[2].u = 0.0f;
		m_cursors[i].verts[2].v = 1.0f;
		m_cursors[i].verts[3].u = 1.0f;
		m_cursors[i].verts[3].v = 1.0f;

		m_cursors[i].vertices.Init(pDevice, NULL, sizeof(usg::PositionUVColVertex), 6, "Cursor", usg::GPU_USAGE_DYNAMIC);
	}

	UpdateCursorVerts(pDevice,CURSOR_POINTER);
	UpdateCursorVerts(pDevice,CURSOR_HUE);
	usg::PipelineStateDecl pipeline;
	pipeline.ePrimType = usg::PT_TRIANGLES;
	pipeline.inputBindings[0].Init(GetVertexDeclaration(usg::VT_POSITION_UV_COL));
	pipeline.uInputBindingCount = 1;

	usg::DescriptorSetLayoutHndl matDescriptors = pDevice->GetDescriptorSetLayout(g_descriptorDecl);
	pipeline.layout.uDescriptorSetCount = 2;
	usg::DescriptorSetLayoutHndl globalDesc = pDevice->GetDescriptorSetLayout(usg::g_sGlobalDescriptors2D);
	pipeline.layout.descriptorSets[0] = globalDesc;
	pipeline.layout.descriptorSets[1] = matDescriptors;

	pipeline.alphaState.SetColor0Only();
	pipeline.alphaState.bBlendEnable = true;
	pipeline.alphaState.blendEq = usg::BLEND_EQUATION_ADD;
	pipeline.alphaState.srcBlend = usg::BLEND_FUNC_SRC_ALPHA;
	pipeline.alphaState.dstBlend = usg::BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
	pipeline.pEffect = usg::ResourceMgr::Inst()->GetEffect(pDevice, "Debug.PosColUV");
	
	usg::RenderPassHndl renderPassHndl = pDevice->GetDisplay(0)->GetRenderPass();
	m_cursors[CURSOR_POINTER].material.Init(pDevice, pDevice->GetPipelineState(renderPassHndl,pipeline), matDescriptors);
	m_cursors[CURSOR_HUE].material.Init(pDevice, pDevice->GetPipelineState(renderPassHndl, pipeline), matDescriptors);
	pipeline.pEffect = usg::ResourceMgr::Inst()->GetEffect(pDevice, "Debug.PosCol");
	pipeline.layout.uDescriptorSetCount = 1;
	m_cursors[CURSOR_SATURATION].material.Init(pDevice, pDevice->GetPipelineState(renderPassHndl, pipeline), usg::DescriptorSetLayoutHndl());

	m_cursors[CURSOR_POINTER].material.SetTexture(0, usg::ResourceMgr::Inst()->GetTexture(pDevice, "colorpointer_white"), m_sampler);
	m_cursors[CURSOR_HUE].material.SetTexture(0, usg::ResourceMgr::Inst()->GetTexture(pDevice, "color_handle"), m_sampler);
	
	m_hueColors[0].Assign(1.0f, 0.0f, 0.0f);
	m_hueColors[1].Assign(1.0f, 0.0f, 1.0f);
	m_hueColors[2].Assign(0.0f, 0.0f, 1.0f);
	m_hueColors[3].Assign(0.0f, 1.0f, 1.0f);
	m_hueColors[4].Assign(0.0f, 1.0f, 0.0f);
	m_hueColors[5].Assign(1.0f, 1.0f, 0.0f);
	m_hueColors[6].Assign(1.0f, 0.0f, 0.0f);
	
	

	// Set up the hue saturation vertices
	{
		usg::MemSet(m_hueSatCPUVerts, 0, sizeof(usg::PositionDiffuseVertex)*uHSVertices);
	
		// Top left
		FillOutVert( g_fHSLeft, g_fHSTop, 1.0f, 1.0f, 1.0f, &m_hueSatCPUVerts[SV_INDEX+0] );
		// Top right (defaulting to red)
		FillOutVert( g_fHSRight, g_fHSTop, 1.0f, 0.0f, 0.0f, &m_hueSatCPUVerts[SV_INDEX+1] );
		// Bottom left
		FillOutVert( g_fHSLeft, g_fHSBottom, 0.0f, 0.0f, 0.0f, &m_hueSatCPUVerts[SV_INDEX+2] );
		// Bottom right
		FillOutVert( g_fHSRight, g_fHSBottom, 0.0f, 0.0f, 0.0f, &m_hueSatCPUVerts[SV_INDEX+3] );

		// The preview
		// Top left
		FillOutVert( g_fPreviewLeft, g_fPreviewTop, 1.0f, 1.0f, 1.0f, &m_hueSatCPUVerts[PREVIEW_INDEX+0] );
		// Top right
		FillOutVert( g_fPreviewRight, g_fPreviewTop, 1.0f, 1.0f, 1.0f, &m_hueSatCPUVerts[PREVIEW_INDEX+1] );
		// Bottom left
		FillOutVert( g_fPreviewLeft, g_fPreviewBottom, 1.0f, 1.0f, 1.0f, &m_hueSatCPUVerts[PREVIEW_INDEX+2] );
		// Bottom right
		FillOutVert( g_fPreviewRight, g_fPreviewBottom, 1.0f, 1.0f, 1.0f, &m_hueSatCPUVerts[PREVIEW_INDEX+3] );


		// Previous
		// Top left
		FillOutVert( g_fPreviousLeft, g_fPreviousTop, 1.0f, 1.0f, 1.0f, &m_hueSatCPUVerts[PREVIOUS_INDEX+0] );
		// Top right
		FillOutVert( g_fPreviousRight, g_fPreviousTop, 1.0f, 1.0f, 1.0f, &m_hueSatCPUVerts[PREVIOUS_INDEX+1] );
		// Bottom left
		FillOutVert( g_fPreviousLeft, g_fPreviousBottom, 1.0f, 1.0f, 1.0f, &m_hueSatCPUVerts[PREVIOUS_INDEX+2] );
		// Bottom right
		FillOutVert( g_fPreviousRight, g_fPreviousBottom, 1.0f, 1.0f, 1.0f, &m_hueSatCPUVerts[PREVIOUS_INDEX+3] );

		float fYPos = g_fHSTop;
		uint32 uIndex = HISTORY_INDEX;
		for(uint32 i=0; i<HISTORY_COUNT; i++)
		{
			// Previous
			// Top left
			FillOutVert( 0.0f, fYPos, 1.0f, 1.0f, 1.0f, &m_hueSatCPUVerts[uIndex++] );
			// Top right
			FillOutVert( g_fHistorySize, fYPos, 1.0f, 1.0f, 1.0f, &m_hueSatCPUVerts[uIndex++] );
			// Bottom left
			FillOutVert( 0.0f, fYPos+g_fHistorySize, 1.0f, 1.0f, 1.0f, &m_hueSatCPUVerts[uIndex++] );
			// Bottom right
			FillOutVert( g_fHistorySize, fYPos+g_fHistorySize, 1.0f, 1.0f, 1.0f, &m_hueSatCPUVerts[uIndex++] );

			m_historyColors[i].Assign(1.0f, 1.0f, 1.0f, 1.0f);

			fYPos += (g_fHistorySize + g_fHistoryGap);
		}


		m_hueSatVertices.Init( pDevice, &m_hueSatCPUVerts, sizeof(usg::PositionDiffuseVertex), uHSVertices, "Hue Sat", usg::GPU_USAGE_DYNAMIC);
	}
	
	{
		usg::PositionDiffuseVertex* pVerts;
		usg::ScratchObj<usg::PositionDiffuseVertex> scratchVerts(pVerts, uRGBVertices);
		float fLeft = g_fBarLeft;
		float fRight = g_fBarRight;
		float fTop = 0.0f;
		float fInc = 1.f/(float)(HUE_RGB_STACKS-1);

		
		for(uint32 i=0; i<HUE_RGB_STACKS; i++)
		{
			FillOutVert( fLeft, fTop + (i*fInc), m_hueColors[i].r(), m_hueColors[i].g(), m_hueColors[i].b(), &pVerts[0+(i*2)] );
			FillOutVert( fRight, fTop + (i*fInc), m_hueColors[i].r(), m_hueColors[i].g(), m_hueColors[i].b(), &pVerts[1+(i*2)] );
		}
		

		m_rgbVertices.Init( pDevice, pVerts, sizeof(usg::PositionDiffuseVertex), uRGBVertices, "RGB color", usg::GPU_USAGE_STATIC);
	}


	pipeline.inputBindings[0].Init(GetVertexDeclaration(usg::VT_POSITION_DIFFUSE));
	pipeline.pEffect = usg::ResourceMgr::Inst()->GetEffect(pDevice, "Debug.PosCol");

	m_material.Init(pDevice, pDevice->GetPipelineState(renderPassHndl, pipeline), usg::DescriptorSetLayoutHndl());
	m_constants.Init(pDevice, g_solidConstantDef);

	m_globalDescriptor.Init(pDevice, globalDesc);
	SolidConstants* pConsts = m_constants.Lock<SolidConstants>();
	pConsts->mProj.Orthographic( 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 10.0f ); 
	m_constants.Unlock();
	m_constants.UpdateData(pDevice);
	m_globalDescriptor.SetConstantSet(0, &m_constants);
	m_globalDescriptor.UpdateDescriptors(pDevice);

	m_cursors[CURSOR_POINTER].material.UpdateDescriptors(pDevice);
	m_cursors[CURSOR_HUE].material.UpdateDescriptors(pDevice);

	m_viewport.InitViewport(0.0, 0.0, 100, 120);
}

void ColorSelection::SetPosition(float fX, float fY)
{
	m_viewport.SetPos((uint32)fX, (uint32)fY);
}

void ColorSelection::SetSize(float fX, float fY)
{
	m_viewport.SetSize((uint32)fX, (uint32)fY);
}


void ColorSelection::UpdateCursorVerts(usg::GFXDevice* pDevice, CURSOR_TYPE eType)
{
	Image& image = m_cursors[eType];
	usg::Vector2f vHalfSize = image.vSize / 2.f;
	float fLeft = image.vPosition.x - vHalfSize.x;
	float fRight = image.vPosition.x + vHalfSize.x;
	float fTop = image.vPosition.y - vHalfSize.y;
	float fBottom = image.vPosition.y + vHalfSize.y;

	// Top left
	image.verts[0].x = fLeft;
	image.verts[0].y = fTop;
	// Top right
	image.verts[1].x = fRight;
	image.verts[1].y = fTop;

	// Bottom left
	image.verts[2].x = fLeft;
	image.verts[2].y = fBottom;

	// Bottom right
	image.verts[3].x = fRight;
	image.verts[3].y = fBottom;

	image.vertices.SetContents(pDevice, image.verts, 0);
}

bool GetInArea(const usg::Vector2f &vPos, const usg::Vector2f &vMin, const usg::Vector2f &vMax, usg::Vector2f &vInterpolation)
{
	if( vPos.x > vMin.x 
		&& vPos.x < vMax.x 
		&& vPos.y > vMin.y 
		&& vPos.y < vMax.y)
	{
	
		usg::Vector2f vRange = vMax - vMin;
		vInterpolation = (vPos - vMin)/vRange;

		return true;
	}

	vInterpolation.Assign(0.0f, 0.0f);
	return false;
}

void ColorSelection::Update(usg::GFXDevice* pDevice, float fElapsed)
{
	const usg::Mouse* pMouse = usg::Input::GetMouse();

	usg::Vector2f vBarMin(m_viewport.GetLeft() +(g_fBarLeft * m_viewport.GetWidth()) , m_viewport.GetBottom() );
	usg::Vector2f vBarMax(m_viewport.GetLeft() + (g_fBarRight * m_viewport.GetWidth()), m_viewport.GetBottom() + m_viewport.GetHeight());

	usg::Vector2f vHSMin(m_viewport.GetLeft() +(g_fHSLeft * m_viewport.GetWidth()) , m_viewport.GetBottom()  +((1.0f-g_fHSBottom) * m_viewport.GetHeight()));
	usg::Vector2f vHSMax(m_viewport.GetLeft() + (g_fHSRight * m_viewport.GetWidth()), m_viewport.GetBottom() + ((1.0f-g_fHSTop) * m_viewport.GetHeight()));

	usg::Vector2f vPrevMin(m_viewport.GetLeft() +(g_fPreviousLeft * m_viewport.GetWidth()) , m_viewport.GetBottom()  +((1.0f-g_fPreviousBottom) * m_viewport.GetHeight()));
	usg::Vector2f vPrevMax(m_viewport.GetLeft() + (g_fPreviousRight * m_viewport.GetWidth()), m_viewport.GetBottom() + ((1.0f-g_fPreviousTop) * m_viewport.GetHeight()));

	uint32 uWidth, uHeight;
	pDevice->GetDisplay(0)->GetDisplayDimensions(uWidth, uHeight, false);
	usg::Vector2f vMousePos(pMouse->GetAxis(usg::MOUSE_POS_X), uHeight-pMouse->GetAxis(usg::MOUSE_POS_Y));

	usg::Vector2f vInterpolation;
	if(pMouse->GetButton(usg::MOUSE_BUTTON_LEFT, usg::BUTTON_STATE_HELD) && GetInArea(vMousePos, vBarMin, vBarMax, vInterpolation) )
	{
		uint32 uIndex = (uint32)((1.0f-vInterpolation.y) * (HUE_RGB_STACKS-1));
		float fGroupSize = 1.f/(HUE_RGB_STACKS-1);
		float fInterpolation =  (1.0f-vInterpolation.y) - (fGroupSize * uIndex);
		fInterpolation /= fGroupSize;

		usg::Color color = usg::Math::Lerp(m_hueColors[uIndex], m_hueColors[uIndex+1], fInterpolation);

		m_cursors[CURSOR_HUE].vPosition.y = usg::Math::Lerp(0.0f, 1.0f, 1.0f-vInterpolation.y);
		UpdateCursorVerts(pDevice, CURSOR_HUE);

		SetHueColor(color);

		m_bVertsDirty = true;
	}

	if(pMouse->GetButton(usg::MOUSE_BUTTON_LEFT, usg::BUTTON_STATE_HELD) && GetInArea(vMousePos, vHSMin, vHSMax, vInterpolation) )
	{
		uint32 uIndex = (uint32)((1.0f-vInterpolation.y) * (HUE_RGB_STACKS-1));
		usg::Vector2f vHSMinVP(g_fHSLeft, g_fHSBottom);
		usg::Vector2f vHSMaxVP(g_fHSRight, g_fHSTop);
		m_cursors[CURSOR_POINTER].vPosition = Lerp(vHSMinVP, vHSMaxVP, vInterpolation);
		vInterpolation.y = 1.0f - vInterpolation.y;
		usg::Color white(1.0f, 1.0f, 1.0f, 1.0f);
		usg::Color black(0.0f, 0.0f, 0.0f, 1.0f);
		usg::Color color = usg::Math::Lerp(white, m_saturatedColor, vInterpolation.x);
		color = usg::Math::Lerp(color, black, vInterpolation.y);
		for (uint32 i = 0; i < 4; i++)
		{
			m_cursors[CURSOR_POINTER].verts[i].c = usg::Math::Lerp(white, black, 1.0f - vInterpolation.y);
		}

		UpdateCursorVerts(pDevice, CURSOR_POINTER);
		SetPreviewColor(color);

		m_bVertsDirty = true;
	}

	for(uint32 i=0; i<HISTORY_COUNT; i++)
	{
		const usg::PositionDiffuseVertex* pVerts = &m_hueSatCPUVerts[HISTORY_INDEX+(i*4)];
		usg::Vector2f vHisMin(m_viewport.GetLeft() +(pVerts[0].x * m_viewport.GetWidth()) , m_viewport.GetBottom()  +((1.0f-pVerts[3].y) * m_viewport.GetHeight()));
		usg::Vector2f vHisMax(m_viewport.GetLeft() + (pVerts[3].x * m_viewport.GetWidth()), m_viewport.GetBottom() + ((1.0f-pVerts[0].y) * m_viewport.GetHeight()));

		if(pMouse->GetButton(usg::MOUSE_BUTTON_LEFT, usg::BUTTON_STATE_PRESSED) && GetInArea(vMousePos, vHisMin, vHisMax, vInterpolation) )
		{
			SetColor(pDevice, m_historyColors[i]);
		}
	}

	if( GetInArea(vMousePos, vPrevMin, vPrevMax, vInterpolation) )
	{
		SetColor(pDevice, m_prevColor);
	}

	
	if(m_bVertsDirty)
	{
		m_hueSatVertices.SetContents( pDevice, &m_hueSatCPUVerts);
	}
}


void ColorSelection::SetHueColor(const usg::Color& color)
{
	m_hueSatCPUVerts[1].r = color.r();
	m_hueSatCPUVerts[1].g = color.g();
	m_hueSatCPUVerts[1].b = color.b();

	m_saturatedColor = color;

}

void ColorSelection::SetPreviewColor(const usg::Color& color)
{
	for(uint32 i=PREVIEW_INDEX; i<PREVIEW_INDEX+4; i++)
	{
		m_hueSatCPUVerts[i].r = color.r();
		m_hueSatCPUVerts[i].g = color.g();
		m_hueSatCPUVerts[i].b = color.b();
	}

	for(uint32 i=PREVIOUS_INDEX; i<PREVIOUS_INDEX+4; i++)
	{
		m_hueSatCPUVerts[i].r = m_prevColor.r();
		m_hueSatCPUVerts[i].g = m_prevColor.g();
		m_hueSatCPUVerts[i].b = m_prevColor.b();
	}

	m_color = color;
}

bool ColorSelection::Draw(usg::GFXContext* pContext, usg::PostFXSys* pPostFXSys)
{
	pContext->SetDescriptorSet(&m_globalDescriptor, 0);
	m_material.Apply(pContext);
	pContext->ApplyViewport(m_viewport);
	pContext->SetVertexBuffer(&m_hueSatVertices);
	pContext->DrawIndexed(&m_hueSatIndices);
	pContext->SetVertexBuffer(&m_rgbVertices);
	pContext->DrawIndexed(&m_indices);

	m_cursors[CURSOR_POINTER].material.Apply(pContext);
	pContext->SetVertexBuffer(&m_cursors[CURSOR_POINTER].vertices);
	pContext->DrawIndexed(&m_imageIndices);

	m_cursors[CURSOR_HUE].material.Apply(pContext);
	pContext->SetVertexBuffer(&m_cursors[CURSOR_HUE].vertices);
	pContext->DrawIndexed(&m_imageIndices);

	return true;
}


usg::Color ColorSelection::GetRGBBaseFromHue(float fHue) const
{
	// FIXME: Should we use true hue? This is better guaranteed to match what the artist is seeing
	uint32 uIndex = (uint32)(fHue * (HUE_RGB_STACKS-1));
	float fGroupSize = 1.f/(HUE_RGB_STACKS-1);
	float fInterpolation =  fHue - (fGroupSize * uIndex);
	fInterpolation /= fGroupSize;

	usg::Color color = usg::Math::Lerp(m_hueColors[uIndex], m_hueColors[uIndex+1], fInterpolation);
	color.a() = 1.0f;

	return color;
}


void ColorSelection::SetColor(usg::GFXDevice* pDevice, const usg::Color& color)
{
	m_prevColor = color;
	SetPreviewColor(color);
	float fHue = GetHue(color);
	m_cursors[CURSOR_HUE].vPosition.y = usg::Math::Lerp(0.0f, 1.0f, fHue);
	UpdateCursorVerts(pDevice, CURSOR_HUE);
	usg::Color hueColor = GetRGBBaseFromHue(fHue);
	SetHueColor(hueColor);

	m_hueSatVertices.SetContents( pDevice, &m_hueSatCPUVerts);
}

float ColorSelection::GetHue(const usg::Color& color) const
{
	float fMin = usg::Math::Min(usg::Math::Min(color.r(), color.g()), color.b());
	float fMax = usg::Math::Max(usg::Math::Max(color.r(), color.g()), color.b());

	float fHue = 0.0f;
	if (fMax == color.r())
	{
		fHue = (color.g() - color.b()) / (fMax - fMin);

	}
	else if (fMax == color.g())
	{
		fHue = 2.f + (color.b() - color.r()) / (fMax - fMin);
	}
	else
	{
		fHue = 4.f + (color.r() - color.g()) / (fMax - fMin);
	}

	return usg::Math::Clamp(1.0f-(fHue/6.f), 0.0f, 1.0f);
}

void ColorSelection::SaveColor(const usg::Color& color)
{

	for(uint32 i=HISTORY_COUNT-1; i>0; i--)
	{
		m_historyColors[i] = m_historyColors[i-1];
		for(uint32 uVert = 0; uVert<4; uVert++)
		{
			uint32 uVertex = (i*4)+(uVert);
			m_hueSatCPUVerts[HISTORY_INDEX+uVertex].r = m_historyColors[i].r();
			m_hueSatCPUVerts[HISTORY_INDEX+uVertex].g = m_historyColors[i].g();
			m_hueSatCPUVerts[HISTORY_INDEX+uVertex].b = m_historyColors[i].b();
			m_hueSatCPUVerts[HISTORY_INDEX+uVertex].a = m_historyColors[i].a();
		}
	}

	m_historyColors[0] = color;
	for(uint32 uVert = 0; uVert<4; uVert++)
	{
		uint32 uVertex = uVert;
		m_hueSatCPUVerts[HISTORY_INDEX+uVertex].r = m_historyColors[0].r();
		m_hueSatCPUVerts[HISTORY_INDEX+uVertex].g = m_historyColors[0].g();
		m_hueSatCPUVerts[HISTORY_INDEX+uVertex].b = m_historyColors[0].b();
		m_hueSatCPUVerts[HISTORY_INDEX+uVertex].a = m_historyColors[0].a();
	}


	m_bVertsDirty = true;
}