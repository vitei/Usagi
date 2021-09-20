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
	usg::string scriptName = "../../Data/Particle/Emitters/";
	scriptName += szScriptName;
	usg::ProtocolBufferFile file(scriptName.c_str());
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
}



void ParticleEditor::Init(usg::GFXDevice* pDevice, usg::ResourceMgr* pResMgr)
{
	usg::Input::Init();

	uint32 uWidth;
	uint32 uHeight;

	m_hwnd = pDevice->GetDisplay(0)->GetHandle();
	pDevice->GetDisplay(0)->GetDisplayDimensions(uWidth, uHeight, false);
	
	m_postFX.Init(pDevice, usg::ResourceMgr::Inst(), uWidth, uHeight, 0);

	usg::Matrix4x4 mEffectMat;
	mEffectMat.LoadIdentity();

	m_guiRend.Init();
	m_guiRend.InitResources(pDevice, usg::ResourceMgr::Inst(), uWidth, uHeight, 20000);

	usg::GUIMenuBar& bar = m_guiRend.GetMainMenuBar();
	bar.SetVisible(true);

	m_windowMenu.Init("Window");
	bar.AddItem(&m_windowMenu);
	m_resetWindow.Init("Reset Layout");
	m_increaseSize.Init("Increase Size");
	m_decreaseSize.Init("Decrease Size");
	m_decreaseSize.SetEnabled(false);
	m_windowMenu.AddItem(&m_resetWindow);
	m_windowMenu.AddItem(&m_increaseSize);
	m_windowMenu.AddItem(&m_decreaseSize);
	m_resetWindow.SetCallbacks(this);
	m_increaseSize.SetCallbacks(this);
	m_decreaseSize.SetCallbacks(this);

	usg::Vector2f vPos(322.0f, 30.f);
	m_effectPreview.Init(pDevice, &m_guiRend, "Effect Preview", vPos);
	vPos.Assign(1132.0f, 30.0f);
	m_emitterPreview.Init(pDevice, &m_guiRend, "Emitter Preview", vPos);

	m_editorShapes.Init(pDevice, &m_emitterPreview.GetScene());

	m_emitter.Alloc(pDevice, &m_emitterPreview.GetScene().GetParticleMgr(), "water_halo", true );
	m_emitter.Init(pDevice, &m_emitterPreview.GetEffect());
	m_emitterWindow.GetVariables() = m_emitter.GetDefinition();
	m_emitterWindow.Init(pDevice, &m_guiRend);

	m_emitter.SetInstanceData(mEffectMat, 1.0f, 0.0f);
	m_emitterPreview.GetEffect().AddEmitter(pDevice, &m_emitter);

	vPos.Assign(740.0f, 120.0f);
	usg::Vector2f vScale(340.f, 100.f);
	
	m_effectGroup.Init(pDevice, &m_effectPreview.GetScene(), &m_guiRend);

	m_guiRend.AddWindow(&m_effectPreview.GetGUIWindow());
	m_guiRend.AddWindow(&m_emitterPreview.GetGUIWindow());

	
	m_emitterWindow.GetShapeSettings().SetShapeSettings(m_emitter.GetShapeDetails());


	m_bIsRunning = true;
} 

void ParticleEditor::Cleanup(usg::GFXDevice* pDevice)
{
	pDevice->WaitIdle();
	m_effectGroup.Cleanup(pDevice);
	m_emitter.Cleanup(pDevice);
	m_effectPreview.Cleanup(pDevice);
	m_emitterPreview.Cleanup(pDevice);
	m_emitterWindow.CleanUp(pDevice);
	m_editorShapes.Cleanup(pDevice);
	m_guiRend.Cleanup(pDevice);
	m_postFX.Cleanup(pDevice);
}

ParticleEditor::~ParticleEditor()
{
	
}

extern uint32 g_uWindowWidth;
extern uint32 g_uWindowHeight;

