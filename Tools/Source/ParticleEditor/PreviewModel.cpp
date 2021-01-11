#include "Engine/Common/Common.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/HID/Gamepad.h"
#include "Engine/HID/Input.h"
#include "Engine/Core/ProtocolBuffers/ProtocolBufferFile.h"
#include "Engine/Scene/Model/Model.h"
#include "Engine/Scene/Model/Bone.h"
#include "PreviewModel.h"

const float g_fTrailSpeed = 20.f;


PreviewModel::~PreviewModel()
{
	
}

void PreviewModel::Cleanup(usg::GFXDevice* pDevice)
{
	if (m_pModel)
	{
		m_pModel->AddToScene(false);
		m_pModel->GPUUpdate(pDevice);
		m_pModel->Cleanup(pDevice);
		vdelete m_pModel;
	}
}

void PreviewModel::Init(usg::GFXDevice* pDevice, usg::Scene* pScene)
{
	usg::Vector2f vPos(0.0f, 130.0f);
	usg::Vector2f vScale(340.f, 100.f);
	m_window.Init("Preview Model", vPos, vScale, usg::GUIWindow::WINDOW_TYPE_COLLAPSABLE);
	m_visible.Init("Show", true);
	m_visible.SetSameLine(true);

	m_modelFileList.Init("Models", ".vmdf", true);
	m_loadFilePaths.Init("Load Dir", m_modelFileList.GetFileNamesRaw(), 0);

	m_loadButton.Init("Load");
	m_loadButton.SetSameLine(true);
	float fDefault[] = { 0.0f, 0.0f, 0.0f };
	m_position.Init("Position", -30.f, 30.f, fDefault, 3);

	m_window.AddItem(&m_loadFilePaths);
	m_window.AddItem(&m_loadButton);
	m_window.AddItem(&m_position);
	m_window.AddItem(&m_visible);

	m_pScene = pScene;


	//m_pModel = vnew(usg::ALLOC_OBJECT) usg::Model;
	//m_pModel->Load(pDevice, m_pScene, "particle_editor/Grandmaster.vmdc", false, usg::RenderMask::RENDER_MASK_ALL, true, true, NULL, NULL, false);
}


void PreviewModel::Update(usg::GFXDevice* pDevice, float fElapsed)
{
	if(m_loadButton.GetValue())
	{
		usg::U8String modelName = m_loadFilePaths.GetSelectedName();
	
		if (m_pModel)
		{
			m_pModel->AddToScene(false);
			m_pModel->GPUUpdate(pDevice);
			m_pModel->Cleanup(pDevice);
			vdelete m_pModel;
		}

		m_pModel = vnew(usg::ALLOC_OBJECT) usg::Model;
		m_pModel->Load(pDevice, m_pScene, usg::ResourceMgr::Inst(), modelName.CStr(), false, usg::RenderMask::RENDER_MASK_ALL, true, false);
	}
	if (m_pModel)
	{
		m_pModel->AddToScene(m_visible.GetValue());
		m_pModel->GPUUpdate(pDevice);
		if (m_visible.GetValue())
		{
			usg::Matrix4x4 mat = usg::Matrix4x4::Identity();
			mat.SetPos(m_position.GetValueV3());
			m_pModel->SetTransform(mat);
			for (uint32 i = 0; i < m_pModel->GetSkeleton().GetBoneCount(); i++)
			{
				usg::Matrix4x4 mBoneMat = m_pModel->GetSkeleton().GetBone(i)->GetBindMatrix(true) * mat;
				m_pModel->GetSkeleton().GetBone(i)->SetTransform(mBoneMat);
				m_pModel->GetSkeleton().GetBone(i)->UpdateConstants(pDevice);
			}
		}
	}
}
