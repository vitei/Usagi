#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include "Engine/Scene/ViewContext.h"
#include "Engine/HID/Input.h"
#include "Engine/HID/Mouse.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Particles/Scripted/EmitterShapes.pb.h"
#include "Engine/Scene/Model/Model.h"
#include "ParticleEditor.h"

static ParticleEditor* g_spParticleEditor = NULL;

bool g_bEnableTestAIPlayer = false;

static const char* g_szPreviewType[] =
{
	"Emitter",
	"Effect", 
	NULL
};


usg::GameInterface* usg::CreateGame()
{
	return vnew(usg::ALLOC_OBJECT) ParticleEditor();
}

const char*	usg::GetGameName()
{
	return "Particle Editor";
}


void ReloadEmitterFromFileOrGetActive(usg::GFXDevice* pDevice, usg::ScriptEmitter* pEmitter, const char* szScriptName)
{
	g_spParticleEditor->ReloadEmitterFromFile(pDevice, pEmitter, szScriptName);
}

void ParticleEditor::ReloadEmitterFromFile(usg::GFXDevice* pDevice, usg::ScriptEmitter* pEmitter, const char* szScriptName)
{
	usg::U8String scriptName = "../../Data/Particle/Emitters/";
	scriptName += szScriptName;
	usg::ProtocolBufferFile file(scriptName.CStr());
	usg::particles::EmitterEmission variables;
	bool bReadSucceeded = file.Read(&variables);
	if (bReadSucceeded)
	{
		pEmitter->SetDefinition(pDevice, variables);
		//pEmitter->FillOutConstantBuffer();
		pEmitter->InitMaterial(pDevice, m_scene.GetRenderPass(0));

		usg::particles::EmitterShapeDetails shapeDef;
		bReadSucceeded = file.Read(&shapeDef);
		pEmitter->CreateEmitterShape(variables.eShape, shapeDef);
	}
}


ParticleEditor::ParticleEditor()
: GameInterface()
{
	m_bPaused = false;
	g_spParticleEditor = this;
}



