// ========================================================================================
//	TTextField.cpp			 	Copyright (C) 2001-2002 Mike Lockwood. All rights reserved.
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

#include "TTextField.h"
#include "TClipboardKeyBehavior.h"
#include "TDrawContext.h"
#include "TTopLevelWindow.h"


TTextField::TTextField(TWindow* parent, const TRect& bounds, TFont* font, bool modifiable)
	:	TTextView(parent, bounds, font, modifiable, false),
		fPastingText(false)
{
	SetBorder(1);
	SetBackColor(kWhiteColor);
	
	AddBehavior(new TClipboardKeyBehavior);
	fInsertionPointOn = false;
	fHideInsertionPointWhenNotTarget = true;
}


TTextField::~TTextField()
{
}


bool TTextField::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	if (command == kFocusAcquiredCommandID)
	{
		SetBorder(2);
		
		if (fTrackingClickCount == 0 && !fPastingText)
		{
			SelectAll();
			return TTextView::DoCommand(sender, receiver, command);
		}
	}
	else if (command == kFocusLostCommandID)
	{
		SetBorder(1);
		
		if (fTrackingClickCount == 0)
		{
			SetSelection(0);
			TDrawContext context(this);
			HideInsertionPoint(context);
			EnableIdling(false);
		}
	}
	
	return TTextView::DoCommand(sender, receiver, command);
}


void TTextField::PastedText()
{
	TTopLevelWindow* topLevel = GetTopLevelWindow();

	ASSERT(topLevel);
	if (topLevel->HasFocusedWindow())
	{
		fPastingText = true;
		topLevel->SetTarget(this);
		fPastingText = false;
	}
}

