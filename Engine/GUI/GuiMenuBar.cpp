/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/ThirdParty/imgui/imgui.h"
#include "GuiMenu.h"
#include "GuiMenuBar.h"

namespace usg
{
	GUIMenuBar::GUIMenuBar(bool bMainMenu) 
		: m_bMainMenu(bMainMenu)
	{
		SetVisible(false);
	}

	GUIMenuBar::~GUIMenuBar()
	{

	}

	
	void GUIMenuBar::AddItem(GUIMenu* pItem)
	{
		m_menus.push_back(pItem);
	}



	bool GUIMenuBar::UpdateAndAddToDrawList()
	{
		if (!m_bVisible)
			return false;

		bool bChanged = false;
		bool bOpened = false;

		if (m_bMainMenu)
		{
			bOpened = ImGui::BeginMainMenuBar();
		}
		else
		{
			bOpened = ImGui::BeginMenuBar();
		}

		if (bOpened)
		{
			for (auto itr : m_menus)
			{
				if (itr->IsVisible())
				{
					bChanged |= itr->UpdateAndAddToDrawList();
					itr->SetHovered(ImGui::IsItemHovered());
				}
				else
				{
					itr->SetHovered(false);
				}
			}

			if (m_bMainMenu)
			{
				ImGui::EndMainMenuBar();
			}
			else
			{
				ImGui::EndMenuBar();
			}
		}

		return bChanged;
	}
}