void ParticleEditor::Init(usg::GFXDevice* pDevice)
{
	usg::AABB worldBounds;
	worldBounds.SetCentreRadii(usg::Vector3f(0.0f, 0.0f, 0.0f), usg::Vector3f(512.0f, 512.0f, 512.0f));

	uint32 uWidth = 750;
	uint32 uHeight = 500;
	float fAspect = (float)uWidth/(float)uHeight;
	m_previewViewport.InitViewport(340, 600, uWidth, uHeight);

	pDevice->GetDisplay(0)->GetDisplayDimensions(uWidth, uHeight, false);
	
	m_postFX.Init(pDevice, uWidth, uHeight, 0);

	usg::ResourceMgr::Inst()->RegisterRenderPass(m_postFX.GetRenderPass());
	// Use the raw textures directly so artists can just place new ones in without having to rake
	// Disabling for now as the directory structure has changed
	//usg::ResourceMgr::Inst()->SetTextureDir("../../Data/Textures/");
	//usg::ResourceMgr::Inst()->EnableReloadingOfDirtyAssets(true);

	m_scene.Init(pDevice, worldBounds, NULL);
	m_pSceneCtxt = m_scene.CreateViewContext(pDevice);
	m_camera.Init(fAspect);
	m_pSceneCtxt->Init(&m_camera.GetCamera(), &m_postFX, 0, usg::RenderNode::RENDER_MASK_ALL);
	
	usg::Matrix4x4 mEffectMat;
	mEffectMat.LoadIdentity();
	m_effect.Init(pDevice, &m_scene, mEffectMat);

	m_editorShapes.Init(pDevice, &m_scene);
	m_previewModel.Init(pDevice, &m_scene, &m_guiRend);

	m_emitter.Alloc(pDevice, &m_scene.GetParticleMgr(), "water_halo", true );
	m_emitter.Init(pDevice, &m_effect);
	m_variables = m_emitter.GetDefinition();
	m_emitter.SetInstanceData(mEffectMat, 1.0f, 0.0f);
	m_effect.AddEmitter(pDevice, &m_emitter);
	m_emitter.SetRenderMask(usg::RenderNode::RENDER_MASK_CUSTOM);
	m_pSceneCtxt->SetRenderMask(usg::RenderNode::RENDER_MASK_CUSTOM);

	/*m_testWindow.Init("Test Window", usg::Vector2f(0.0f, 0.0f), usg::Vector2f(400.f, 200.f), 10);
	m_testButton.Init("Test Button");
	m_testColor.Init("Test color");
	m_testWindow.AddItem(&m_testButton);
	m_testWindow.AddItem(&m_testColor);*/
	m_guiRend.Init();
	m_guiRend.InitResources(pDevice, m_scene, uWidth, uHeight, 20000);
	//m_guiRend.AddWindow(&m_testWindow);
	usg::Vector2f vPos(350.0f, 460.0f);
	usg::Vector2f vScale(340.f, 600.f);
	m_emitterWindow.Init("Emitter", vPos, vScale, 20 );

	vPos.Assign(700.0f, 460.0f);
	m_effectWindow.Init("Effect", vPos, vScale, 20 );

	vPos.Assign(1450.0f, 0.0f);
	vScale.Assign(340.f, 1100.f);
	m_lifeMotionWindow.Init("Life and motion", vPos, vScale, 20);

	vPos.Assign(1100.0f, 200.0f);
	vScale.Assign(340.f, 860);
	m_particleAppearanceWindow.Init("Particle appearance", vPos, vScale, 20);

	vPos.Assign(0.0f, 0.0f);
	vScale.Assign(340.f, 120.f);
	m_previewWindow.Init("Preview", vPos, vScale, 20);

	vPos.Assign(740.0f, 120.0f);
	vScale.Assign(340.f, 100.f);
	m_fileWindow.Init("File", vPos, vScale, 20, usg::GUIWindow::WINDOW_TYPE_COLLAPSABLE);
	//m_loadFilePaths.Init("Load dir", m_fileNames, 0);
	m_loadButton.Init("Load");
	m_loadButton.SetSameLine(true);
	m_saveFile.Init("Save Dir", "");
	m_saveButton.Init("Save");
	m_saveButton.SetSameLine(true);

	//m_guiRend.AddWindow(&m_fileWindow);
	m_fileWindow.AddItem(&m_loadFilePaths);
	m_fileWindow.AddItem(&m_loadButton);
	m_fileWindow.AddItem(&m_saveFile);
	m_fileWindow.AddItem(&m_saveButton);

	m_effectGroup.Init(pDevice, &m_scene, &m_guiRend, &m_colorSelection);

	m_previewType.Init("Preview mode", g_szPreviewType, 0);
	m_previewButtons[BUTTON_PLAY].InitAsTexture(pDevice, "Play", usg::ResourceMgr::Inst()->GetTexture(pDevice, "play") );
	m_previewButtons[BUTTON_PAUSE].InitAsTexture(pDevice, "Play", usg::ResourceMgr::Inst()->GetTexture(pDevice, "pause") );
	m_previewButtons[BUTTON_RESTART].InitAsTexture(pDevice, "Play", usg::ResourceMgr::Inst()->GetTexture(pDevice, "backtostart") );
	for(uint32 i=0; i<BUTTON_COUNT; i++)
	{
		m_previewButtons[i].SetSameLine(true);
		m_previewWindow.AddItem(&m_previewButtons[i]);
	}

	m_repeat.Init("Repeat", true);
	m_repeat.SetSameLine(true);
	m_previewWindow.AddItem(&m_repeat);
	m_clearColor.Init("Background");
	usg::Color defaultCol(0.1f, 0.1f, 0.1f);
	m_clearColor.SetValue(defaultCol);
	m_previewWindow.AddItem(&m_clearColor);
	m_previewWindow.AddItem(&m_previewType);

	m_guiRend.AddWindow(&m_emitterWindow);
	m_guiRend.AddWindow(&m_lifeMotionWindow);
	m_guiRend.AddWindow(&m_particleAppearanceWindow);
	m_guiRend.AddWindow(&m_effectWindow);
	m_guiRend.AddWindow(&m_previewWindow);

	m_emitterWindow.AddItem(&m_fileWindow);
	m_shapeSettings.AddToWindow(m_emitterWindow);
	m_emissionSettings.AddToWindow(m_emitterWindow);

	m_particleSettings.AddToWindow(m_lifeMotionWindow);
	m_motionParams.AddToWindow(m_lifeMotionWindow);
	m_rotationSettings.AddToWindow(m_lifeMotionWindow);
	
	m_textureSettings.AddToWindow(m_particleAppearanceWindow);
	m_colorSettings.AddToWindow(m_particleAppearanceWindow);
	m_alphaSettings.AddToWindow(m_particleAppearanceWindow);
	m_scaleSettings.AddToWindow(m_particleAppearanceWindow);

	m_blendSettings.AddToWindow(m_effectWindow);
	m_sortSettings.AddToWindow(m_effectWindow);

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
	
	for(usg::List<EmitterModifier>::Iterator it = m_modifiers.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->Init(pDevice, &m_guiRend);
	}

	m_colorSettings.SetColorSelection(&m_colorSelection);

	for(usg::List<EmitterModifier>::Iterator it = m_modifiers.Begin(); !it.IsEnd(); ++it)
	{
		(*it)->SetWidgetsFromDefinition(m_variables);
	}
	m_shapeSettings.SetShapeSettings(m_emitter.GetShapeDetails());

	m_variables.blend.alphaTestFunc = usg::ALPHA_TEST_ALWAYS;
	m_variables.blend.alphaTestReference = 0.0f;

	m_fileList.Init("../../Data/Particle/Emitters/", ".vpb");
	m_loadFilePaths.Init("Load Dir", m_fileList.GetFileNamesRaw(), 0);

	m_colorSelection.Init(pDevice, m_scene);
	m_colorSelection.SetPosition(1150.0f, 880.0f);
	m_colorSelection.SetSize(240.f, 200.0f);


	usg::DirLight* pDirLight = m_scene.GetLightMgr().AddDirectionalLight(pDevice, false);
	usg::Color ambient(0.4f, 0.4f, 0.4f);
	usg::Color diffuse(0.8f, 0.8f, 0.8f);
	usg::Vector4f vDirection(0.4f, -1.0f, 0.4f, 0.0f);
	vDirection.Normalise();
	pDirLight->SetAmbient(ambient);
	pDirLight->SetDiffuse(diffuse);
	pDirLight->SetDirection(vDirection);
	pDirLight->SwitchOn(true);

	m_bIsRunning = true;
} 

