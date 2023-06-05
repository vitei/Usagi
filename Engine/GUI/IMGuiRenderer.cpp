/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/ThirdParty/imgui/imgui.h"
#include "Engine/HID/Input.h"
#include "Engine/HID/Keyboard.h"
#include "Engine/HID/Mouse.h"
#include "Engine/Scene/Scene.h"
#include "Engine/Layout/Global2D.h"
#include "Engine/Graphics/Device/GFXContext.h"
#include "Engine/Scene/RenderGroup.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Graphics/StandardVertDecl.h"
#include "IMGuiRenderer.h"


namespace usg
{

static const DescriptorDeclaration decl[] =
{
	DESCRIPTOR_ELEMENT(0, DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, SHADER_FLAG_PIXEL),
	DESCRIPTOR_END()
};

static IMGuiRenderer* g_spGUIRenderer = NULL;
static GFXDevice* g_pDevice = NULL;

IMGuiRenderer::IMGuiRenderer() 
	: m_pIMGUIContext(nullptr)
	, m_mainMenuBar(true)
{
	g_spGUIRenderer = this;
	m_vOffset.Assign(0.0f, 0.0f);
	m_bActive = false;
	SetLayer(RenderLayer::LAYER_OVERLAY);
	SetPriority(128);	// After the opaque, very last
}

IMGuiRenderer::~IMGuiRenderer()
{
	//ImGui::Shutdown();
	ImGui::DestroyContext(m_pIMGUIContext);
	m_pIMGUIContext = nullptr;
	g_spGUIRenderer = NULL;
	m_texHndl.reset();
}


struct VertexConstantBuffer
{
    Matrix4x4        mProjMat;
};

static const ShaderConstantDecl g_vertexConstantDecl[] =
{
	SHADER_CONSTANT_ELEMENT(VertexConstantBuffer, mProjMat,	CT_MATRIX_44, 1),
	SHADER_CONSTANT_END()
};

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)

void RenderFunction(ImDrawData* pDraw)
{
	g_spGUIRenderer->DrawInt(pDraw);
}

void IMGuiRenderer::DrawInt(ImDrawData* pDrawData)
{
	GFXContext* pContext = m_pContext; 
	// Copy and convert all vertices into a single contiguous buffer
	PositionUVColVertex* pVerts = NULL;
	ScratchObj<PositionUVColVertex> vertScratch(pVerts, m_uMaxVerts);
	PositionUVColVertex* pVert = pVerts;

	float fZPos = 0.0f;
	uint32 uVertCount = 0;
	for (int n = 0; n < pDrawData->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = pDrawData->CmdLists[n];
		for (int i = 0; i < cmd_list->IdxBuffer.size(); i++)
		{
			const ImDrawVert& vert = cmd_list->VtxBuffer[cmd_list->IdxBuffer[i]];
			pVert->u = vert.uv.x;
			pVert->v = 1.0f - vert.uv.y;

			pVert->c.AssignRGBA32(vert.col);
			pVert->x = vert.pos.x;
			pVert->y = vert.pos.y;
			pVert->z = fZPos;

			pVert++;
		}
		uVertCount += (uint32)cmd_list->IdxBuffer.size();
	}
	m_vertexBuffer.SetContents(g_pDevice, pVerts, uVertCount);

	pContext->SetPipelineState(m_pipelineState);
	pContext->SetVertexBuffer(&m_vertexBuffer);

	// Render command lists
	uint32 uScreenHeight = (uint32)pContext->GetActiveViewport().height;
	int Offset = 0;
	for (int n = 0; n < pDrawData->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = pDrawData->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				// TODO: Add support for scissor rects so we don't end up drawing outside of the window
				//const D3D11_RECT r = { (LONG)pcmd->clip_rect.x, (LONG)pcmd->clip_rect.y, (LONG)pcmd->clip_rect.z, (LONG)pcmd->clip_rect.w };
				//pContext->BindTexture(0, (Texture*)pcmd->texture_id, m_sampler);
				pContext->SetDescriptorSet(&m_globalDescriptor, 0);
				pContext->SetDescriptorSet((DescriptorSet*)pcmd->TextureId, 1);
				pContext->SetScissorRect((uint32)pcmd->ClipRect.x, (uint32)(uScreenHeight - pcmd->ClipRect.w), (uint32)(pcmd->ClipRect.z - pcmd->ClipRect.x), (uint32)(pcmd->ClipRect.w - pcmd->ClipRect.y));
				//g_pd3dDeviceContext->RSSetScissorRects(1, &r); 
				pContext->DrawImmediate(pcmd->ElemCount, pcmd->VtxOffset + Offset);
				Offset += pcmd->ElemCount;
			}
		}
	}

	pContext->DisableScissor();
	m_pContext = NULL;
}


