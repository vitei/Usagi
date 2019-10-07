#include "Engine/Common/Common.h"
#include "Engine/GUI/IMGuiRenderer.h"
#include "Engine/HID/Input.h"
#include "Engine/HID/Mouse.h"
#include "ColorSettings.h"

static const char* g_szColorAnimMode[] =
{
	"Constant Color",
	"Random Color",
	"Animation",
	NULL
};

ColorSettings::ColorSettings()
{

}


ColorSettings::~ColorSettings()
{

}

void ColorSettings::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer)
{
	usg::Vector2f vWindowPos(340.0f, 130.0f);
	usg::Vector2f vWindowSize(320.f, 240.f);

	m_window.Init("Color Settings", vWindowPos, vWindowSize, 1.0f, usg::GUIWindow::WINDOW_TYPE_COLLAPSABLE);
	m_colorAnimMode.Init("Color mode", g_szColorAnimMode, 0);
	m_sliders[SLIDER_TIME_IN_END].Init("Color 1 hold end", 0.0f, 1.0f, 0.0f);
	m_sliders[SLIDER_PEAK_TIME].Init("Color 1 > 2 end", 0.0f, 1.0f, 0.3f);
	m_sliders[SLIDER_TIME_OUT_START].Init("Color 2 > 3 start", 0.0f, 1.0f, 0.7f);
	m_sliders[SLIDER_ENV_COLOR_LERP].Init("Env color lerp", 0.0f, 1.0f, 0.0f);
	int repeat = 1;
	m_repetition.Init("Anim repeat", &repeat, 1, 0, 100);

	usg::Color cWhite(1.0f, 1.0f, 1.0f, 1.0f);
	m_colors[0].Init("Color 1");
	m_colors[0].SetValue(cWhite);
	m_colors[1].Init("Color 2");
	m_colors[1].SetValue(cWhite);
	m_colors[2].Init("Color 3");
	m_colors[2].SetValue(cWhite);

	m_window.AddItem(&m_colorAnimMode);
	for(uint32 i=0; i<COLOR_COUNT; i++)
	{
		m_window.AddItem(&m_colors[i]);
	}

	for(uint32 i=0; i<SLIDER_COUNT; i++)
	{
		m_window.AddItem(&m_sliders[i]);
	}
	m_window.AddItem(&m_repetition);
	//pRenderer->AddWindow(&m_window);
}

void ColorSettings::SetWidgetsFromDefinition(usg::particles::EmitterEmission& structData)
{
	usg::particles::ParticleColor& colorVars = structData.particleColor;

	m_sliders[SLIDER_TIME_IN_END].SetValue(colorVars.fInTimeEnd);
	m_sliders[SLIDER_PEAK_TIME].SetValue(colorVars.fPeak);
	m_sliders[SLIDER_TIME_OUT_START].SetValue(colorVars.fOutTimeStart);
	m_colors[0].SetValue(colorVars.cColor0);
	m_colors[1].SetValue(colorVars.cColor1);
	m_colors[2].SetValue(colorVars.cColor2);
	m_randomRepetition.SetValue(colorVars.bRandomRepetitionPos);
	m_colorAnimMode.SetSelected(colorVars.eColorMode);
	m_sliders[SLIDER_ENV_COLOR_LERP].SetValue(colorVars.fLerpEnvColor);

	int repetition = (int)colorVars.uRepetitionCount;
	m_repetition.SetValues(&repetition);
}

bool ColorSettings::Update(usg::GFXDevice* pDevice, usg::particles::EmitterEmission& structData, usg::ScriptEmitter* pEffect)
{
	bool bAltered = false;
	usg::particles::ParticleColor& colorVars = structData.particleColor;

	bAltered |= Compare(colorVars.fInTimeEnd,m_sliders[SLIDER_TIME_IN_END].GetValue());
	bAltered |= Compare(colorVars.fPeak,m_sliders[SLIDER_PEAK_TIME].GetValue());
	bAltered |= Compare(colorVars.fOutTimeStart,m_sliders[SLIDER_TIME_OUT_START].GetValue());
	bAltered |= Compare(colorVars.fLerpEnvColor, m_sliders[SLIDER_ENV_COLOR_LERP].GetValue());
	bAltered |= Compare(colorVars.cColor0, m_colors[0].GetValue());
	bAltered |= Compare(colorVars.cColor1, m_colors[1].GetValue());
	bAltered |= Compare(colorVars.cColor2, m_colors[2].GetValue());
	bAltered |= Compare(colorVars.bRandomRepetitionPos, m_randomRepetition.GetValue());
	bAltered |= Compare(colorVars.eColorMode, m_colorAnimMode.GetSelected());
	bAltered |= Compare(colorVars.uRepetitionCount, m_repetition.GetValue(0));

	bool bShowColorAnim = false;
	bool bShowColor23 = false;

	switch(colorVars.eColorMode)
	{
	case usg::particles::PARTICLE_COLOR_ANIMATION:
		bShowColorAnim = true;
		bShowColor23 = true;
		break;
	case usg::particles::PARTICLE_COLOR_RANDOM:
		bShowColor23 = true;
		break;
	default:
		break;
	}


	m_sliders[SLIDER_TIME_IN_END].SetVisible(bShowColorAnim);
	m_sliders[SLIDER_PEAK_TIME].SetVisible(bShowColorAnim);
	m_sliders[SLIDER_TIME_OUT_START].SetVisible(bShowColorAnim);
	m_repetition.SetVisible(bShowColorAnim);
	m_colors[1].SetVisible(bShowColor23);
	m_colors[2].SetVisible(bShowColor23);

	if(bAltered)
	{
		// TODO: Update the emission parameters
	}

	// Nothing we do here makes it necessary to re-create the effect
	return bAltered;
}