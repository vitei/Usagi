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



	bool GUIMenu::UpdateAndAddToDrawList(const GUIContext& ctxt)
{
		bool bChanged = false;
		float fTime = (float)ImGui::GetTime();

		if (ImGui::BeginMenu(m_szName.c_str(), true))
		{

			for (auto itr : m_items)
			{
				if (itr->IsVisible())
				{
					bChanged |= itr->UpdateAndAddToDrawList(ctxt);
					itr->SetHovered(ImGui::IsItemHovered(), fTime);
				}
				else
				{
					itr->SetHovered(false, fTime);
				}
				itr->CommonDraw();
			}

			ImGui::EndMenu();
		}

		return bChanged;
	}
}