void IMGuiRenderer::PostUpdate(float fElapsed)
{

}

void IMGuiRenderer::BufferUpdate(GFXDevice* pDevice)
{
	m_constantSet.UpdateData(pDevice);
	m_globalDescriptor.UpdateDescriptors(pDevice);
	g_pDevice = pDevice;
} 

bool IMGuiRenderer::Draw(GFXContext* pContext, RenderContext& renderContext)
{
	m_pContext = pContext;

	ImGui::Render();
	ImDrawData* draw_data = ImGui::GetDrawData();
	RenderFunction(draw_data);

	m_bActive = false;

	return true;
}


void IMGuiRenderer::CreateFontsTexture(GFXDevice* pDevice)
{
    ImGuiIO& io = ImGui::GetIO();

    // Build
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
	io.ConfigWindowsMoveFromTitleBarOnly = true;

    // Create DX11 texture
	DescriptorSetLayoutHndl global = pDevice->GetDescriptorSetLayout(g_sGlobalDescriptors2D);
	DescriptorSetLayoutHndl layout = pDevice->GetDescriptorSetLayout(decl);
	m_texDescriptor.Init(pDevice, layout);
	m_globalDescriptor.Init(pDevice, global);
	m_globalDescriptor.SetConstantSet(0, &m_constantSet);
	m_globalDescriptor.UpdateDescriptors(pDevice);
	m_texture.CreateRaw(pDevice, ColorFormat::RGBA_8888, width, height, pixels);


    // Create texture sampler   
	SamplerDecl samplerDecl(SAMP_FILTER_LINEAR, SAMP_WRAP_REPEAT);
    m_sampler = pDevice->GetSampler(samplerDecl);
	m_texHndl = &m_texture;
	m_texDescriptor.SetImageSamplerPair(0, m_texHndl, m_sampler, 0);
	m_texDescriptor.UpdateDescriptors(pDevice);

	// Store our identifier  
	io.Fonts->TexID = (void *)&m_texDescriptor;

    // Cleanup (don't clear the input data if you want to append new fonts later)
    io.Fonts->ClearInputData();
    io.Fonts->ClearTexData();
}

void IMGuiRenderer::Resize(GFXDevice* device, uint32 uWidth, uint32 uHeight)
{
	m_uScreenWidth = uWidth;
	m_uScreenHeight = uHeight;
}

void IMGuiRenderer::InitResources(GFXDevice* pDevice, ResourceMgr* pResMgr, uint32 uWidth, uint32 uHeight, uint32 uMaxVerts)
{
	m_uMaxVerts = uMaxVerts;
	m_uScreenWidth = uWidth;
	m_uScreenHeight = uHeight;

    // Create the blending setup
	PipelineStateDecl pipeline;
	pipeline.ePrimType = PT_TRIANGLES;
    {
		AlphaStateDecl& desc = pipeline.alphaState;
		desc.SetColor0Only();
        desc.bBlendEnable = true;
        desc.srcBlend = BLEND_FUNC_SRC_ALPHA;
		desc.dstBlend = BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
		desc.blendEq = BLEND_EQUATION_ADD;
		desc.srcBlendAlpha = BLEND_FUNC_SRC_ALPHA;
		desc.dstBlendAlpha = BLEND_FUNC_ONE_MINUS_SRC_ALPHA;
		desc.blendEqAlpha = BLEND_EQUATION_ADD;
    }

	{
		DepthStencilStateDecl& depthDecl = pipeline.depthState;

		depthDecl.bDepthWrite = false;
		depthDecl.bDepthEnable = false;
		depthDecl.eDepthFunc = DEPTH_TEST_ALWAYS;
		depthDecl.bStencilEnable = false;
		depthDecl.eStencilTest = STENCIL_TEST_ALWAYS;
	}
	
	{
		RasterizerStateDecl& rasDecl = pipeline.rasterizerState;
		rasDecl.eCullFace	= CULL_FACE_NONE;
	}
	pipeline.pEffect = pResMgr->GetEffect(pDevice, "Debug.PosColUV");
	pipeline.inputBindings[0].Init(GetVertexDeclaration(VT_POSITION_UV_COL));
	pipeline.uInputBindingCount = 1;

	pipeline.layout.uDescriptorSetCount = 2;
	DescriptorSetLayoutHndl globalDesc = pDevice->GetDescriptorSetLayout(g_sGlobalDescriptors2D);
	pipeline.layout.descriptorSets[0] = globalDesc;
	pipeline.layout.descriptorSets[1] = pDevice->GetDescriptorSetLayout(decl);

	m_pipelineState = pDevice->GetPipelineState(pDevice->GetDisplay(0)->GetRenderPass(), pipeline);

    // Create the vertex buffer
	m_vertexBuffer.Init(pDevice, NULL, sizeof(PositionUVColVertex), uMaxVerts, "IMGuiRenderer", GPU_USAGE_DYNAMIC, GPU_LOCATION_STANDARD);
	m_constantSet.Init(pDevice, g_vertexConstantDecl);

    CreateFontsTexture(pDevice);
}

