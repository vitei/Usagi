/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//  A GUI menu, contains a gui callback class for all the types
//	it can implment
****************************************************************************/
#ifndef _USG_GUI_GUI_MENU_H
#define _USG_GUI_GUI_MENU_H

#include "Engine/Core/stl/list.h"
#include "Engine/Core/stl/string.h"
#include "GuiItems.h"

namespace usg
{
	class GUIMenu : public GUIItem
	{
	public:
		GUIMenu();
		virtual ~GUIMenu();

		void Init(const char* szName);
		void AddItem(GUIItem* pItem);
		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);
		virtual GuiItemType GetItemType() const { return GuiItemType::MENU; }

	private:

		usg::string		m_szName;
		list<GUIItem*>	m_items;
	};

}

#endif 
