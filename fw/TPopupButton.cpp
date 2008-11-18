// ========================================================================================
//	TButton.cpp				 	Copyright (C) 2001-2003 Mike Lockwood. All rights reserved.
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

#include "TPopupButton.h"
#include "TPopupMenu.h"

#include <stdio.h>

TPopupButton::TPopupButton(TWindow* parent, const TRect& bounds, const TChar* title, 
				 TPopupMenu* menu)
	:	TButton(parent, bounds, title, kNoCommandID, false),
		fPopupMenu(menu)
{
	ASSERT(menu);
}


TPopupButton::~TPopupButton()
{
	delete fPopupMenu;
}


void TPopupButton::DoMouseDown(const TPoint& point, TMouseButton button, TModifierState state)
{
	if (button == kLeftButton && fEnabled)
	{
		fPressed = true;
		Redraw();
		
		fPopupMenu->Display(this, gZeroPoint, GetCurrentEventTime());
	}
}


void TPopupButton::DoMouseUp(const TPoint& point, TMouseButton button, TModifierState state)
{
}


void TPopupButton::DoMouseMoved(const TPoint& point, TModifierState state)
{
}


bool TPopupButton::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	if (command == kSelectionChangedCommandID && sender == fPopupMenu)
	{
		fPressed = false;
		Redraw();	
	}
	
	return TButton::DoCommand(sender, receiver, command);
}
