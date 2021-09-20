#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/Display.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Scene/ViewContext.h"
#include "EffectGroup.h"
#include "EmitterInstance.h"
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

void EmitterInstance::Init(usg::GFXDevice* pDevice, usg::Scene& scene, EffectGroup* pParent, uint32 uIndex)
{
	usg::Vector2f vPos(0.0f, 0.0f);
	usg::Vector2f vScale(300.f, 190.f);
	usg::U8String name;
	m_pParent = pParent;
	name.ParseString("Effect %d", uIndex);
	m_emitterWindow.Init(name.CStr(), vPos, vScale, usg::GUIWindow::WINDOW_TYPE_CHILD);
	pParent->GetWindow().AddItem(&m_emitterWindow);
	m_removeEmitterButton.Init("Remove Emitter");
	m_loadEmitterButton.Init("Load");
	m_loadEmitterButton.SetSameLine(true);
	m_loadEmitterButton.SetToolTip("Load into the emitter window (WARNING: Will overwrite current file");

	m_emitterName.Init("Emitter");

	float fDefault0[] = { 0.0f, 0.0f, 0.0f };
	float fDefault1[] = { 1.0f, 1.0f, 1.0f };

	m_position.Init("Position", -20.0f, 20.0f, fDefault0, 3);
	m_rotation.Init("Rotation", -360.0f, 360.0f, fDefault0, 3);
	m_scale.Init("Scale", 0.001f, 20.0f, fDefault1, 3);
	m_scale.SetToolTip("Multiplier on emission volume size and velocities");
	m_particleScale.Init("Particle Scale", 0.01f, 20.0f, 1.0f);
	m_particleScale.SetToolTip("Multiplier on particle size");
	m_startTime.Init("Start time", 0.0f, 5.0f, 0.0f);
	m_startTime.SetToolTip("Delay before starting emission");
	m_parameterWindow.Init("Parameters", vPos, vScale, usg::GUIWindow::WINDOW_TYPE_COLLAPSABLE);
	m_parameterWindow.SetDefaultCollapsed(true);

	m_changeAssetButton.Init("Change Emitter");
	m_changeAssetButton.SetExtension("vpb");
	m_changeAssetButton.AddFilter("Vitei ProtoBuf", "* .vpb");
	m_changeAssetButton.SetStartPath("..\\..\\Data\\Particle\\Emitters\\");

	m_emitterWindow.AddItem(&m_emitterName);
	m_emitterWindow.AddItem(&m_loadEmitterButton);
	m_emitterWindow.AddItem(&m_parameterWindow);
	m_parameterWindow.AddItem(&m_position);
	m_parameterWindow.AddItem(&m_rotation);
	m_parameterWindow.AddItem(&m_scale);
	m_parameterWindow.AddItem(&m_particleScale);
	m_parameterWindow.AddItem(&m_startTime);
	m_parameterWindow.AddItem(&m_changeAssetButton);
	m_parameterWindow.AddItem(&m_removeEmitterButton);

	m_emitter.Alloc(pDevice, &scene.GetParticleMgr(), "water_halo", true);
	m_emitter.SetRenderMask(usg::RenderMask::RENDER_MASK_CUSTOM_0 << 1);

	Add(false);
}

void EmitterInstance::Cleanup(usg::GFXDevice* pDevice)
{
	m_emitter.Cleanup(pDevice);
}


bool EmitterInstance::Update(usg::GFXDevice* pDevice, float fElapsed)
{
	if(m_removeEmitterButton.GetValue())
	{
		RemoveFromScene();
		return false;
	}

	//m_emitterSelect.UpdateOptions(m_pParent->GetFileList().GetFileNamesRaw());
	bool bAltered = false;
	bAltered |= Compare(m_emitterData.vPosition, m_position.GetValueV3());
	bAltered |= Compare(m_emitterData.vRotation, m_rotation.GetValueV3());
	bAltered |= Compare(m_emitterData.vScale, m_scale.GetValueV3());
	bAltered |= Compare(m_emitterData.fParticleScale, m_particleScale.GetValue());
	bAltered |= Compare(m_emitterData.fReleaseFrame, m_startTime.GetValue(0));
	
	m_emitterData.has_fReleaseFrame = true;

	if(bAltered)
	{
		UpdateInstanceMatrix();
	}

	m_emitterWindow.SetSize(m_parameterWindow.GetCollapsed() ? usg::Vector2f(300.f, 60.f) : usg::Vector2f(300.f, 220.f));

	usg::U8String emitterName = m_emitterData.emitterName;
	emitterName += ".vpb";
	if(m_changeAssetButton.GetValue())
	{
		LoadEmitter(pDevice, m_changeAssetButton.GetLastResult().relFileName.c_str());
	}
	

	return true;
}

