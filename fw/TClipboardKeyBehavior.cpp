// ========================================================================================
//	TClipboardKeyBehavior.cpp	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TClipboardKeyBehavior.h"
#include "TCommandID.h"

#include <X11/keysym.h>


TClipboardKeyBehavior::TClipboardKeyBehavior()
{
}


TClipboardKeyBehavior::~TClipboardKeyBehavior()
{
}


bool TClipboardKeyBehavior::DoKeyDown(KeySym key, TModifierState state, const char* string)
{
	if (state & Mod1Mask)
	{
		switch (key)
		{
			case XK_z:
				fOwner->HandleCommand(fOwner, fOwner, kUndoCommandID);
				return true;
			
			case XK_x:
				fOwner->HandleCommand(fOwner, fOwner, kCutCommandID);
				return true;
			
			case XK_c:
				fOwner->HandleCommand(fOwner, fOwner, kCopyCommandID);
				return true;
			
			case XK_v:
				fOwner->HandleCommand(fOwner, fOwner, kPasteCommandID);
				return true;
		}
	}
	
	return false;
}