void ParticleEditor::CleanUp(usg::GFXDevice* pDevice)
{

}

ParticleEditor::~ParticleEditor()
{

}


void ParticleEditor::Update(usg::GFXDevice* pDevice)
{
	m_timer.Update();
	float fElapsed = m_timer.GetDeltaGameTime();
	m_guiRend.PreUpdate(fElapsed);
	m_guiRend.BufferUpdate(pDevice);
	m_previewModel.Update(pDevice, fElapsed);
	bool bRestart = m_previewButtons[BUTTON_RESTART].GetValue();

	if(m_previewButtons[BUTTON_PAUSE].GetValue())
		m_bPaused = true;

	if(m_previewButtons[BUTTON_PLAY].GetValue())
		m_bPaused = false;

	m_colorSelection.Update(pDevice, fElapsed);

	m_camera.Update(fElapsed);
	m_effectGroup.Update(pDevice, fElapsed, m_repeat.GetValue(), m_bPaused, bRestart);

	if(m_previewType.GetSelected() == 0)
	{
		m_pSceneCtxt->SetRenderMask(usg::RenderNode::RENDER_MASK_CUSTOM);
		if (m_variables.has_cBackgroundColor && m_previewType.GetSelected() == 0)
		{
			m_clearColor.SetValue(m_variables.cBackgroundColor);
		}
	}
	else
	{
		m_pSceneCtxt->SetRenderMask(usg::RenderNode::RENDER_MASK_CUSTOM<<1);
		m_clearColor.SetValue(m_effectGroup.GetBackgroundColor());
	}

	// Treating our particle effect seperately to a standard one managed by the particle mgr
	if(m_previewType.GetSelected() == 0)
	{
		if(m_effect.IsAlive())
		{
			if(!m_bPaused)
			{
				m_effect.Update(fElapsed);
			}
		}
		else
		{
			bRestart |= m_repeat.GetValue();
		}
	}

	bool bLoad = m_loadButton.GetValue();
	usg::U8String loadName;
	if (m_effectGroup.LoadEmitterRequested(loadName))
	{
		loadName += ".vpb";
		m_loadFilePaths.SetSelectedByName(loadName.CStr());
		bLoad = true;
	}
	
	if(bLoad)
	{
		usg::U8String scriptName = "../../Data/Particle/Emitters/";
		m_activeEdit = m_fileList.GetFileName(m_loadFilePaths.GetSelected());
		scriptName += m_activeEdit;
		m_saveFile.SetInput(m_fileList.GetFileName(m_loadFilePaths.GetSelected()));
		usg::ProtocolBufferFile file(scriptName.CStr());
		bool bReadSucceeded = file.Read(&m_variables);
		m_emitter.SetDefinition(pDevice, m_variables);
		if(m_variables.has_cBackgroundColor && m_previewType.GetSelected() == 0)
		{
			m_clearColor.SetValue(m_variables.cBackgroundColor);
		}

		if(bReadSucceeded)
		{
			for(usg::List<EmitterModifier>::Iterator it = m_modifiers.Begin(); !it.IsEnd(); ++it)
			{
				(*it)->SetWidgetsFromDefinition(m_variables);
			}
		}

		usg::particles::EmitterShapeDetails shapeDef;
		bReadSucceeded = file.Read(&shapeDef);
		m_shapeSettings.SetShapeSettings(shapeDef);
		m_emitter.CreateEmitterShape(m_variables.eShape, shapeDef);
		bRestart = true;
	}

	if(bRestart)
	{
		usg::Matrix4x4 mEffectMat;
		mEffectMat.LoadIdentity();
		
		//m_emitter.Init(&m_effect);
		m_effect.Kill(true);
		m_effect.Init(pDevice, &m_scene, mEffectMat);
		m_emitter.SetInstanceData(mEffectMat, 1.0f, 0.0f);
		m_effect.AddEmitter(pDevice, &m_emitter);
		
	}

	bool bUpdated = false;
	for(usg::List<EmitterModifier>::Iterator it = m_modifiers.Begin(); !it.IsEnd(); ++it)
	{
		bUpdated |= (*it)->Update(pDevice, m_variables, &m_emitter);
	}

	if(bUpdated)
	{
		usg::U8String emitterName = m_activeEdit;
		m_emitter.SetDefinition(pDevice, m_variables);
		if(emitterName.Length() > 0)
		{
			emitterName.TruncateExtension();
			m_effectGroup.EmitterModified(pDevice, emitterName.CStr(), m_variables, *m_shapeSettings.GetShapeDetails());
			if (m_previewType.GetSelected() == 1)
			{
				m_clearColor.SetValue(m_effectGroup.GetBackgroundColor());
			}
		}
	}
	
	static uint32 uFrame = 0;
	if(uFrame > 33)
	{
		m_fileList.Update();
		uFrame = 0;
	}
	uFrame++;

	m_loadFilePaths.UpdateOptions(m_fileList.GetFileNamesRaw());

	m_guiRend.PostUpdate(fElapsed);

	if(m_saveButton.GetValue() && m_saveFile.GetInput()[0] != '\0')
	{
		usg::U8String scriptName = "../../Data/Particle/Emitters/";
		m_activeEdit = m_saveFile.GetInput();
		if(!m_activeEdit.HasExtension("vpb"))
		{
			m_activeEdit += ".vpb";
		}
		// TODO: If we saved as a different name to the file we were editing then re-load the existing effect group
		// as the changes need to be discarded
		scriptName += m_activeEdit;
		usg::ProtocolBufferFile file(scriptName.CStr(), usg::FILE_ACCESS_WRITE);
		bool bWritten  = file.Write(&m_variables);
		ASSERT(bWritten);
		bWritten = file.Write(m_shapeSettings.GetShapeDetails());
		ASSERT(bWritten);
	}

	m_editorShapes.Update(pDevice, m_variables.eShape, m_shapeSettings.GetShapeDetails(), fElapsed);

	m_effect.UpdateBuffers(pDevice);
	m_emitter.UpdateBuffers(pDevice);
	m_scene.TransformUpdate(fElapsed);
	m_scene.Update(pDevice);
}