void EmitterInstance::LoadEmitter(usg::GFXDevice* pDevice, const char* szEmitterName)
{
	m_pParent->GetEffect().RemoveEmitter(&m_emitter);
	ReloadEmitterFromFileOrGetActive(pDevice, &m_emitter, szEmitterName);
	usg::U8String emitterName = szEmitterName;
	emitterName.TruncateExtension();
	str::Copy(m_emitterData.emitterName, emitterName.CStr(), sizeof(m_emitterData.emitterName));
	m_emitterName.SetText(szEmitterName);
	m_pParent->Reset(pDevice);
	UpdateInstanceMatrix();
}

void EmitterInstance::AddToScene(usg::GFXDevice* pDevice, usg::particles::EmitterData* pInstance)
{
	if(pInstance==NULL)
	{
		usg::U8String selectName = m_emitterName.GetName();
		selectName.TruncateExtension();
		str::Copy(m_emitterData.emitterName, selectName.CStr(), sizeof(m_emitterData.emitterName));
		m_emitterData.vPosition = usg::Vector3f::ZERO;
		m_emitterData.vRotation = usg::Vector3f::ZERO;
		m_emitterData.vScale = usg::Vector3f::ONE;
		m_emitterData.fParticleScale = 1.0f;
		m_emitterData.fReleaseFrame = 0.0f;
	}
	else
	{
		m_emitterData = *pInstance;
	}

	m_position.SetValue(m_emitterData.vPosition);
	m_rotation.SetValue(m_emitterData.vRotation);
	m_scale.SetValue(m_emitterData.vScale);
	m_particleScale.SetValue(m_emitterData.fParticleScale);
	if(m_emitterData.has_fReleaseFrame)
		m_startTime.SetValue(m_emitterData.fReleaseFrame);
	else 
		m_startTime.SetValue(0.0f);

	usg::string emitterName = m_emitterData.emitterName;
	emitterName += ".vpb";
	m_emitterName.SetText(emitterName.c_str());

	ReloadEmitterFromFileOrGetActive(pDevice, &m_emitter, emitterName.c_str());
	m_pParent->Reset(pDevice);
	UpdateInstanceMatrix();

	Add(true);
}


void EmitterInstance::RemoveFromScene()
{
	Add(false);
	m_pParent->GetEffect().RemoveEmitter(&m_emitter);
}

void EmitterInstance::UpdateInstanceMatrix()
{
	usg::Matrix4x4 mInstanceMat;
	usg::Matrix4x4 mScale;
	mScale.MakeScale(m_emitterData.vScale);
	mInstanceMat.LoadIdentity();
	mInstanceMat.MakeRotate(usg::Math::DegToRad(m_emitterData.vRotation.x), -usg::Math::DegToRad(m_emitterData.vRotation.y), usg::Math::DegToRad(m_emitterData.vRotation.z));
	mInstanceMat.SetTranslation(m_emitterData.vPosition);
	mInstanceMat = mInstanceMat * mScale;
	m_emitter.SetInstanceData(mInstanceMat, m_emitterData.fParticleScale, m_emitterData.fReleaseFrame);
}

void EmitterInstance::UpdateEmitter(usg::GFXDevice* pDevice, usg::Scene& scene, const usg::particles::EmitterEmission& emitterData, const usg::particles::EmitterShapeDetails& shapeData)
{
	m_emitter.SetDefinition(pDevice, emitterData);
	m_emitter.InitMaterial(pDevice);
	m_emitter.CreateEmitterShape(emitterData.eShape, shapeData);
}

