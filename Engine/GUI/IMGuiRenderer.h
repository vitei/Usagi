/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
// ***************************************************************
//  The renderer for the IMGui debug GUI stuff. Note you can only
//	have one per project
// ***************************************************************
#ifndef _USG_GUI_GUI_RENDERER_H
#define _USG_GUI_GUI_RENDERER_H
#include "Engine/Common/Common.h"
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
		void InitResources(GFXDevice* device, usg::Scene& scene, uint32 uWidth, uint32 uHeight, uint32 uMaxVerts = 12000);
		void AddWindow(GUIWindow* pWindow) { m_windows.AddToEnd(pWindow); }
		// Can do this or simply call directly
		void AddToScene(GFXDevice* pDevice, Scene* pScene);
		
		void BufferUpdate(GFXDevice* pDevice);
		bool PreUpdate(float fElapsed);

		void PostUpdate(float fElapsed);
		// For viewports, fake screens etc
		void SetMouseOffset(const Vector2f &vOffset) { m_vOffset = vOffset; }

		virtual bool Draw(GFXContext* pContext, RenderContext& renderContext);

		void DrawInt(struct ImDrawList** const cmd_lists, int cmd_lists_count);
		bool Active()const { return m_bActive; }
	private:
		void CreateFontsTexture(GFXDevice* pDevice);
		void Shutdown();

		RenderGroup*			m_pRenderGroup;
		Scene*					m_pScene;
		bool					m_bActive;

		PipelineStateHndl		m_pipelineState;
		ConstantSet				m_constantSet;
		DescriptorSet			m_texDescriptor;
		Texture					m_texture;
		SamplerHndl				m_sampler;
		Vector2f				m_vOffset;
		uint32					m_uMaxVerts;
		uint32					m_uScreenWidth;
		uint32					m_uScreenHeight;

		VertexBuffer			m_vertexBuffer;
		GFXContext*				m_pContext;	// Should be null except when we are performing the rendering via callbacks

		List<GUIWindow>			m_windows;
	};

}

#endif 
