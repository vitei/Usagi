#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "SortSettings.h"

static const char* g_szLayerStrings[] =
{
	"Opaque",
	"Translucent",
	"Subtractive",
	"Additive",
	NULL
};

SortSettings::SortSettings()
{
	m_bForceUpdate = false;
}


SortSettings::~SortSettings()
{

}

void SortSettings::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer)
{
	usg::Vector2f vWindowPos(340.0f, 400.0f);
	usg::Vector2f vWindowSize(320.f, 145.f);
	m_window.Init("Sorting", vWindowPos, vWindowSize, 1.0f, usg::GUIWindow::WINDOW_TYPE_COLLAPSABLE);
	m_renderLayer.Init("Layer", g_szLayerStrings, 0);
	int defaultPriority = 11;
	m_priority.Init("Priority", &defaultPriority, 1, 0, 128);
	m_depthWrite.Init("Write depth", false);
	m_depthOffset.Init("Depth offset", -1.0f, 1.0f, 0.0f);

	m_window.AddItem(&m_renderLayer);
	m_window.AddItem(&m_priority);
	m_window.AddItem(&m_depthWrite);
	m_window.AddItem(&m_depthOffset);

}

void SortSettings::SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData)
{
	usg::particles::SortSettings& sortVars = structData.sortSettings;
	m_renderLayer.SetSelected(sortVars.eRenderLayer);
	int iPriority = (int)sortVars.uPriority;
	m_priority.SetValues(&iPriority);
	m_depthWrite.SetValue(sortVars.bWriteDepth);
	if (sortVars.has_fDepthOffset)
	{
		m_depthOffset.SetValue(sortVars.fDepthOffset);
	}

	m_bForceUpdate = true;
}

bool SortSettings::Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect)
{
	bool bAltered = false;
	usg::particles::SortSettings& sortVars = structData.sortSettings;

	bAltered |= Compare(sortVars.bWriteDepth,m_depthWrite.GetValue());
	bAltered |= Compare(sortVars.fDepthOffset, m_depthOffset.GetValue(0));

	if(bAltered || m_bForceUpdate)
	{
	/*	usg::PipelineStateDecl pipeline;
		pDevice->GetPipelineDeclaration(pEffect->GetGetpipeline)
		usg::DepthStencilStateDecl& depthDecl = pipeline.depthState;
		depthDecl.bDepthWrite		= sortVars.bWriteDepth;
		depthDecl.bDepthEnable		= true;
		depthDecl.eDepthFunc 		= usg::DEPTH_TEST_LESS;
		depthDecl.bStencilEnable	= false;
		depthDecl.eStencilTest		= usg::STENCIL_TEST_ALWAYS;


		usg::Material& material = pEffect->GetMaterial();

		usg::RasterizerStateDecl& rasterizerState = pipeline.rasterizerState;
		rasterizerState.eCullFace = usg::CULL_FACE_NONE;
		rasterizerState.bUseDepthBias = m_depthOffset.GetValue() != 0.0f;
		rasterizerState.fDepthBias = m_depthOffset.GetValue();


		material.SetPipelineState(pDevice->GetPipelineState(pipeline));*/

		sortVars.has_fDepthOffset = true;

		m_bForceUpdate = false;
	}

	bAltered |= Compare(sortVars.eRenderLayer,m_renderLayer.GetSelected());
	bAltered |= Compare(sortVars.uPriority,m_priority.GetValue(0));

	pEffect->SetPriority(sortVars.uPriority);


	// Nothing we do here makes it necessary to re-create the effect
	return bAltered;
}