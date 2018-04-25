#ifndef _USG_PARTICLE_EDITOR_SHAPE_SETTINGS_H_
#define _USG_PARTICLE_EDITOR_SHAPE_SETTINGS_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Graphics/Device/StateEnums.pb.h"
#include "Engine/GUI/GuiWindow.h"
#include "Engine/GUI/GuiItems.h"
#include "EmitterModifier.h"
#include "FloatAnim.h"

namespace usg
{
	class IMGuiRenderer;
}

class ShapeSettings : public EmitterModifier
{
public:
	ShapeSettings();
	~ShapeSettings();

	virtual void Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer);
	virtual void SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData);
	virtual bool Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect);

	void SetShapeSettings(const usg::particles::EmitterShapeDetails& details);
	usg::particles::EmitterShapeDetails* GetShapeDetails() { return &m_shapeDetails; }
private:
	usg::GUIComboBox		m_shapeType;
	usg::GUISlider			m_scale;
	usg::GUISlider			m_rotation;
	usg::GUIFloat			m_position;
	usg::GUIFloat			m_radius;
	usg::GUISlider			m_sideLength;

	usg::GUISlider			m_hollowness;
	usg::GUIWindow			m_arcWindow;
	usg::GUIText			m_arcTitle;
	usg::GUISlider			m_arcWidthDeg;
	usg::GUISlider			m_arcStartDeg;
	usg::GUICheckBox		m_randomizeStart;
	usg::GUICheckBox		m_identityMatrix;
	
	// Emitter motion
	usg::GUIFloat			m_velocity;
	usg::GUIFloat			m_shapeSpread;
	usg::GUISlider			m_velocityRandom;
	usg::GUIFloat			m_gravityDir;

	usg::particles::EmitterShapeDetails	m_shapeDetails;
};


#endif