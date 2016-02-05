// ========================================================================================
//	TPopupMenu.cpp			   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TPopupMenu.h"
#include "TMenuBar.h"
#include "TTopLevelWindow.h"


TPopupMenu::TPopupMenu(const TMenuItemRec* menuItemRec)
	:	TMenu("", menuItemRec),
		fOwningWindow(NULL),
		fSelectedItemIndex(-1)
{
	fFont = TMenuBar::GetDefaultFont();
}


TPopupMenu::~TPopupMenu()
{
}


void TPopupMenu::Display(TWindow* owningWindow, const TPoint& point, Time eventTime)
{
	ASSERT(owningWindow);
	fOwningWindow = owningWindow;

	TPoint where(point);
	owningWindow->LocalToRoot(where);
	where.h -= 4;
	where.v -= 4;
	
	fSelectedItemIndex = -1;

	TTopLevelWindow* topLevel = owningWindow->GetTopLevelWindow();
	ASSERT(topLevel);
	topLevel->SetTarget(this);
	SetNextHandler(owningWindow);
	
	fMouseMoved = false;
	fTrackingMouseUp = false;
	
	PreDisplayMenu();
	ShowMenu(where);
	GrabPointer(eventTime);
}


void TPopupMenu::PreDisplayMenu()
{
	DisableAll();
	HandleSetupMenu(this);
}


void TPopupMenu::MenuItemSelected(TMenu* menu, int itemIndex, Time time)
{
	fSelectedItemIndex = itemIndex;

	if (itemIndex >= 0)
	{
		TMenuItem* item = menu->GetItem(itemIndex);
		ASSERT(item);
		if (item->GetCommandID() != kNoCommandID)
			fOwningWindow->HandleCommand(this, this, item->GetCommandID());
	}
	
	Show(false);
	fOwningWindow->HandleCommand(this, this, kSelectionChangedCommandID);	
}


bool TPopupMenu::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	if (command == kFocusLostCommandID && receiver == this)
	{
		Show(false);
		fOwningWindow->HandleCommand(this, this, kSelectionChangedCommandID);
	}

	return TMenu::DoCommand(sender, receiver, command);
}


void TPopupMenu::DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state)
{
	TMenu::DoMouseDown(point, button, state);
}


void TPopupMenu::DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (!fMouseMoved)
		fTrackingMouseUp = true;
	else if (!fTrackingMouseUp)
		TMenu::DoMouseUp(point, button, state);
}


void TPopupMenu::DoMouseMoved(const TPoint& point, TModifierState state)
{
	fMouseMoved = true;
	
	TMenu::DoMouseMoved(point, state);
}


TMenuItem* TPopupMenu::GetSelectedItem()
{
	if (fSelectedItemIndex >= 0)
		return GetItem(fSelectedItemIndex);
	else
		return NULL;
}


