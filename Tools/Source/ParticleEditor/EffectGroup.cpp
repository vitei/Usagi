#include "Engine/Common/Common.h"
#include "Engine/HID/Gamepad.h"
#include "Engine/HID/Input.h"
#include "EffectGroup.h"

const float g_fTrailSpeed = 20.f;

void EffectGroup::Init(usg::GFXDevice* pDevice, usg::Scene* pScene, usg::IMGuiRenderer* pRenderer)
{
	usg::Vector2f vPos(0.0f, 240.0f);
	usg::Vector2f vScale(340.f, 750.f);
	m_window.Init("Effect Group", vPos, vScale);

	m_fileMenu.Init("File");
	m_saveItem.Init("Save");
	m_saveAsItem.Init("Save As...");
	m_saveAsItem.SetFilters(".vpb (Vitei ProtoBuf)\0*.vpb\0\0");
	m_saveAsItem.SetStartPath("..\\..\\Data\\Particle\\Effects\\");
	m_saveAsItem.SetExtension("vpb");
	m_loadItem.Init("Load");
	m_loadItem.SetFilters(".vpb (Vitei ProtoBuf)\0*.vpb\0\0");
	m_loadItem.SetStartPath("..\\..\\Data\\Particle\\Effects\\");
	
	m_fileMenu.AddItem(&m_loadItem);
	m_fileMenu.AddItem(&m_saveItem);
	m_fileMenu.AddItem(&m_saveAsItem);

	usg::GUIMenuBar& menuBar = m_window.GetMenuBar();
	menuBar.SetVisible(true);
	menuBar.AddItem(&m_fileMenu);

	m_addEmitterButton.Init("Add Emitter");
	m_addTrailButton.Init("Add trail");
	m_addTrailButton.SetSameLine(true);

	int uInstances = 1;
	m_instanceCount.Init("Preload", &uInstances, 1, 0, 16);

	pRenderer->AddWindow(&m_window);

	m_fileList.Init("../../Data/Particle/Effects/", ".vpb");
	m_instanceFileList.Init("../../Data/Particle/Emitters/", ".vpb");
	m_textureFileList.Init("../../Usagi/Data/Textures/ribbon", ".dds");
	m_loadFilePaths.Init("Load Dir", m_fileList.GetFileNamesRaw(), 0);

	m_loadButton.Init("Load");
	m_loadButton.SetSameLine(true);
	m_saveFile.Init("Save Dir", "");
	m_saveButton.Init("Save");
	m_saveButton.SetSameLine(true);

	m_window.AddItem(&m_loadFilePaths);
	m_window.AddItem(&m_loadButton);
	m_window.AddItem(&m_saveFile);
	m_window.AddItem(&m_saveButton);
	m_window.AddItem(&m_instanceCount);
	m_window.AddItem(&m_addEmitterButton);
	m_window.AddItem(&m_addTrailButton);

	for(uint32 i=0; i<MAX_INSTANCES; i++)
	{
		m_instances[i].Init(pDevice, *pScene, this, i);
	}

	for(uint32 i=0; i<MAX_RIBBONS; i++)
	{
		m_ribbons[i].Init(pDevice, *pScene, this, i);
	}

	usg::Matrix4x4 mEffectMat;
	mEffectMat.LoadIdentity();
	m_effect.Init(pDevice, pScene, mEffectMat);

	m_pScene = pScene;
}

void EffectGroup::LoadCallback(const char* szName, const char* szFilePath)
{

}

void EffectGroup::SaveCallback(const char* szName, const char* szFilePath)
{

}

void EffectGroup::CleanUp(usg::GFXDevice* pDevice)
{
	for (uint32 i = 0; i < MAX_RIBBONS; i++)
	{
		m_ribbons[i].CleanUp(pDevice);
	}

	for (uint32 i = 0; i < MAX_INSTANCES; i++)
	{
		m_instances[i].CleanUp(pDevice);
	}
	m_effect.CleanUp(pDevice);
}

