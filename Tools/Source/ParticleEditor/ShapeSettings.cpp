#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "ShapeSettings.h"

static const char* g_szShapeType[] =
{
	"Point",
	"Sphere",
	"Cylinder",
	"Cube",
	NULL
};


ShapeSettings::ShapeSettings()
{

}


ShapeSettings::~ShapeSettings()
{

}

void ShapeSettings::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer)
{
	usg::Vector2f vWindowPos(800.0f, 600.0f);
	usg::Vector2f vWindowSize(320.f, 240.f);
	m_window.Init("Shape settings", vWindowPos, vWindowSize, usg::GUIWindow::WINDOW_TYPE_COLLAPSABLE);

	float fDefaultZero[] = { 0.0f, 0.0f, 0.0f};
	float fDefaultOne[] = {1.0f, 1.0f, 1.0f};
	m_shapeType.Init("Type", g_szShapeType, 0);
	m_scale.Init("Scale", 0.0f, 5.0f, fDefaultOne, 3);
	m_rotation.Init("Rotation", 0.0f, 360.0f, fDefaultZero, 3);
	m_position.Init("Position", 0.0f, 5.0f, fDefaultZero, 3);
	m_identityMatrix.Init("Identity matrix", true);

	m_velocity.Init("Velocity", -200.f, 200.f, fDefaultZero, 3 );
	m_velocityRandom.Init("Velocity rand Frac", 0.0f, 1.0f, 0.0f );
	m_gravityDir.Init("Gravity", -20.f, 50.f, fDefaultZero, 3 );

	m_shapeSpread.Init("Particle Escape Vel", -10.0f, 10.0f, 0.0f);


	m_hollowness.Init("Hollowness", 0.0f, 1.0f, 0.0f);

	vWindowSize.Assign(300.f, 100.f);
	m_arcWindow.Init("Arc", vWindowPos, vWindowSize, usg::GUIWindow::WINDOW_TYPE_CHILD);

	m_arcTitle.Init("Arc");
	m_arcWidthDeg.Init("Width", 0.0f, 360.f, 360.f);
	m_arcStartDeg.Init("Start ang", 0.0f, 360.f, 0.0f);
	m_randomizeStart.Init("Randomize start", false);

	float fDefault[] = {1.0f, 1.0f, 1.0f};
	m_radius.Init("Radius", 0.0f, 20.0f, fDefault, 3);
	m_sideLength.Init("Side Length", 0.0f, 20.0f, fDefault, 3);

	m_arcWindow.AddItem(&m_arcTitle);
	m_arcWindow.AddItem(&m_arcWidthDeg);
	m_arcWindow.AddItem(&m_arcStartDeg);
	m_arcWindow.AddItem(&m_randomizeStart);


	m_window.AddItem(&m_shapeType);
	m_window.AddItem(&m_radius);
	m_window.AddItem(&m_sideLength);
	m_window.AddItem(&m_scale);
	m_window.AddItem(&m_rotation);
	m_window.AddItem(&m_position);
	m_window.AddItem(&m_identityMatrix);
	m_window.AddItem(&m_velocity);
	m_window.AddItem(&m_velocityRandom);
	m_window.AddItem(&m_gravityDir);
	m_window.AddItem(&m_hollowness);
	m_window.AddItem(&m_shapeSpread);

	m_window.AddItem(&m_arcWindow);

	//pRenderer->AddWindow(&m_window);
}

void ShapeSettings::SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData)
{
	m_shapeType.SetSelected(structData.eShape);
}

