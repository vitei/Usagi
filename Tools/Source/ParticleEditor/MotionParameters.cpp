#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "MotionParameters.h"


MotionParameters::MotionParameters()
{

}


MotionParameters::~MotionParameters()
{

}

void MotionParameters::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer)
{
	usg::Vector2f vWindowPos(0.0f, 500.0f);
	usg::Vector2f vWindowSize(320.f, 340.f);
	float defaultVec[] = { 0.0f, 1.0f, 0.0f };
	m_window.Init("Motion params", vWindowPos, vWindowSize, usg::GUIWindow::WINDOW_TYPE_COLLAPSABLE);
	m_sliders[SLIDER_POSITION_RANDOM].Init("Position random", 0.0f, 2.0f, 0.0f);
	m_sliders[SLIDER_POSITION_RANDOM].SetToolTip("A random offset in any direction to apply to the particle spawn location");
	m_window.AddItem(&m_sliders[SLIDER_POSITION_RANDOM]);
	m_anims[FLOAT_ANIM_OMNI_VELOCITY].Init(&m_window, "Omni speed");
	m_anims[FLOAT_ANIM_OMNI_VELOCITY].SetToolTip("Random velocity scale to apply spherically");
	m_sliders[SLIDER_VELOCITY_DIR].Init("Velocity Dir", -1.0f, 1.0f, defaultVec, 3);
	m_sliders[SLIDER_VELOCITY_DIR].SetToolTip("The base vector to used for any directional velocity");
	m_window.AddItem(&m_sliders[SLIDER_VELOCITY_DIR]);
	m_sliders[SLIDER_DIR_VELOCITY_CONE_DEG].Init("Dir Vel Cone Deg", 0.0f, 180.0f, 0.0f);
	m_sliders[SLIDER_DIR_VELOCITY_CONE_DEG].SetToolTip("The size (in degrees) of the cone formed around that directional velocity");
	m_window.AddItem(&m_sliders[SLIDER_DIR_VELOCITY_CONE_DEG]);
	m_anims[FLOAT_ANIM_DIR_VELOCITY].Init(&m_window, "Dir speed");
	m_anims[FLOAT_ANIM_DIR_VELOCITY].SetToolTip("The directional speed particles are initialized with on spawn");
	m_sliders[SLIDER_SPEED_RANDOMNESS].Init("Speed randomness", 0.0f, 1.0f, 0.0f);
	m_sliders[SLIDER_SPEED_RANDOMNESS].SetToolTip("Random multiplier to apply to the total speed (+ or - frac)");
	m_window.AddItem(&m_sliders[SLIDER_SPEED_RANDOMNESS]);

	defaultVec[1] = -1.0f;
	m_sliders[SLIDER_GRAVITY_DIR].Init("Gravity Dir", -1.0f, 1.0f, defaultVec, 3);
	m_sliders[SLIDER_GRAVITY_DIR].SetToolTip("Direction of gravity");
	m_window.AddItem(&m_sliders[SLIDER_GRAVITY_DIR]);
	m_floatEntries[FLOAT_ENTRY_GRAVITY_STRENGTH].Init("Gravity strength", 0.0f, 20.0f, 0.0f);
	m_floatEntries[FLOAT_ENTRY_GRAVITY_STRENGTH].SetToolTip("Strength of gravity");
	m_window.AddItem(&m_floatEntries[FLOAT_ENTRY_GRAVITY_STRENGTH]);
	m_sliders[SLIDER_DRAG].Init("Drag", 0.0f, 1.0f, 0.0f);
	m_sliders[SLIDER_DRAG].SetToolTip("Drag (air resistance) applied to velocity");

	m_window.AddItem(&m_sliders[SLIDER_DRAG]);

	m_checkBoxes[CHECKBOX_INHERIT_VELOCITY].Init("Inherit velocity", false);
	m_checkBoxes[CHECKBOX_INHERIT_VELOCITY].SetToolTip("Inherit velocity from the parent effect or move independently");

	m_window.AddItem(&m_checkBoxes[CHECKBOX_INHERIT_VELOCITY]);
	m_checkBoxes[CHECKBOX_LOCAL_SPACE].Init("Local-space", false);
	m_checkBoxes[CHECKBOX_LOCAL_SPACE].SetToolTip("If true the emitter will move with the effect (and object it is attached to)");
	m_window.AddItem(&m_checkBoxes[CHECKBOX_LOCAL_SPACE]);
	m_checkBoxes[CHECKBOX_UPDATE_ON_CPU].Init("CPU update", false);
	m_checkBoxes[CHECKBOX_UPDATE_ON_CPU].SetToolTip("Update on CPU (better quality motion, particularly for drag)");
	m_window.AddItem(&m_checkBoxes[CHECKBOX_UPDATE_ON_CPU]);
	
	//pRenderer->AddWindow(&m_window);
}

