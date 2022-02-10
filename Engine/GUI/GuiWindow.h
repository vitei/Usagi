/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
// ***************************************************************
//  A GUI window, contains a gui callback class for all the types
//	it can implment
// ***************************************************************
#ifndef _USG_GUI_GUI_WINDOW_H
#define _USG_GUI_GUI_WINDOW_H

#include "Engine/Core/stl/list.h"
#include "Engine/Maths/Vector2f.h"
#include "GuiMenuBar.h"
#include "GuiItems.h"

namespace usg
{
	class GUIItem;

	class GUIWindow : public GUIItem
	{
	public:
		GUIWindow();
		virtual ~GUIWindow();

		enum WindowType
		{
			WINDOW_TYPE_PARENT = 0,
			WINDOW_TYPE_CHILD,
			WINDOW_TYPE_COLLAPSABLE,
			WINDOW_TYPE_DUMMY	// A container for items that doesn't imapct appearance
		};

		void Init(const char* szName, const Vector2f& vPos, const Vector2f& vSize, WindowType eType = WINDOW_TYPE_PARENT);
		void AddItem(GUIItem* pItem);
		void SetSize(const Vector2f& vSize) { m_vSize = vSize;  }
		void SetDefaultCollapsed(bool bCollapsed) { m_bDefaultCollapsed = bCollapsed; }
		bool GetCollapsed() { return m_bCollapsed;  }
		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);
		virtual GuiItemType GetItemType() const { return GuiItemType::WINDOW; }

		void SetScale(float fScale) { m_fScale = fScale; }
		void SetShowBorders(bool bShow) { m_bShowBorders = bShow; }

		GUIMenuBar& GetMenuBar() { return m_menuBar; }

		list<GUIItem*>& GetItems() { return m_items; }
	private:

		Vector2f	m_vPosition;
		Vector2f	m_vSize;
		float		m_fScale;
		WindowType	m_eWindowType;
		bool		m_bShowBorders;
		bool		m_bDefaultCollapsed;
		bool		m_bCollapsed;
	

		GUIMenuBar		m_menuBar;
		list<GUIItem*>	m_items;

	};

}

#endif 
