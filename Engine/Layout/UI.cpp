#include "Engine/Common/Common.h"
#include "Engine/Resource/ResourceMgr.h"
#include "Engine/Layout/StringTable.h"
#include "UI.h"

namespace usg
{

	UI::UI()
	{

	}

	UI::~UI()
	{

	}

	void UI::Init(usg::GFXDevice* pDevice, usg::ResourceMgr* pRes, const usg::RenderPassHndl& renderPass, const char* szName, bool bOffscreen)
	{
		m_dirName = szName;
		str::TruncateToPath(m_dirName);
		m_dirName += "/";

		usg::string pakName = szName;
		str::TruncateExtension(pakName);
		pRes->LoadPackage(pDevice, pakName.c_str());
		usg::ProtocolBufferFile* pFile = pRes->GetBufferedFile(szName);
		if (!pFile)
		{
			ASSERT(false);
			return;
		}

		UIDef* pUI;
		usg::ScratchObj<UIDef>  chainMem(pUI, 1);

		bool bReadSucceeded = pFile->Read(pUI);

		m_parentWindow.Init(pDevice, pRes, renderPass, nullptr, *pUI, pUI->windows[0], m_dirName, bOffscreen);

		for (uint32 i = 1; i < pUI->windows_count; i++)
		{
			UIWindow* pParent = GetWindow(pUI->windows[i].parentName);
			if (pParent)
			{
				pParent->CreateChildWindow(pDevice, pRes, renderPass, *pUI, pUI->windows[i], bOffscreen);
			}
			else
			{
				ASSERT(false);
			}
		}
	}

	bool UI::IsRefValid(const UIItemRef& ref) const
	{
		if (ref.pWindow && ref.uItemIdx != USG_INVALID_ID)
			return true;

		return false;
	}


	bool UI::GetItemRef(const char* szName, UIItemType eType, UIItemRef& out)

	{
		if (m_parentWindow.GetItemRef(szName, eType, out))
		{
			out.eType = eType;
			return true;
		}
		return false;
	}


	usg::Vector2f UI::GetOriginalItemSize(const UIItemRef& ref) const
	{
		if (!IsRefValid(ref))
			return usg::Vector2f::ZERO;

		return ref.pWindow->GetOriginalItemSize(ref.uItemIdx, ref.eType);
	}

	const char* UI::GetOriginalItemText(const UIItemRef& ref) const
	{
		if (!IsRefValid(ref))
			return nullptr;

		if (ref.eType != UI_ITEM_TEXT)
			return nullptr;

		return ref.pWindow->GetOriginalText(ref.uItemIdx);

	}

	usg::Color UI::GetOriginalItemColor(const UIItemRef& ref) const
	{
		if (!IsRefValid(ref))
			return usg::Color::White;

		return ref.pWindow->GetOriginalColor(ref.uItemIdx, ref.eType);
	}

	void UI::SetItemSize(const UIItemRef& ref, const usg::Vector2f& vSize, bool bRelative) const
	{
		if (!IsRefValid(ref))
			return;


		ref.pWindow->SetItemSize(ref.uItemIdx, ref.eType, vSize, bRelative);
	}


	void UI::SetItemVisible(const UIItemRef& ref, bool bVisible)
	{
		if (!IsRefValid(ref))
			return;


		ref.pWindow->SetItemVisible(ref.uItemIdx, ref.eType, bVisible);
	}

	void UI::SetButtonEnabled(const UIItemRef& ref, bool bEnabled)
	{
		if (!IsRefValid(ref) || ref.eType != UI_ITEM_BUTTON)
			return;


		ref.pWindow->SetButtonEnabled(ref.uItemIdx, bEnabled);
	}

	void UI::SetItemPos(const UIItemRef& ref, const usg::Vector2f& vPos, bool bRelative) const
	{
		if (!IsRefValid(ref))
			return;


		ref.pWindow->SetItemPos(ref.uItemIdx, ref.eType, vPos, bRelative);
	}

	void UI::SetItemColor(const UIItemRef& ref, const usg::Color& cColor)
	{
		if (!IsRefValid(ref))
			return;


		ref.pWindow->SetItemColor(ref.uItemIdx, ref.eType, cColor);
	}

	void UI::SetText(const UIItemRef& ref, const char* szNewText)
	{
		if (!IsRefValid(ref))
			return;

		if (ref.eType != UI_ITEM_TEXT)
			return;

		ref.pWindow->SetText(ref.uItemIdx, szNewText);
	}

	void UI::SetUVRange(const UIItemRef& ref, const usg::Vector2f& vUVMin, const usg::Vector2f& vUVMax)
	{
		if (!IsRefValid(ref))
			return;

		if (ref.eType != UI_ITEM_IMAGE)
			return;

		ref.pWindow->SetUVRange(ref.uItemIdx, vUVMin, vUVMax);
	}

	void UI::SetText(const UIItemRef& ref, uint32 uKeyStringCRC)
	{
		usg::StringTable::KeyString string = usg::StringTable::Inst()->Find(uKeyStringCRC);
		if (string.pStr)
		{
			SetText(ref, string.pStr->text);
		}
	}

	void UI::RegisterCustomItem(CustomUIItem* pItem, const UIItemRef& ref)
	{
		if (!IsRefValid(ref) || ref.eType != UI_ITEM_CUSTOM)
			return;

		ref.pWindow->RegisterCustomItem(ref.uItemIdx, pItem);
	}

	void UI::Cleanup(usg::GFXDevice* pDevice)
	{
		m_parentWindow.CleanUpRecursive(pDevice);
	}

	void UI::GPUUpdate(usg::GFXDevice* pDevice)
	{
		m_parentWindow.GPUUpdate(pDevice);
	}

	bool UI::Update(float fElapsed, const UIInput* pInput, UIResults* pResults)
	{
		m_parentWindow.Update(nullptr, fElapsed, pInput, pResults);
		return true;
	}


	void UI::SetHUDItemPos(const char* szName, usg::Vector2f vPos, bool bRelative)
	{
		m_parentWindow.SetItemPos(szName, vPos, bRelative);
	}

	bool UI::Draw(usg::GFXContext* pContext)
	{
		m_parentWindow.Draw(pContext);

		return true;
	}

	void UI::RenderPassChanged(usg::GFXDevice* pDevice, uint32 uContextId, const usg::RenderPassHndl& renderPass, const usg::SceneRenderPasses& passes)
	{
		pDevice->ChangePipelineStateRenderPass(renderPass, m_pipelineHndl);
	}

	UIWindow* UI::GetWindow(const usg::string& name)
	{
		if (name == m_parentWindow.GetName())
		{
			return &m_parentWindow;
		}
		return m_parentWindow.GetChildWindow(name);
	}

}