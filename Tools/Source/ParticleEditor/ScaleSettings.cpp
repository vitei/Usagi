#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "ScaleSettings.h"

ScaleSettings::ScaleSettings()
{

}


ScaleSettings::~ScaleSettings()
{

}

void ScaleSettings::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer)
{
	usg::Vector2f vWindowPos(340.0f, 600.0f);
	usg::Vector2f vWindowSize(320.f, 300.f);

	m_window.Init("Scaling", vWindowPos, vWindowSize, usg::GUIWindow::WINDOW_TYPE_COLLAPSABLE);
	m_sliders[SLIDER_RANDOMNESS].Init("Randomise", 0.0f, 1.0f, 0.0f);
	m_sliders[SLIDER_RANDOMNESS].SetToolTip("How much to randomize the particles base scale");
	m_sliders[SLIDER_INITIAL].Init("Initial Frac", 0.0f, 5.0f, 0.0f);
	m_sliders[SLIDER_INITIAL].SetToolTip("What fraction of the base size should the particle be at emission");
	m_sliders[SLIDER_INTERMEDIATE].Init("Intermediate Frac", 0.0f, 5.0f, 1.0f);
	m_sliders[SLIDER_INTERMEDIATE].SetToolTip("The standard multiplier for the particle");
	m_sliders[SLIDER_ENDING].Init("Ending Frac", 0.0f, 5.0f, 2.0f);
	m_sliders[SLIDER_ENDING].SetToolTip("The size of the particle at the end of its life");
	m_sliders[SLIDER_BEGIN_SCALE_IN].Init("End scale in", 0.0f, 1.0f, 0.0f);
	m_sliders[SLIDER_BEGIN_SCALE_IN].SetToolTip("How long to reach the intermediate scale");
	m_sliders[SLIDER_START_SCALE_OUT].Init("Start scale out", 0.0f, 1.0f, 0.0f);
	m_sliders[SLIDER_START_SCALE_OUT].SetToolTip("How long into the particles life (as a frac) to start scaling to it's final size");

	m_baseScale.Init(&m_window, "Base scale value");
	m_baseScale.SetToolTip("The base size of particles that will be emitted at a given time during emission");

	for(uint32 i=0; i<SLIDER_COUNT; i++)
	{
		m_window.AddItem(&m_sliders[i]);
	}
	//pRenderer->AddWindow(&m_window);
}

void ScaleSettings::SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData)
{
	usg::particles::ParticleScale& scaleVars = structData.particleScale;
	m_sliders[SLIDER_RANDOMNESS].SetValue(scaleVars.fRandomness);
	m_sliders[SLIDER_INITIAL].SetValue(scaleVars.fInitial);
	m_sliders[SLIDER_INTERMEDIATE].SetValue(scaleVars.fIntermediate);
	m_sliders[SLIDER_ENDING].SetValue(scaleVars.fEnding);
	m_sliders[SLIDER_BEGIN_SCALE_IN].SetValue(scaleVars.fBeginScaleIn);
	m_sliders[SLIDER_START_SCALE_OUT].SetValue(scaleVars.fStartScaleOut);
	m_baseScale.SetFromDefinition(scaleVars.standardValue);
}

bool ScaleSettings::Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect, float fElapsed)
{
	bool bAltered = false;
	usg::particles::ParticleScale& scaleVars = structData.particleScale;

	bool bOneFrame = structData.emission.eEmissionType == usg::particles::EMISSION_TYPE_ONE_SHOT;
	m_baseScale.SetSingleOnly(bOneFrame);

	bAltered |= Compare(scaleVars.fRandomness,m_sliders[SLIDER_RANDOMNESS].GetValue());
	bAltered |= Compare(scaleVars.fInitial,m_sliders[SLIDER_INITIAL].GetValue());
	bAltered |= Compare(scaleVars.fIntermediate,m_sliders[SLIDER_INTERMEDIATE].GetValue());
	bAltered |= Compare(scaleVars.fEnding,m_sliders[SLIDER_ENDING].GetValue());
	bAltered |= Compare(scaleVars.fBeginScaleIn,m_sliders[SLIDER_BEGIN_SCALE_IN].GetValue());
	bAltered |= Compare(scaleVars.fStartScaleOut,m_sliders[SLIDER_START_SCALE_OUT].GetValue());
	bAltered |= m_baseScale.Update(scaleVars.standardValue);

	if(bAltered)
	{
		// TODO: Update the emission parameters
	}

	// Nothing we do here makes it necessary to re-create the effect
	return bAltered;
}