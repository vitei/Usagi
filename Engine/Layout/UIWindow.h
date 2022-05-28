/****************************************************************************
//	Filename: HUDWindow.h
*****************************************************************************/
#pragma once

#include "Engine/Scene/RenderNode.h"
#include "Engine/Layout/Fonts/Text.h"
#include "Engine/Maths/Vector2f.h"
#include "Engine/Maths/Matrix4x4.h"
#include "Engine/Layout/Fonts/Text.h"
#include "Engine/Core/stl/map.h"
#include "Engine/Core/stl/string.h"
#include "Engine/Scene/Common/CustomEffectRuntime.h"
#include "Engine/Layout/UI.pb.h"

namespace usg
{

	class CustomUIItem;

	enum UIItemType
	{
		UI_ITEM_IMAGE = 0,
		UI_ITEM_TEXT,
		UI_ITEM_CUSTOM,
		UI_ITEM_BUTTON,
		UI_ITEM_INVALID
	};

	enum UIActionType
	{
		UI_ACTION_NONE = 0,
		UI_ACTION_HIGHLIGHT,
		UI_ACTION_HIGHLIGHT_END,
		UI_ACTION_ACTIVATED,
		UI_ACTION_DEACTIVATED,
		UI_ACTION_INCREMENT,
		UI_ACTION_DECREMENT
	};

	struct UIItemRef
	{
		class UIWindow* pWindow = nullptr;
		uint32	 	    uItemIdx = USG_INVALID_ID;
		UIItemType		eType = UI_ITEM_INVALID;
	};


	struct UIResult
	{
		uint32			actionCRC;
		UIActionType	eActionType;
		int				iValue;
	};

	struct UIInput
	{
		usg::Vector2f	vCursorLocation = usg::Vector2f::ZERO;
		bool			bSelect = false;
	};

	// Action CRC and result type
	typedef usg::map< uint32, UIResult > UIResults;

	class UIWindow : public usg::RenderNode
	{
	private:
		struct ActionData;
	public:
		UIWindow();
		virtual ~UIWindow();

		void Init(usg::GFXDevice* pDevice, usg::ResourceMgr* pRes, const usg::RenderPassHndl& renderPass,
			const UIWindow* pParent, const UIDef& uiDef, const UIWindowDef& windowDef, usg::string path, bool bOffscreen);
		void SetPos(usg::Vector2f vPos);
		void SetSize(usg::Vector2f vSize);
		void SetItemPos(const char* szName, usg::Vector2f vPos, bool bRelative);
		void SetMatrixDirty() { m_bMatrixDirty = true; }
		void SetName(const usg::string& name) { m_name = name; }
		void Update(const UIWindow* pParent, float fElapsed, const UIInput* pInput, UIResults* pResults);
		void GPUUpdate(usg::GFXDevice* pDevice);
		void Draw(usg::GFXContext* pContext);
		void CleanUpRecursive(usg::GFXDevice* pDevice);
		void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }
		bool GetEnabled() const { return m_bEnabled; }

		bool GetItemRef(const char* szName, enum UIItemType eType, struct UIItemRef& out);
		const char* GetOriginalText(uint32 uTextIdx) const;
		usg::Color GetOriginalColor(uint32 uIndex, enum UIItemType eType) const;
		usg::Vector2f GetOriginalItemSize(uint32 uIndex, enum UIItemType eType) const;
		void SetText(uint32 uIndex, const char* szText);
		void SetUVRange(uint32 uIndex, const usg::Vector2f& vUVMin, const usg::Vector2f& vUVMax);
		void SetItemPos(uint32 uIndex, enum UIItemType eType, const usg::Vector2f& vPos, bool bRelative);
		void SetItemSize(uint32 uIndex, enum UIItemType eType, const usg::Vector2f& vSize, bool bRelative);
		void SetItemVisible(uint32 uIndex, enum UIItemType eType, bool bVisible);

		void SetItemColor(uint32 uIndex, enum UIItemType eType, const usg::Color& cColor);
		void SetButtonEnabled(uint32 uIndex, bool bEnabled);
		void RegisterCustomItem(uint32 uIndex, class CustomUIItem* pItem);

		UIWindow* CreateChildWindow(usg::GFXDevice* pDevice, usg::ResourceMgr* pRes, const usg::RenderPassHndl& renderPass, const UIDef& def, const UIWindowDef& windowDef, bool bOffscreen);
		UIWindow* GetChildWindow(const usg::string& name);

