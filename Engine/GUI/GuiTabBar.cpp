/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/ThirdParty/imgui/imgui.h"
#include "GuiTab.h"
#include "GuiTabBar.h"

namespace usg
{
	GUITabBar::GUITabBar()
	{
		
	}

	GUITabBar::~GUITabBar()
	{

	}

	
	void GUITabBar::AddItem(GUITab* pItem)
	{
		m_menus.push_back(pItem);
	}



	bool GUITabBar::UpdateAndAddToDrawList(const GUIContext& ctxt)
{
		if (!m_bVisible)
			return false;

		bool bChanged = false;
		bool bOpened = false;
		real rTime = (real)ImGui::GetTime();

		bOpened = ImGui::BeginTabBar(m_szName);

		if (bOpened)
		{
			for (auto itr : m_menus)
			{
				if (itr->IsVisible())
				{
					bChanged |= itr->UpdateAndAddToDrawList(ctxt);
					itr->SetHovered(ImGui::IsItemHovered(), rTime);
				}
				else
				{
					itr->SetHovered(false, rTime);
				}
				itr->CommonDraw();
			}

			ImGui::EndTabBar();
		}

		return bChanged;
	}
}