void MotionParameters::SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData)
{
	m_sliders[SLIDER_POSITION_RANDOM].SetValue(structData.fPositionRandomness, 0);
	m_sliders[SLIDER_VELOCITY_DIR].SetValue(structData.vVelocityDir);
	m_sliders[SLIDER_DIR_VELOCITY_CONE_DEG].SetValue(structData.fVelocityDirConeDeg);
	m_sliders[SLIDER_SPEED_RANDOMNESS].SetValue(structData.fSpeedRandomness);
	m_sliders[SLIDER_GRAVITY_DIR].SetValue(structData.vGravityDir);
	m_floatEntries[FLOAT_ENTRY_GRAVITY_STRENGTH].SetValue(structData.fGravityStrength);
	m_sliders[SLIDER_DRAG].SetValue(structData.fDrag);

	m_anims[FLOAT_ANIM_OMNI_VELOCITY].SetFromDefinition(structData.omniVelocity);
	m_anims[FLOAT_ANIM_DIR_VELOCITY].SetFromDefinition(structData.dirVelocity);

	m_checkBoxes[CHECKBOX_UPDATE_ON_CPU].SetValue(structData.bCPUPositionUpdate);
	m_checkBoxes[CHECKBOX_INHERIT_VELOCITY].SetValue(structData.bInheritVelocity);
	m_checkBoxes[CHECKBOX_LOCAL_SPACE].SetValue(structData.has_bLocalEffect ? structData.bLocalEffect : false);
}

bool MotionParameters::Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect, float fElapsed)
{
	bool bAltered = false;

	bool bOneFrame = structData.emission.eEmissionType == usg::particles::EMISSION_TYPE_ONE_SHOT;
	for (int i = 0; i < FLOAT_ANIM_COUNT; i++)
	{
		m_anims[i].SetSingleOnly(bOneFrame);
	}

	bAltered |= Compare(structData.fPositionRandomness, m_sliders[SLIDER_POSITION_RANDOM].GetValue());
	bAltered |= CompareUnitVector(structData.vVelocityDir, m_sliders[SLIDER_VELOCITY_DIR].GetValueV3(), usg::Vector3f::Y_AXIS);
	m_sliders[SLIDER_VELOCITY_DIR].SetValue(structData.vVelocityDir);
	bAltered |= Compare(structData.fVelocityDirConeDeg, m_sliders[SLIDER_DIR_VELOCITY_CONE_DEG].GetValue());
	bAltered |= Compare(structData.fSpeedRandomness, m_sliders[SLIDER_SPEED_RANDOMNESS].GetValue());
	bAltered |= CompareUnitVector(structData.vGravityDir, m_sliders[SLIDER_GRAVITY_DIR].GetValueV3(), -usg::Vector3f::Y_AXIS);
	m_sliders[SLIDER_GRAVITY_DIR].SetValue(structData.vGravityDir);
	bAltered |= Compare(structData.fGravityStrength, m_floatEntries[FLOAT_ENTRY_GRAVITY_STRENGTH].GetValue());
	bAltered |= Compare(structData.fDrag, m_sliders[SLIDER_DRAG].GetValue());
	bAltered |= Compare(structData.fDrag, m_sliders[SLIDER_DRAG].GetValue());

	bAltered |= m_anims[FLOAT_ANIM_OMNI_VELOCITY].Update(structData.omniVelocity);
	bAltered |= m_anims[FLOAT_ANIM_DIR_VELOCITY].Update(structData.dirVelocity);

	bAltered |= Compare(structData.bCPUPositionUpdate, m_checkBoxes[CHECKBOX_UPDATE_ON_CPU].GetValue());
	bAltered |= Compare(structData.bInheritVelocity, m_checkBoxes[CHECKBOX_INHERIT_VELOCITY].GetValue());
	if( Compare(structData.bLocalEffect, m_checkBoxes[CHECKBOX_LOCAL_SPACE].GetValue()) )
	{
		structData.has_bLocalEffect = true;
		bAltered = true;
	}
	

	if(bAltered)
	{
		// TODO: Update the emission parameters
	}

	// Nothing we do here makes it necessary to re-create the effect
	return bAltered;
}
