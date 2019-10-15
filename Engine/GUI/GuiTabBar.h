/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//  A GUI tab, contains a gui callback class for all the types
//	it can implment
****************************************************************************/
#ifndef _USG_GUI_GUI_TAB_BAR_H
#define _USG_GUI_GUI_TAB_BAR_H

#include "Engine/Core/stl/list.h"
#include "Engine/Core/stl/string.h"
#include "GuiItems.h"

namespace usg
{
	class GUITab; 

	class GUITabBar : public GUIItem
	{
	public:
		GUITabBar();
		virtual ~GUITabBar();

		void Init(const char* szName) { InitBase(szName); }
		void AddItem(GUITab* pItem);
		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);
		virtual GuiItemType GetItemType() const { return GuiItemType::TAB_BAR; }

	private:

		bool			m_bMainMenu;
	
		list<GUITab*>	m_menus;

	};

}

#endif 
