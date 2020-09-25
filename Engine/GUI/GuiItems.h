/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
// ***************************************************************
//  A button in the debug rendering
// ***************************************************************
#ifndef _USG_GUI_GUI_BUTTON_H
#define _USG_GUI_GUI_BUTTON_H

#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Scene/RenderNode.h"
#include "Engine/Graphics/Materials/Material.h"

namespace usg
{
	enum class GuiItemType
	{
		BUTTON = 0,
		TEXT,
		COLOR_SELECT,
		COMBO_BOX,
		SLIDER,
		CHECK_BOX,
		INT_INPUT,
		WINDOW,
		TEXT_INPUT,
		TEXTURE,
		MENU_BAR,
		MENU,
		TAB_BAR,
		TAB,
		MENU_ITEM
	};

	enum 
	{
		RESET_LAYOUT_FLAG = (1<<0),
		RESET_SIZE_FLAG = (1<<1)
	};

	struct GUIContext
	{
		float fScale = 1.0f;
		uint32 uFlags = 0;
	};

	struct GUILoadResult
	{
		usg::string fileName;
		usg::string relFileName;
	};

	// Bit cleaner for our purposes than standard callbacks as we'll want a few per type
	class GUICallbacks
	{
	public:
		virtual void LoadCallback(const char* szName, const char* szFilePath, const char* szRelPath) {}
		virtual void MultiLoadCallback(const char* szName, const usg::vector<FilePathResult>& results) {}
		virtual void SaveCallback(const char* szName, const char* szFilePath, const char* szRelPath) {}
		virtual void FloatChanged(const char* szName, float* pData, uint32 uCount) {}
		virtual void IntChanged(const char* szName, int* pData, uint32 uCount) {}
		virtual void BoolChanged(const char* szName, bool* pData, uint32 uCount) {}
		virtual void FileOption(const char* szName) {}
	private:
	};

	class GUIItem
	{
	public:
		GUIItem();
		virtual ~GUIItem();
		const char* GetName() const { return m_szName; }

		virtual GuiItemType GetItemType() const = 0;
		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt) = 0;
		virtual void CommonDraw();

		void SetVisible(bool bVisible) { m_bVisible = bVisible; }
		void SetCallbacks(GUICallbacks* pCallbacks) { m_pCallbacks = pCallbacks; }
		void SetSameLine(bool bSameLine) { m_bSameLine = bSameLine; }
		bool IsVisible() const { return m_bVisible; }
		void SetHovered(bool bHovered, float fTime) { if (bHovered && !m_bHovered) { m_fNewHoverTime = fTime; } m_bHovered = bHovered; }
		bool IsHovered() const { return m_bHovered; }
		void SetToolTip(const char* szTooltip) { m_toolTip = szTooltip; }
	protected:
		void InitBase(const char* szName);
		void UpdateBase();

		GUICallbacks* m_pCallbacks;
		bool	m_bSameLine;
		bool	m_bHovered;
		bool	m_bVisible;
		float	m_fNewHoverTime;
		string	m_toolTip;
		char	m_szName[USG_MAX_PATH];

