#include "Engine/Common/Common.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Device, GFXDevice_ps.h)
#include "Engine/Scene/ViewContext.h"
#include "Engine/HID/Input.h"
#include "Engine/HID/Mouse.h"
#include "Engine/Scene/SceneContext.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Graphics/Lights/LightMgr.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Particles/Scripted/EmitterShapes.pb.h"
#include "Engine/Scene/Model/Model.h"
#include "ParticleEditor.h"

static ParticleEditor* g_spParticleEditor = NULL;

bool g_bEnableTestAIPlayer = false;

class ViewportHack : public usg::RenderNode
{
public:
	ViewportHack()
	{
		SetLayer(usg::RenderLayer::LAYER_SKY);
		SetPriority(0);
		m_pRenderGroup = nullptr;
	}

	void Init(usg::GFXDevice* pDevice, usg::Scene* pScene, const usg::Viewport& viewport)
	{
		m_pRenderGroup = pScene->CreateRenderGroup(nullptr);

		RenderNode* pNode = (RenderNode*)this;
		m_pRenderGroup->AddRenderNodes(pDevice, &pNode, 1, 0);
		m_viewport = viewport;
	}

	virtual bool Draw(usg::GFXContext* pContext, RenderContext& renderContext)
	{
		pContext->ApplyViewport(m_viewport);
		return true;
	}
private:
	usg::Viewport			m_viewport;
	usg::RenderGroup*		m_pRenderGroup;

};

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
		pEmitter->InitMaterial(pDevice);

		usg::particles::EmitterShapeDetails shapeDef;
		bReadSucceeded = file.Read(&shapeDef);
		pEmitter->CreateEmitterShape(variables.eShape, shapeDef);
	}
}


ParticleEditor::ParticleEditor()
: GameInterface()
{
	g_spParticleEditor = this;
	m_pDirLight = nullptr;
	m_pViewportHack = nullptr;
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
	
	m_postFX.Init(pDevice, usg::ResourceMgr::Inst(), uWidth, uHeight, 0);

	// Use the raw textures directly so artists can just place new ones in without having to rake
	// Disabling for now as the directory structure has changed
	//usg::ResourceMgr::Inst()->SetTextureDir("../../Data/Textures/");
	//usg::ResourceMgr::Inst()->EnableReloadingOfDirtyAssets(true);

	m_scene.Init(pDevice, worldBounds, NULL);
	m_pSceneCtxt = m_scene.CreateViewContext(pDevice);
	m_camera.Init(fAspect);
	m_pSceneCtxt->Init(pDevice, &m_postFX, 0, usg::RenderMask::RENDER_MASK_ALL);
	m_pSceneCtxt->SetCamera(&m_camera.GetCamera());

	m_pViewportHack = vnew(usg::ALLOC_OBJECT)ViewportHack;
	m_pViewportHack->Init(pDevice, &m_scene, m_previewViewport);
	
	usg::Matrix4x4 mEffectMat;
	mEffectMat.LoadIdentity();
	m_effect.Init(pDevice, &m_effectPreview.GetScene(), mEffectMat);

	m_effectPreview.Init(pDevice, &m_guiRend, "Effect Preview");
	m_emitterPreview.Init(pDevice, &m_guiRend, "Emitter Preview");

	m_editorShapes.Init(pDevice, &m_emitterPreview.GetScene());

	m_emitter.Alloc(pDevice, &m_emitterPreview.GetScene().GetParticleMgr(), "water_halo", true );
	m_emitter.Init(pDevice, &m_effect);
	m_emitterWindow.GetVariables() = m_emitter.GetDefinition();
	m_emitterWindow.Init(pDevice, &m_guiRend);

	m_emitter.SetInstanceData(mEffectMat, 1.0f, 0.0f);
	m_effect.AddEmitter(pDevice, &m_emitter);
	m_emitter.SetRenderMask(usg::RenderMask::RENDER_MASK_CUSTOM);
	m_pSceneCtxt->SetRenderMask(usg::RenderMask::RENDER_MASK_CUSTOM);

	/*m_testWindow.Init("Test Window", usg::Vector2f(0.0f, 0.0f), usg::Vector2f(400.f, 200.f), 10);
	m_testButton.Init("Test Button");
	m_testColor.Init("Test color");
	m_testWindow.AddItem(&m_testButton);
	m_testWindow.AddItem(&m_testColor);*/
	m_guiRend.Init();
	m_guiRend.InitResources(pDevice, m_scene, uWidth, uHeight, 20000);
	//m_guiRend.AddWindow(&m_testWindow);

	usg::Vector2f vPos(740.0f, 120.0f);
	usg::Vector2f vScale(340.f, 100.f);
	m_fileWindow.Init("File", vPos, vScale, usg::GUIWindow::WINDOW_TYPE_COLLAPSABLE);
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

	m_effectGroup.Init(pDevice, &m_effectPreview.GetScene(), &m_guiRend);

	m_guiRend.AddWindow(&m_effectPreview.GetGUIWindow());
	m_guiRend.AddWindow(&m_emitterPreview.GetGUIWindow());

	
	m_emitterWindow.GetShapeSettings().SetShapeSettings(m_emitter.GetShapeDetails());

	m_fileList.Init("../../Data/Particle/Emitters/", ".vpb");
	m_loadFilePaths.Init("Load Dir", m_fileList.GetFileNamesRaw(), 0);

	usg::DirLight* pDirLight = m_scene.GetLightMgr().AddDirectionalLight(pDevice, false);
	usg::Color ambient(0.4f, 0.4f, 0.4f);
	usg::Color diffuse(0.8f, 0.8f, 0.8f);
	usg::Vector4f vDirection(0.4f, -1.0f, 0.4f, 0.0f);
	vDirection.Normalise();
	pDirLight->SetAmbient(ambient);
	pDirLight->SetDiffuse(diffuse);
	pDirLight->SetDirection(vDirection);
	pDirLight->SwitchOn(true);
	m_pDirLight = pDirLight;

	m_bIsRunning = true;
} 

