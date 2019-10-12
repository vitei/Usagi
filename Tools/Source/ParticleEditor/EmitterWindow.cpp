#include "Engine/Common/Common.h"
#include "Engine/HID/Gamepad.h"
#include "Engine/HID/Input.h"
#include "EmitterWindow.h"


EmitterWindow::EmitterWindow()
	: m_saveAsItem(true)
	, m_loadItem(false)
	, m_bLoaded(true)
{

}


void EmitterWindow::Init(usg::GFXDevice* pDevice, usg::IMGuiRenderer* pRenderer)
{
	usg::Vector2f vPos(1200.0f, 0.0f);
	usg::Vector2f vScale(370.f, 700.f);
	m_emitterWindow.Init("Emitter", vPos, vScale);

	m_fileName.Init("");

	m_emitterTabBar.Init("Emitter");
	m_emitterShapeTab.Init("Shape");
	m_emissionTab.Init("Emit");
	m_textureTab.Init("Texture");
	m_colorTab.Init("Color");
	m_blendTab.Init("Blend");
	m_scaleTab.Init("Scale");
	m_motionTab.Init("Motion");
	m_emitterTabBar.AddItem(&m_emitterShapeTab);
	m_emitterTabBar.AddItem(&m_emissionTab);
	m_emitterTabBar.AddItem(&m_textureTab);
	m_emitterTabBar.AddItem(&m_colorTab);
	m_emitterTabBar.AddItem(&m_blendTab);
	m_emitterTabBar.AddItem(&m_scaleTab);
	m_emitterTabBar.AddItem(&m_motionTab);

	m_shapeSettings.AddToTab(m_emitterShapeTab);
	m_emissionSettings.AddToTab(m_emissionTab);

	m_particleSettings.AddToTab(m_emissionTab);
	m_motionParams.AddToTab(m_motionTab);
	m_rotationSettings.AddToTab(m_motionTab);

	m_textureSettings.AddToTab(m_textureTab);
	m_colorSettings.AddToTab(m_colorTab);
	m_alphaSettings.AddToTab(m_colorTab);
	m_scaleSettings.AddToTab(m_scaleTab);

	m_blendSettings.AddToTab(m_blendTab);
	m_sortSettings.AddToTab(m_blendTab);

	m_modifiers.AddToEnd(&m_shapeSettings);
	m_modifiers.AddToEnd(&m_emissionSettings);

	m_modifiers.AddToEnd(&m_sortSettings);
	m_modifiers.AddToEnd(&m_blendSettings);
	m_modifiers.AddToEnd(&m_rotationSettings);
	m_modifiers.AddToEnd(&m_colorSettings);
	m_modifiers.AddToEnd(&m_alphaSettings);
	m_modifiers.AddToEnd(&m_scaleSettings);
	m_modifiers.AddToEnd(&m_motionParams);
	m_modifiers.AddToEnd(&m_textureSettings);
	m_modifiers.AddToEnd(&m_particleSettings);

	m_fileMenu.Init("File");
	m_saveItem.Init("Save");
	m_saveItem.SetEnabled(false);
	m_saveAsItem.Init("Save As...");
	m_saveAsItem.AddFilter("Vitei ProtoBuf", "*.vpb");
	m_saveAsItem.SetStartPath("..\\..\\Data\\Particle\\Emitters\\");
	m_saveAsItem.SetExtension("vpb");
	m_saveAsItem.SetCallbacks(this);
	m_loadItem.Init("Load");
	m_loadItem.AddFilter("Vitei ProtoBuf", "* .vpb");
	m_loadItem.SetStartPath("..\\..\\Data\\Particle\\Emitters\\");
	m_loadItem.SetCallbacks(this);

	for (usg::List<EmitterModifier>::Iterator it = m_modifiers.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->Init(pDevice, pRenderer);
	}


	for (usg::List<EmitterModifier>::Iterator it = m_modifiers.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->SetWidgetsFromDefinition(m_variables);
	}


	m_fileMenu.AddItem(&m_loadItem);
	m_fileMenu.AddItem(&m_saveItem);
	m_fileMenu.AddItem(&m_saveAsItem);

	usg::GUIMenuBar& menuBar = m_emitterWindow.GetMenuBar();
	menuBar.SetVisible(true);
	menuBar.AddItem(&m_fileMenu);
	m_emitterWindow.AddItem(&m_fileName);
	m_emitterWindow.AddItem(&m_emitterTabBar);

	pRenderer->AddWindow(&m_emitterWindow);
}

void EmitterWindow::CleanUp(usg::GFXDevice* pDevice)
{
	m_textureSettings.CleanUp(pDevice);
}

void EmitterWindow::Update(usg::GFXDevice* pDevice, float fElapsed)
{
	
}

void EmitterWindow::LoadCallback(const char* szName, const char* szFilePath, const char* szRelPath)
{
	usg::U8String scriptName = szFilePath;
	m_fileName.SetText(szRelPath);
	usg::ProtocolBufferFile file(scriptName.CStr());
	bool bReadSucceeded = file.Read(&m_variables);

	if (bReadSucceeded)
	{
		for (usg::List<EmitterModifier>::Iterator it = m_modifiers.Begin(); !it.IsEnd(); ++it)
		{
			(*it)->SetWidgetsFromDefinition(m_variables);
		}
	}

	usg::particles::EmitterShapeDetails shapeDef;
	bReadSucceeded = file.Read(&shapeDef);
	m_shapeSettings.SetShapeSettings(shapeDef);

	m_bLoaded = true;
}

void EmitterWindow::SaveCallback(const char* szName, const char* szFilePath, const char* szRelPath)
{
	usg::U8String scriptName = szName;

	usg::ProtocolBufferFile file(scriptName.CStr(), usg::FILE_ACCESS_WRITE);
	bool bWritten = file.Write(&m_variables);
	ASSERT(bWritten);
	bWritten = file.Write(m_shapeSettings.GetShapeDetails());
	m_fileName.SetText(szRelPath);
	ASSERT(bWritten);
}

void EmitterWindow::FileOption(const char* szName)
{
	if (strcmp(szName, "Save") == 0)
	{
		usg::string name = m_loadItem.GetStartPath();
		name += m_fileName.GetName();
		// Just ride the save as code
		SaveCallback(m_saveAsItem.GetName(), name.c_str(), m_fileName.GetName());
	}
}