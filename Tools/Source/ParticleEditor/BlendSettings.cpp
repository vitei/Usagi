#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "BlendSettings.h"

static const char* g_szBlendFunc[] =
{
	"Zero",
	"One",
	"Source Color",
	"Inv Source Color",
	"Dest Color",
	"Inv Dest Color",
	"Src Alpha",
	"Inv Src Alpha",
	"Dst Alpha",
	"Inv Dest Alpha",
	"Constant Color",
	"Inv Constant Color",
	"Constant Alpha",
	"Inverse Constant Alpha",
	"Alpha Saturate",
	NULL
};

static const char* g_szBlendEq[] =
{
	"Add",
	"Subtract",
	"Reverse Subtract",
	"Minimum",
	"Maximum", 
	NULL
};

static const char* g_alphaTest[] =
{
	//"Never",		 	// ALPHA_TEST_NEVER
	"Always",			// ALPHA_TEST_ALWAYS
	//"Equal",			// ALPHA_TEST_EQUAL,
	//"Not equal",		// ALPHA_TEST_NOTEQUAL
	//"Less",				// ALPHA_TEST_LESS
	//"Less or equal",	// ALPHA_TEST_LEQUAL
	"Greater"			// ALPHA_TEST_GREATER
	//"Equal"				// ALPHA_TEST_GEQUAL
};

BlendSettings::BlendSettings()
{
	m_bForceUpdate = false;
}


BlendSettings::~BlendSettings()
{

}

void BlendSettings::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer)
{
	usg::Vector2f vWindowPos(0.0f, 0.0f);
	usg::Vector2f vWindowSize(320.f, 240.f);
	m_window.Init("Blend", vWindowPos, vWindowSize, usg::GUIWindow::WINDOW_TYPE_COLLAPSABLE);
	m_comboBoxes[COMBO_BOX_RGB_SRC].Init("Rgb Source", g_szBlendFunc, usg::BLEND_FUNC_SRC_ALPHA);
	m_comboBoxes[COMBO_BOX_RGB_DST].Init("Rgb Dest", g_szBlendFunc, usg::BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
	m_comboBoxes[COMBO_BOX_RGB_OP].Init("Rgb Op", g_szBlendEq, usg::BLEND_EQUATION_ADD);
	m_comboBoxes[COMBO_BOX_ALPHA_SRC].Init("Alpha Source", g_szBlendFunc, usg::BLEND_FUNC_SRC_ALPHA);
	m_comboBoxes[COMBO_BOX_ALPHA_DST].Init("Alpha Dest", g_szBlendFunc, usg::BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
	m_comboBoxes[COMBO_BOX_ALPHA_OP].Init("Alpha Op", g_szBlendEq, usg::BLEND_EQUATION_ADD);
	m_comboBoxes[COMBO_BOX_ALPHA_TEST_OP].Init("Alpha Test", g_alphaTest, 0);
	m_alphaRef.Init("Alpha Ref", 0.0f, 1.0f, 0.0f);
	m_softDistance.Init("Softness Rng", 0.0f, 3.0f, 1.0f);

	m_constColorSelect.Init("Const Color");
	usg::Color tmp(1.0f, 1.0f, 1.0f, 1.0f);
	m_constColorSelect.SetValue(tmp);

	for(uint32 i=0; i<COMBO_BOX_ALPHA_TEST_OP; i++)
	{
		m_window.AddItem(&m_comboBoxes[i]);
	}
	m_window.AddItem(&m_constColorSelect);

	m_window.AddItem(&m_comboBoxes[COMBO_BOX_ALPHA_TEST_OP]);
	m_window.AddItem(&m_alphaRef);
	m_window.AddItem(&m_softDistance);
	//pRenderer->AddWindow(&m_window);
}

void BlendSettings::SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData)
{
	usg::AlphaStateGroup& alphaDecl = structData.blend;
	m_comboBoxes[COMBO_BOX_RGB_SRC].SetSelected(alphaDecl.rgbSrcFunc);
	m_comboBoxes[COMBO_BOX_RGB_DST].SetSelected(alphaDecl.rgbDestFunc);
	m_comboBoxes[COMBO_BOX_RGB_OP].SetSelected(alphaDecl.rgbOp);
	m_comboBoxes[COMBO_BOX_ALPHA_SRC].SetSelected(alphaDecl.alphaSrcFunc);
	m_comboBoxes[COMBO_BOX_ALPHA_DST].SetSelected(alphaDecl.alphaDestFunc);
	m_comboBoxes[COMBO_BOX_ALPHA_OP].SetSelected(alphaDecl.alphaOp);
	m_comboBoxes[COMBO_BOX_ALPHA_TEST_OP].SetSelected(alphaDecl.alphaTestFunc == usg::ALPHA_TEST_ALWAYS ? 0 : 1);
	m_alphaRef.SetValue(alphaDecl.alphaTestReference);
	m_softDistance.SetValue(structData.fSoftFadeDistance);
	
	m_bForceUpdate = true;
}

bool BlendSettings::Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect)
{
	bool bAltered = false;
	usg::AlphaStateGroup& alphaDecl = structData.blend;

	bAltered |= Compare(alphaDecl.rgbSrcFunc,m_comboBoxes[COMBO_BOX_RGB_SRC].GetSelected());
	bAltered |= Compare(alphaDecl.rgbDestFunc,m_comboBoxes[COMBO_BOX_RGB_DST].GetSelected());
	bAltered |= Compare(alphaDecl.rgbOp,m_comboBoxes[COMBO_BOX_RGB_OP].GetSelected());
	bAltered |= Compare(alphaDecl.alphaSrcFunc,m_comboBoxes[COMBO_BOX_ALPHA_SRC].GetSelected());
	bAltered |= Compare(alphaDecl.alphaDestFunc,m_comboBoxes[COMBO_BOX_ALPHA_DST].GetSelected());
	bAltered |= Compare(alphaDecl.alphaOp,m_comboBoxes[COMBO_BOX_ALPHA_OP].GetSelected());
	bAltered |= Compare(alphaDecl.alphaTestFunc,m_comboBoxes[COMBO_BOX_ALPHA_TEST_OP].GetSelected() ? usg::ALPHA_TEST_GREATER : usg::ALPHA_TEST_ALWAYS);
	bAltered |= Compare(alphaDecl.alphaTestReference,m_alphaRef.GetValue());
	bAltered |= Compare(structData.fSoftFadeDistance, m_softDistance.GetValue());

	if(bAltered || m_bForceUpdate)
	{
		usg::Material& material = pEffect->GetMaterial();
		usg::PipelineStateDecl decl;	
		usg::RenderPassHndl passHndl;
		pDevice->GetPipelineDeclaration(material.GetPipelineStateHndl(), decl, passHndl);
		decl.alphaState.InitFromDefinition(structData.blend);
		
		material.SetPipelineState(pDevice->GetPipelineState(passHndl, decl));
		m_bForceUpdate = false;
	}

	// Nothing we do here makes it necessary to re-create the effect
	return bAltered;
}