		const usg::Matrix4x4& GetGlobalMatrix() const { return m_globalMatrix; }
		const usg::Matrix4x4& GetLocalMatrix() const { return m_localMatrix; }
		usg::Vector2f GetPosition() const { return m_windowPos; }
		usg::Vector2f GetSize() const { return m_windowSize; }
		const usg::string& GetName() const { return m_name; }

		const ButtonTemplateDef* GetUITemplate(const UIDef& uiDef, const char* szName);

	private:
		usg::Vector2f GetPos(const usg::Vector2f& vBasePos, const usg::Vector2f& vSize, UIHorAlign eHorAlign, UIVertAlign eVerAlign, const UIWindow* pOwner);
		int GetTextAlign(UIHorAlign eHorAlign, UIVertAlign eVerAlign);
		void UpdateVertices();
		void UpdateMatrix(const UIWindow* pParent);
		void UpdateButtons(float fElapsed);
		void SetMousePos(const UIWindow* pParent, const UIInput* pInput, UIResults* pResults);
		void AddResult(UIResults* pResult, uint32 uActionCRC, UIActionType eAction, int iValue = 1);
		bool IsMouseInRangeOfButton(uint32 uButton);
		bool IsMouseInRangeOfText(uint32 uText);
		bool SetButtonHighlighted(uint32 uButton, bool bHighlighted, UIResults* pResults);
		UIActionType SetButtonPressed(uint32 uButton, UIResults* pResults);
		bool IsPair(uint32 uButton) const;


		struct VertexData
		{
			usg::Vector3f vPosition;
			usg::Vector2f vSize;
			usg::Vector4f vTexCoordRange;
			usg::Color cColor;
		};

		struct HUDItemData
		{
			ImageDef		def;
			ImageDef		defOverride;
			// FIXME: Optimization would be to combine these
			usg::DescriptorSet	descriptor;
		};

		struct ActionData
		{
			uint32					uActionId;
			usg::vector<UIItemRef>	uiItems;

			bool bHighlighted = false;
			bool bActive = false;
		};

		enum class ButtonAnimState
		{
			BUTTON_ANIM_STATE_ACTIVATE = 0,
			BUTTON_ANIM_STATE_DEACTIVATE = 1,
			BUTTON_ANIM_STATE_ACTIVE = 2,
			BUTTON_ANIM_STATE_DEACTIVATED = 3,
			BUTTON_ANIM_STATE_ACTIVE_HIGHLIGHT = 4,
			BUTTON_ANIM_STATE_DEACTIVATED_HIGHLIGHT = 5
		};

		void SetButtonUVs(usg::Vector2f& vUVMin, usg::Vector2f& vUVMax, const ButtonTemplateDef* pButtonDef, uint32 uFrame);
		void SetButtonAnimState(uint32 uButton, ButtonAnimState eState);
		memsize GetActionIdIndex(uint32 uIdx);

		struct ButtonData
		{
			ButtonTemplateDef templateDef;
			ButtonInstanceDef def;
			ButtonAnimState eAnimState;

			// Need our own highlighted for left/right
			bool bHighlighted = false;
			bool bActive = false;
			uint32 uActiveFrame = 0;
			float fStateTime = 0.0f;
		};

		struct TextItemData
		{
			TextItemDef		def;
			usg::string		originalText;
			usg::Text		text;
		};

		struct CustomItemData
		{
			CustomItemDef def;
			CustomUIItem* pItem;
		};

		usg::vector<UIWindow*>		m_children;
		usg::string					m_name;
		usg::string					m_path;
		usg::Matrix4x4				m_localMatrix;
		usg::Matrix4x4				m_globalMatrix;
		UIHorAlign					m_horAlign;
		UIVertAlign					m_vertAlign;
		usg::Vector2f				m_windowPos;
		usg::Vector2f				m_windowSize;
		usg::Vector2f				m_vMousePos;
		uint32						m_uItemCounts[UIItemType::UI_ITEM_INVALID];

		HUDItemData* m_pUIItemsDefs;
		TextItemData* m_pTextItemDefs;
		CustomItemData* m_pCustomItemDefs;
		ButtonData* m_pButtonDefs;
		usg::PipelineStateHndl		m_pipelineState;
		usg::vector<VertexData>		m_vertices;
		usg::vector<ActionData>		m_actionData;
		usg::CustomEffectRuntime	m_runtimeEffect;
		usg::DescriptorSet			m_descriptor;
		usg::ConstantSet			m_windowConstants;
		usg::VertexBuffer			m_vertexData;
		usg::RenderPassHndl			m_renderPass;

		bool						m_bMatrixDirty;
		bool						m_bVertsDirty;
		bool						m_bEnabled;
		bool						m_bFirst;
	};

}

