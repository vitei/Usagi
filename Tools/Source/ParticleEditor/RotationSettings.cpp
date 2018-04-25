#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "RotationSettings.h"

RotationSettings::RotationSettings()
{

}


RotationSettings::~RotationSettings()
{

}

void RotationSettings::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer)
{
	usg::Vector2f vWindowPos(340.0f, 0.0f);
	usg::Vector2f vWindowSize(320.f, 120.f);
	m_window.Init("Rotation", vWindowPos, vWindowSize, 1.0f, usg::GUIWindow::WINDOW_TYPE_COLLAPSABLE);
	m_sliders[SLIDER_BASE_ROTATION].Init("Base value", 0.0f, 360.0f, 0.0f);
	m_sliders[SLIDER_RANDOMISE].Init("Randomise", 0.0f, 360.0f, 360.0f);
	m_sliders[SLIDER_SPEED].Init("Speed", -720.0f, 720.0f, 0.0f);
	m_sliders[SLIDER_SPEED_RANDOMISE].Init("Speed randomise", 0.0f, 720.0f, 0.0f);

	for(uint32 i=0; i<SLIDER_COUNT; i++)
	{
		m_window.AddItem(&m_sliders[i]);
	}
	//pRenderer->AddWindow(&m_window);
}

void RotationSettings::SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData)
{
	usg::particles::ParticleRotation& rotationVars = structData.particleRotation;
	m_sliders[SLIDER_BASE_ROTATION].SetValue(rotationVars.fBaseRotation);
	m_sliders[SLIDER_RANDOMISE].SetValue(rotationVars.fRandomise);
	m_sliders[SLIDER_SPEED].SetValue(rotationVars.fSpeed);
	m_sliders[SLIDER_SPEED_RANDOMISE].SetValue(rotationVars.fSpeedRandomise);
}

bool RotationSettings::Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect)
{
	bool bAltered = false;
	usg::particles::ParticleRotation& rotationVars = structData.particleRotation;


	bool bShowRotation = structData.eParticleType == usg::particles::PARTICLE_TYPE_BILLBOARD;
	
	if(bShowRotation)
	{
		bAltered |= Compare(rotationVars.fBaseRotation,m_sliders[SLIDER_BASE_ROTATION].GetValue());
		bAltered |= Compare(rotationVars.fRandomise,m_sliders[SLIDER_RANDOMISE].GetValue());
		bAltered |= Compare(rotationVars.fSpeed,m_sliders[SLIDER_SPEED].GetValue());
		bAltered |= Compare(rotationVars.fSpeedRandomise,m_sliders[SLIDER_SPEED_RANDOMISE].GetValue());
	}

	m_window.SetVisible(bShowRotation);

	if(bAltered)
	{
		// TODO: Update the emission parameters
	}

	// Nothing we do here makes it necessary to re-create the effect
	return bAltered;
}