/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/ThirdParty/imgui/imgui.h"
#include "GUIMenu.h"

namespace usg
{
	GUIMenu::GUIMenu()
	{
	}

	GUIMenu::~GUIMenu()
	{

	}

	void GUIMenu::Init(const char* szName)
	{
		m_szName = szName;
	
	}
	
	void GUIMenu::AddItem(GUIItem* pItem)
	{
		m_items.push_back(pItem);
	}



	bool GUIMenu::UpdateAndAddToDrawList()
	{
		bool bChanged = false;

		if (ImGui::BeginMenu(m_szName.c_str(), true))
		{

			for (auto itr : m_items)
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

			ImGui::EndMenu();
		}

		return bChanged;
	}
}
