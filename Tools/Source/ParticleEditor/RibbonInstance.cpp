#include "Engine/Common/Common.h"
#include "Engine/HID/Mouse.h"
#include "Engine/HID/Input.h"
#include "EffectGroup.h"
#include "RibbonInstance.h"
#include "ParticleEditor.h"

template <class VariableType, class ComparisonType>
inline bool Compare(VariableType& inOut, const ComparisonType newValue)
{
	if(inOut == newValue)
	{
		return false;
	}
	else
	{
		inOut = (VariableType)newValue;
		return true;
	}
}

void RibbonInstance::Cleanup(usg::GFXDevice* pDevice)
{
	m_trail.Cleanup(pDevice);
}


void RibbonInstance::Init(usg::GFXDevice* pDevice, usg::Scene& scene, EffectGroup* pParent, uint32 uIndex)
{
	usg::Vector2f vPos(0.0f, 0.0f);
	usg::Vector2f vScale(300.f, 170.f);
	usg::U8String name;
	m_pParent = pParent;
	name.ParseString("Ribbon %d", uIndex);
	m_emitterWindow.Init(name.CStr(), vPos, vScale, usg::GUIWindow::WINDOW_TYPE_CHILD);
	pParent->GetWindow().AddItem(&m_emitterWindow);
	m_removeTrailButton.Init("Remove Trail");

	float fDefault0[] = { 0.0f, 0.0f, 0.0f };
	float fDefault1[] = { 1.0f, 1.0f, 1.0f };

	m_textureSelect.Init("Emitter", pParent->GetRibbonTexFileList().GetFileNamesRaw(), 0);

	m_position.Init("Position", -20.0f, 20.0f, fDefault0, 3);
	m_position.SetToolTip("Offset from emitter");
	m_colors[COLOR_START].Init("Start Color");
	m_colors[COLOR_START].SetToolTip("Color at emission time");
	m_colors[COLOR_END].Init("End Color");
	m_colors[COLOR_END].SetToolTip("Color at fade out time");
	m_lifeTime.Init("Life Time", 0.0f, 5.0f, 0.5f);
	m_lifeTime.SetToolTip("Time from emission to fade out in seconds");
	m_scale.Init("Scale", 0.0f, 10.f, 0.15f);
	m_scale.SetToolTip("Width in m");

	m_emitterWindow.AddItem(&m_textureSelect);
	m_emitterWindow.AddItem(&m_position);
	for(uint32 i=0; i<COLOR_COUNT; i++)
		m_emitterWindow.AddItem(&m_colors[i]);
	m_emitterWindow.AddItem(&m_lifeTime);
	m_emitterWindow.AddItem(&m_scale);
	m_emitterWindow.AddItem(&m_removeTrailButton);

	const char * const p_string = "ribbon/trail";
	str::Copy(m_emitterData.textureName, p_string, sizeof(m_emitterData.textureName));
	m_emitterData.vPosition = usg::Vector3f::ZERO;
	m_emitterData.cStartColor.Assign(1.0f, 1.0f, 1.0f, 1.0f);
	m_emitterData.cEndColor.Assign(0.0f, 0.0f, 0.0f, 0.0f);
	m_emitterData.fLifeTime = 0.5f;
	m_emitterData.fLineWidth = 0.15f;

	m_trail.Alloc(pDevice, &scene.GetParticleMgr(), &m_emitterData, true);
	m_trail.SetRenderMask(usg::RenderMask::RENDER_MASK_CUSTOM << 1);

	Add(false);
}


bool RibbonInstance::Update(usg::GFXDevice* pDevice, float fElapsed)
{
	if(m_removeTrailButton.GetValue())
	{
		RemoveFromScene();
		return false;
	}

	bool bAltered = false;
	m_textureSelect.UpdateOptions(m_pParent->GetRibbonTexFileList().GetFileNamesRaw());
	bAltered |= Compare(m_emitterData.vPosition, m_position.GetValueV3());
	bAltered |= Compare(m_emitterData.cStartColor, m_colors[COLOR_START].GetValue());
	bAltered |= Compare(m_emitterData.cEndColor, m_colors[COLOR_END].GetValue());
	bAltered |= Compare(m_emitterData.fLifeTime, m_lifeTime.GetValue(0));
	bAltered |= Compare(m_emitterData.fLineWidth, m_scale.GetValue(0));

	usg::U8String emitterName = m_emitterData.textureName;
	emitterName += ".dds";
	if(emitterName != m_textureSelect.GetSelectedName())
	{
		m_pParent->GetEffect().RemoveEmitter(&m_trail);
		emitterName = m_textureSelect.GetSelectedName();
		emitterName.TruncateExtension();
		str::Copy(m_emitterData.textureName, emitterName.CStr(), sizeof(m_emitterData.textureName));
		m_pParent->Reset(pDevice);
		UpdateInstanceMatrix(pDevice);
	}


	if(bAltered)
	{
		UpdateInstanceMatrix(pDevice);
	}

	// FIXME: Update the trail details

	return true;
}

void RibbonInstance::AddToScene(usg::GFXDevice* pDevice, usg::particles::RibbonData* pInstance)
{
	if(pInstance==NULL)
	{
		usg::U8String selectName = m_textureSelect.GetSelectedName();
		selectName.TruncateExtension();
		str::Copy(m_emitterData.textureName, selectName.CStr(), sizeof(m_emitterData.textureName));
		m_emitterData.vPosition = usg::Vector3f::ZERO;
		m_emitterData.cStartColor.Assign(1.0f, 1.0f, 1.0f, 1.0f);
		m_emitterData.cEndColor.Assign(0.0f, 0.0f, 0.0f, 0.0f);
		m_emitterData.fLifeTime = 0.5f;
		m_emitterData.fLineWidth = 0.15f;
	}
	else
	{
		m_emitterData = *pInstance;
	}


	usg::U8String emitterName = m_emitterData.textureName;
	emitterName += ".dds";
	m_textureSelect.SetSelectedByName(emitterName.CStr());

	m_position.SetValue(m_emitterData.vPosition);
	m_colors[COLOR_START].SetValue(m_emitterData.cStartColor);
	m_colors[COLOR_END].SetValue(m_emitterData.cEndColor);
	m_lifeTime.SetValue(m_emitterData.fLifeTime);
	m_scale.SetValue(m_emitterData.fLineWidth);
	
	// FIXME: Add our ribbon effect
	
	m_pParent->Reset(pDevice);
	UpdateInstanceMatrix(pDevice);

	Add(true);
}


void RibbonInstance::RemoveFromScene()
{
	Add(false);
	m_pParent->GetEffect().RemoveEmitter(&m_trail);
}

void RibbonInstance::UpdateInstanceMatrix(usg::GFXDevice* pDevice)
{
	usg::Matrix4x4 mInstanceMat;
	usg::Matrix4x4 mScale;
	/*mScale.MakeScale(m_emitterData.vScale);
	mInstanceMat.LoadIdentity();
	mInstanceMat.MakeRotate(usg::Math::DegToRad(m_emitterData.vRotation.x), -usg::Math::DegToRad(m_emitterData.vRotation.y), usg::Math::DegToRad(m_emitterData.vRotation.z));
	mInstanceMat.SetTranslation(m_emitterData.vPosition);
	mInstanceMat = mInstanceMat * mScale;*/
	m_trail.SetDeclaration(pDevice, &m_emitterData);
}