void ParticleEditor::Draw(usg::GFXDevice* pDevice)
{
	// Produce the 3D render list.
	pDevice->Begin();
    
	// Get the immediate context, and begin it, applying defaults.
	usg::GFXContext* pGFXCtxt = pDevice->GetImmediateCtxt();
	usg::Display* pDisplay = pDevice->GetDisplay(0);
	usg::Display* pDisplayDRC = pDevice->GetDisplay(1);

	
	pGFXCtxt->Begin(true);

	pGFXCtxt->ApplyDefaults();

	m_postFX.BeginScene(pGFXCtxt, usg::PostFXSys::TRANSFER_FLAGS_CLEAR);

	bool bClearColorChanged = false;
	if(m_clearColor.IsHovered())
	{
		if(usg::Input::GetMouse()->GetButton(usg::MOUSE_BUTTON_RIGHT, usg::BUTTON_STATE_PRESSED))
		{
			m_clearColor.SetValue( m_colorSelection.GetColor() );
		}
	}
	if (m_previewType.GetSelected() == 0)
	{
		m_variables.cBackgroundColor = m_clearColor.GetValue();
		m_variables.has_cBackgroundColor = true;
	}
	else
	{
		m_effectGroup.SetBackgroundColor(m_clearColor.GetValue());
	}

	m_postFX.GetInitialRT()->SetClearColor(m_clearColor.GetValue());
	pGFXCtxt->ApplyViewport(m_previewViewport);
	pGFXCtxt->ClearRenderTarget();

	m_pSceneCtxt->PreDraw(pGFXCtxt, usg::VIEW_LEFT_EYE);
	m_pSceneCtxt->DrawScene(pGFXCtxt);

	pGFXCtxt->ApplyViewport(m_postFX.GetInitialRT()->GetViewport());
	 //m_postFX.GetActiveRT()->SetClearColor(Color(1.f, 1.0f, 0.0f, 1.0f));
	//pGFXCtxt->ClearRenderTarget( m_postFX.GetFinalRT(), RenderTarget::CLEAR_FLAG_COLOR_0 );
	usg::RenderNode::RenderContext context;
	context.pPostFX = &m_postFX;
	context.eRenderPass = usg::RenderNode::RENDER_PASS_FORWARD;

	m_guiRend.Draw(pGFXCtxt, context);
	m_colorSelection.Draw(pGFXCtxt, &m_postFX);
	pGFXCtxt->SetRenderTarget(NULL);
	pGFXCtxt->Transfer(m_postFX.GetInitialRT(), pDisplay);


	if(pDisplayDRC)
		pGFXCtxt->Transfer(m_postFX.GetInitialRT(), pDisplayDRC);

	m_postFX.EndScene();
	pDisplay->Present();
	if(pDisplayDRC)
		pDisplayDRC->Present();

	// End the context and device.
	pGFXCtxt->End();
	pDevice->End();
}

void ParticleEditor::OnMessage(usg::GFXDevice* const pDevice, const uint32 messageID, const void* const pParameters)
{

}