/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/String/String_Util.h"
#include "Engine/ThirdParty/imgui/imgui.h"
#include "GuiItems.h"
#include "GuiWindow.h"

namespace usg
{
	GUIWindow::GUIWindow() 
		: m_menuBar(false)
		, m_items(300)
	{
		m_fScale = 1.0f;
		m_bShowBorders = false;
		m_bDefaultCollapsed = false;
		m_bCollapsed = false;
	}

	GUIWindow::~GUIWindow()
	{

	}

	void GUIWindow::Init(const char* szName, const Vector2f& vPos, const Vector2f& vSize, WindowType eType)
	{
		m_vPosition = vPos;
		m_vSize = vSize;

		str::Copy(m_szName, szName, USG_IDENTIFIER_LEN);
		m_eWindowType = eType;
	
	}
	
	void GUIWindow::AddItem(GUIItem* pItem)
	{
		m_items.push_back(pItem);
	}

	bool GUIWindow::UpdateAndAddToDrawList(const GUIContext& ctxt)
{
		Vector2f vPos = m_vPosition * m_fScale * ctxt.fScale;
		Vector2f vScale = m_vSize * m_fScale * ctxt.fScale;
		bool bChanged = false;
		real rTime = (real)ImGui::GetTime();

		switch(m_eWindowType)
		{
			case WINDOW_TYPE_CHILD:
			{
				ImGui::BeginChild(m_szName, ImVec2(vScale.x, vScale.y), true, 0);// m_bShowBorders ? ImGuiWindowFlags_ShowBorders : 0);
				CommonDraw();
			}
			break;
			case WINDOW_TYPE_PARENT:
			{
				uint32 uFlags = (ctxt.uFlags & RESET_LAYOUT_FLAG) == 0 ? ImGuiCond_Once : ImGuiCond_Always;
				uint32 uSizeFlags = (ctxt.uFlags & RESET_SIZE_FLAG) == 0 ? ImGuiCond_Once : ImGuiCond_Always;
				ImGui::SetNextWindowPos(ImVec2(vPos.x, vPos.y), uFlags);	// Don't allow our menus to be moved (for now)
				ImGui::SetNextWindowSize(ImVec2(vScale.x, vScale.y), uSizeFlags);
				bool bReturn;
				ImGui::Begin(m_szName, &bReturn, m_menuBar.IsVisible() ? ImGuiWindowFlags_MenuBar : 0);
			}
			break;
			case WINDOW_TYPE_COLLAPSABLE:
			{
				m_bCollapsed = !ImGui::CollapsingHeader(m_szName, m_bDefaultCollapsed ? 0 : ImGuiTreeNodeFlags_DefaultOpen);// NULL, true, !m_bDefaultCollapsed);
				if( m_bCollapsed )
					return false;
			}
			break;
			case WINDOW_TYPE_DUMMY:
				break;
			default:
				ASSERT(false);
		}	

		ImGui::PushItemWidth(ctxt.fScale * 180.f);
		ImGui::SetWindowFontScale(ctxt.fScale * m_fScale);

		m_menuBar.UpdateAndAddToDrawList(ctxt);

		if(m_bVisible)
		{
			for(list<GUIItem*>::iterator it = m_items.begin(); it != m_items.end(); ++it)
			{
				GUIItem* pItem = (*it);
				if(pItem->IsVisible())
				{
					bChanged = pItem->UpdateAndAddToDrawList(ctxt) || bChanged;
					pItem->SetHovered(ImGui::IsItemHovered(), rTime);
				}
				else
				{
					pItem->SetHovered(false, rTime);
				}
				pItem->CommonDraw();
			}
		}
		ImGui::PopItemWidth();

		switch(m_eWindowType)
		{
			case WINDOW_TYPE_CHILD:
				ImGui::EndChild();
			break;
			case WINDOW_TYPE_PARENT:
				ImGui::End();
			break;
			case WINDOW_TYPE_DUMMY:
				break;
			default:
				break;
		}

		return bChanged;
	}
}