void IMGuiRenderer::AddToScene(GFXDevice* pDevice, Scene* pScene)
{
	m_pScene = pScene;
	m_pRenderGroup = pScene->CreateRenderGroup(NULL);


	RenderNode* pNode = this;
	m_pRenderGroup->AddRenderNodes( pDevice, &pNode, 1, 0 );
}


void IMGuiRenderer::Init()
{
	m_pIMGUIContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(m_pIMGUIContext);
	ImGuiIO& io = ImGui::GetIO();
    
    //io.RenderDrawListsFn = RenderFunction;
}

void IMGuiRenderer::Cleanup(GFXDevice* pDevice)
{
	m_texture.Cleanup(pDevice);
	m_globalDescriptor.Cleanup(pDevice);
	m_texDescriptor.Cleanup(pDevice);
	m_vertexBuffer.Cleanup(pDevice);
	m_constantSet.Cleanup(pDevice);
}



static ImGuiKey UsgKeyToImGui(int iKey)
{
	switch (iKey)
	{
	case KEYBOARD_KEY_DELETE: return ImGuiKey_Delete;
	case KEYBOARD_KEY_LEFT: return ImGuiKey_LeftArrow;
	case KEYBOARD_KEY_RIGHT: return ImGuiKey_RightArrow;
	case KEYBOARD_KEY_UP: return ImGuiKey_UpArrow;
	case KEYBOARD_KEY_DOWN: return ImGuiKey_DownArrow;
	case KEYBOARD_KEY_PGUP: return ImGuiKey_PageUp;
	case KEYBOARD_KEY_PGDN: return ImGuiKey_PageDown;
	case KEYBOARD_KEY_HOME: return ImGuiKey_Home;
	case KEYBOARD_KEY_END: return ImGuiKey_End;
	case KEYBOARD_KEY_INSERT: return ImGuiKey_Insert;
	case KEYBOARD_KEY_BACK: return ImGuiKey_Backspace;
	case ' ': return ImGuiKey_Space;
	case KEYBOARD_KEY_RETURN: return ImGuiKey_Enter;
	case KEYBOARD_KEY_ESCAPE: return ImGuiKey_Escape;
	case '\'': return ImGuiKey_Apostrophe;
	case ',': return ImGuiKey_Comma;
	case KEYBOARD_KEY_MINUS: return ImGuiKey_Minus;
	case '.': return ImGuiKey_Period;
	case '/': return ImGuiKey_Slash;
	case ';': return ImGuiKey_Semicolon;
	case KEYBOARD_KEY_EQUALS: return ImGuiKey_Equal;
	case KEYBOARD_KEY_CAPS: return ImGuiKey_CapsLock;
	case KEYBOARD_KEY_SCROLL: return ImGuiKey_ScrollLock;
	case KEYBOARD_KEY_NUMLOCK: return ImGuiKey_NumLock;
	case KEYBOARD_KEY_PRINT: return ImGuiKey_PrintScreen;
	case KEYBOARD_KEY_PAUSE: return ImGuiKey_Pause;
	case KEYBOARD_KEY_NUMPAD0: return ImGuiKey_Keypad0;
	case KEYBOARD_KEY_NUMPAD1: return ImGuiKey_Keypad1;
	case KEYBOARD_KEY_NUMPAD2: return ImGuiKey_Keypad2;
	case KEYBOARD_KEY_NUMPAD3: return ImGuiKey_Keypad3;
	case KEYBOARD_KEY_NUMPAD4: return ImGuiKey_Keypad4;
	case KEYBOARD_KEY_NUMPAD5: return ImGuiKey_Keypad5;
	case KEYBOARD_KEY_NUMPAD6: return ImGuiKey_Keypad6;
	case KEYBOARD_KEY_NUMPAD7: return ImGuiKey_Keypad7;
	case KEYBOARD_KEY_NUMPAD8: return ImGuiKey_Keypad8;
	case KEYBOARD_KEY_NUMPAD9: return ImGuiKey_Keypad9;
	case '\\': return ImGuiKey_KeypadDivide;
	case '*': return ImGuiKey_KeypadMultiply;
	case KEYBOARD_KEY_SUBTRACT: return ImGuiKey_KeypadSubtract;
	case KEYBOARD_KEY_PLUS: return ImGuiKey_KeypadAdd;
	case KEYBOARD_KEY_SHIFT: return ImGuiKey_LeftShift;
	case KEYBOARD_KEY_CONTROL: return ImGuiKey_LeftCtrl;
	case KEYBOARD_KEY_ALT: return ImGuiKey_LeftAlt;
	case '0': return ImGuiKey_0;
	case '1': return ImGuiKey_1;
	case '2': return ImGuiKey_2;
	case '3': return ImGuiKey_3;
	case '4': return ImGuiKey_4;
	case '5': return ImGuiKey_5;
	case '6': return ImGuiKey_6;
	case '7': return ImGuiKey_7;
	case '8': return ImGuiKey_8;
	case '9': return ImGuiKey_9;
	case 'A': return ImGuiKey_A;
	case 'B': return ImGuiKey_B;
	case 'C': return ImGuiKey_C;
	case 'D': return ImGuiKey_D;
	case 'E': return ImGuiKey_E;
	case 'F': return ImGuiKey_F;
	case 'G': return ImGuiKey_G;
	case 'H': return ImGuiKey_H;
	case 'I': return ImGuiKey_I;
	case 'J': return ImGuiKey_J;
	case 'K': return ImGuiKey_K;
	case 'L': return ImGuiKey_L;
	case 'M': return ImGuiKey_M;
	case 'N': return ImGuiKey_N;
	case 'O': return ImGuiKey_O;
	case 'P': return ImGuiKey_P;
	case 'Q': return ImGuiKey_Q;
	case 'R': return ImGuiKey_R;
	case 'S': return ImGuiKey_S;
	case 'T': return ImGuiKey_T;
	case 'U': return ImGuiKey_U;
	case 'V': return ImGuiKey_V;
	case 'W': return ImGuiKey_W;
	case 'X': return ImGuiKey_X;
	case 'Y': return ImGuiKey_Y;
	case 'Z': return ImGuiKey_Z;
	case KEYBOARD_KEY_F1: return ImGuiKey_F1;
	case KEYBOARD_KEY_F2: return ImGuiKey_F2;
	case KEYBOARD_KEY_F3: return ImGuiKey_F3;
	case KEYBOARD_KEY_F4: return ImGuiKey_F4;
	case KEYBOARD_KEY_F5: return ImGuiKey_F5;
	case KEYBOARD_KEY_F6: return ImGuiKey_F6;
	case KEYBOARD_KEY_F7: return ImGuiKey_F7;
	case KEYBOARD_KEY_F8: return ImGuiKey_F8;
	case KEYBOARD_KEY_F9: return ImGuiKey_F9;
	case KEYBOARD_KEY_F10: return ImGuiKey_F10;
	case KEYBOARD_KEY_F11: return ImGuiKey_F11;
	case KEYBOARD_KEY_F12: return ImGuiKey_F12;
	default: return ImGuiKey_None;
	}
}

