// ========================================================================================
//	TMenuItem.cpp			 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TMenuItem.h"

#include "intl.h"

#include <X11/X.h>
#include <ctype.h>

const TModifierState kModifierMask = ControlMask|Mod1Mask|ShiftMask;

TMenuItem::TMenuItem(const TChar* title, TCommandID commandID, TModifierState shortCutModifier, TChar shortCutKey)
	:	fTitle(title),
		fCommandID(commandID),
		fShortCutModifier(shortCutModifier),
		fShortCutKey(toupper(shortCutKey)),
		fEnabled(false),
		fChecked(false)
{
}


TMenuItem::~TMenuItem()
{
}


TMenuItem* TMenuItem::FindShortCut(KeySym key, TModifierState modifier, const char* string)
{
	if (key < 256 && fShortCutKey == toupper(key) && (fShortCutModifier == (modifier & kModifierMask)))
		return this;
	else
		return NULL;
}


void TMenuItem::GetShortCutLabel(TString& shortCutString)
{
	if (fShortCutKey && fShortCutModifier)
	{
		shortCutString.SetEmpty();
		
		if ((fShortCutModifier & ControlMask) == ControlMask)
			shortCutString += _("Ctrl-");
		if ((fShortCutModifier & Mod1Mask) == Mod1Mask)
			shortCutString += _("Meta-");		
		if ((fShortCutModifier & ShiftMask) == ShiftMask)
			shortCutString += _("Shift-");

		shortCutString += fShortCutKey;
	}
}


void TMenuItem::Enable(bool enable, bool check)
{ 
	fEnabled = enable; 
	fChecked = check;
}


void TMenuItem::DisableAll()
{
	fEnabled = false; 
	fChecked = false;
}


void TMenuItem::EnableCommand(TCommandID command, bool check)
{
	if (command == fCommandID && command != kNoCommandID)
		Enable(true, check);
}


bool TMenuItem::IsSeparator() const
{
	return false;
}


