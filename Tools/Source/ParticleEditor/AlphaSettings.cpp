#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "AlphaSettings.h"

AlphaSettings::AlphaSettings()
{

}


AlphaSettings::~AlphaSettings()
{

}

void AlphaSettings::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer)
{
	usg::Vector2f vWindowPos(340.0f, 400.0f);
	usg::Vector2f vWindowSize(320.f, 145.f);
	m_window.Init("Alpha", vWindowPos, vWindowSize, usg::GUIWindow::WINDOW_TYPE_COLLAPSABLE);

	m_sliders[SLIDER_INITIAL_ALPHA].Init("Start alpha", 0.0f, 1.0f, 0.0f);
	m_sliders[SLIDER_INTERMEDIATE_ALPHA].Init("Standard alpha", 0.0f, 1.0f, 1.0f);
	m_sliders[SLIDER_END_ALPHA].Init("Final alpha", 0.0f, 1.0f, 0.0f);
	m_sliders[SLIDER_FINISH_IN_TIME].Init("Finish in time", 0.0f, 1.0f, 0.2f);
	m_sliders[SLIDER_FADE_OUT_TIMING].Init("Start out time", 0.0f, 1.0f, 0.8f);


	for(uint32 i=0; i<SLIDER_COUNT; i++)
	{
		m_window.AddItem(&m_sliders[i]);
	}
	//pRenderer->AddWindow(&m_window);
}

void AlphaSettings::SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData)
{
	usg::particles::ParticleAlpha& alphaVars = structData.particleAlpha;
	m_sliders[SLIDER_INITIAL_ALPHA].SetValue(alphaVars.fInitialAlpha);
	m_sliders[SLIDER_INTERMEDIATE_ALPHA].SetValue(alphaVars.fIntermediateAlpha);
	m_sliders[SLIDER_END_ALPHA].SetValue(alphaVars.fEndAlpha);
	m_sliders[SLIDER_FINISH_IN_TIME].SetValue(alphaVars.fFinishInTime);
	m_sliders[SLIDER_FADE_OUT_TIMING].SetValue(alphaVars.fOutStartTiming);
}

bool AlphaSettings::Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect)
{
	bool bAltered = false;
	usg::particles::ParticleAlpha& alphaVars = structData.particleAlpha;


	bAltered |= Compare(alphaVars.fInitialAlpha,m_sliders[SLIDER_INITIAL_ALPHA].GetValue());
	bAltered |= Compare(alphaVars.fIntermediateAlpha,m_sliders[SLIDER_INTERMEDIATE_ALPHA].GetValue());
	bAltered |= Compare(alphaVars.fEndAlpha,m_sliders[SLIDER_END_ALPHA].GetValue());
	bAltered |= Compare(alphaVars.fFinishInTime,m_sliders[SLIDER_FINISH_IN_TIME].GetValue());
	bAltered |= Compare(alphaVars.fOutStartTiming,m_sliders[SLIDER_FADE_OUT_TIMING].GetValue());

	if(bAltered)
	{
		// TODO: Update the emission parameters
	}

	// Nothing we do here makes it necessary to re-create the effect
	return bAltered;
}