bool IMGuiRenderer::PreUpdate(float fElapsed)
{
	// TODO: On PC resize any assets which need resizing here
	bool bChanged = false;
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    io.DisplaySize = ImVec2((float)(m_uScreenWidth), (float)(m_uScreenHeight));
    io.DeltaTime = fElapsed;

    
	memset(io.KeysDown, 0, sizeof(io.KeysDown));
	io.KeyCtrl = false;
	io.KeyAlt = false;
	io.KeyShift = false;
	memset(io.MouseDown, 0, sizeof(io.MouseDown));
	
	const usg::Keyboard* pKeyboard = Input::GetKeyboard();
	if(pKeyboard)
	{
		io.KeyCtrl = pKeyboard->GetToggleHeld(KEYBOARD_TOGGLE_CTRL);
		io.KeyShift = pKeyboard->GetToggleHeld(KEYBOARD_TOGGLE_SHIFT);
		io.KeyAlt = pKeyboard->GetToggleHeld(KEYBOARD_TOGGLE_ALT);

		for (uint32 i = KEYBOARD_KEY_DELETE; i < KEYBOARD_KEY_COUNT; i++)
		{
			ImGuiKey key = UsgKeyToImGui(i);
			if(key != ImGuiKey_None)
			{
				if(pKeyboard->GetKey(i, BUTTON_STATE_PRESSED))
				{
					io.AddKeyEvent(key, true);
				}
				if (pKeyboard->GetKey(i, BUTTON_STATE_RELEASED))
				{
					io.AddKeyEvent(key, false);
				}
			}
		}

		for (uint32 i = 0; i < pKeyboard->GetInputCharCount(); i++)
		{
			int iChar = pKeyboard->GetInputChar(i);
			if(iChar < KEYBOARD_KEY_DELETE)
			{
				io.AddInputCharacter(pKeyboard->GetInputChar(i));
			}
		}


	}

	const usg::Mouse* pMouse = Input::GetMouse();
	if(pMouse)
	{
		io.MouseDown[0] = pMouse->GetButton(MOUSE_BUTTON_LEFT);
		io.MouseDown[1] = pMouse->GetButton(MOUSE_BUTTON_RIGHT);
		io.MouseDown[2] = pMouse->GetButton(MOUSE_BUTTON_MIDDLE);
		io.MousePos.x = pMouse->GetAxis(MOUSE_POS_X);
		io.MousePos.y = pMouse->GetAxis(MOUSE_POS_Y);
		io.MouseWheel += pMouse->GetAxis(MOUSE_DELTA_WHEEL);

		// Fake windows etc
		io.MousePos.x -= m_vOffset.x;
		io.MousePos.y -= m_vOffset.y;
	}

	const usg::Gamepad* pGamepad = Input::GetGamepad(0);
	if(pGamepad)
	{
		if(pGamepad->HasCapabilities(CAP_POINTER) && !pMouse)
		{
			// TODO: Fake screen via mouse
			Vector2f vScreenPos;
			bool bTouch = pGamepad->GetScreenTouch(vScreenPos);
			io.MouseDown[0] |= bTouch;

			if(bTouch)
			{
				vScreenPos.y = 1.0f - vScreenPos.y;
				vScreenPos = vScreenPos * Vector2f(320.f, 240.f);
				vScreenPos += m_vOffset;

				// Fake windows etc
				io.MousePos.x = vScreenPos.x;
				io.MousePos.y = vScreenPos.y;
			}
		}
	}

	io.MouseDrawCursor = false;//(pMouse == NULL);

    // io.KeysDown : filled by WM_KEYDOWN/WM_KEYUP events
    // io.MousePos : filled by WM_MOUSEMOVE events
    // io.MouseDown : filled by WM_*BUTTON* events
    // io.MouseWheel : filled by WM_MOUSEWHEEL events

    // Hide OS mouse cursor if ImGui is drawing it
	//io.MouseDrawCursor = true;
    //SetCursor(io.MouseDrawCursor ? NULL : LoadCursor(NULL, IDC_ARROW));

    // Start the frame
    ImGui::NewFrame();
	m_bActive = true;

	bChanged |= m_mainMenuBar.UpdateAndAddToDrawList(m_drawCtxt);
	for(list<GUIWindow*>::iterator it = m_windows.begin(); it != m_windows.end(); ++it)
	{
		bChanged = (*it)->UpdateAndAddToDrawList(m_drawCtxt) || bChanged;
	}
	// Clear flags
	m_drawCtxt.uFlags &= ~(RESET_LAYOUT_FLAG| RESET_SIZE_FLAG);
	
	/*
	bool show_test_window = true;
	bool show_another_window = false;
	ImVec4 clear_col = ImColor(114, 144, 154);
	
	{
		static float f = 0.0f;
		ImGui::Text("Hello, world!");
		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
		ImGui::ColorEdit3("clear color", (float*)&clear_col);
		if (ImGui::Button("Test Window")) show_test_window ^= 1;
		if (ImGui::Button("Another Window")) show_another_window ^= 1;
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	}

	// 2. Show another simple window, this time using an explicit Begin/End pair
	if (show_another_window)
	{
		ImGui::SetNextWindowSize(ImVec2(200,100), ImGuiSetCond_FirstUseEver);
		ImGui::Begin("Another Window", &show_another_window);
		ImGui::Text("Hello");
		ImGui::End();
	}

	// 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if (show_test_window)
	{
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);     // Normally user code doesn't need/want to call it because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
		ImGui::ShowTestWindow(&show_test_window);
	}*/

	// Setup orthographic projection matrix into our constant buffer
	{
		VertexConstantBuffer* pLightingData = m_constantSet.Lock<VertexConstantBuffer>();

		const float L = 0.0f;
		const float R = ImGui::GetIO().DisplaySize.x;
		const float B = ImGui::GetIO().DisplaySize.y;
		const float T = 0.0f;
		pLightingData->mProjMat.Orthographic(L, R, B, T, 0.0f, 10.0f);

		m_constantSet.Unlock();
	}

	return bChanged;
}

}