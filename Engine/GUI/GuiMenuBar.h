/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//  A GUI menu, contains a gui callback class for all the types
//	it can implment
****************************************************************************/
#ifndef _USG_GUI_GUI_MENU_BAR_H
#define _USG_GUI_GUI_MENU_BAR_H

#include "Engine/Core/stl/list.h"
#include "Engine/Core/stl/string.h"
#include "GuiItems.h"

namespace usg
{
	class GUIMenu; 

	class GUIMenuBar : public GUIItem
	{
	public:
		GUIMenuBar(bool bMainMenu);
		virtual ~GUIMenuBar();

		void AddItem(GUIMenu* pItem);
		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);
		virtual GuiItemType GetItemType() const { return GuiItemType::MENU_BAR; }

	private:

		bool			m_bMainMenu;
	
		list<GUIMenu*>	m_menus;

	};

}

#endif 