void ShapeSettings::SetShapeSettings(const usg::particles::EmitterShapeDetails& details)
{
	m_shapeDetails = details;
	m_position.SetValue(m_shapeDetails.baseShape.vPosition);
	m_rotation.SetValue(m_shapeDetails.baseShape.vRotation);
	m_scale.SetValue(m_shapeDetails.baseShape.vScale);
	m_hollowness.SetValue(1.0f-m_shapeDetails.baseShape.fHollowness);
	m_randomizeStart.SetValue(m_shapeDetails.arc.bRandomizeStartAngle);
	m_arcStartDeg.SetValue(m_shapeDetails.arc.fArcStartDeg);
	m_arcWidthDeg.SetValue(m_shapeDetails.arc.fArcWidthDeg);
	m_radius.SetValue(m_shapeDetails.vShapeExtents);
	m_sideLength.SetValue(m_shapeDetails.vShapeExtents);
	m_shapeSpread.SetValue(m_shapeDetails.baseShape.fShapeExpandVel);
	m_velocityRandom.SetValue(m_shapeDetails.baseShape.fSpeedRand);
	m_gravityDir.SetValue(m_shapeDetails.baseShape.vGravity);
	m_velocity.SetValue(m_shapeDetails.baseShape.vVelocity);

	m_identityMatrix.SetValue( m_shapeDetails.baseShape.vPosition == usg::Vector3f::ZERO && m_shapeDetails.baseShape.vRotation == usg::Vector3f::ZERO && m_shapeDetails.baseShape.vScale == usg::Vector3f::ONE);
}

bool ShapeSettings::Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect)
{
	bool bAltered = false;

	bAltered |= Compare(structData.eShape,m_shapeType.GetSelected());
	if(m_identityMatrix.GetValue())
	{
		m_position.SetValue(usg::Vector3f::ZERO);
		m_rotation.SetValue(usg::Vector3f::ZERO);
		m_scale.SetValue(usg::Vector3f::ONE);
	}
	m_position.SetVisible(!m_identityMatrix.GetValue());
	m_rotation.SetVisible(!m_identityMatrix.GetValue());
	m_scale.SetVisible(!m_identityMatrix.GetValue());

	bAltered |= Compare(m_shapeDetails.baseShape.fShapeExpandVel, m_shapeSpread.GetValue());
	bAltered |= Compare(m_shapeDetails.baseShape.vPosition, m_position.GetValueV3());
	bAltered |= Compare(m_shapeDetails.baseShape.vRotation, m_rotation.GetValueV3());
	bAltered |= Compare(m_shapeDetails.baseShape.vScale, m_scale.GetValueV3());
	
	bAltered |= Compare(m_shapeDetails.baseShape.vGravity, m_gravityDir.GetValueV3());
	bAltered |= Compare(m_shapeDetails.baseShape.vVelocity, m_velocity.GetValueV3());
	bAltered |= Compare(m_shapeDetails.baseShape.fSpeedRand, m_velocityRandom.GetValue(0));

	//if(bAltered)
	{
		switch(structData.eShape)
		{
		case usg::particles::EMITTER_SHAPE_SPHERE:
		case usg::particles::EMITTER_SHAPE_CYLINDER:
		case usg::particles::EMITTER_SHAPE_CUBE:
			m_hollowness.SetVisible(true);
			bAltered |= Compare(m_shapeDetails.baseShape.fHollowness, m_hollowness.GetValue(0));
			break;
		default:
			m_hollowness.SetVisible(false);
		}

		switch(structData.eShape)
		{
		case usg::particles::EMITTER_SHAPE_SPHERE:
		case usg::particles::EMITTER_SHAPE_CYLINDER:
			m_arcWindow.SetVisible(true);
			bAltered |= Compare(m_shapeDetails.arc.bRandomizeStartAngle, m_randomizeStart.GetValue());
			bAltered |= Compare(m_shapeDetails.arc.fArcStartDeg, m_arcStartDeg.GetValue());;
			bAltered |= Compare(m_shapeDetails.arc.fArcWidthDeg, m_arcWidthDeg.GetValue());
			m_radius.SetVisible(true);
			bAltered |= Compare(m_shapeDetails.vShapeExtents, m_radius.GetValueV3());
			break;
		default:
			m_arcWindow.SetVisible(false);
			m_radius.SetVisible(false);
		}

		bool bShowCube = (structData.eShape == usg::particles::EMITTER_SHAPE_CUBE);
		m_sideLength.SetVisible(bShowCube);


		if(bShowCube)
		{
			bAltered |= Compare(m_shapeDetails.vShapeExtents, m_sideLength.GetValueV3());
		}
	}

	if(bAltered)
	{
		pEffect->CreateEmitterShape(structData.eShape, m_shapeDetails);
	}

	// Nothing we do here makes it necessary to re-create the effect
	return bAltered;
}