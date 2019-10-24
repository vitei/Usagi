#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "BlendSettings.h"

static const char* g_szBlendFunc[] =
{
	"0",
	"1",
	"Particle RGB",
	"1-Particle RGB",
	"Background RGB",
	"1 - Background RGB",
	"Particle Alpha",
	"1 - Particle Alpha",
	"Background Alpha",
	"1 - Background Alpha",
	"Constant Color",
	"1 - Constant Color",
	"Constant Alpha",
	"1 - Constant Alpha",
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
	m_comboBoxes[COMBO_BOX_RGB_SRC].Init("Src RGB Multiplier", g_szBlendFunc, usg::BLEND_FUNC_SRC_ALPHA);
	m_comboBoxes[COMBO_BOX_RGB_SRC].SetToolTip("Multiplier for particle RGB color");
	m_comboBoxes[COMBO_BOX_RGB_OP].Init("RGB Combine Op", g_szBlendEq, usg::BLEND_EQUATION_ADD);
	m_comboBoxes[COMBO_BOX_RGB_OP].SetToolTip("How the particle color and bg color are combined");
	m_comboBoxes[COMBO_BOX_RGB_DST].Init("Dest RGB Multipler", g_szBlendFunc, usg::BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
	m_comboBoxes[COMBO_BOX_RGB_DST].SetToolTip("Multiplier for the background color");
	
	m_comboBoxes[COMBO_BOX_ALPHA_SRC].Init("Src Alpha Multiplier", g_szBlendFunc, usg::BLEND_FUNC_SRC_ALPHA);
	m_comboBoxes[COMBO_BOX_ALPHA_SRC].SetToolTip("Advanced use only: Multiplier for particle alpha");

	m_comboBoxes[COMBO_BOX_ALPHA_DST].Init("Dest Alpha Multiplier", g_szBlendFunc, usg::BLEND_FUNC_ONE_MINUS_SRC_ALPHA);
	m_comboBoxes[COMBO_BOX_ALPHA_DST].SetToolTip("Advanced use only: Multiplier for background alpha");
	m_comboBoxes[COMBO_BOX_ALPHA_OP].Init("Alpha Combine Op", g_szBlendEq, usg::BLEND_EQUATION_ADD);
	m_comboBoxes[COMBO_BOX_ALPHA_OP].SetToolTip("OP: \"(ParticleAlpha * SRC) OP (BackgroundAlpha * DST)\"");
	m_comboBoxes[COMBO_BOX_ALPHA_TEST_OP].Init("Alpha Test", g_alphaTest, 0);
	m_comboBoxes[COMBO_BOX_ALPHA_TEST_OP].SetToolTip("Advanced use only: Operation for combining particle and background alpha");
	m_alphaRef.Init("Alpha Ref", 0.0f, 1.0f, 0.0f);
	m_alphaRef.SetToolTip("Alpha comparison value (failed pixels won't render");
	m_softDistance.Init("Softness Rng", 0.0f, 10.0f, 1.0f);
	m_softDistance.SetToolTip("Depth distance over which the particle fades in (values over 0 have a negative impact on performance)");
	m_cameraOffset.Init("Camera Offset", 0.0f, 10.0f, 1.0f);
	m_cameraOffset.SetToolTip("The particle will be displaced by this many units towards the viewer");


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
	m_window.AddItem(&m_cameraOffset);
	//pRenderer->AddWindow(&m_window);
}

void BlendSettings::SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData)
{
	usg::AlphaStateGroup& alphaDecl = structData.blend;

	if (!structData.has_fCameraOffset)
	{
		structData.has_fCameraOffset = true;
		structData.fCameraOffset = 0.0f;
	}

	m_comboBoxes[COMBO_BOX_RGB_SRC].SetSelected(alphaDecl.rgbSrcFunc);
	m_comboBoxes[COMBO_BOX_RGB_DST].SetSelected(alphaDecl.rgbDestFunc);
	m_comboBoxes[COMBO_BOX_RGB_OP].SetSelected(alphaDecl.rgbOp);
	m_comboBoxes[COMBO_BOX_ALPHA_SRC].SetSelected(alphaDecl.alphaSrcFunc);
	m_comboBoxes[COMBO_BOX_ALPHA_DST].SetSelected(alphaDecl.alphaDestFunc);
	m_comboBoxes[COMBO_BOX_ALPHA_OP].SetSelected(alphaDecl.alphaOp);
	m_comboBoxes[COMBO_BOX_ALPHA_TEST_OP].SetSelected(alphaDecl.alphaTestFunc == usg::ALPHA_TEST_ALWAYS ? 0 : 1);
	m_alphaRef.SetValue(alphaDecl.alphaTestReference);
	m_softDistance.SetValue(structData.fSoftFadeDistance);
	m_cameraOffset.SetValue(structData.fCameraOffset);

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
	bAltered |= Compare(structData.fCameraOffset, m_cameraOffset.GetValue());

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