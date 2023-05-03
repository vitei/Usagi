/****************************************************************************
//	Filename: UI.h
*****************************************************************************/
#pragma once

#include "Engine/Scene/RenderNode.h"
#include "Engine/Layout/Fonts/Text.h"
#include "Engine/Game/SystemsMode.h"
#include "UIWindow.h"

namespace usg
{

	class UI
	{
	public:
		UI();
		virtual ~UI();

		void Init(usg::GFXDevice* pDevice, usg::ResourceMgr* pRes, const usg::RenderPassHndl& renderPass, const char* szName, bool bOffscreen = true);
		void Cleanup(usg::GFXDevice* pDevice);
		bool Update(float fElapsed, const UIInput* pInput = nullptr, UIResults* pResults = nullptr);

		void GPUUpdate(usg::GFXDevice* pDevice);
		void SetHUDItemPos(const char* szName, usg::Vector2f vPos, bool bRelative);
		virtual bool Draw(usg::GFXContext* pContext);
		virtual void RenderPassChanged(usg::GFXDevice* pDevice, uint32 uContextId, const usg::RenderPassHndl& renderPass, const usg::SceneRenderPasses& passes);
		UIWindow* GetWindow(const usg::string& name);


		bool GetItemRef(const char* szName, UIItemType eType, UIItemRef& out);
		const char* GetOriginalItemText(const UIItemRef& ref) const;
		usg::Color GetOriginalItemColor(const UIItemRef& ref) const;
		usg::Vector2f GetOriginalItemSize(const UIItemRef& ref) const;
		void SetItemPos(const UIItemRef& ref, const usg::Vector2f& vPos, bool bRelative) const;
		void SetItemSize(const UIItemRef& ref, const usg::Vector2f& vSize, bool bRelative) const;
		void SetItemColor(const UIItemRef& ref, const usg::Color& cColor);
		void SetText(const UIItemRef& ref, const char* szNewText);
		void SetText(const UIItemRef& ref, uint32 uKeyStringCRC);
		void SetTexture(const UIItemRef& ref, usg::TextureHndl texHndl);

		void SetButtonEnabled(const UIItemRef& ref, bool bEnabled);
		void SetItemVisible(const UIItemRef& ref, bool bVisible);
		void SetUVRange(const UIItemRef& ref, const usg::Vector2f& vUVMin, const usg::Vector2f& vUVMax);
		void RegisterCustomItem(CustomUIItem* pItem, const UIItemRef& ref);
		bool IsRefValid(const UIItemRef& ref) const;

	private:
		// Use the same pipeline between windows, but each has their own matrix
		usg::vector<UIWindow*>	m_parentWindows;
		usg::PipelineStateHndl	m_pipelineHndl;
		usg::string				m_dirName;
	};

}

