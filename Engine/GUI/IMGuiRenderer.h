/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
// ***************************************************************
//  The renderer for the IMGui debug GUI stuff. Note you can only
//	have one per project
// ***************************************************************
#ifndef _USG_GUI_GUI_RENDERER_H
#define _USG_GUI_GUI_RENDERER_H

#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Scene/RenderNode.h"
#include "Engine/Graphics/Materials/Material.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Engine/Graphics/Primitives/VertexBuffer.h"
#include "Engine/Graphics/Textures/Texture.h"
#include "GuiWindow.h"

namespace usg
{
	class IMGuiRenderer : public RenderNode
	{
	public:
		IMGuiRenderer();
		virtual ~IMGuiRenderer();

		// TODO: Generate additional vertex buffers as needed
		void Init();
		void Cleanup(GFXDevice* device);
		void InitResources(GFXDevice* device, ResourceMgr* pMgr, uint32 uWidth, uint32 uHeight, uint32 uMaxVerts = 12000);
		void Resize(GFXDevice* device, uint32 uWidth, uint32 uHeight);
		void AddWindow(GUIWindow* pWindow) { m_windows.AddToEnd(pWindow); }
		// Can do this or simply call directly
		void AddToScene(GFXDevice* pDevice, Scene* pScene);

		void BufferUpdate(GFXDevice* pDevice);
		bool PreUpdate(float fElapsed);

		void PostUpdate(float fElapsed);
		// For viewports, fake screens etc
		void SetMouseOffset(const Vector2f& vOffset) { m_vOffset = vOffset; }

		virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);

		void DrawInt(struct ImDrawData* pDrawData);
		bool Active()const { return m_bActive; }

		GUIMenuBar& GetMainMenuBar() { return m_mainMenuBar; }

		void RequestWindowReset() { m_drawCtxt.uFlags |= (RESET_LAYOUT_FLAG| RESET_SIZE_FLAG); }
		void SetGlobalScale(float fScale) { m_drawCtxt.fScale = fScale; RequestWindowReset(); }
		float GetScale() const { return m_drawCtxt.fScale; }

	private:
		void CreateFontsTexture(GFXDevice* pDevice);
		void Shutdown();

		struct ImGuiContext*	m_pIMGUIContext;
		RenderGroup*			m_pRenderGroup;
		Scene*					m_pScene;
		bool					m_bActive;

		PipelineStateHndl		m_pipelineState;
		ConstantSet				m_constantSet;
		DescriptorSet			m_globalDescriptor;
		DescriptorSet			m_texDescriptor;
		Texture					m_texture;
		TextureHndl				m_texHndl;
		SamplerHndl				m_sampler;
		Vector2f				m_vOffset;
		uint32					m_uMaxVerts;
		uint32					m_uScreenWidth;
		uint32					m_uScreenHeight;
		GUIContext				m_drawCtxt;

		VertexBuffer			m_vertexBuffer;
		GFXContext*				m_pContext;	// Should be null except when we are performing the rendering via callbacks

		GUIMenuBar				m_mainMenuBar;
		List<GUIWindow>			m_windows;
	};

}

#endif 