void ParticleEditor::CleanUp(usg::GFXDevice* pDevice)
{
	pDevice->WaitIdle();
	m_effectGroup.CleanUp(pDevice);
	m_emitter.CleanUp(pDevice);
	m_effect.CleanUp(pDevice);
	m_effectPreview.CleanUp(pDevice);
	m_emitterPreview.CleanUp(pDevice);
	m_emitterWindow.CleanUp(pDevice);
	m_editorShapes.CleanUp(pDevice);
	m_guiRend.CleanUp(pDevice);
	m_scene.GetLightMgr().RemoveDirLight(m_pDirLight);
	m_scene.DeleteViewContext(m_pSceneCtxt);
	m_scene.Cleanup(pDevice);
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
	m_effectPreview.Update(pDevice, fElapsed);
	m_emitterWindow.Update(pDevice, fElapsed);
	m_emitterPreview.Update(pDevice, fElapsed);

	m_camera.Update(fElapsed);
	m_effectGroup.Update(pDevice, fElapsed, m_effectPreview.GetRepeat(), m_effectPreview.GetPaused(), m_effectPreview.GetRestart());


	// Treating our particle effect seperately to a standard one managed by the particle mgr
	if(m_effect.IsAlive())
	{
		if(!m_effectPreview.GetPaused())
		{
			m_effect.Update(fElapsed);
		}
	}


	bool bLoad = m_emitterWindow.GetLoaded();
	usg::U8String loadName;
	if (m_effectGroup.LoadEmitterRequested(loadName))
	{
		loadName += ".vpb";
		m_loadFilePaths.SetSelectedByName(loadName.CStr());
		bLoad = true;
	}
	
	bool bRestart = m_effectPreview.GetRestart();
	if(bLoad)
	{
		m_emitter.SetDefinition(pDevice, m_emitterWindow.GetVariables());
		if (m_emitterWindow.GetVariables().has_cBackgroundColor)
		{
			m_effectGroup.SetBackgroundColor(m_emitterWindow.GetVariables().cBackgroundColor);
		}

		m_emitter.CreateEmitterShape(m_emitterWindow.GetVariables().eShape, *m_emitterWindow.GetShapeSettings().GetShapeDetails() );
	}

	if(bRestart)
	{
		usg::Matrix4x4 mEffectMat;
		mEffectMat.LoadIdentity();
		
		//m_emitter.Init(&m_effect);
		m_effect.Kill(true);
		m_effect.Init(pDevice, &m_effectPreview.GetScene(), mEffectMat);
		m_emitter.SetInstanceData(mEffectMat, 1.0f, 0.0f);
		m_effect.AddEmitter(pDevice, &m_emitter);
		
	}

	bool bUpdated = false;
	for(usg::List<EmitterModifier>::Iterator it = m_emitterWindow.GetModifiers().Begin(); !it.IsEnd(); ++it)
	{
		bUpdated |= (*it)->Update(pDevice, m_emitterWindow.GetVariables(), &m_emitter);
	}

	if(bUpdated)
	{
		usg::U8String emitterName = m_activeEdit;
		m_emitter.SetDefinition(pDevice, m_emitterWindow.GetVariables());
		if(emitterName.Length() > 0)
		{
			emitterName.TruncateExtension();
			m_effectGroup.EmitterModified(pDevice, emitterName.CStr(), m_emitterWindow.GetVariables(), *m_emitterWindow.GetShapeSettings().GetShapeDetails());
			m_effectPreview.SetBackgroundColor(m_effectGroup.GetBackgroundColor());
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
		bool bWritten  = file.Write(&m_emitterWindow.GetVariables());
		ASSERT(bWritten);
		usg::particles::EmitterShapeDetails* pDetails = m_emitterWindow.GetShapeSettings().GetShapeDetails();
		bWritten = file.Write(pDetails);
		ASSERT(bWritten);
	}

	m_editorShapes.Update(pDevice, m_emitterWindow.GetVariables().eShape, m_emitterWindow.GetShapeSettings().GetShapeDetails(), fElapsed);

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

	m_effectPreview.Draw(pGFXCtxt);
	m_emitterPreview.Draw(pGFXCtxt);

	m_postFX.BeginScene(pGFXCtxt, usg::PostFXSys::TRANSFER_FLAGS_CLEAR);

	bool bClearColorChanged = false;

	m_emitterWindow.GetVariables().cBackgroundColor = m_effectPreview.GetBackgroundColor();
	m_emitterWindow.GetVariables().has_cBackgroundColor = true;

	m_effectGroup.SetBackgroundColor(m_effectPreview.GetBackgroundColor());


	m_postFX.GetInitialRT()->SetClearColor(m_effectPreview.GetBackgroundColor());
	pGFXCtxt->ApplyViewport(m_previewViewport);
	pGFXCtxt->ClearRenderTarget();

	m_pSceneCtxt->PreDraw(pGFXCtxt, usg::VIEW_CENTRAL);
	m_pSceneCtxt->DrawScene(pGFXCtxt);

	pGFXCtxt->ApplyViewport(m_postFX.GetInitialRT()->GetViewport());
	 //m_postFX.GetActiveRT()->SetClearColor(Color(1.f, 1.0f, 0.0f, 1.0f));
	//pGFXCtxt->ClearRenderTarget( m_postFX.GetFinalRT(), RenderTarget::CLEAR_FLAG_COLOR_0 );
	usg::RenderNode::RenderContext context;
	context.pPostFX = &m_postFX;
	context.eRenderPass = usg::RenderNode::RENDER_PASS_FORWARD;

	pGFXCtxt->Transfer(m_postFX.GetInitialRT(), pDisplay);
	pGFXCtxt->RenderToDisplay(pDisplay);

	m_guiRend.Draw(pGFXCtxt, context);


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
	switch (messageID)
	{
	case 'WSZE':
	{
		usg::Display* const pDisplay = pDevice->GetDisplay(0);
		uint32 uWidth, uHeight;
		uint32 uWidthOld, uHeightOld;

		pDisplay->GetDisplayDimensions(uWidthOld, uHeightOld, false);
		pDisplay->Resize(pDevice); // Before obtaining dimensions, we need to force display to update internal size
		pDisplay->GetDisplayDimensions(uWidth, uHeight, false);
		m_guiRend.Resize(pDevice, uWidth, uHeight);
	}
	break;
	case 'WMIN':
	{
		usg::Display* const pDisplay = pDevice->GetDisplay(0);

		pDisplay->Minimized(pDevice);

	}
	case 'ONSZ':
	{
		// About to resize
		pDevice->WaitIdle();
	}
	break;
	default:
		// Does nothing
		break;
	}
}