bool EffectGroup::LoadEmitterRequested(usg::U8String& name)
{
	for (uint32 i = 0; i < MAX_INSTANCES; i++)
	{
		if (m_instances[i].GetActive())
		{
			if (m_instances[i].GetLoadRequest())
			{
				name = m_instances[i].GetData().emitterName;
				return true;
			}
		}
	}
	return false;
}

void EffectGroup::EmitterModified(usg::GFXDevice* pDevice, const char* szName, const usg::particles::EmitterEmission& emitterData, const usg::particles::EmitterShapeDetails& shapeData)
{
	for(uint32 i=0; i<MAX_INSTANCES; i++)
	{
		if( m_instances[i].GetActive() )
		{
			if( str::Compare(m_instances[i].GetData().emitterName, szName) )
			{
				m_instances[i].UpdateEmitter(pDevice, *m_pScene, emitterData, shapeData);
			}
		}
	}
}


void EffectGroup::Update(usg::GFXDevice* pDevice, float fElapsed, bool bRepeat, bool bPause, bool bRestart)
{
	static uint32 uFrame = 0;
	if((uFrame % 536) == 0)
	{
		m_fileList.Update();
	}
	if((uFrame % 690) == 0)
	{
		m_instanceFileList.Update();
	}
	if((uFrame % 1035) == 0)
	{
		m_textureFileList.Update();
	}
	uFrame++;

	m_loadFilePaths.UpdateOptions(m_fileList.GetFileNamesRaw());

	for(uint32 i=0; i<MAX_INSTANCES; i++)
	{
		if(m_instances[i].GetActive())
		{
			m_instances[i].Update(pDevice, fElapsed);
		}
	}

	for(uint32 i=0; i<MAX_RIBBONS; i++)
	{
		if(m_ribbons[i].GetActive())
		{
			m_ribbons[i].Update(pDevice, fElapsed);
		}
	}

	if(m_addEmitterButton.GetValue())
	{
		for(uint32 i=0; i<MAX_INSTANCES; i++)
		{
			if(!m_instances[i].GetActive())
			{
				m_instances[i].AddToScene(pDevice);
				break;
			}
		}
	}

	if(m_addTrailButton.GetValue())
	{
		for(uint32 i=0; i<MAX_RIBBONS; i++)
		{
			if(!m_ribbons[i].GetActive())
			{
				m_ribbons[i].AddToScene(pDevice);
				break;
			}
		}
	}

	if (m_instanceCount.GetValue(0) != (int)m_effectGroup.uPreloadCount)
	{
		m_effectGroup.uPreloadCount = (uint32)m_instanceCount.GetValue(0);
		m_effectGroup.has_uPreloadCount = true;
	}

	if(m_loadButton.GetValue())
	{
		usg::U8String scriptName = "../../Data/Particle/Effects/";
		scriptName += m_loadFilePaths.GetSelectedName();
		m_saveFile.SetInput(m_loadFilePaths.GetSelectedName());
		usg::ProtocolBufferFile file(scriptName.CStr());
		bool bReadSucceeded = file.Read(&m_effectGroup);

		if(bReadSucceeded)
		{
			// The emitters
			uint32 uEmitter = 0;
			for(; uEmitter<m_effectGroup.emitters_count; uEmitter++)
			{
				m_instances[uEmitter].AddToScene(pDevice, &m_effectGroup.emitters[uEmitter]);
			}

			for(;uEmitter < MAX_INSTANCES; uEmitter++)
			{
				if(m_instances[uEmitter].GetActive())
					m_instances[uEmitter].RemoveFromScene();
			}

			if (m_effectGroup.has_uPreloadCount == false)
			{
				m_effectGroup.uPreloadCount = 1;
				m_effectGroup.has_uPreloadCount = true;
			}
			int preload = (int)m_effectGroup.uPreloadCount;
			m_instanceCount.SetValues(&preload);

			// Now process the ribbon trails
			uEmitter = 0;
			for(; uEmitter<m_effectGroup.ribbons_count; uEmitter++)
			{
				m_ribbons[uEmitter].AddToScene(pDevice, &m_effectGroup.ribbons[uEmitter]);
			}

			for(;uEmitter < MAX_RIBBONS; uEmitter++)
			{
				if(m_ribbons[uEmitter].GetActive())
					m_ribbons[uEmitter].RemoveFromScene();
			}
		}
		bRestart = true;
	}

	if(m_saveButton.GetValue() && str::StringLength(m_saveFile.GetInput()) > 0)
	{
		m_effectGroup.emitters_count = 0;
		m_effectGroup.ribbons_count = 0;
		for (int i = 0; i < MAX_INSTANCES; i++)
		{
			if(m_instances[i].GetActive())
			{
				m_effectGroup.emitters[i] = m_instances[i].GetData();
				m_effectGroup.emitters_count++;
			}
		}

		for(int i=0; i < MAX_RIBBONS; i++)
		{
			if(m_ribbons[i].GetActive())
			{
				m_effectGroup.ribbons[i] = m_ribbons[i].GetData();
				m_effectGroup.ribbons_count++;
			}
		}

		usg::U8String scriptName = "../../Data/Particle/Effects/";
		scriptName += m_saveFile.GetInput();
		if(!scriptName.HasExtension("vpb"))
		{
			scriptName += ".vpb";
		}
		usg::ProtocolBufferFile file(scriptName.CStr(), usg::FILE_ACCESS_WRITE);
		bool bWritten  = file.Write(&m_effectGroup);
		ASSERT(bWritten);
	}

	usg::Gamepad* pPad = usg::Input::GetGamepad(0);
	if(pPad && m_effect.IsAlive())
	{
		usg::Matrix4x4 mMove;
		usg::Matrix4x4 mPrev = m_effect.GetMatrix();

		if(pPad->GetButtonDown(usg::GAMEPAD_BUTTON_THUMB_L))
		{
			mPrev = usg::Matrix4x4::Identity();
		}
		usg::Vector3f vTrans(pPad->GetAxisValue(usg::GAMEPAD_AXIS_LEFT_X),
			pPad->GetAxisValue(usg::GAMEPAD_AXIS_LEFT_Y),
			pPad->GetAxisValue(usg::GAMEPAD_AXIS_RIGHT_Y));

		mMove = usg::Matrix4x4::TranslationMatrix(vTrans*fElapsed*g_fTrailSpeed);

		m_effect.SetWorldMat( mMove * mPrev );

	}

	if(m_effect.IsAlive())
	{
		if(!bPause)
		{
			m_effect.Update(fElapsed);
			m_effect.UpdateBuffers(pDevice);
		}
	}
	else
	{
		bRestart|=bRepeat;
	}

	if(bRestart)
	{
		Reset(pDevice);
	}
}