		static const float ms_fToolTipDelay;

	};

	class GUIMenuItem : public GUIItem
	{
	public:
		GUIMenuItem() : m_bEnabled(true) {}
		virtual ~GUIMenuItem() {}

		void Init(const char* szName, const char* szShortCut = nullptr);
		virtual GuiItemType GetItemType() const { return GuiItemType::MENU_ITEM; }
		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);
		void SetEnabled(bool bEnabled) { m_bEnabled = bEnabled; }
	protected:
		virtual void Run();
		usg::string m_szShortCut;
		bool			m_bEnabled;
	};

	class GUIMenuLoadSave : public GUIMenuItem
	{
		typedef GUIMenuItem Inherited;
	public:
		GUIMenuLoadSave(bool bSave) { m_bSave = bSave; }
		virtual ~GUIMenuLoadSave() {}

		void AddFilter(const char* szDisplay, const char* szPattern);
		void SetExtension(const char* szExt);
		void SetStartPath(const char* szPath) { m_szPath = szPath; }
		const char* GetStartPath() const { return m_szPath.c_str(); }
		virtual GuiItemType GetItemType() const { return GuiItemType::MENU_ITEM; }
		const GUILoadResult& GetLastResult() { return m_lastResult; }
	private:
		virtual void Run();

		GUILoadResult m_lastResult;
		usg::vector<usg::string> m_filterStrings;
		usg::string m_szExt;
		usg::string m_szPath;
		bool		m_bSave;
	};


	class GUIText : public GUIItem
	{
	public:
		GUIText() {} 
		virtual ~GUIText() {}

		void Init(const char* szName);
		void SetText(const char* szName);
		virtual GuiItemType GetItemType() const { return GuiItemType::TEXT; }
		void SetColor(const Color& color);
		const Color& GetColor() { return m_color; }
		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);
	private:
		Color m_color;
	};

	class GUIButton : public GUIItem
	{
	public:
		GUIButton();
		virtual ~GUIButton();

		void Init(const char* szName);
		void InitAsTexture(GFXDevice* pDevice, const char* szName, TextureHndl tex);
		void Cleanup(GFXDevice* pDevice);
		void SetValue(bool bValue) { m_bValue = bValue; }
		bool GetValue() const { return m_bValue; }
		void SetUVs(Vector2f vUVMin, Vector2f vUVMax);
		void SetScale(Vector2f vScale) { m_vScale = vScale; }
		void SetTexture(GFXDevice* pDevice, TextureHndl pTexture);

		virtual GuiItemType GetItemType() const { return GuiItemType::BUTTON; }
		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);
	private:
		usg::DescriptorSet	m_descriptor;	// Not passing raw textures in this brave new world
		usg::TextureHndl	m_pTexture;
		usg::SamplerHndl	m_sampler;
		Vector2f			m_vScale;
		Vector2f			m_vUVMin;
		Vector2f			m_vUVMax;
		bool				m_bHasTexture;
		bool				m_bTexDescValid;
		bool				m_bValue;
	};

	class GUILoadButton : public GUIButton
	{
		using Inherited = GUIButton;
	public:
		GUILoadButton();
		virtual ~GUILoadButton();

		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);
		void AddFilter(const char* szDisplay, const char* szPattern);
		void SetExtension(const char* szExt) { m_szExt = szExt; }
		void SetStartPath(const char* szPath) { m_szPath = szPath; }
		const GUILoadResult& GetLastResult() const { return m_lastResult; }
		void SetAllowMultiple(bool bAllow) { m_bAllowMultiple = bAllow; }
	private:
		GUILoadResult m_lastResult;
		usg::vector<usg::string> m_filterStrings;
		usg::string m_szExt;
		usg::string m_szPath;
		bool		m_bAllowMultiple;
	};

	class GUIColorSelect : public GUIItem
	{
	public:
		GUIColorSelect() {}
		virtual ~GUIColorSelect() {}
		
		void Init(const char* szName);
		void SetValue(const Color& color);
		Color GetValue() { return Color(m_color) * (1/255.f); }
		float* GetValueInt() { return m_color; }
		virtual GuiItemType GetItemType() const { return GuiItemType::COLOR_SELECT; }
		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);
	private:
		float m_color[4];
	};


	class GUIComboBox : public GUIItem
	{
	public:
		GUIComboBox() { m_uSelected = 0; m_uItems = 0; }
		virtual ~GUIComboBox() {}


		void Init(const char* szName, const char** szStringArray, uint32 uSelected = 0);	// NULL terminated
		void Init(const char* szName, const char* szZeroSeperatedString, uint32 uSelect = 0);

		void UpdateOptions(const char* szZeroSeperatedString);

		uint32 GetSelected() const { return m_uSelected; }
		void SetSelected(uint32 uSelected);
		void SetSelectedByName(const char* szName);
		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);

		virtual GuiItemType GetItemType() const { return GuiItemType::COMBO_BOX; }
		const char* GetSelectedName() { return m_selectedName.CStr(); }
	private:
		const char*	m_szZeroSepNames;
		const char** m_szNames;
		U8String	m_selectedName;
		uint32		m_uItems;
		uint32		m_uSelected;
	};


	class GUISlider : public GUIItem
	{
	public:
		GUISlider() { m_fMinValue = 0.0f; m_fMaxValue = 0.0f; memset(m_fValue, 0, sizeof(m_fValue)); }
		virtual ~GUISlider() {}


		void Init(const char* szName, float fMin, float fMax, const float* fDefault, uint32 uItems = 1);
		void Init(const char* szName, float fMin, float fMax, float fDefault);
		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);
		float GetValue(uint32 uIndex = 0) const { return m_fValue[uIndex]; }
		Vector2f GetValueV2() const { ASSERT(m_uCount==2); return Vector2f(m_fValue[0], m_fValue[1]); }
		Vector3f GetValueV3() const { ASSERT(m_uCount==3); return Vector3f(m_fValue[0], m_fValue[1], m_fValue[2]); }
		Vector4f GetValueV4() const { ASSERT(m_uCount==4); return Vector4f(m_fValue[0], m_fValue[1], m_fValue[2], m_fValue[3]); }
		void SetValue(const Vector2f &vec) { ASSERT(m_uCount==2); m_fValue[0] = vec.x;  m_fValue[1] = vec.y; }
		void SetValue(const Vector3f &vec) { ASSERT(m_uCount==3); m_fValue[0] = vec.x;  m_fValue[1] = vec.y; m_fValue[2] = vec.z; }
		void SetValue(const Vector4f &vec) { ASSERT(m_uCount==4); m_fValue[0] = vec.x;  m_fValue[1] = vec.y; m_fValue[2] = vec.z; m_fValue[3] = vec.w;}
		void SetValue(float fValue, uint32 uIndex=0) { m_fValue[uIndex] = fValue; }

		virtual GuiItemType GetItemType() const { return GuiItemType::SLIDER; }

	protected:
		enum
		{
			MAX_ITEMS = 4
		};

		uint32	m_uCount;
		float32 m_fMinValue;
		float32 m_fMaxValue;
		float32 m_fValue[MAX_ITEMS];
	};

	class GUICheckBox : public GUIItem
	{
	public:
		GUICheckBox() { m_bValue = false; }
		virtual ~GUICheckBox() {}


		void Init(const char* szName, bool bDefault);
		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);
		bool GetValue() const { return m_bValue; }
		void SetValue(bool bValue) { m_bValue = bValue; }

		virtual GuiItemType GetItemType() const { return GuiItemType::CHECK_BOX; }

	private:
		bool m_bValue;
	};


	class GUIIntInput : public GUIItem
	{
	public:
		enum 
		{
			MAX_VALUES = 4
		};

		GUIIntInput() {}
		virtual ~GUIIntInput() {}

		void Init(const char* szName, int* pDefaults, const uint32 uCount, const int iMin = 0, const int iMax = 10);
		const int* GetValue() { return m_values; }
		void SetRange(const int iMin, const int iMax) { m_iMin = iMin; m_iMax = iMax; }
		int GetValue(uint32 uIndex) { return m_values[uIndex]; }
		void SetValues(int* pValues);
		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);
		virtual GuiItemType GetItemType() const { return GuiItemType::INT_INPUT; }

	private:
		int		m_values[MAX_VALUES];
		int		m_iMin;
		int		m_iMax;
		uint32	m_uCount;
	};

	class GUITexture : public GUIItem
	{
	public:
		GUITexture();
		virtual ~GUITexture();

		void Init(GFXDevice* pDevice, const char* szName, Vector2f vSize, usg::TextureHndl pTexture);
		void Cleanup(GFXDevice* pDevice);
		void SetSize(Vector2f vSize) { m_vScale = vSize; }
		void SetUVs(Vector2f vUVMin, Vector2f vUVMax);
		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);
		virtual GuiItemType GetItemType() const { return GuiItemType::INT_INPUT; }
		void SetTexture(GFXDevice* pDevice, usg::TextureHndl pTexture);

	private:
		usg::DescriptorSet	m_descriptor;	// Not passing raw textures in this brave new world
		usg::TextureHndl	m_pTexture;
		usg::SamplerHndl	m_sampler;
		usg::Vector2f		m_vScale;
		Vector2f			m_vUVMin;
		Vector2f			m_vUVMax;
		bool				m_bDescValid;

	};

	class GUITextInput : public GUIItem
	{
	public:
		GUITextInput();
		virtual ~GUITextInput();

		void Init(const char* szName, const char* szDefault);
		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);
		virtual GuiItemType GetItemType() const { return GuiItemType::TEXT_INPUT; }
		bool GetUpdated() { return m_bUpdated; }
		const char* GetInput() { return m_input; }
		void SetInput(const char* szData);

	private:

		bool m_bUpdated;
		char m_input[USG_IDENTIFIER_LEN];
	};

	class GUIFloat : public GUISlider
	{
	public:
		GUIFloat() {  }
		virtual ~GUIFloat() {}

		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);
		

	private:
	};


}

#endif 
