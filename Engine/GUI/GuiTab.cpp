/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/ThirdParty/imgui/imgui.h"
#include "GUITab.h"

namespace usg
{
	GUITab::GUITab()
	{
	}

	GUITab::~GUITab()
	{

	}

	void GUITab::Init(const char* szName)
	{
		m_szName = szName;
	
	}
	
	void GUITab::AddItem(GUIItem* pItem)
	{
		m_items.push_back(pItem);
	}



	bool GUITab::UpdateAndAddToDrawList()
	{
		bool bChanged = false;
		float fTime = (float)ImGui::GetTime();

		if (ImGui::BeginTabItem(m_szName.c_str()))
		{

			for (auto itr : m_items)
			{
				if (itr->IsVisible())
				{
					bChanged |= itr->UpdateAndAddToDrawList();
					itr->SetHovered(ImGui::IsItemHovered(), fTime);
				}
				else
				{
					itr->SetHovered(false, fTime);
				}
				itr->CommonDraw();
			}

			ImGui::EndTabItem();
		}

		return bChanged;
	}
}