void ParticleEditor::FileOption(const char* szName)
{
	if (strcmp("Reset Layout", szName) == 0)
	{
		m_guiRend.RequestWindowReset();
	}
	else if (strcmp("Increase Size", szName) == 0)
	{
		float fScale = m_guiRend.GetScale() + 0.25f;
		if (fScale >= 2.0f)
		{
			m_increaseSize.SetEnabled(false);
		}
		m_decreaseSize.SetEnabled(true);
		m_guiRend.SetGlobalScale(fScale);
		SetWindowPos(m_hwnd, 0, 0, 0, (int)(g_uWindowWidth * fScale), int((g_uWindowHeight+20.f) * fScale), 0);
	}
	else if (strcmp("Decrease Size", szName) == 0)
	{
		float fScale = m_guiRend.GetScale() - 0.25f;
		if (fScale <= 1.0f)
		{
			m_decreaseSize.SetEnabled(false);
		}
		m_increaseSize.SetEnabled(true);
		m_guiRend.SetGlobalScale(fScale);
		SetWindowPos(m_hwnd, 0, 0, 0, (int)(g_uWindowWidth * fScale), (int)((g_uWindowHeight + 20.f) * fScale), 0);
	}
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

	m_effectGroup.Update(pDevice, fElapsed, m_effectPreview.GetPlaySpeed(), m_effectPreview.GetRepeat(), m_effectPreview.GetPaused(), m_effectPreview.GetRestart());



	bool bLoad = m_emitterWindow.GetLoaded();	
	bool bRestart = m_emitterPreview.GetRestart() || (m_emitterPreview.GetRepeat() && !m_emitterPreview.GetEffect().IsAlive());
	if(bLoad)
	{
		m_emitterPreview.SetReload();
		m_emitter.SetDefinition(pDevice, m_emitterWindow.GetVariables());
		if (m_emitterWindow.GetVariables().has_cBackgroundColor)
		{
			m_effectGroup.SetBackgroundColor(m_emitterWindow.GetVariables().cBackgroundColor);
		}

		m_emitter.CreateEmitterShape(m_emitterWindow.GetVariables().eShape, *m_emitterWindow.GetShapeSettings().GetShapeDetails() );
		bRestart = true;
	}


	if(bRestart)
	{
		usg::Matrix4x4 mEffectMat;
		mEffectMat.LoadIdentity();

		//m_emitter.Init(&m_effect);
		m_emitterPreview.GetEffect().Kill(true);
		m_emitterPreview.GetEffect().Init(pDevice, &m_emitterPreview.GetScene(), mEffectMat);

		m_emitter.SetInstanceData(mEffectMat, 1.0f, 0.0f);
		m_emitterPreview.GetEffect().AddEmitter(pDevice, &m_emitter);
	}

	bool bUpdated = false;
	for(usg::List<EmitterModifier>::Iterator it = m_emitterWindow.GetModifiers().Begin(); !it.IsEnd(); ++it)
	{
		bUpdated |= (*it)->Update(pDevice, m_emitterWindow.GetVariables(),  &m_emitter);
	}

	if(bUpdated)
	{
		usg::U8String emitterName = m_emitterWindow.GetEditFileName();
		m_emitter.SetDefinition(pDevice, m_emitterWindow.GetVariables());
		if(emitterName.Length() > 0)
		{
			emitterName.TruncateExtension();
			m_effectGroup.EmitterModified(pDevice, emitterName.CStr(), m_emitterWindow.GetVariables(), *m_emitterWindow.GetShapeSettings().GetShapeDetails());
		}
	}
	
	m_guiRend.PostUpdate(fElapsed);


	m_editorShapes.Update(pDevice, m_emitterWindow.GetVariables().eShape, m_emitterWindow.GetShapeSettings().GetShapeDetails(), fElapsed);
	m_editorShapes.Enable(m_emitterWindow.IsShapeTabOpen());

	m_emitter.UpdateBuffers(pDevice);
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
	pGFXCtxt->ClearRenderTarget();

	usg::RenderNode::RenderContext context;
	context.pPostFX = &m_postFX;
	context.eRenderPass = usg::RenderNode::RENDER_PASS_FORWARD;

	pGFXCtxt->Transfer(m_postFX.GetFinalRT(), pDisplay);
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
