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
		: m_bOpen(true)
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



	bool GUITab::UpdateAndAddToDrawList(const GUIContext& ctxt)
{
		bool bChanged = false;
		float fTime = (float)ImGui::GetTime();

		if (ImGui::BeginTabItem(m_szName.c_str()))
		{
			m_bOpen = true;
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

			ImGui::EndTabItem();
		}
		else
		{
			m_bOpen = false;
		}

		return bChanged;
	}
}
