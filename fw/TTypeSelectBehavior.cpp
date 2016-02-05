// ========================================================================================
//	TTypeSelectBehavior.cpp	   Copyright (C) 2001-2006 Mike Voydanoff. All rights reserved.
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

#include "TTypeSelectBehavior.h"
#include "TTypeSelectable.h"
#include "TApplication.h"

#include <X11/keysym.h>
#include <ctype.h>

static const TTime kTimeout = 1000;


TTypeSelectBehavior::TTypeSelectBehavior(bool caseSensitive)
	:	fLastKeyTime(0),
		fCaseSensitive(caseSensitive)
{
}


TTypeSelectBehavior::~TTypeSelectBehavior()
{
}


bool TTypeSelectBehavior::DoKeyDown(KeySym key, TModifierState state, const char* string)
{
	TTypeSelectable* view = dynamic_cast<TTypeSelectable*>(fOwner);
	if (!view)
		return false;
		
	// Alt-A to select all
	if (key == 'a' && (state & Mod1Mask) && view->TTypeSelectable_IsMultiSelect())
	{
		view->TTypeSelectable_SelectAll();
		return true;
	}

	if (key == XK_Left || key == XK_Right || key == XK_Up || key == XK_Down ||
		key == XK_Page_Up || key == XK_Page_Down || key == XK_Return || key == XK_KP_Enter || key == XK_Escape || 
		(state & (ControlMask | Mod1Mask)))
		return false;	// let the list view handle it.

	// avoid shift key and other things like that
	if (key < 32 || key > 127)
		return false;

	TTime time = gApplication->GetCurrentTime();

	if (fLastKeyTime + kTimeout < time)
		fKeys.SetEmpty(); 

	fLastKeyTime = time;

	if (fCaseSensitive)
		fKeys += (TChar)key;
	else
		fKeys += (TChar)tolower(key);


	int rows = view->TTypeSelectable_GetCount();

	int i;
	for (i = 0; i < rows; i++)
	{
		if (! view->TTypeSelectable_AllowSelect(i))
			continue;

		TString	text;
		view->TTypeSelectable_GetText(i, text);

		if (!fCaseSensitive)
			text.ToLower();

		if (Tstrncmp(text, fKeys, fKeys.GetLength()) == 0)
			break;
	}

	if (i != view->TTypeSelectable_GetSelection() && i < rows)
		view->TTypeSelectable_Select(i);

	return true;
}


bool TTypeSelectBehavior::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	if (command == kFlushTypeSelectCommandID)
	{
		fKeys.SetEmpty();
		return true;
	}
	else
		return false;
}

