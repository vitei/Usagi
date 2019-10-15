/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//  A GUI tab, contains a gui callback class for all the types
//	it can implment
****************************************************************************/
#ifndef _USG_GUI_GUI_TAB_H
#define _USG_GUI_GUI_TAB_H

#include "Engine/Core/stl/list.h"
#include "Engine/Core/stl/string.h"
#include "GuiItems.h"

namespace usg
{
	class GUITab : public GUIItem
	{
	public:
		GUITab();
		virtual ~GUITab();

		void Init(const char* szName);
		void AddItem(GUIItem* pItem);
		virtual bool UpdateAndAddToDrawList(const GUIContext& ctxt);
		virtual GuiItemType GetItemType() const { return GuiItemType::TAB; }
		bool IsOpen() const { return m_bOpen; }

	private:

		usg::string		m_szName;
		list<GUIItem*>	m_items;
		bool			m_bOpen;
	};

}

#endif 
