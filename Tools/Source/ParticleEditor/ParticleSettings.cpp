#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "ParticleSettings.h"


static const char* g_szParticleType[] =
{
	"Billboard",
	"Velocity facing",
	"Trail",
	"User direction",
	"Y Axis Align",
	NULL
};

ParticleSettings::ParticleSettings()
{

}


ParticleSettings::~ParticleSettings()
{

}

void ParticleSettings::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer)
{
	usg::Vector2f vWindowPos(800.0f, 300.0f);
	usg::Vector2f vWindowSize(320.f, 240.f);
	
	m_window.Init("Particle init", vWindowPos, vWindowSize, usg::GUIWindow::WINDOW_TYPE_COLLAPSABLE);
	m_life.Init(&m_window, "Lifetime");
	m_life.SetToolTip("Lifetime particles are spawned with over the duration of the effect");

	m_particleType.Init("Type", g_szParticleType, 0);
	m_particleType.SetToolTip("The type of particle this emitter spawns");
	m_lifeRandomness.Init("Life randomness", 0.0f, 1.0f, 0.0f);
	m_lifeRandomness.SetToolTip("A random value (+ or -) of this scale will be added to each particles lifetime");
	float fDefaultVector[] = { 0.0f, 0.0f, 0.0f };
	m_userRotation.Init("User rotation", -360.0f, 360.0f, fDefaultVector, 3);
	m_userRotation.SetToolTip("Custom orientation for user oriented particles");


	float fDefault[] = {0.0f, 0.0f };
	m_particleCenter.Init("Pivot", -0.5f, 0.5f, fDefault, 2);
	m_particleCenter.SetToolTip("The center point of a billboard for both rotation and placement");

	m_window.AddItem(&m_lifeRandomness);
	m_window.AddItem(&m_particleType);
	m_window.AddItem(&m_particleCenter);
	m_window.AddItem(&m_userRotation);
	
	//pRenderer->AddWindow(&m_window);
}

void ParticleSettings::SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData)
{
	m_life.SetFromDefinition(structData.life);
	m_particleType.SetSelected(structData.eParticleType);
	m_lifeRandomness.SetValue(structData.fLifeRandomness, 0);
	m_particleCenter.SetValue(structData.vParticleCenter.x, 0);
	m_particleCenter.SetValue(structData.vParticleCenter.y, 1);
	m_userRotation.SetValue(structData.emission.vUserRotation);
}

bool ParticleSettings::Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect, float fElapsed)
{
	bool bOneFrame = structData.emission.eEmissionType == usg::particles::EMISSION_TYPE_ONE_SHOT;
	m_life.SetSingleOnly(bOneFrame);
	bool bAltered = false;
	bAltered = m_life.Update(structData.life);

	if (m_particleType.GetSelected() == ::usg::particles::ParticleType::PARTICLE_TYPE_TRAIL)
	{
		m_particleType.SetSelected(::usg::particles::ParticleType::PARTICLE_TYPE_BILLBOARD);
	}

	bAltered |= Compare(structData.eParticleType, m_particleType.GetSelected());
	bAltered |= Compare(structData.fLifeRandomness,m_lifeRandomness.GetValue(0));
	bAltered |= Compare(structData.vParticleCenter,m_particleCenter.GetValueV2());
	bAltered |= Compare(structData.emission.vUserRotation, m_userRotation.GetValueV3());

	if(bAltered)
	{
		// TODO: Update the emission parameters
		m_userRotation.SetVisible(structData.eParticleType == usg::particles::PARTICLE_TYPE_USER_ORIENTED);
	}

	// Nothing we do here makes it necessary to re-create the effect
	return bAltered;
}