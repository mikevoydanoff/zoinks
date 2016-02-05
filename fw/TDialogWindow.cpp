// ========================================================================================
//	TDialogWindow.cpp		   Copyright (C) 2001-2002 Mike Voydanoff. All rights reserved.
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

#include "TDialogWindow.h"
#include "TDrawContext.h"
#include "TApplication.h"
#include "TButton.h"

#include <ctype.h>
#include <X11/keysym.h>


TDialogWindow::TDialogWindow(const TRect& bounds, const TChar* title, bool modal, TWindow* owner)
	:	TTopLevelWindow(bounds, title),
		fOwner(owner),
		fNextDialog(NULL),
		fOKButton(NULL),
		fDismissed(false),
		fModal(modal),
		fOKEnabled(true)
{		
	SetBackColor(kLightGrayColor);
}


TDialogWindow::~TDialogWindow()
{
}


void TDialogWindow::Create()
{
	TTopLevelWindow::Create();
	
	if (fOwner)
		XSetTransientForHint(sDisplay, fWindow, fOwner->GetXWindow());
}


void TDialogWindow::DoClose()
{
	if (fModal)
	{
		// avoid reentrancy problem
		if (fDismissed)
			TTopLevelWindow::DoClose();
		else
		{
			fDismissed = true;
			gApplication->DismissDialog(this, kCancelCommandID);
		}
	}
	else
		TTopLevelWindow::DoClose();
}


bool TDialogWindow::DoKeyDown(KeySym key, TModifierState state, const char* string)
{
	if (fModal && fOKEnabled && (key == XK_Return || key == XK_KP_Enter))
	{
		fDismissed = true;
		gApplication->DismissDialog(this, kOKCommandID);
		return true;
	}
	else if (key == XK_Escape || (key < 256 && tolower(key) == 'w' && (state & Mod1Mask)))
	{
		if (fModal)
		{
			fDismissed = true;
			gApplication->DismissDialog(this, kCancelCommandID);
		}
		else
			Close();
	
		return true;
	}
	else
		return TTopLevelWindow::DoKeyDown(key, state, string);
}


bool TDialogWindow::DoCommand(TCommandHandler* sender, TCommandHandler* receiver, TCommandID command)
{
	ASSERT( !(!fOKEnabled && command == kOKCommandID) );

	if (fModal && sender->DismissesDialog(command))
	{
		fDismissed = true;
		gApplication->DismissDialog(this, command);
		return true;
	}
	else
		return TWindow::DoCommand(sender, receiver, command);
}


void TDialogWindow::SetOKEnabled(bool okEnabled)
{
	fOKEnabled = okEnabled;
	if (fOKButton)
		fOKButton->SetEnabled(okEnabled);
}
