#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "EmissionSettings.h"

static const char* g_szTypeStrings[] =
{
	"Timed",
	"Infinite",
	"One shot",
	NULL
};

EmissionSettings::EmissionSettings()
{

}


EmissionSettings::~EmissionSettings()
{

}

void EmissionSettings::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer)
{
	int maxParticles = 20;
	usg::Vector2f vWindowPos(0.0f, 300.0f);
	usg::Vector2f vWindowSize(320.f, 350.f);
	m_window.Init("Emission", vWindowPos, vWindowSize, usg::GUIWindow::WINDOW_TYPE_COLLAPSABLE);
	//m_oneTimeCheckBox.Init("One shot", false);
	m_emissionType.Init("Mode", g_szTypeStrings, 0);
	m_emissionType.SetToolTip("Release timining (set period, infinite, all at once)");
	m_maxParticles.Init("Max particles", &maxParticles, 1, 0, 2000 );
	m_maxParticles.SetToolTip("The maximum number of particles to be active at one time");
	m_sliders[SLIDER_RELEASE_INTERVAL].Init("Interval", 0.0f, 2.0f, 0.0f);
	m_sliders[SLIDER_RELEASE_INTERVAL].SetToolTip("Time between releasing groups particles");
	m_sliders[SLIDER_RELEASE_INTERVAL_RANDOM].Init("Interval random", 0.0f, 2.0f, 0.0f);
	m_sliders[SLIDER_RELEASE_INTERVAL_RANDOM].SetToolTip("Maximum randomness (in seconds) of release");
	m_releaseRandom.Init("Release random", 0.0f, 1.0f, 0.0f);
	m_releaseRandom.SetToolTip("Chance each particle will be released");
	m_sliders[SLIDER_EMISSION_TIME].Init("Effect duration", 0.0f, 60.0f, 1.0f);
	m_sliders[SLIDER_EMISSION_TIME].SetToolTip("Time until emission stops");


	m_window.AddItem(&m_emissionType);
	m_window.AddItem(&m_maxParticles);
	for(uint32 i=0; i<SLIDER_COUNT; i++)
	{
		m_window.AddItem(&m_sliders[i]);
	}
	m_window.AddItem(&m_releaseRandom);

	m_emissionTiming.Init(&m_window, "Emission Rate");

	//pRenderer->AddWindow(&m_window);
}

void EmissionSettings::SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData)
{
	usg::particles::EmissionVariables& emissionVars = structData.emission;

	m_sliders[SLIDER_RELEASE_INTERVAL].SetValue(emissionVars.fReleaseInterval);
	m_sliders[SLIDER_RELEASE_INTERVAL_RANDOM].SetValue(emissionVars.fReleaseIntervalRandom);
	if(emissionVars.has_fReleaseRandom)
	{
		m_releaseRandom.SetValue(emissionVars.fReleaseRandom);
	}
	else
	{
		m_releaseRandom.SetValue(1.0f);
		emissionVars.fReleaseRandom = 1.0f;
		emissionVars.has_fReleaseRandom = true;
	}
	m_sliders[SLIDER_EMISSION_TIME].SetValue(emissionVars.fEmissionTime);
	m_emissionTiming.SetFromDefinition(emissionVars.emissionRate);
	m_emissionType.SetSelected(emissionVars.eEmissionType);
	int maxParticles = (int)emissionVars.uMaxParticles;
	m_maxParticles.SetValues(&maxParticles);
}

bool EmissionSettings::Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect, float fElapsed)
{
	bool bAltered = false;
	usg::particles::EmissionVariables& emissionVars = structData.emission;

	m_emissionTiming.SetVisible(emissionVars.eEmissionType != usg::particles::EMISSION_TYPE_ONE_SHOT);
	bAltered |= m_emissionTiming.Update(emissionVars.emissionRate);

	bAltered |= Compare(emissionVars.fReleaseInterval,m_sliders[SLIDER_RELEASE_INTERVAL].GetValue());
	bAltered |= Compare(emissionVars.fReleaseIntervalRandom,m_sliders[SLIDER_RELEASE_INTERVAL_RANDOM].GetValue());
	bAltered |= Compare(emissionVars.fReleaseRandom, m_releaseRandom.GetValue());

	bAltered |= Compare(emissionVars.fEmissionTime,m_sliders[SLIDER_EMISSION_TIME].GetValue());
	bAltered |= Compare(emissionVars.uMaxParticles, m_maxParticles.GetValue()[0]);
	bAltered |= Compare(emissionVars.eEmissionType, m_emissionType.GetSelected());


	m_sliders[SLIDER_EMISSION_TIME].SetVisible( emissionVars.eEmissionType == usg::particles::EMISSION_TYPE_TIMED );

	if(bAltered)
	{
		// TODO: Update the emission parameters
	}

	// Nothing we do here makes it necessary to re-create the effect
	return bAltered;
}