// ========================================================================================
//	TWindowsMenu.cpp		   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
// ========================================================================================
/*
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "FWCommon.h"

#include "TWindowsMenu.h"
#include "TTopLevelWindow.h"


TWindowsMenu::TWindowsMenu(const TChar* title, const TMenuItemRec* menuItemRec)
	:	TMenu(title, menuItemRec)
{
	fPermanentItemCount = fItemList.GetSize();
}


TWindowsMenu::~TWindowsMenu()
{
}


void TWindowsMenu::PreDisplayMenu()
{
	int32 size = fItemList.GetSize();
	
	while (size > fPermanentItemCount)
	{
		fItemList.DeleteAt(size - 1);
		size--;
	}
	
	TMenu::PreDisplayMenu();

	TListIterator<TTopLevelWindow> iter(TTopLevelWindow::GetWindowList());
	TTopLevelWindow* window;

	while ((window = iter.Next()) != NULL)
	{
		TMenuItem* item = new TMenuItem(window->GetTitle());
		item->Enable(true);
		AddItem(item);
	}
}


void TWindowsMenu::MenuItemSelected(TMenu* menu, int itemIndex, Time time)
{
	if (itemIndex >= 0 && itemIndex < fPermanentItemCount)
		TMenu::MenuItemSelected(menu, itemIndex, time);
	else
	{
		itemIndex -= fPermanentItemCount;

		TList<TTopLevelWindow>& windowList = TTopLevelWindow::GetWindowList();
	
		if (itemIndex >= 0 && itemIndex < windowList.GetSize())
			windowList[itemIndex]->Raise();
	
		TMenu::MenuItemSelected(menu, -1, time);
	}
}