usg::Color EffectGroup::GetBackgroundColor() const
{
	if(m_effectGroup.has_cBackgroundColor)
	{
		return m_effectGroup.cBackgroundColor;
	}

	return usg::Color(0.1f, 0.1f, 0.1f, 0.0f);
}

void EffectGroup::SetBackgroundColor(const usg::Color& color)
{
	m_effectGroup.cBackgroundColor = color;
	m_effectGroup.has_cBackgroundColor = true;
}

void EffectGroup::Reset(usg::GFXDevice* pDevice)
{
	usg::Matrix4x4 mEffectMat;
	mEffectMat.LoadIdentity();
	m_effect.Kill(true);
	m_effect.Init(pDevice, m_pScene, mEffectMat);
	for(uint32 i=0; i<MAX_INSTANCES; i++)
	{
		if(m_instances[i].GetActive())
		{
			m_instances[i].GetEmitter().InitMaterial(pDevice);
			m_effect.AddEmitter(pDevice, &m_instances[i].GetEmitter());
		}
	}

	for(uint32 i=0; i<MAX_RIBBONS; i++)
	{
		if(m_ribbons[i].GetActive())
		{
			m_effect.AddEmitter(pDevice, &m_ribbons[i].GetEmitter());
		}
	}

	// Be sure to reset the position
	m_effect.SetWorldMat(usg::Matrix4x4::Identity());
}

